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

		// Create an Options object with default values for all fields.
		Options(){}
	};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_