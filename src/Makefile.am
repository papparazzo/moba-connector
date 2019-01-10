bin_PROGRAMS = moba-connector
moba_connector_SOURCES = \
    main.cpp \
    bridge.cpp \
    cs2read.cpp \
    cs2write.cpp \
    brakevector.cpp \
    msgloop.cpp

moba_connector_LDADD = /usr/local/lib/libMoBaCommon.a
LDFLAGS += -L/usr/local/lib -pthread

ACLOCAL_AMFLAGS = -I m4
