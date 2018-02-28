
#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <vector>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "port/port.h"

namespace leveldb {

class Arena {
public:
	Arena();
	~Arena();

	char* Allocate(size_t bytes);

	char* AllocateAligned(size_t bytes);

	size_t MemoryUsage() const {
		return reinterpret_cast<uintptr_t>(memory_usage_.NoBarrierLoad());
	}

private:
	char* AllocateFallback(size_t bytes);
	char* AllocateNewBlock(size_t block_bytes);

	char* alloc_ptr_;
	size_t alloc_bytes_remaining_;

	std::vector<char*> blocks_;

	port::AtomicPointer memory_usage_;

	Arena(const Arena&);
	void operator=(const Arena&);
};

inline char* Arena::Allocate(size_t bytes) {
	assert(bytes > 0);

	if (bytes <= alloc_bytes_remaining_) 
	{
		char* result = alloc_ptr_;
		alloc_ptr_ += bytes;
		alloc_bytes_remaining_ -= bytes;
		return result;
	}

	return AllocateFallback(bytes);
}

}

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_
