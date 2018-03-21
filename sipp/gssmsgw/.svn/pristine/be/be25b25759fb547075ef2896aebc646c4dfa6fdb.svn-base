#include "stdafx.h"

#include "cngppacket.h"


#define new DEBUG_NEW


bool CNGPHeader::fromStream(cacti::Stream &content)
{
	FAIL_RETURN(content.get32(totalLength));
	FAIL_RETURN(content.get32(command));
	FAIL_RETURN(content.get32(status));
	FAIL_RETURN(content.get32(sequence));
	return true;
}


bool CNGPHeader::toStream(cacti::Stream &content)
{
	content.put32(totalLength);
	content.put32(command);
	content.put32(status);
	content.put32(sequence);

	return true;
}


CNGPHeader& CNGPHeader::operator = (const CNGPHeader& other)
{
	if(this != &other)
	{
		totalLength = other.totalLength;
		command = other.command;
		status = other.status;
		sequence = other.sequence;
	}

	return *this;
}

bool CNGPPacket::fromStream(cacti::Stream &content, bool bOnlyBody)
{
	checkBuffer();
	if(!bOnlyBody)
	{
		FAIL_RETURN(header.fromStream(content));
	}
	FAIL_RETURN(decodeBody(content));

	return true;
}

bool CNGPPacket::toStream(cacti::Stream &content)
{
	checkBuffer();

	header.toStream(content);
	encodeBody(content);
	size_t length = content.size();
	if(length <= 0 && length > CNGP::Max_Packet_Size)
	{
		return false;
	}

	u8 *tmp = (u8 *)content.getWriteBuffer();
	content.put32(tmp, (u32)length);

	return true;
}

bool CNGPPacket::encodeBody(cacti::Stream &content)
{
	return true;
}

bool CNGPPacket::decodeBody(cacti::Stream &content)
{
	return true;
}


void CNGPLogin::checkBuffer()
{
	clientID.resize(10);
	authenticatorClient.resize(16);
}

bool CNGPLogin::encodeBody(cacti::Stream &content)
{
	content.putStream(&clientID[0], clientID.size());
	content.putStream(&authenticatorClient[0], authenticatorClient.size());
	content.put8(loginMode);
	content.put32(timeStamp);
	content.put8(clientVersion);

	return true;
}

void CNGPLoginResp::checkBuffer()
{
	authenticatorServer.resize(16);
}

bool CNGPLoginResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&authenticatorServer[0], authenticatorServer.size()));
	FAIL_RETURN(content.get8(serverVersion));

	return true;
}


void CNGPSubmit::checkBuffer()
{
	clientID.resize(10);
	serviceID.resize(10);
	feeType.resize(2);
	//feeUserType.resize(1);
	feeCode.resize(6);
	validTime.resize(17);
	atTime.resize(17);
	srcTermID.resize(21);
	chargeTermID.resize(21);
	destTermID.resize(21*destTermIDCount);
	msgContent.resize(msgLength);
}

bool CNGPSubmit::encodeBody(cacti::Stream &content)
{
	content.putStream(&clientID[0], clientID.size());
	content.put8(subType);				
	content.put8(needReport);			
	content.put8(priority);				
	content.putStream(&serviceID[0], serviceID.size());		
	content.putStream(&feeType[0], feeType.size());
	//content.putStream(&feeUserType[0], feeUserType.size());	
	content.put8(feeUserType);
	content.putStream(&feeCode[0], feeCode.size());			
	content.put8(msgFormat);			
	content.putStream(&validTime[0], validTime.size());		
	content.putStream(&atTime[0], atTime.size());				
	content.putStream(&srcTermID[0], srcTermID.size());		
	content.putStream(&chargeTermID[0], chargeTermID.size());	
	content.put8(destTermIDCount);
	content.putStream(&destTermID[0], destTermID.size());		
	content.put8(msgLength);			
	content.putStream(&msgContent[0], msgContent.size());		

	return true;
}

void CNGPSubmitResp::checkBuffer()
{
	msgID.resize(10);
}

bool CNGPSubmitResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&msgID[0], msgID.size()));

	return true;
}

void CNGPDeliver::checkBuffer()
{
	msgID.resize(10);
	recvTime.resize(14);
	srcTermID.resize(21);
	destTermID.resize(21);
//	msgContent.resize();
}

bool CNGPDeliver::decodeBody(cacti::Stream &content)
{
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

	return true;
}

void CNGPDeliverResp::checkBuffer()
{
	msgID.resize(10);
}

bool CNGPDeliverResp::encodeBody(cacti::Stream &content)
{
	content.putStream(&msgID[0], msgID.size());

	return true;
}

bool CNGPExit::encodeBody(cacti::Stream &content)
{
	return true;
}


bool CNGPExitResp::decodeBody(cacti::Stream &content)
{
	return true;
}


bool CNGPActiveTest::encodeBody(cacti::Stream &content)
{
	return true;
}


bool CNGPActiveTestResp::decodeBody(cacti::Stream &content)
{
	return true;
}

bool CNGPQueryUserState::encodeBody(cacti::Stream &content)
{
	content.putStream(&userAddr[0],userAddr.size());
	content.putStream(&smgwNo[0],smgwNo.size());
	return true;
}

bool CNGPQueryUserStateResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.getStream(&userStatus[0],userStatus.size()));
	FAIL_RETURN(content.get32(count));
	return true;

}
bool CNGPPaymentRequest::encodeBody(cacti::Stream &content)
{
	content.putStream(&messagId[0],messagId.size());
	content.put8(smType);
	content.put8(subType);
	content.putStream(&chargeTermId[0],chargeTermId.size());
	content.putStream(&spId[0],spId.size());
	content.putStream(&destTermId[0],destTermId.size());
	content.putStream(&serviceId[0],serviceId.size());
	content.putStream(&feeType[0],feeType.size());
	content.putStream(&feeCode[0],feeCode.size());
	content.put8(priority);
	content.put8(msgLength);
	content.putStream(&areaCode[0],areaCode.size());
	content.putStream(&smgwNo[0],smgwNo.size());
	content.putStream(&fwdSmgwNo[0],smgwNo.size());
	content.putStream(&smscNo[0],smscNo.size());
	content.putStream(&recvTime[0],recvTime.size());
	content.putStream(&doneTime[0],doneTime.size());
	return true;
}

bool CNGPPaymentRequestResp::decodeBody(cacti::Stream &content)
{
	FAIL_RETURN(content.get8(resultNotifyCode));
	return true;

}

bool CNGPPaymentAffirm::encodeBody(cacti::Stream &content)
{
	content.putStream(&messagId[0], messagId.size());
	content.put8(responseStatus);				
	content.putStream(&feeAddr[0],feeAddr.size());			
	return true;
}

bool CNGPPaymentAffirmResp::decodeBody(cacti::Stream &content)
{
	// no message body
	return true;

}

CNGPPacketPtr CNGPPacketFactory::createPacket(u32 cmd)
{
	switch(cmd)
	{
	case CNGP_Login:
		return CNGPPacketPtr(new CNGPLogin());
	case CNGP_Login_Resp:
		return CNGPPacketPtr(new CNGPLoginResp());
	case CNGP_Exit:
		return CNGPPacketPtr(new CNGPExit());
	case CNGP_Exit_Resp:
		return CNGPPacketPtr(new CNGPExitResp());
	case CNGP_Submit:
		return CNGPPacketPtr(new CNGPSubmit());
	case CNGP_Submit_Resp:
		return CNGPPacketPtr(new CNGPSubmitResp());
	case CNGP_Deliver:
		return CNGPPacketPtr(new CNGPDeliver());	
	case CNGP_Deliver_Resp:
		return CNGPPacketPtr(new CNGPDeliverResp());
	case CNGP_Active_Test:
		return CNGPPacketPtr(new CNGPActiveTest());
	case CNGP_Active_Test_Resp:
		return CNGPPacketPtr(new CNGPActiveTestResp());
	case CNGP_Query_UserState:
		return CNGPPacketPtr(new CNGPQueryUserState());
	case CNGP_Query_UserState_Resp:
		return CNGPPacketPtr(new CNGPQueryUserStateResp());
	case CNGP_Payment_Request:
		return CNGPPacketPtr(new CNGPPaymentRequest());
	case CNGP_Payment_Request_Resp:
		return CNGPPacketPtr(new CNGPPaymentRequestResp());
	case CNGP_Payment_Affirm:
		return CNGPPacketPtr(new CNGPPaymentAffirm());
	case CNGP_Payment_Affirm_Resp:
		return CNGPPacketPtr(new CNGPPaymentAffirmResp());

	default:
		return CNGPPacketPtr(new CNGPPacket(cmd));
	}
}