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
#include <glib.h>
#include <gdbus.h>

#include "main.h"
#include "dbus.h"

#define SAMSUNG_MODEM_MANAGER_PATH			"/"
#define SAMSUNG_MODEM_MANAGER_INTERFACE		"org.samsung.modem.Manager"

static DBusMessage *manager_get_properties(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	DBusMessage *reply;
	DBusMessageIter iter, dict;

	reply = dbus_message_new_method_return(msg);
	if (reply == NULL)
		return NULL;

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					PROPERTIES_ARRAY_SIGNATURE,
					&dict);

	char *status = "unknown";
	__dbus_dict_append(&dict, "status", DBUS_TYPE_STRING, &status);

	dbus_message_iter_close_container(&iter, &dict);

	return reply;
}

static DBusMessage *manager_set_property(DBusConnection *conn, DBusMessage *msg,
					void *data)
{
	DBusMessageIter iter;
	DBusMessageIter var;
	const char *property;

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
		__dbus_pending_reply(msg,
				dbus_message_new_method_return(msg));
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

int __manager_init(void)
{
	DBusConnection *conn;
	gboolean ret;

	conn = get_dbus_connection();

	ret = g_dbus_register_interface(conn, SAMSUNG_MODEM_MANAGER_PATH,
					SAMSUNG_MODEM_MANAGER_INTERFACE,
					manager_methods, manager_signals,
					NULL, NULL, NULL);

	if (ret == FALSE)
		return -1;

	return 0;
}

void __manager_cleanup(void)
{
	DBusConnection *conn = get_dbus_connection();

	g_dbus_unregister_interface(conn, SAMSUNG_MODEM_MANAGER_PATH,
					SAMSUNG_MODEM_MANAGER_INTERFACE);
}

// vim:ts=4:sw=4:noexpandtab
