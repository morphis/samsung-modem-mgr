/*
 *  samsung-modem-mgr
 *
 *  Copyright (C) 2012  Simon Busch. All rights reserved.
 *
 *  Some parts of the code are fairly copied from the ofono project under
 *  the terms of the GPLv2.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <gdbus.h>

#include <radio.h>

#include "main.h"
#include "dbus.h"
#include "rfs.h"

#define SAMSUNG_MODEM_MANAGER_PATH			"/"
#define SAMSUNG_MODEM_MANAGER_INTERFACE		"org.samsung.modem.Manager"

enum modem_state {
	OFFLINE,
	INITIALIZING,
	ONLINE
};

struct manager {
	struct ipc_client *client;
	struct rfs_manager *rfs;
	enum modem_state state;
	gboolean powered;
};

const char* modem_state_to_string(enum modem_state state)
{
	switch (state)
	{
		case OFFLINE:
			return "offline";
		case INITIALIZING:
			return "initializing";
		case ONLINE:
			return "online";
	}

	return "unknown";
}

static DBusMessage *manager_get_properties(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	DBusMessage *reply;
	DBusMessageIter iter, dict;
	struct manager *mgr = data;
	const char *status;

	reply = dbus_message_new_method_return(msg);
	if (reply == NULL)
		return NULL;

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					PROPERTIES_ARRAY_SIGNATURE,
					&dict);

	status = modem_state_to_string(mgr->state);
	__dbus_dict_append(&dict, "Status", DBUS_TYPE_STRING, &status);

	__dbus_dict_append(&dict, "Powered", DBUS_TYPE_BOOLEAN, &mgr->powered);

	dbus_message_iter_close_container(&iter, &dict);

	return reply;
}

static void notify_status_changed(DBusConnection *conn, enum modem_state state)
{
	const char *status = modem_state_to_string(state);
	__dbus_signal_property_changed(conn, SAMSUNG_MODEM_MANAGER_PATH,
		SAMSUNG_MODEM_MANAGER_INTERFACE,
		"Status", DBUS_TYPE_STRING,
		&status);
}

static void notify_powered_changed(DBusConnection *conn, gboolean powered)
{
	__dbus_signal_property_changed(conn, SAMSUNG_MODEM_MANAGER_PATH,
		SAMSUNG_MODEM_MANAGER_INTERFACE,
		"Powered", DBUS_TYPE_BOOLEAN,
		&powered);
}
static int set_powered(DBusConnection *conn, struct manager *mgr, gboolean powered)
{
	const char *status;

	if (powered)
	{
		g_debug("Powering modem up ...");

		mgr->state = INITIALIZING;

		notify_status_changed(conn, mgr->state);

		if (ipc_client_bootstrap_modem(mgr->client) < 0 ||
			ipc_client_power_on(mgr->client) < 0) {
			g_error("Can't bootstrap and power on the modem");
			mgr->state = OFFLINE;
		}
		else {
			/* now start up the RFS client */
			if (rfs_manager_start(mgr->rfs) < 0) {
				g_error("Can't start RFS manager. Shutting down ...");
				ipc_client_power_off(mgr->client);
				mgr->state = OFFLINE;
			}
			else {
				notify_powered_changed(conn, powered);
				mgr->powered = powered;
				mgr->state = ONLINE;
				g_debug("Everything fine, modem is ready now.");
			}
		}
	}
	else {
		g_debug("Stopping RFS manager ...");
		rfs_manager_stop(mgr->rfs);

		g_debug("Powering modem down ...");
		ipc_client_power_off(mgr->client);

		mgr->powered = powered;
		notify_powered_changed(conn, powered);

		mgr->state = OFFLINE;
	}

	g_debug("Changed manager power state to %s", powered ? "online" : "offline");

	notify_status_changed(conn, mgr->state);

	return 0;
}

static DBusMessage *manager_set_property(DBusConnection *conn, DBusMessage *msg,
					void *data)
{
	DBusMessageIter iter;
	DBusMessageIter var;
	const char *property;
	struct manager *mgr = data;

	if (!dbus_message_iter_init(msg, &iter))
		return __dbus_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return __dbus_error_invalid_args(msg);

	dbus_message_iter_get_basic(&iter, &property);
	dbus_message_iter_next(&iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
		return __dbus_error_invalid_args(msg);

	dbus_message_iter_recurse(&iter, &var);

	if (g_str_equal(property, "Powered") == TRUE) {
		gboolean powered = FALSE;
		int err;

		if (dbus_message_iter_get_arg_type(&var) != DBUS_TYPE_BOOLEAN)
			return __dbus_error_invalid_args(msg);

		dbus_message_iter_get_basic(&var, &powered);

		if (mgr->state == INITIALIZING)
			return __dbus_error_busy(msg);

		if (mgr->powered == powered)
			return dbus_message_new_method_return(msg);

		g_dbus_send_reply(conn, msg, DBUS_TYPE_INVALID);

		err = set_powered(conn, mgr, powered);
		if (err < 0)
			return NULL;

		return NULL;
	}

	return __dbus_error_invalid_args(msg);
}

static const GDBusMethodTable manager_methods[] = {
	{ GDBUS_METHOD("GetProperties",
		NULL, GDBUS_ARGS({ "properties", "a{sv}" }),
		manager_get_properties) },
	{ GDBUS_ASYNC_METHOD("SetProperty",
		GDBUS_ARGS({ "property", "s" }, { "value", "v" }),
		NULL, manager_set_property) },
	{ }
};

static const GDBusSignalTable manager_signals[] = {
	{ GDBUS_SIGNAL("PropertyChanged",
		GDBUS_ARGS({ "property", "s" }, { "value", "v" })) },
	{ }
};

static void log_handler(const char *message, void *user_data)
{
	g_debug("%s", message);
}

struct manager *manager_create(void)
{
	struct manager *mgr = NULL;

	mgr = g_try_new0(struct manager, 1);

	if (mgr == NULL)
		return NULL;

	mgr->state = OFFLINE;
	mgr->powered = FALSE;
	mgr->client = ipc_client_new(IPC_CLIENT_TYPE_FMT);
	ipc_client_set_log_handler(mgr->client, log_handler, NULL);
	mgr->rfs = rfs_manager_new();

	return mgr;
}

int manager_init(struct manager *mgr)
{
	DBusConnection *conn;
	gboolean ret;

	conn = get_dbus_connection();

	ret = g_dbus_register_interface(conn, SAMSUNG_MODEM_MANAGER_PATH,
					SAMSUNG_MODEM_MANAGER_INTERFACE,
					manager_methods, manager_signals,
					NULL, mgr, NULL);

	if (ret == FALSE)
		return -1;

	g_debug("Initialized DBus manager interface");

	return 0;
}

void manager_cleanup(struct manager *mgr)
{
	DBusConnection *conn = get_dbus_connection();

	if (mgr->state == ONLINE)
		set_powered(conn, mgr, FALSE);

	g_dbus_unregister_interface(conn, SAMSUNG_MODEM_MANAGER_PATH,
					SAMSUNG_MODEM_MANAGER_INTERFACE);

	ipc_client_free(mgr->client);
	rfs_manager_free(mgr->rfs);

	g_free(mgr);
}

// vim:ts=4:sw=4:noexpandtab
