#include "srvsock/packet.h"
#include "srvsock/srvsock.h"
#include "ylx.h"
#include "startup/commander.h"


extern Commander *gp_cmd;
//
//void OnMyRecv(char *buf,WORD len)
//{
//    ParseMsg((BYTE *)buf);
//}
//
//
int ParseMsg(BYTE *pbuf)
{

    S_Msg_Header msg = *(S_Msg_Header *)pbuf;
    switch (htonl(msg.CommandId))
    {
    case E_CMDID_Stat:
        //SendPacket((char*)pbuf, sizeof(S_Msg_Header));
        //RunLog("recv E_CMDID_Stat");
//        m_logger.debug("recv E_CMDID_Stat");
        break;
    case E_CMDID_Submit:
		{
			S_Msg msg = ParseCMDID_Submit(pbuf);

			BYTE respbuf[2048];
			BYTE errdesc[] = "I don't known!";
			BYTE mtsn[] = "12345678";

//			sendYlxSubmit("12121","12121");

			gp_cmd->handleUICommand("1");
			int len = MakeSubmitRespPacket(respbuf, 0, msg.CMDID_Submit.SerialID, errdesc, mtsn);
			//SendPacket((char *)respbuf, len);

			//RunLog("[caller=%s],[callee=%s],[sms=%s]", msg.CMDID_Submit.Originnumber, msg.CMDID_Submit.Callee, msg.CMDID_Submit.MsgContent);
//			m_logger.debug("[caller=%s],[callee=%s],[sms=%s]", msg.CMDID_Submit.Originnumber, msg.CMDID_Submit.Callee, msg.CMDID_Submit.MsgContent);
		}
		break;
    case E_CMDID_SubmitResp://下发短信消息的回复
        //RunLog("recv E_CMDID_SubmitResp");
//        m_logger.debug("recv E_CMDID_SubmitResp");
        break;
    case E_CMDID_Report:  //运营商短消息报告
        //RunLog("recv E_CMDID_Report");
//        m_logger.debug("recv E_CMDID_Report");
        break;
    case E_CMDID_ReportResp://
        //RunLog("recv E_CMDID_ReportResp");
//		m_logger.debug("recv E_CMDID_ReportResp");
	
		break;
    }
    return 0;
}

#define MAX_MSU_LEN  2048
#define MAX_BUFFER_SIZE MAX_MSU_LEN*10

#define MakeTLV(a)  \
    *(WORD *)&buf[pos] = (WORD)htons(strlen((char*)a));\
    pos += 2;\
    memcpy((void*)&buf[pos], a, strlen((char*)a));\
    pos += strlen((char*)a);


int  MakeSubmitRespPacket(BYTE *buf,DWORD result,BYTE *sn,BYTE *errdesc ,BYTE *mtsn)
{
    int pos = 0;
    S_Msg_Header *hdr = (S_Msg_Header *)buf;
    hdr->SequenceId = htonl(0x12345678);
    hdr->CommandId = htonl(E_CMDID_SubmitResp);

    pos += sizeof(S_Msg_Header);
    MakeTLV(mtsn);

    *(DWORD*)&buf[pos] = htonl(result);
    pos += 4;

    MakeTLV(sn);
    MakeTLV(errdesc);
    hdr->TotalLength = htonl(pos);
    return pos;
}


#define ParseTLV(len,storebuf) \
    len = htons(*(WORD*)&pbuf[pos]);\
    memcpy((void *)&storebuf, &pbuf[pos + 2], len);\
    pos += 2 + len;


S_Msg ParseCMDID_Submit(BYTE *pbuf)
{
    S_Msg msg;
    int pos = 0;
    memset((void*)&msg, 0, sizeof(msg));

    msg.TotalLength = htonl(*(DWORD*)pbuf);
    msg.CommandId = htonl(*(DWORD*)&pbuf[4]);
    msg.SequenceId = htonl(*(DWORD*)&pbuf[8]);

    pos = sizeof(S_Msg_Header);

    ParseTLV(msg.CMDID_Submit.SerialID_length, msg.CMDID_Submit.SerialID);

    msg.CMDID_Submit.type = htons(*(WORD*)&pbuf[pos]);
    pos += 2;
    ParseTLV(msg.CMDID_Submit.OriginnumberLength, msg.CMDID_Submit.Originnumber);


    ParseTLV(msg.CMDID_Submit.CalleeLength, msg.CMDID_Submit.Callee);

    msg.CMDID_Submit.Msgcoding = (BYTE)pbuf[pos++];

    ParseTLV(msg.CMDID_Submit.MsgContentLength, msg.CMDID_Submit.MsgContent);
    return msg;
}
