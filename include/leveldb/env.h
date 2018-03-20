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

}