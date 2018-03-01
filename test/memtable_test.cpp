
#include <algorithm>
#include <iostream>
#include "db/memtable.h"
#include "include/leveldb/slice.h"
#include "include/leveldb/comparator.h"
#include "include/leveldb/status.h"

using namespace leveldb;

class BytewiseComparatorImpl : public Comparator {
public:
	BytewiseComparatorImpl() { }

	virtual const char* Name() const {
		return "leveldb.BytewiseComparator";
	}

	virtual int Compare(const Slice& a, const Slice& b) const {
		return a.compare(b);
	}

	virtual void FindShortestSeparator(
		std::string* start,
		const Slice& limit) const {
			// Find length of common prefix
			size_t min_length = std::min(start->size(), limit.size());
			size_t diff_index = 0;
			while ((diff_index < min_length) &&
				((*start)[diff_index] == limit[diff_index])) {
					diff_index++;
			}

			if (diff_index >= min_length) {
				// Do not shorten if one string is a prefix of the other
			} else {
				uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
				if (diff_byte < static_cast<uint8_t>(0xff) &&
					diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
						(*start)[diff_index]++;
						start->resize(diff_index + 1);
						assert(Compare(*start, limit) < 0);
				}
			}
	}

	virtual void FindShortSuccessor(std::string* key) const {
		// Find first character that can be incremented
		size_t n = key->size();
		for (size_t i = 0; i < n; i++) {
			const uint8_t byte = (*key)[i];
			if (byte != static_cast<uint8_t>(0xff)) {
				(*key)[i] = byte + 1;
				key->resize(i+1);
				return;
			}
		}
		// *key is a run of 0xffs.  Leave it alone.
	}
};

void MemTableTest()
{
	BytewiseComparatorImpl* imp = new BytewiseComparatorImpl();
	InternalKeyComparator comparator(imp);

	MemTable table(comparator);

	std::string key1 = "key1";
	std::string val1 = "value1";
	table.Add(1, kTypeValue, key1, val1);

	std::string key12 = "key12";
	std::string val12 = "value12";
	table.Add(12, kTypeValue, key1, val12);

	LookupKey look(key1, 13);
	std::string value;
	Status s;
	table.Get(look, &value, &s);
	std::cout << "get key:" << key1 << " value is " << value << std::endl;
}