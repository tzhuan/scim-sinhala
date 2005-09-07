AM_CPPFLAGS = \
	-DSCIM_SINHALA_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
	-I/usr/local/include/scim-1.0/ \
	-I/usr/local/scim-1.0/ \
	-I/usr/include/gtk-2.0/ \
	-I/usr/include/glib-2.0/

INCLUDE	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src

noinst_HEADERS = \
	intl.h \
	scim_sinhala_imengine_factory.h \
	scim_sinhala_imengine.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = sinhala.la

sinhala_la_SOURCES = \
	scim_sinhala_action.h \
	scim_sinhala_action.cpp \
	scim_sinhala_imengine_factory.cpp \
	scim_sinhala_imengine.cpp

sinhala_la_CFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@ -I/usr/include/scim-1.0/
sinhala_la_CXXFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@ -I/usr/include/scim-1.0/

sinhala_la_LDFLAGS = \
	-avoid-version \
	-rpath $(moduledir) \
	-module \
	@LIBTOOL_EXPORT_OPTIONS@ \
	@SCIM_LIBS@
