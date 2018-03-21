#ifndef __MSG_TLV_H
#define __MSG_TLV_H

#pragma once

#include "boost/shared_ptr.hpp"
#include "cacti/message/Stream.h"
#include "cacti/message/TLV.h"

#include <string>
#include <map>

using namespace cacti;
using namespace std;

static const u16 TLV_MAX_LENGTH = 0x1FFF;

class MsgTLV
{
public:
	MsgTLV();
	MsgTLV(u16 myType);
	virtual ~MsgTLV();

public:
	u16  getType() { return tagType; };
	u16  getLength() { return tagLength; };


	bool fromStream(cacti::Stream &content);
	bool toStream(cacti::Stream &content);
	MsgTLV& operator = (const MsgTLV& other);

	MsgTLV& operator = (u8 val);
	MsgTLV& operator = (u16 val);
	MsgTLV& operator = (u32 val);
	MsgTLV& operator = (const u8array& val);
	MsgTLV& operator = (const string& val);

	operator u8() const;
	operator u16() const;
	operator u32() const;
	operator u8array() const;
	operator string() const;

protected:
	u16 tagType;
	u16 tagLength;
	u8* tagValue;
};

typedef boost::shared_ptr<MsgTLV> MsgTLVPtr;
typedef std::vector<MsgTLVPtr> MsgTLVList;


#endif