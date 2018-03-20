#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include "include/leveldb/export.h"

namespace leveldb {

	class Comparator;

	// DB contents are stored in a set of blocks, each of which holds a
	// sequence of key,value pairs.  Each block may be compressed before
	// being stored in a file.  The following enum describes which
	// compression method (if any) is used to compress a block.
	enum CompressionType {
		// NOTE: do not change the values of existing entries, as these are
		// part of the persistent format on disk.
		kNoCompression     = 0x0,
		kSnappyCompression = 0x1
	};

	// Options to control the behavior of a database (passed to DB::Open)
	struct LEVELDB_EXPORT Options {
		// -------------------
		// Parameters that affect behavior

		// Comparator used to define the order of keys in the table.
		// Default: a comparator that uses lexicographic byte-wise ordering
		//
		// REQUIRES: The client must ensure that the comparator supplied
		// here has the same name and orders keys *exactly* the same as the
		// comparator provided to previous open calls on the same DB.
		const Comparator* comparator;

		// Number of keys between restart points for delta encoding of keys.
		// This parameter can be changed dynamically.  Most clients should
		// leave this parameter alone.
		//
		// Default: 16
		int block_restart_interval;

		// If non-NULL, use the specified filter policy to reduce disk reads.
		// Many applications will benefit from passing the result of
		// NewBloomFilterPolicy() here.
		//
		// Default: NULL
		const FilterPolicy* filter_policy;

		// Approximate size of user data packed per block.  Note that the
		// block size specified here corresponds to uncompressed data.  The
		// actual size of the unit read from disk may be smaller if
		// compression is enabled.  This parameter can be changed dynamically.
		//
		// Default: 4K
		size_t block_size;

		// Compress blocks using the specified compression algorithm.  This
		// parameter can be changed dynamically.
		//
		// Default: kSnappyCompression, which gives lightweight but fast
		// compression.
		//
		// Typical speeds of kSnappyCompression on an Intel(R) Core(TM)2 2.4GHz:
		//    ~200-500MB/s compression
		//    ~400-800MB/s decompression
		// Note that these speeds are significantly faster than most
		// persistent storage speeds, and therefore it is typically never
		// worth switching to kNoCompression.  Even if the input data is
		// incompressible, the kSnappyCompression implementation will
		// efficiently detect that and will switch to uncompressed mode.
		CompressionType compression;

		// Create an Options object with default values for all fields.
		Options(){}
	};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_