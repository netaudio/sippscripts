#include "stdafx.h"

#include "cacti/util/StringUtil.h"
#include "cacti/util/IniFile.h"
#include "smgpprotocol.h"
#include "smgpclient.h"

#include "evtmsg.h"
#include "tag.h"
#include "reqmsg.h"
#include "resmsg.h"

#define new DEBUG_NEW


void SMGPProtocol::test(int unit)
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
		handleClientMsg(content);
	}

	if(unit == 2)
	{
		for(int i = 1; i <= 1000; i++)
		{
			UserTransferMessagePtr utm = UTM::create(m_myID, m_myID, 0);
			(*utm)[_TagServiceCode] = "1181177";
			(*utm)[_TagCalling] = "118100";
			(*utm)[_TagCallee] = "18903009909";
			char msg[100];
			itoa(i,msg,10);
			(*utm)[_TagMessage] = msg;

			std::vector<std::string> m_paramName;
			std::vector<std::string> m_paramValues;

			m_paramName.push_back("SyncOrder");
			m_paramValues.push_back("0");
			m_paramName.push_back("SubmitMsgType");
			m_paramValues.push_back("8");
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

			sendSubmit(m_myID, utm);
		}
	}
	if(unit==3 )
	{
		UserTransferMessagePtr utm = UTM::create(m_myID, m_smshttpgw,_ResQuerySendWindowSize );
		handleQuerySendWindowSize(m_smshttpgw,utm);
	}
}
