#include "include/leveldb/table.h"

#include "include/leveldb/cache.h"
#include "include/leveldb/comparator.h"
#include "include/leveldb/env.h"
#include "include/leveldb/filter_policy.h"
#include "include/leveldb/options.h"
#include "table/block.h"
#include "table/filter_block.h"
#include "table/format.h"
//#include "table/two_level_iterator.h"
#include "util/coding.h"

namespace leveldb {

struct Table::Rep {
	Options options;
	Status status;
	RandomAccessFile* file;
	uint64_t cache_id;
	FilterBlockBuilder* filter;
	const char* filter_data;
	BlockHandle metaindex_handle;
	Block* index_block;
	~Rep() {
		delete filter;
		delete[] filter_data;
		delete index_block;
	}
};

Status Table::Open(const Options& options, RandomAccessFile* file, uint64_t size, Table** table)
{
	*table = NULL;
	if (size < Footer::kEncodedLength)
	{
		return Status::Corruption("file is too short to be an sstable");
	}

	char footer_space[Footer::kEncodedLength];
	Slice footer_input;
	Status s = file->Read(size - Footer::kEncodedLength, Footer::kEncodedLength, &footer_input, footer_space);
	if (!s.ok()) return s;

	Footer footer;
	s = footer.DecodeFrom(&footer_input);
	if (!s.ok()) return s;

	BlockContents index_block_contents;
	if (s.ok()) {
		ReadOptions opt;
		if (options.paranoid_checks) {
			opt.verify_checksums = true;
		}
		s = ReadBlock(file,opt, footer.index_handle(), &index_block_contents);
	}
	if (s.ok())	{
		Block* index_block = new Block(index_block_contents);
		Rep* rep = new Table::Rep;
		rep->options = options;
		rep->file = file;
		rep->metaindex_handle = footer.metaindex_handle();
		rep->index_block = index_block;
		rep->cache_id = (options.block_cache ? options.block_cache->NewId() : 0);
		rep->filter_data = NULL;
		rep->filter = NULL;
		*table = new Table(rep);
		(*table)->ReadMeta(footer);
	}

	return s;
}

void Table::ReadMeta(const Footer& footer)
{
	if (rep_->options.filter_policy == NULL) {
		return;
	}

	ReadOptions opt;
	if (rep_->options.paranoid_checks) {
		opt.verify_checksums = true;
	}

	BlockContents contents;
	if (!ReadBlock(rep_->file, opt, footer.metaindex_handle(), &contents).ok()) {
		return;
	}
	Block* meta = new Block(contents);
	Iterator* iter = meta->NewIterator(BytewiseComparator());
	std::string key = "filter.";
	key.append(rep_->options.filter_policy->Name());
	iter->Seek(key);
	if (iter->Valid() && iter->key() == Slice(key)) {
		ReadFilter(iter->value());
	}
	delete iter;
	delete meta;
}

void Table::ReadFilter(const Slice& filter_handle_value)
{
	Slice v = filter_handle_value;
	BlockHandle filter_handle;
	if (!filter_handle.DecodeFrom(&v).ok()) {
		return;
	}

	ReadOptions opt;
	if (rep_->options.paranoid_checks) {
		opt.verify_checksums = true;
	}
	BlockContents block;
	if (!ReadBlock(rep_->file, opt, filter_handle, &block).ok()) {
		return;
	}
	if (block.heap_allocated) {
		rep_->filter_data = block.data.data();
	}
	rep_->filter = new FilterBlockBuilder(rep_->options.filter_policy, block.data);
}

Table::~Table() {
	delete rep_;
}

static void DeleteBlock(void* arg, void* ignored) {
	delete reinterpret_cast<Block*>(arg);
}

static void DeleteCachedBlock(const Slice& key, void* value) {
	Block* block = reinterpret_cast<Block*>(value);
	delete block;
}

static void ReleaseBlock(void* arg, void* h) {
	Cache* cache = reinterpret_cast<Cache*>(arg);
	Cache::Handle* handle = reinterpret_cast<Cache::Handle*>(h);
	cache->Release(handle);
}

}