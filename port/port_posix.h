#ifndef STORAGE_LEVELDB_PORT_PORT_POSIX_H_
#define STORAGE_LEVELDB_PORT_PORT_POSIX_H_

#include "port/atomic_pointer.h"

#if defined(_MSC_VER)
#define snprintf _snprintf_s 

#endif

#endif