#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "include/leveldb/export.h"
#include "include/leveldb/status.h"

namespace leveldb 
{

class LEVELDB_EXPORT WritableFile {
public:
	WritableFile() { }
	virtual ~WritableFile();

	virtual Status Append(const Slice& data) = 0;

	virtual Status Close() = 0;

	virtual Status Flush() = 0;

	virtual Status Sync() = 0;

private:
	// No copying allowed
	WritableFile(const WritableFile&);
	void operator=(const WritableFile&);
};

// A file abstraction for randomly reading the contents of a file.
class LEVELDB_EXPORT RandomAccessFile {
public:
	RandomAccessFile() { }
	virtual ~RandomAccessFile();

	// Read up to "n" bytes from the file starting at "offset".
	// "scratch[0..n-1]" may be written by this routine.  Sets "*result"
	// to the data that was read (including if fewer than "n" bytes were
	// successfully read).  May set "*result" to point at data in
	// "scratch[0..n-1]", so "scratch[0..n-1]" must be live when
	// "*result" is used.  If an error was encountered, returns a non-OK
	// status.
	//
	// Safe for concurrent use by multiple threads.
	virtual Status Read(uint64_t offset, size_t n, Slice* result,
		char* scratch) const = 0;

private:
	// No copying allowed
	RandomAccessFile(const RandomAccessFile&);
	void operator=(const RandomAccessFile&);
};

class LEVELDB_EXPORT Env {
public:
	Env() { }
	virtual ~Env();

	virtual Status NewWritableFile(const std::string& fname,
		WritableFile** result) = 0;

};

}