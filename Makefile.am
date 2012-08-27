sbin_PROGRAMS = src/samsung-modem-mgr

DEBUG_CFLAGS =
if WANT_DEBUG
DEBUG_CFLAGS += -g -O0
endif

src_samsung_modem_mgr_SOURCES = \
	src/main.c \
	src/dbus.c \
	src/manager.c \
	src/rfs.c \
	gdbus/mainloop.c \
	gdbus/object.c \
	gdbus/watch.c \
	gdbus/polkit.c
src_samsung_modem_mgr_CFLAGS = \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(SAMSUNG_IPC_CFLAGS) \
	-I$(top_srcdir)/gdbus
src_samsung_modem_mgr_LDFLAGS = \
	$(GLIB_LIBS) \
	$(DBUS_LIBS) \
	$(SAMSUNG_IPC_LIBS)

if DATAFILES
dbusconfdir = @DBUS_CONFDIR@
dist_dbusconf_DATA = src/samsung-modem-mgr.conf
if SYSTEMD
systemdunitdir = @SYSTEMD_UNITDIR@
systemdunit_DATA = src/samsung-modem-mgr.service
endif
endif


EXTRA_DIST = \
	MAINTAINERS

MAINTAINERCLEANFILES = \
	aclocal.m4 compile config.guess config.sub \
	configure depcomp install-sh ltmain.sh     \
	Makefile.in missing config.h.in            \
	mkinstalldirs *~ \
	$(NULL)
