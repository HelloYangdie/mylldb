#include "port/port_win.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>

namespace leveldb {
namespace port {



void InitOnce(OnceType* once, void (*initializer)()) 
{
	initializer();
}


}
}