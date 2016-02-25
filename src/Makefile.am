lib_LTLIBRARIES = libpanel-gtk-@API_VERSION@.la
noinst_PROGRAMS = test-panel

libpanel_gtk_@API_VERSION@_la_SOURCES = \
	panel-gtk.h \
	pnl-dock.c \
	pnl-dock.h \
	pnl-dock-edge.c \
	pnl-dock-edge.h \
	pnl-dock-edge-child.c \
	pnl-dock-edge-child.h \
	pnl-panel.h \
	pnl-panel-child.h \
	pnl-version.h \
	$(NULL)

libpanel_gtk_@API_VERSION@_la_CFLAGS = \
	-DPANEL_GTK_COMPILATION \
	$(PANEL_GTK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(NULL)
libpanel_gtk_@API_VERSION@_la_LIBADD = \
	$(PANEL_GTK_LIBS) \
	$(NULL)
libpanel_gtk_@API_VERSION@_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(NULL)

test_panel_SOURCES = test-panel.c
test_panel_LDADD = \
	$(PANEL_GTK_LIBS) \
	libpanel-gtk-@API_VERSION@.la
test_panel_CFLAGS = $(PANEL_GTK_CFLAGS)

-include $(top_srcdir)/git.mk
