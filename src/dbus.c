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

#define SAMSUNG_MODEM_ERROR_INTERFACE "org.samsung.modem.Error"

DBusMessage *__dbus_error_invalid_args(DBusMessage *msg)
{
	return g_dbus_create_error(msg, SAMSUNG_MODEM_ERROR_INTERFACE
					".InvalidArguments",
					"Invalid arguments in method call");
}

DBusMessage *__dbus_error_failed(DBusMessage *msg)
{
	return g_dbus_create_error(msg, SAMSUNG_MODEM_ERROR_INTERFACE
					".Failed",
					"Operation failed");
}

DBusMessage *__dbus_error_busy(DBusMessage *msg)
{
	return g_dbus_create_error(msg, SAMSUNG_MODEM_ERROR_INTERFACE
					".Busy",
					"Manager is busy");
}

static void append_variant(DBusMessageIter *iter,
				int type, void *value)
{
	char sig[2];
	DBusMessageIter valueiter;

	sig[0] = type;
	sig[1] = 0;

	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
						sig, &valueiter);

	dbus_message_iter_append_basic(&valueiter, type, value);

	dbus_message_iter_close_container(iter, &valueiter);
}

void __dbus_dict_append(DBusMessageIter *dict,
			const char *key, int type, void *value)
{
	DBusMessageIter keyiter;

	if (type == DBUS_TYPE_STRING) {
		const char *str = *((const char **) value);
		if (str == NULL)
			return;
	}

	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &keyiter);

	dbus_message_iter_append_basic(&keyiter, DBUS_TYPE_STRING, &key);

	append_variant(&keyiter, type, value);

	dbus_message_iter_close_container(dict, &keyiter);
}

void __dbus_pending_reply(DBusMessage **msg, DBusMessage *reply)
{
	DBusConnection *conn = get_dbus_connection();

	g_dbus_send_message(conn, reply);

	dbus_message_unref(*msg);
	*msg = NULL;
}

int __dbus_signal_property_changed(DBusConnection *conn,
					const char *path,
					const char *interface,
					const char *name,
					int type, void *value)
{
	DBusMessage *signal;
	DBusMessageIter iter;

	signal = dbus_message_new_signal(path, interface, "PropertyChanged");
	if (signal == NULL) {
		return -1;
	}

	dbus_message_iter_init_append(signal, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &name);

	append_variant(&iter, type, value);

	return g_dbus_send_message(conn, signal);
}

// vim:ts=4:sw=4:noexpandtab
