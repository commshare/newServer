/*****************************************************************************************
Filename:  impdubase.h
Author: jack			Version: im-1.0 		Date:2017/6/26
Description:   网络层PDU数据类，负责基于protobuf协议包头、包体的数据封装、数据包的读写访问等。
*****************************************************************************************/

#ifndef __IMPDUBASE_H__
#define __IMPDUBASE_H__
#include <memory>
#include "UtilPdu.h"
#include "google/protobuf/message_lite.h"
#include "im.pub.pb.h"


#define IM_PDU_HEADER_LEN		20
#define IM_PDU_VERSION			1
#define OFFSET_PDU_HDR_PID           0
#define OFFSET_PDU_HDR_VERSION       2
#define OFFSET_PDU_HDR_SESSIONID     3
#define OFFSET_PDU_HDR_ENCRYPT       15
#define OFFSET_PDU_HDR_CMDID         16
#define OFFSET_PDU_HDR_BODYSIZE   	 18
#define SIZE_PDU_HDR_SESSIONID		 12


using namespace std;
using namespace im;


#define ALLOC_FAIL_ASSERT(p) if (p == NULL) { \
throw CPduException(m_pdu_header.service_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed"); \
}

#define CHECK_PB_PARSE_MSG(ret) { \
    if (ret == false) \
    {\
        log("parse pb msg failed.");\
        return;\
    }\
}

typedef struct
{
	uint16_t nPID;	 	// pdu identification , default as 'US'
	uint8_t  nVersion;  // pdu version
	string 	 sSessionId;// fixed length 12 byte size;
	uint8_t  bEncrypt;	// one byte for identifing whether or not encrypt content.
	uint16_t nCmdId;	// command id of protobuf protocol. 	
	uint16_t nBodySize;  // size of body
} PduHeader_t;

class  CImPdu
{
public:
    CImPdu();
    virtual ~CImPdu() {}
    
    uchar_t* GetBuffer();			//Get the start address of current pdu.
    uint16_t GetLength();			//Get total of pdu length.
    uchar_t* GetBodyData();			//Get body.
    uint16_t GetBodyLength();   	//Get body length.
    
    
    uint8_t GetVersion() { return m_pdu_header.nVersion; }
	uint8_t GetEncrypt() { return m_pdu_header.bEncrypt;}
    uint16_t GetPID() { return m_pdu_header.nPID; }		//PID as protocol header flag. default is 'US'		
    uint16_t GetCommandId() { return m_pdu_header.nCmdId; }
    string  GetSessionId() { return m_pdu_header.sSessionId; }
    net_handle_t GetSockHandle() { return m_sock_handle; } 
    void SetVersion(uint8_t version);
    void SetPID(uint16_t nPID);
	void SetEncrypt(uint8_t bEncrypt);
    void SetCommandId(uint16_t command_id);  
    void SetSessionId(string sSessionId);
    void SetSockHandle(net_handle_t sock_handle) {  m_sock_handle = sock_handle; } // identify the source socket of data. 
    void WriteHeader();
    
    static bool IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len); //Check the pdu is valid or not . 
    static std::shared_ptr<CImPdu> ReadPdu(uchar_t* buf, uint32_t len);
    void Write(uchar_t* buf, uint32_t len) { m_buf.Write((void*)buf, len); } 
    int ReadPduHeader(uchar_t* buf, uint32_t len);
    void SetPBMsg(const google::protobuf::MessageLite* msg);		//Allocate buffer to fill with pb header and body.
	void SetPBWithoutMsg(uint16_t nCmdId,string sSessionId);		//Allocate buffer to fill pb with no body.
    
protected:
    CSimpleBuffer	m_buf;
    PduHeader_t		m_pdu_header;
    net_handle_t    m_sock_handle;
   
};


#endif /* IMPDUBASE_H_ */
