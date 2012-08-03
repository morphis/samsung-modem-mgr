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

#define SAMSUNG_MODEM_MANAGER_PATH			"/"
#define SAMSUNG_MODEM_MANAGER_INTERFACE		"org.samsung.modem.Manager"

static DBusMessage *manager_get_properties(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	return __dbus_error_failed(msg);
}

static DBusMessage *manager_set_property(DBusConnection *conn, DBusMessage *msg,
					void *data)
{
	return __dbus_error_failed(msg);
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
