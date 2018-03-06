#ifndef STORAGE_LEVELDB_PORT_PORT_WIN_H_
#define STORAGE_LEVELDB_PORT_PORT_WIN_H_

#include "port/atomic_pointer.h"

#define snprintf _snprintf_s 

#include <windows.h>

namespace leveldb{
namespace port {

typedef INIT_ONCE OnceType;
#define LEVELDB_ONCE_INIT INIT_ONCE_STATIC_INIT
extern void InitOnce(OnceType* once, void (*initializer)());

}
}

#endif