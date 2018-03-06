
#ifndef STORAGE_LEVELDB_INCLUDE_ITERATOR_H_
#define STORAGE_LEVELDB_INCLUDE_ITERATOR_H_

#include "include/leveldb/export.h"
#include "include/leveldb/slice.h"
#include "include/leveldb/status.h"

namespace leveldb
{

class LEVELDB_EXPORT Iterator 
{
public:
	Iterator();
	virtual ~Iterator();

	virtual bool Valid() const = 0;

	virtual void SeekToFirst() = 0;

	virtual void SeekToLast() = 0;

	virtual void Seek(const Slice& target) = 0;

	virtual void Next() = 0;

	virtual void Prev() = 0;

	virtual Slice key() const = 0;

	virtual Slice value() const = 0;

	virtual Status status() const = 0;

	typedef void (*CleanupFunction)(void* arg1, void* arg2);
	void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);
private:
	struct Cleanup {
		CleanupFunction function;
		void* arg1;
		void* arg2;
		Cleanup* next;
	};

	Cleanup cleanup_;

	Iterator(const Iterator&);
	void operator=(const Iterator&);
};

// Return an empty iterator (yields nothing).
LEVELDB_EXPORT Iterator* NewEmptyIterator();

// Return an empty iterator with the specified status.
LEVELDB_EXPORT Iterator* NewErrorIterator(const Status& status);

}

#endif