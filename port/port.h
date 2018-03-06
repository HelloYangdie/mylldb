#ifndef STORAGE_LEVELDB_PORT_PORT_H_
#define STORAGE_LEVELDB_PORT_PORT_H_

#if defined(__GNUC__)
#include "port/port_posix.h"
#elif defined(_MSC_VER)
#include "port/port_win.h"
#endif

#endif