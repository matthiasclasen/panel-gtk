SUBDIRS = icons

# pkg-config data
# Note that the template file is called panel-gtk.pc.in, but generates a
# versioned .pc file using some magic in AC_CONFIG_FILES.
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = panel-gtk-$(API_VERSION).pc

DISTCLEANFILES = $(pkgconfig_DATA)
EXTRA_DIST = panel-gtk.pc.in

-include $(top_srcdir)/git.mk
