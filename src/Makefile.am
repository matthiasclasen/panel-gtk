lib_LTLIBRARIES = libpanel-gtk-@API_VERSION@.la
noinst_PROGRAMS = test-panel test-multi-paned test-tab-strip

DISTCLEANFILES =
CLEANFILES =
BUILT_SOURCES =

headersdir = $(includedir)/panel-gtk-@API_VERSION@
headers_DATA = \
	pnl-animation.h \
	pnl-dock-bin.h \
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
	pnl-dock-item.c \
	pnl-dock-manager.c \
	pnl-dock-overlay-edge-private.h \
	pnl-dock-overlay-edge.c \
	pnl-dock-overlay.c \
	pnl-dock-paned-private.h \
	pnl-dock-paned.c \
	pnl-dock-stack.c \
	pnl-dock-tab-strip.c \
	pnl-dock-tab-strip.h \
	pnl-dock-widget.c \
	pnl-dock-window.c \
	pnl-dock.c \
	pnl-frame-source.c \
	pnl-multi-paned.c \
	pnl-resources.c \
	pnl-resources.h \
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

test_multi_paned_SOURCES = test-multi-paned.c
test_multi_paned_LDADD = \
	$(PANEL_GTK_LIBS) \
	libpanel-gtk-@API_VERSION@.la
test_multi_paned_CFLAGS = $(PANEL_GTK_CFLAGS)

test_tab_strip_SOURCES = test-tab-strip.c
test_tab_strip_LDADD = \
	$(PANEL_GTK_LIBS) \
	libpanel-gtk-@API_VERSION@.la
test_tab_strip_CFLAGS = $(PANEL_GTK_CFLAGS)

EXTRA_DIST = \
	test-panel.ui \
	test-panel.css

glib_resources_h = pnl-resources.h
glib_resources_c = pnl-resources.c
glib_resources_xml = pnl.gresource.xml
glib_resources_namespace = pnl
include $(top_srcdir)/gresources.mk

-include $(top_srcdir)/git.mk
