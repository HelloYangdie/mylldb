#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "include/leveldb/slice.h"

namespace leveldb {

class FilterPolicy;

class FilterBlockBuilder {
public:
	explicit FilterBlockBuilder(const FilterPolicy*);

	void StartBlock(uint64_t block_offset);
	void AddKey(const Slice& key);
	Slice Finish();

private:
	void GenerateFilter();

	const FilterPolicy* policy_;
	std::string keys_;              // Flattened key contents
	std::vector<size_t> start_;     // Starting index in keys_ of each key
	std::string result_;            // Filter data computed so far
	std::vector<Slice> tmp_keys_;   // policy_->CreateFilter() argument
	std::vector<uint32_t> filter_offsets_;

	// No copying allowed
	FilterBlockBuilder(const FilterBlockBuilder&);
	void operator=(const FilterBlockBuilder&);
};

}