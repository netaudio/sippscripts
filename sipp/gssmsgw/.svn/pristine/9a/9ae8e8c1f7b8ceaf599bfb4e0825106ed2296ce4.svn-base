#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"
#include "sgipprotocol.h"
#include "BaseSGIPProtocol.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW


void BaseSGIPProtocol::test(string userPhone,string linkId,const char* gname){
	UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, _ReqSendSMS);
	(*utm)[_TagGatewayName]=gname;
			
			(*utm)[_TagCalling] = gname;
			(*utm)[_TagCallee] = userPhone;
	
			(*utm)[_TagMessage] = "÷–Œƒ≤‚ ‘ aa";

			std::vector<std::string> m_paramName;
			std::vector<std::string> m_paramValues;

			m_paramName.push_back("Protocol");
			m_paramValues.push_back("SGIP");
			m_paramName.push_back("SubmitMsgType");
			m_paramValues.push_back("8");
			m_paramName.push_back("MsgFormat");
			m_paramValues.push_back("15");
			m_paramName.push_back("SPDealResult");
			m_paramValues.push_back("0");
			m_paramName.push_back("SPDealResult");
			m_paramValues.push_back("0");
			m_paramName.push_back("LinkID");
			
			m_paramValues.push_back(linkId);
/*		
			if(i % 3 == 0)
			{
				m_paramName.push_back("groupSms");
				m_paramValues.push_back("15");
			}
			*/

			(*utm)[_TagParameterName] = m_paramName;
			(*utm)[_TagParameterValue] = m_paramValues;

			ServiceIdentifier si;
			si.m_appid=1;
			si.m_appport=70;
			postMessage(si,*utm);
		
			//sendSubmit(m_myID, utm);
}

void BaseSGIPProtocol::test(int unit)
{
	if(unit == 1)
	{
		std::string deliver = "00000090000000030000024d08616108081620020016000f3230303730383038313632303135303230333737373737373700000000000000000000313036363136373700000000000000000000000000154447207465737431202030323033373737373737370000000000000000000300143038303831363230313530323030303030303034000b00010f000c000100";
		u8array bin;
		char tmp[3];
		const char* hex=deliver.c_str();
		for(size_t i=0; i<deliver.length(); i+=2)
		{
			strncpy(tmp, hex+i, 2);
			bin.push_back((u8)strtol(tmp, NULL, 16));
		}

		cacti::Stream content;
		content.putStream(&bin[0], deliver.length()/2);
			(content);
	}

	if(unit == 2)
	{
		for(int i = 1; i <= 1; i++)
		{
			UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, _EvtReceiveMessage);
			(*utm)[_TagGatewayName]=m_userName;
			(*utm)[_TagServiceId] = m_serviceCode;
			(*utm)[_TagCalling] = m_userName;
			(*utm)[_TagCallee] = m_testNumber;
			(*utm)[_TagOriCalling] = "123";
			(*utm)[_TagOriCallee] = "testsaaa";
		
			char msg[100];
			itoa(i,msg,10);
			(*utm)[_TagMessage] = "test";

			std::vector<std::string> m_paramName;
			std::vector<std::string> m_paramValues;

			m_paramName.push_back("Protocol");
			m_paramValues.push_back("SGIP");
			m_paramName.push_back("MTFlag");
			m_paramValues.push_back("2");
			m_paramName.push_back("MsgFormat");
			m_paramValues.push_back("15");
			m_paramName.push_back("SPDealResult");
			m_paramValues.push_back("0");
			m_paramName.push_back("SPDealResult");
			m_paramValues.push_back("0");
			m_paramName.push_back("LinkID");
			if(i==1)
				m_paramValues.push_back("1234");
			else
				m_paramValues.push_back("4567");
/*		
			if(i % 3 == 0)
			{
				m_paramName.push_back("groupSms");
				m_paramValues.push_back("15");
			}
			*/

			(*utm)[_TagParameterName] = m_paramName;
			(*utm)[_TagParameterValue] = m_paramValues;

			ServiceIdentifier si;
			si.m_appid=1;
			si.m_appport=70;

			//postToOwner(m_myID,utm);
		sendSubmit(si,utm);
			//postMessage(si,*utm);
		}
	}
	if(unit==3 )
	{
		UserTransferMessagePtr utm = UTM::create(m_myID, m_smshttpgw,_ResQuerySendWindowSize );
		handleQuerySendWindowSize(m_smshttpgw,utm);
	}if(unit==4){
		SGIPDeliver sd;
		sd.messageCoding=15;
		string content="aaa";
		sd.messageContent.assign(content.begin(),content.end());
		sd.messageLength=3;
		string spno="10660549";

		sd.spNumber.assign(spno.begin(),spno.end());
		string un="18022300024";
		sd.userNumber.assign(un.begin(),un.end());
		
		handleDeliver(&sd);
	}
}
