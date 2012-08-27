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
#include <stdlib.h>
#include <glib.h>

#include <radio.h>

struct rfs_manager
{
	struct ipc_client *client;
	GIOChannel *io;
	guint read_watch;
};

static gboolean received_data(GIOChannel *channel, GIOCondition cond,
							  gpointer user_data)
{
	struct rfs_manager *mgr = user_data;
	struct ipc_message_info resp;
	int ret;

	if (cond & G_IO_NVAL)
		return FALSE;

	ret = ipc_client_recv(mgr->client, &resp);
	if (ret < 0) {
		g_error("Could not receive IPC message from modem");
		return FALSE;
	}

	switch (resp.cmd) {
	case IPC_RFS_NV_READ_ITEM:
		ipc_rfs_send_io_confirm_for_nv_read_item(mgr->client, &resp);
		break;
	case IPC_RFS_NV_WRITE_ITEM:
		ipc_rfs_send_io_confirm_for_nv_write_item(mgr->client, &resp);
		break;
	}

	ipc_client_response_free(mgr->client, &resp);

	return TRUE;
}

static void read_watch_destroy(gpointer user_data)
{
	struct rfs_manager *mgr = user_data;

	mgr->read_watch = 0;
}

struct rfs_manager* rfs_manager_new(void)
{
	struct rfs_manager *mgr;

	mgr = g_try_new0(struct rfs_manager, 1);
	if (!mgr)
		return NULL;

	mgr->client = ipc_client_new(IPC_CLIENT_TYPE_RFS);

	return mgr;
}

int rfs_manager_start(struct rfs_manager *mgr)
{
	int fd;

	if (!mgr)
		return -1;

	ipc_client_open(mgr->client);

	fd = ipc_client_get_handlers_common_data_fd(mgr->client);
	mgr->io = g_io_channel_unix_new(fd);

	g_io_channel_set_encoding(mgr->io, NULL, NULL);
	g_io_channel_set_buffered(mgr->io, FALSE);

	mgr->read_watch = g_io_add_watch_full(mgr->io, G_PRIORITY_DEFAULT,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				received_data, mgr, read_watch_destroy);

	g_io_channel_unref(mgr->io);

	return 0;
}

int rfs_manager_stop(struct rfs_manager *mgr)
{
	if (!mgr)
		return -1;

	if (mgr->read_watch > 0)
		g_source_remove(mgr->read_watch);

	return 0;
}

void rfs_manager_free(struct rfs_manager *mgr)
{
	free(mgr);
}

// vim:ts=4:sw=4:noexpandtab
