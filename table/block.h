#ifndef STORAGE_LEVELDB_TABLE_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_H_

#include <stdint.h>
#include "include/leveldb/iterator.h"

namespace leveldb 
{

struct BlockContents;
class Comparator;

class Block
{
public:
	explicit Block(const struct BlockContents& contents);

	~Block(void) { if (owned_) delete[] data_;}

	size_t size() const { return size_; }

	Iterator* NewIterator(const Comparator* comparator);
private:
	 uint32_t NumRestarts() const;

	 const char* data_;
	 size_t size_;
	 uint32_t restart_offset_;
	 bool owned_;

	 Block(const Block&);

	 void operator=(const Block&);

	 class Iter;
};

}
#endif