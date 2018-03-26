#include <assert.h>
#include "include/leveldb/table_builder.h"
#include "include/leveldb/env.h"
#include "table/block_builder.h"
#include "table/filter_block.h"
#include "table/format.h"
#include "util/coding.h"
#include "util/crc32c.h"

namespace leveldb 
{

struct TableBuilder::Rep
{
	Options options;             // data block的选项
	Options index_block_options; // index block的选项
	WritableFile* file;          // sstable文件
	uint64_t offset;             // 要写入data block在sstable文件中的偏移，初始0  
	Status status;
	BlockBuilder data_block;     //当前操作的data block 
	BlockBuilder index_block;    //sstable的index block  
	std::string last_key;        //当前data block最后的k/v对的key 
	uint64_t num_entries;        //当前data block的个数，初始0  
	bool closed;                 //调用了Finish() or Abandon()，初始false
	FilterBlockBuilder* filter_block; //根据filter数据快速定位key是否在block中  
	bool pending_index_entry;         //data_block块是否结束，是否生成indexblock
	BlockHandle pending_handle;  // Handle to add to index block

	std::string compressed_output;

	Rep(const Options& opt, WritableFile* f)
		: options(opt),
		index_block_options(opt),
		file(f),
		offset(0),
		data_block(&options),
		index_block(&index_block_options),
		num_entries(0),
		closed(false),
		filter_block(opt.filter_policy == NULL ? NULL
		: new FilterBlockBuilder(opt.filter_policy)),
		pending_index_entry(false) {
			index_block_options.block_restart_interval = 1;
	}
};

TableBuilder::TableBuilder(const Options& options, WritableFile* file)
	: rep_( new Rep(options, file))
{
	if (rep_->filter_block != NULL)
	{
		rep_->filter_block->StartBlock(0);
	}
}

TableBuilder::~TableBuilder() 
{
	assert(rep_->closed);
	delete rep_->filter_block;
	delete rep_;
}

Status TableBuilder::ChangeOptions(const Options& options) {
	// Note: if more fields are added to Options, update
	// this function to catch changes that should not be allowed to
	// change in the middle of building a Table.
	if (options.comparator != rep_->options.comparator) {
		return Status::InvalidArgument("changing comparator while building table");
	}

	// Note that any live BlockBuilders point to rep_->options and therefore
	// will automatically pick up the updated options.
	rep_->options = options;
	rep_->index_block_options = options;
	rep_->index_block_options.block_restart_interval = 1;
	return Status::OK();
}

void TableBuilder::Add(const Slice& key, const Slice& value) 
{
	Rep* r = rep_;
	assert(!r->closed);

	if (r->num_entries > 0)
	{
		assert(r->options.comparator->Compare(key, Slice(r->last_key)) > 0);
	}

	if (r->pending_index_entry)
	{
		assert(r->data_block.empty());
		r->options.comparator->FindShortestSeparator(&r->last_key, key);
		std::string handle_encoding;
		r->pending_handle.EncodeTo(&handle_encoding);
		r->index_block.Add(r->last_key, Slice(handle_encoding));
		r->pending_index_entry = false;
	}

	if (r->filter_block != NULL)
	{
		r->filter_block->AddKey(key);
	}

	r->last_key.assign(key.data(), key.size());
	r->num_entries++;
	r->data_block.Add(key, value);

	const size_t estimated_block_size = r->data_block.CurrentSizeEstimate();
	if (estimated_block_size >= r->options.block_size)
	{
		Flush();
	}
}

void TableBuilder::Flush()
{
	Rep* r = rep_;
	assert(!r->closed);
	if (!ok()) return;
	if (r->data_block.empty()) return;
	assert(!r->pending_index_entry);
	WriteBlock(&r->data_block, &r->pending_handle);
	if (ok())
	{
		r->pending_index_entry = true;
		r->status = r->file->Flush();
	}

	if (r->filter_block != NULL)
	{
		r->filter_block->StartBlock(r->offset);
	}
}

void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle)
{
	assert(ok());
	Rep* r = rep_;
	Slice raw = block->Finish();

	Slice block_contents;
	CompressionType type = r->options.compression;
	switch (type)
	{
	case kNoCompression:
		block_contents = raw;
		break;

	case kSnappyCompression: 
		{
			/*
			std::string* compressed = &r->compressed_output;
			if (port::Snappy_Compress(raw.data(), raw.size(), compressed) &&
				compressed->size() < raw.size() - (raw.size() / 8u)) {
					block_contents = *compressed;
			} else {
				// Snappy not supported, or compressed less than 12.5%, so just
				// store uncompressed form
				block_contents = raw;
				type = kNoCompression;
			}*/
			block_contents = raw;
			break;
		}
	}

	WriteRawBlock(block_contents, type, handle);
	block->Reset();
}

void TableBuilder::WriteRawBlock(const Slice& data, CompressionType type, BlockHandle* handle)
{
	Rep* r = rep_;
	handle->set_offset(r->offset);
	handle->set_size(data.size());
	r->status = r->file->Append(data);
	if (r->status.ok())
	{
		char trailer[kBlockTrailerSize];
		trailer[0] = type;
		uint32_t crc = crc32c::Value(data.data(), data.size());
		crc = crc32c::Extend(crc, trailer, 1);
		EncodeFixed32(trailer+1, crc32c::Mask(crc));
		r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
		if (r->status.ok())
		{
			r->offset += data.size() + kBlockTrailerSize;
		}
	}
}

Status TableBuilder::status() const {
	return rep_->status;
}

Status TableBuilder::Finish()
{
	Rep* r = rep_;
	Flush();
	assert(!r->closed);
	r->closed = true;

	//write filter block
	BlockHandle filter_block_handle, metaindex_block_handle, index_block_handle;
	if (ok() && r->filter_block != NULL)
	{
		WriteRawBlock(r->filter_block->Finish(), kNoCompression, &filter_block_handle);
	}

	if (ok())
	{
		//write filter index block
		BlockBuilder meta_index_block(&r->options);
		if (r->filter_block != NULL)
		{
			std::string key = "filter.";
			key.append(r->options.filter_policy->Name());
			std::string handle_encoding;
			filter_block_handle.EncodeTo(&handle_encoding);
			meta_index_block.Add(key, handle_encoding);
		}
		WriteBlock(&meta_index_block, &metaindex_block_handle);
	}

	if (ok())
	{
		//write index block
		if (r->pending_index_entry)
		{
			r->options.comparator->FindShortestSeparator(&r->last_key);
			std::string handle_encoding;
			r->pending_handle.EncodeTo(&handle_encoding);
			r->index_block.Add(r->last_key, Slice(handle_encoding));
			r->pending_index_entry = false;
		}
		WriteBlock(&r->index_block, &index_block_handle);
	}

}

void TableBuilder::Abandon() {
	Rep* r = rep_;
	assert(!r->closed);
	r->closed = true;
}

uint64_t TableBuilder::NumEntries() const {
	return rep_->num_entries;
}

uint64_t TableBuilder::FileSize() const {
	return rep_->offset;
}

}

