#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "MsgTLV.h"

#define new DEBUG_NEW

MsgTLV::MsgTLV()
{
	tagType = 0;
	tagLength = 0;
	tagValue = NULL;
}

MsgTLV::MsgTLV(u16 myType)
{
	tagType = myType;
	tagLength = 0;
	tagValue = NULL;
}

MsgTLV::~MsgTLV()
{
	if(tagValue != NULL)
	{
		delete[] tagValue;
		tagValue = NULL;
	}
}

bool MsgTLV::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get16(tagType));
	FAIL_RETURN(content.get16(tagLength));
	if(tagLength <=0)
		return false;

	if(tagLength >= TLV_MAX_LENGTH)
		return false;

	delete[] tagValue;
	tagValue = new u8[tagLength];

	if(!content.getStream(tagValue, tagLength))
	{
		delete[] tagValue;
		tagValue = NULL;
		tagLength = 0;
		tagType = 0;
		return false;
	}

	return true;
}

bool MsgTLV::toStream(cacti::Stream &content)
{
	if(tagValue == NULL || tagLength == 0)
		return false;

	content.put16(tagType);
	content.put16(tagLength);
	content.putStream(tagValue, tagLength);

	return true;
}

MsgTLV& MsgTLV::operator = (const MsgTLV& other)
{
	if(this != &other)
	{
		tagType = other.tagType;
		tagLength = other.tagLength;
		if(other.tagLength > 0)
		{
			delete[] tagValue;
			tagValue = new u8[tagLength];
			memcpy(tagValue, other.tagValue, other.tagLength);
		}
	}

	return *this;
}

MsgTLV& MsgTLV::operator = (u8 val)
{
	tagLength = 1;
	delete[] tagValue;
	tagValue = new u8[tagLength];
	tagValue[0] = val;

	return *this;
}

MsgTLV& MsgTLV::operator = (u16 val)
{
	tagLength = 2;
	delete[] tagValue;
	tagValue = new u8[tagLength];
	tagValue[0] = u8(val >> 8);
	tagValue[1] = u8(val);

	return *this;
}

MsgTLV& MsgTLV::operator = (u32 val)
{
	tagLength = 4;
	delete[] tagValue;
	tagValue = new u8[tagLength];
	Stream::put32(tagValue, val);

	return *this;
}

MsgTLV& MsgTLV::operator = (const u8array& val)
{
	if(val.size() >= TLV_MAX_LENGTH || val.size() <= 0)
		return *this;

	tagLength = 1*(u16)val.size();
	delete[] tagValue;
	tagValue = new u8[tagLength];
	for(size_t i=0; i<tagLength; i++)
		tagValue[i] = val[i];

	return *this;
}

MsgTLV& MsgTLV::operator = (const string& val)
{
	if(val.length() >= TLV_MAX_LENGTH || val.length() <= 0)
		return *this;

	tagLength = 1*(u16)val.length();
	delete[] tagValue;
	tagValue = new u8[tagLength];
	for(size_t i=0; i<tagLength; i++)
		tagValue[i] = val[i];

	return *this;
}

MsgTLV::operator u8() const
{
	u8 ret = 0;
	if(tagValue && tagLength >= 1)
	{
		ret = tagValue[0];
	}
	return ret;
}

MsgTLV::operator u16() const
{
	u16 ret = 0;
	if(tagValue && tagLength >= 2)
	{
		ret = (u8)tagValue[0];
		ret = (ret << 8) | (u8)tagValue[1];
	}
	return ret;
}

MsgTLV::operator u32() const
{
	u32 ret = 0;
	if(tagValue && tagLength >= 4)
	{
		ret = Stream::get32(tagValue);
	}
	return ret;
}

MsgTLV::operator u8array() const
{
	u8array ret;
	if(tagValue)
	{
		ret.resize(tagLength);
		std::copy(tagValue, tagValue+tagLength, ret.begin());
	}
	return ret;
}

MsgTLV::operator string() const
{
	string ret;
	if(tagValue)
	{
		ret.resize(tagLength);
		std::copy(tagValue, tagValue+tagLength, ret.begin());
	}
	return ret;
}