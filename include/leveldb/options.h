#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include "include/leveldb/export.h"

namespace leveldb {

class Cache;
class Comparator;
class Snapshot;

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

	// If true, the implementation will do aggressive checking of the
	// data it is processing and will stop early if it detects any
	// errors.  This may have unforeseen ramifications: for example, a
	// corruption of one DB entry may cause a large number of entries to
	// become unreadable or for the entire DB to become unopenable.
	// Default: false
	bool paranoid_checks;

	// Control over blocks (user data is stored in a set of blocks, and
	// a block is the unit of reading from disk).

	// If non-NULL, use the specified cache for blocks.
	// If NULL, leveldb will automatically create and use an 8MB internal cache.
	// Default: NULL
	Cache* block_cache;

	// Create an Options object with default values for all fields.
	Options(){}
};

// Options that control read operations
struct LEVELDB_EXPORT ReadOptions {
	// If true, all data read from underlying storage will be
	// verified against corresponding checksums.
	// Default: false
	bool verify_checksums;

	// Should the data read for this iteration be cached in memory?
	// Callers may wish to set this field to false for bulk scans.
	// Default: true
	bool fill_cache;

	// If "snapshot" is non-NULL, read as of the supplied snapshot
	// (which must belong to the DB that is being read and which must
	// not have been released).  If "snapshot" is NULL, use an implicit
	// snapshot of the state at the beginning of this read operation.
	// Default: NULL
	const Snapshot* snapshot;

	ReadOptions()
		: verify_checksums(false),
		fill_cache(true),
		snapshot(NULL) {
	}
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_