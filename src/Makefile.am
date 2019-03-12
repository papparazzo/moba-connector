bin_PROGRAMS = moba-connector
moba_connector_SOURCES = \
    main.cpp \
    cs2reader.cpp \
    jsonreader.cpp \
    cs2writer.cpp \
    brakevector.cpp \
    watchdog.cpp \
    configdata.cpp

moba_connector_LDADD = -lMoBaCommon $(srcdir)/../modules/lib-msghandling/src/moba/libmsghandling.a
LDFLAGS += -L/usr/local/lib -pthread

AM_CPPFLAGS = -I$(srcdir)/../modules/lib-msghandling/src -g3 -ggdb -O3 -Wall

ACLOCAL_AMFLAGS = -I m4
