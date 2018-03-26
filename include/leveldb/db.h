
#ifndef STORAGE_LEVELDB_INCLUDE_DB_H_
#define STORAGE_LEVELDB_INCLUDE_DB_H_

#include <stdint.h>
#include "include/leveldb/iterator.h"

namespace leveldb 
{

class LEVELDB_EXPORT Snapshot {
protected:
	virtual ~Snapshot();
};

}

#endif