
#ifndef STORAGE_LEVELDB_TABLE_FORMAT_H_
#define STORAGE_LEVELDB_TABLE_FORMAT_H_

#include "include/leveldb/slice.h"

namespace leveldb 
{

// kTableMagicNumber was picked by running
//    echo http://code.google.com/p/leveldb/ | sha1sum
// and taking the leading 64 bits.
static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

// 1-byte type + 32-bit crc
static const size_t kBlockTrailerSize = 5;

struct BlockContents 
{
  Slice data;           // Actual contents of data
  bool cachable;        // True iff data can be cached
  bool heap_allocated;  // True iff caller should delete[] data.data()
};

class BlockHandle {
public:
	BlockHandle();

	// The offset of the block in the file.
	uint64_t offset() const { return offset_; }
	void set_offset(uint64_t offset) { offset_ = offset; }

	// The size of the stored block
	uint64_t size() const { return size_; }
	void set_size(uint64_t size) { size_ = size; }

	void EncodeTo(std::string* dst) const;
	Status DecodeFrom(Slice* input);

	// Maximum encoding length of a BlockHandle
	enum { kMaxEncodedLength = 10 + 10 };

private:
	uint64_t offset_;
	uint64_t size_;
};

class Footer {
public:
	Footer() { }

	// The block handle for the metaindex block of the table
	const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
	void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

	// The block handle for the index block of the table
	const BlockHandle& index_handle() const {
		return index_handle_;
	}
	void set_index_handle(const BlockHandle& h) {
		index_handle_ = h;
	}

	void EncodeTo(std::string* dst) const;
	Status DecodeFrom(Slice* input);

	// Encoded length of a Footer.  Note that the serialization of a
	// Footer will always occupy exactly this many bytes.  It consists
	// of two block handles and a magic number.
	enum {
		kEncodedLength = 2*BlockHandle::kMaxEncodedLength + 8
	};

private:
	BlockHandle metaindex_handle_;
	BlockHandle index_handle_;
};

extern Status ReadBlock(RandomAccessFile* file, 
					const ReadOptions& options,
					const BlockHandle& handle, 
					BlockContents* result);

}

#endif