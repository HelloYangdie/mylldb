#include "util/coding.h"

namespace leveldb 
{

int VarintLength(uint64_t v) 
{
	int len = 1;
	while (v >= 128) {
		v >>= 7;
		len++;
	}
	return len;
}

void PutFixed32(std::string* dst, uint32_t value) {
	char buf[sizeof(value)];
	EncodeFixed32(buf, value);
	dst->append(buf, sizeof(buf));
}

void PutFixed64(std::string* dst, uint64_t value) {
	char buf[sizeof(value)];
	EncodeFixed64(buf, value);
	dst->append(buf, sizeof(buf));
}

void PutVarint32(std::string* dst, uint32_t v) {
	char buf[5];
	char* ptr = EncodeVarint32(buf, v);
	dst->append(buf, ptr - buf);
}

void PutVarint64(std::string* dst, uint64_t v) {
	char buf[10];
	char* ptr = EncodeVarint64(buf, v);
	dst->append(buf, ptr - buf);
}

void PutLengthPrefixedSlice(std::string* dst, const Slice& value) {
	PutVarint32(dst, value.size());
	dst->append(value.data(), value.size());
}

char* EncodeVarint32(char* dst, uint32_t v)
{
	unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
	static const int B = 128;
	if (v < (1<<7)) {
		*(ptr++) = v;
	} else if (v < (1<<14)) {
		*(ptr++) = v | B;
		*(ptr++) = v>>7;
	} else if (v < (1<<21)) {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = v>>14;
	} else if (v < (1<<28)) {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = (v>>14) | B;
		*(ptr++) = v>>21;
	} else {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = (v>>14) | B;
		*(ptr++) = (v>>21) | B;
		*(ptr++) = v>>28;
	}
	return reinterpret_cast<char*>(ptr);
}

/*
static_cast
	a) �������νṹ�л����������֮��ָ��������õ�ת����
	up-casting (���������ָ�������ת���ɻ����ָ��������ñ�ʾ)�ǰ�ȫ�ģ�
	down-casting(�ѻ���ָ�������ת���������ָ���������)�ǲ���ȫ�ġ�
	b) ���ڻ�����������֮���ת�������intת����char������ת���İ�ȫ��ҲҪ�ɿ�����Ա����֤��
	c) ���԰ѿ�ָ��ת����Ŀ�����͵Ŀ�ָ��(null pointer)��
	d) ���κ����͵ı��ʽת����void���͡�
reinterpret_cast:
	ת��һ��ָ��Ϊ�������͵�ָ�룬Ҳ����һ��ָ��ת��Ϊ�������ͣ���֮��Ȼ��
	����������ܹ��ڷ���ص�����֮�����ת����
	�������ֻ�Ǽ򵥵Ĵ�һ��ָ�뵽���ָ���ֵ�Ķ����ƿ�����
	������֮��ָ������ݲ����κ����͵ļ���ת��������һ��ǿ��ת����
*/
char* EncodeVarint64(char* dst, uint64_t v)
{
	static const int B = 128;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
	while (v >= B) {
		*(ptr++) = (v & (B-1)) | B;
		v >>= 7;
	}
	*(ptr++) = static_cast<unsigned char>(v);
	return reinterpret_cast<char*>(ptr);
}

void EncodeFixed32(char* dst, uint32_t value)
{
	memcpy(dst, &value, sizeof(value));
}

void EncodeFixed64(char* dst, uint64_t value)
{
	memcpy(dst, &value, sizeof(value));
}

const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value)
{
	uint32_t result = 0;
	uint32_t byte = 0;
	for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7)
	{
		byte = *(reinterpret_cast<const unsigned char*>(p));
		p++;
		if (byte & 128)
		{
			result |= ((byte & 0x7F) << shift);
		}
		else
		{
			 result |= (byte << shift);
			 *value = result;
			 return reinterpret_cast<const char*>(p);
		}
	}

	return NULL;
}

}