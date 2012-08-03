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

// vim:ts=4:sw=4:noexpandtab
