#include "stdafx.h"

#include <cacti/util/MD5.h>
#include <cacti/util/StringUtil.h>
#include "smgppacket.h"
#include <time.h>

#define new DEBUG_NEW


bool SMGPHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(sequence));

	return true;
}


bool SMGPHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(sequence);

	return true;
}


SMGPHeader& SMGPHeader::operator = (const SMGPHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		sequence = other.sequence;
	}

	return *this;
}

bool SMGPPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool SMGPPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > SMGP::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool SMGPPacket::toArray(u8array &content)
{
	cacti::Stream buf;
	toStream(buf);
	buf.reset();
	content.resize(buf.size());
	buf.getStream(&content[0], buf.size());
	return true;
}
bool SMGPPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPPacket::decodeBody(cacti::Stream &content)
{
	return true;
}

const MsgTLV&  SMGPPacket::getTLV(u16 tlvType)
{
	MsgTLVList::const_iterator it = tlvList.begin();
	for( ; it != tlvList.end(); it++)
	{
		if((*it)->getType() == tlvType)
			return *(*it);			
	}

	return  m_tlv;
}

const char* SMGPPacket::toString(const char* split)
{

	return split;	
}


void SMGPLogin::checkBuffer()
{
	clientID.resize(8);
	authenticatorClient.resize(16);
}

bool SMGPLogin::encodeBody(cacti::Stream &content)
{
	content.putStream(&clientID[0], clientID.size());
	content.putStream(&authenticatorClient[0], authenticatorClient.size());
	content.put8(loginMode);
	content.put32(timeStamp);
	content.put8(clientVersion);

	return true;
}

bool SMGPLogin::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&clientID[0], clientID.size()));
	FAIL_RETURN(content.getStream(&authenticatorClient[0], authenticatorClient.size()));
	FAIL_RETURN(content.get8(loginMode));
	FAIL_RETURN(content.get32(timeStamp));
	FAIL_RETURN(content.get8(clientVersion));

	return true;
}


void SMGPLoginResp::checkBuffer()
{
	authenticatorServer.resize(16);
}

bool SMGPLoginResp::encodeBody(cacti::Stream &content)
{
	content.put32(status);
	content.putStream(&authenticatorServer[0], authenticatorServer.size());
	content.put8(serverVersion);

	return true;
}

bool SMGPLoginResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(status));
	FAIL_RETURN(content.getStream(&authenticatorServer[0], authenticatorServer.size()));
	FAIL_RETURN(content.get8(serverVersion));

	return true;
}

SMGPSubmit::SMGPSubmit(u8 ver)
	:SMGPPacket(SMGP_Submit, ver),
	TP_pid(SMGP::Tag_TP_pid),
	TP_udhi(SMGP::Tag_TP_udhi),
	LinkID(SMGP::Tag_LinkID),
	MsgSrc(SMGP::Tag_MsgSrc),
	ChargeUserType(SMGP::Tag_ChargeUserType),
	ChargeTermType(SMGP::Tag_ChargeTermType),
	ChargeTermPseudo(SMGP::Tag_ChargeTermPseudo),
	DestTermType(SMGP::Tag_DestTermType),
	DestTermPseudo(SMGP::Tag_DestTermPseudo),
	PkTotal(SMGP::Tag_PkTotal),
	PkNumber(SMGP::Tag_PkNumber),
	SubmitMsgType(SMGP::Tag_SubmitMsgType),
	SPDealResult(SMGP::Tag_SPDealReslt),
	MServiceID(SMGP::Tag_MServiceID)
{

};

void SMGPSubmit::checkBuffer()
{
	serviceID.resize(10);
	feeType.resize(2);
	feeCode.resize(6);
	fixedFee.resize(6);
	validTime.resize(17);
	atTime.resize(17);
	srcTermID.resize(21);
	chargeTermID.resize(21);
	destTermID.resize(21);
	//msgContent.resize(msgLength);
	reserve.resize(8);
}

bool SMGPSubmit::encodeBody(cacti::Stream &content)
{
//	checkBuffer();

	content.put8(msgType);				
	content.put8(needReport);			
	content.put8(priority);				
	content.putStream(&serviceID[0], serviceID.size());		
	content.putStream(&feeType[0], feeType.size());			
	content.putStream(&feeCode[0], feeCode.size());			
	content.putStream(&fixedFee[0], fixedFee.size());			
	content.put8(msgFormat);			
	content.putStream(&validTime[0], validTime.size());		
	content.putStream(&atTime[0], atTime.size());				
	content.putStream(&srcTermID[0], srcTermID.size());		
	content.putStream(&chargeTermID[0], chargeTermID.size());	
	content.put8(destTermIDCount);
	if(destTermIDCount > 0)
	{
		destTermID.resize(21*destTermIDCount);
		content.putStream(&destTermID[0], destTermID.size());		
	}
	content.put8(msgLength);
	if(msgLength > 0)
	{
		msgContent.resize(msgLength);
		content.putStream(&msgContent[0], msgContent.size());
	}
	content.putStream(&reserve[0], reserve.size());			

	if(getVersion() == SMGP::Version_30)
	{
//  	TP_pid.toStream(content);
 		TP_udhi.toStream(content);
  		
		string t_linkid = LinkID;
		if(strlen(t_linkid.c_str()) > 1)  //for huawei gw, cannot send linkid="0"
		{
			LinkID.toStream(content);
		}

		MsgSrc.toStream(content);
// 		ChargeUserType.toStream(content);
// 		ChargeTermType.toStream(content);
// 		if((u8)ChargeTermType == 1)
// 		{
// 			ChargeTermPseudo.toStream(content);
// 		}
// 		DestTermType.toStream(content);
// 		if((u8)DestTermType == 1)
// 		{
// 			DestTermPseudo.toStream(content);
// 		}

		PkTotal.toStream(content);
  		PkNumber.toStream(content);
 		SubmitMsgType.toStream(content);
 		SPDealResult.toStream(content);
		MServiceID.toStream(content);
	}

	return true;
}

bool SMGPSubmit::decodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	FAIL_RETURN(content.get8(msgType));				
	FAIL_RETURN(content.get8(needReport));			
	FAIL_RETURN(content.get8(priority));				
	FAIL_RETURN(content.getStream(&serviceID[0], serviceID.size()));
	FAIL_RETURN(content.getStream(&feeType[0], feeType.size()));			
	FAIL_RETURN(content.getStream(&feeCode[0], feeCode.size()));			
	FAIL_RETURN(content.getStream(&fixedFee[0], fixedFee.size()));			
	FAIL_RETURN(content.get8(msgFormat));			
	FAIL_RETURN(content.getStream(&validTime[0], validTime.size()));
	FAIL_RETURN(content.getStream(&atTime[0], atTime.size()));				
	FAIL_RETURN(content.getStream(&srcTermID[0], srcTermID.size()));		
	FAIL_RETURN(content.getStream(&chargeTermID[0], chargeTermID.size()));	
	FAIL_RETURN(content.get8(destTermIDCount));
	if(destTermIDCount > 0)
	{
		destTermID.resize(21*destTermIDCount);
		FAIL_RETURN(content.getStream(&destTermID[0], destTermID.size()));
	}
	FAIL_RETURN(content.get8(msgLength));
	if(msgLength > 0)
	{
		msgContent.resize(msgLength);
		FAIL_RETURN(content.getStream(&msgContent[0], msgContent.size()));
	}
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));		

	if(getVersion() == SMGP::Version_30)
	{
		TP_udhi.fromStream(content);
		LinkID.fromStream(content);

		MsgSrc.fromStream(content);

		PkTotal.fromStream(content);
		PkNumber.fromStream(content);
		SubmitMsgType.fromStream(content);
		SPDealResult.fromStream(content);
		MServiceID.fromStream(content);
	}

	return true;
}

void SMGPSubmitResp::checkBuffer()
{
	msgID.resize(10);
}

bool SMGPSubmitResp::encodeBody(cacti::Stream &content)
{
	content.putStream(&msgID[0], msgID.size());
	content.put32(status);

	return true;
}

bool SMGPSubmitResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&msgID[0], msgID.size()));
	FAIL_RETURN(content.get32(status));

	return true;
}


SMGPDeliver::SMGPDeliver(u8 ver)
	:SMGPPacket(SMGP_Deliver, ver),
	TP_pid(SMGP::Tag_TP_pid),
	TP_udhi(SMGP::Tag_TP_udhi),
	LinkID(SMGP::Tag_LinkID),
	SrcTermType(SMGP::Tag_SrcTermType),
	SrcTermPseudo(SMGP::Tag_SrcTermPseudo),
	SubmitMsgType(SMGP::Tag_SubmitMsgType),
	SPDealResult(SMGP::Tag_SPDealReslt)
{

};

void SMGPDeliver::checkBuffer()
{
	msgID.resize(10);
	recvTime.resize(14);
	srcTermID.resize(21);
	destTermID.resize(21);
//	msgContent.resize();
	reserve.resize(8);
}

bool SMGPDeliver::encodeBody(cacti::Stream &content)
{
	//	checkBuffer();

	content.putStream(&msgID[0], msgID.size());
	content.put8(isReport);
	content.put8(msgFormat);
	content.putStream(&recvTime[0], recvTime.size());
	content.putStream(&srcTermID[0], srcTermID.size());
	content.putStream(&destTermID[0], destTermID.size());
	content.put8(msgLength);
	if(msgLength > 0)
	{
		msgContent.resize(msgLength);
		content.putStream(&msgContent[0], msgLength);
	}
	content.putStream(&reserve[0], reserve.size());

	if(getVersion() == SMGP::Version_30)
	{
		LinkID.toStream(content);
		TP_pid.toStream(content);
		TP_udhi.toStream(content);

		SrcTermType.toStream(content);
		SrcTermPseudo.toStream(content);
		SubmitMsgType.toStream(content);
		SPDealResult.toStream(content);
	}
	return true;
}

bool SMGPDeliver::decodeBody(cacti::Stream &content)
{
//	checkBuffer();

	FAIL_RETURN(content.getStream(&msgID[0], msgID.size()));
	FAIL_RETURN(content.get8(isReport));
	FAIL_RETURN(content.get8(msgFormat));
	FAIL_RETURN(content.getStream(&recvTime[0], recvTime.size()));
	FAIL_RETURN(content.getStream(&srcTermID[0], srcTermID.size()));
	FAIL_RETURN(content.getStream(&destTermID[0], destTermID.size()));
	FAIL_RETURN(content.get8(msgLength));
	if(msgLength > 0)
	{
		msgContent.resize(msgLength);
		FAIL_RETURN(content.getStream(&msgContent[0], msgLength));
	}
	FAIL_RETURN(content.getStream(&reserve[0], reserve.size()));

	if(getVersion() == SMGP::Version_30)
	{
		for(int i = 0; i < 7; i++)
		{
			MsgTLVPtr tlv = MsgTLVPtr(new MsgTLV());
			if(tlv->fromStream(content))
			{
				tlvList.push_back(tlv);
			}
			else
			{
				break;
			}
		}

		LinkID = getTLV(SMGP::Tag_LinkID);
		TP_pid = getTLV(SMGP::Tag_TP_pid);
		TP_udhi = getTLV(SMGP::Tag_TP_udhi);

		SrcTermType = getTLV(SMGP::Tag_SrcTermType);
		SrcTermPseudo = getTLV(SMGP::Tag_SrcTermPseudo);
		SubmitMsgType = getTLV(SMGP::Tag_SubmitMsgType);
		SPDealResult = getTLV(SMGP::Tag_SPDealReslt);
	}
	return true;
}

void SMGPDeliverResp::checkBuffer()
{
	msgID.resize(10);
}

bool SMGPDeliverResp::encodeBody(cacti::Stream &content)
{
	content.putStream(&msgID[0], msgID.size());
	content.put32(status);

	return true;
}

bool SMGPDeliverResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&msgID[0], msgID.size()));
	FAIL_RETURN(content.get32(status));

	return true;
}

bool SMGPExit::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPExit::decodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPExitResp::encodeBody(cacti::Stream &content)
{
	return true;
}


bool SMGPExitResp::decodeBody(cacti::Stream &content)
{
	return true;
}


bool SMGPActiveTest::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPActiveTest::decodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPActiveTestResp::encodeBody(cacti::Stream &content)
{
	return true;
}

bool SMGPActiveTestResp::decodeBody(cacti::Stream &content)
{
	return true;
}

SMGPPacketPtr SMGPPacketFactory::createPacket(u32 cmd, u8 ver)
{
	switch(cmd)
	{
	case SMGP_Login:
		return SMGPPacketPtr(new SMGPLogin());
	case SMGP_Login_Resp:
		return SMGPPacketPtr(new SMGPLoginResp());
	case SMGP_Exit:
		return SMGPPacketPtr(new SMGPExit());
	case SMGP_Exit_Resp:
		return SMGPPacketPtr(new SMGPExitResp());
	case SMGP_Submit:
		return SMGPPacketPtr(new SMGPSubmit(ver));
	case SMGP_Submit_Resp:
		return SMGPPacketPtr(new SMGPSubmitResp());
	case SMGP_Deliver:
		return SMGPPacketPtr(new SMGPDeliver(ver));	
	case SMGP_Deliver_Resp:
		return SMGPPacketPtr(new SMGPDeliverResp());
	case SMGP_Active_Test:
		return SMGPPacketPtr(new SMGPActiveTest());
	case SMGP_Active_Test_Resp:
		return SMGPPacketPtr(new SMGPActiveTestResp());
	default:
		return SMGPPacketPtr(new SMGPPacket(cmd));
	}
}