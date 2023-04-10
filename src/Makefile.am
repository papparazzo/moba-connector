bin_PROGRAMS = moba-connector
moba_connector_SOURCES = \
    main.cpp \
    jsonreader.cpp \
    jsonwriter.cpp \
    brakevector.cpp \
    watchdog.cpp

moba_connector_LDADD = \
  -lmobacommon \
  $(srcdir)/../modules/lib-msghandling/src/moba/libmsghandling.a \
  $(srcdir)/../modules/lib-cs2interface/src/moba/libcs2interface.a

AM_LDFLAGS = \
  -L/usr/local/lib -pthread

AM_CPPFLAGS = \
  -I$(srcdir)/../modules/lib-msghandling/src \
  -I$(srcdir)/../modules/lib-cs2interface/src -g3 -ggdb -O3 -Wall

ACLOCAL_AMFLAGS = -I m4
