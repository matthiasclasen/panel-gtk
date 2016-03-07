lib_LTLIBRARIES = libpanel-gtk-@API_VERSION@.la
noinst_PROGRAMS = test-panel

headersdir = $(includedir)/panel-gtk-@API_VERSION@
headers_DATA = \
	pnl-animation.h \
	pnl-dock-bin.h \
	pnl-dock-group.h \
	pnl-dock-item.h \
	pnl-dock-manager.h \
	pnl-dock-overlay.h \
	pnl-dock-paned.h \
	pnl-dock-stack.h \
	pnl-dock-types.h \
	pnl-dock-widget.h \
	pnl-dock-window.h \
	pnl-dock.h \
	pnl-frame-source.h \
	pnl-multi-paned.h \
	pnl-tab-strip.h \
	pnl-tab.h \
	pnl-version.h \
	pnl.h \
	$(NULL)

libpanel_gtk_@API_VERSION@_la_SOURCES = \
	$(headers_DATA) \
	pnl-animation.c \
	pnl-dock-bin-edge-child-private.h \
	pnl-dock-bin-edge-child.c \
	pnl-dock-bin-edge-private.h \
	pnl-dock-bin-edge.c \
	pnl-dock-bin.c \
	pnl-dock-group.c \
	pnl-dock-item.c \
	pnl-dock-manager.c \
	pnl-dock-overlay-edge-private.h \
	pnl-dock-overlay-edge.c \
	pnl-dock-overlay.c \
	pnl-dock-paned-private.h \
	pnl-dock-paned.c \
	pnl-dock-stack.c \
	pnl-dock-widget.c \
	pnl-dock-window.c \
	pnl-dock.c \
	pnl-frame-source.c \
	pnl-multi-paned.c \
	pnl-tab-strip.c \
	pnl-tab.c \
	pnl-util-private.h \
	pnl-util.c \
	$(NULL)

libpanel_gtk_@API_VERSION@_la_CFLAGS = \
	-DPNL_COMPILATION \
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

EXTRA_DIST = \
	test-panel.ui \
	test-panel.css

-include $(top_srcdir)/git.mk
