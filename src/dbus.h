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

#ifndef DBUS_H_
#define DBUS_H_

/* Essentially a{sv} */
#define PROPERTIES_ARRAY_SIGNATURE DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING \
					DBUS_TYPE_STRING_AS_STRING \
					DBUS_TYPE_VARIANT_AS_STRING \
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING

DBusMessage *__dbus_error_invalid_args(DBusMessage *msg);
DBusMessage *__dbus_error_failed(DBusMessage *msg);
DBusMessage *__dbus_error_busy(DBusMessage *msg);
void __dbus_dict_append(DBusMessageIter *dict,
			const char *key, int type, void *value);

#endif

// vim:ts=4:sw=4:noexpandtab
