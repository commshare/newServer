#ifndef __UTIL_H__
#define __UTIL_H__

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996, 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/resource.h> 
#include <fcntl.h>
#include <limits.h>

#include "ostype.h"
#include "utilpdu.h"
#include "lock.h"
#include "im_time.h"

#include "slog/slog_api.h"
#ifndef _WIN32
#include <strings.h>
#endif

#include <sys/stat.h>
#include <assert.h>


#ifdef _WIN32
#define	snprintf	sprintf_s
#else
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#endif

#include<uuid/uuid.h>
#include <memory>



using namespace std;


#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter
#define BASE_VERSION	"2.0-"
#define BASE_SUBVERSION	"20171117001"
//defined by jack for generating unique id code, 2017/6/7
#define UID_SIZE 12				// Generate unique code for server communication.  added by jack 
struct MachineWithPid 			// Server machine and pid code. 
{
	unsigned char _machineNumber[3];
	unsigned short _pid;
	bool operator!=(const MachineWithPid& rhs) const;
};

struct UidCode_t
{
	union UID_CODE									// 12 bytes total size , unique code ,add by jack . 
	{
		struct 
		{
    		// 12 bytes total
       	 	unsigned char _time[4];
        	MachineWithPid _machineWithPid;
        	unsigned char _inc[3];
    	};
    	uchar_t code[UID_SIZE];
		unsigned int nReserverd[3];
	} Uid_Item;
	
	bool operator < (const UidCode_t &uid) const
    {
    	return memcmp(Uid_Item.code,uid.Uid_Item.code,UID_SIZE) < 0;
    	
    }

	bool operator == (const UidCode_t &uid) const
    {
    	return memcmp(Uid_Item.code,uid.Uid_Item.code,UID_SIZE) == 0;
    	
    }

    string toString() const
    {
       char *buf = new char[UID_SIZE*2 +1]{0};
       char *tmp = buf;
       for(int i =0 ; i < UID_SIZE; i++)
       {
           sprintf(buf, "%02X", Uid_Item.code[i]);
           buf += strlen(buf);
       }

       string str(tmp);
       delete[] tmp;
       return str;
    }
};
class SecurityRand 
{
   public:
	   virtual ~SecurityRand(){}

	   virtual int64_t nextInt64() = 0;

	   static SecurityRand* create();
};

class IfStreamSecurityRand : public SecurityRand 
{
public:
	IfStreamSecurityRand( const char* fn ) 
	{
		_in = new std::ifstream( fn, std::ios::binary | std::ios::in );
		if ( !_in->is_open() ) 
		{
			std::cerr << "can't open " << fn << " " << strerror(errno) << std::endl;
			abort();
		}
	}

	~IfStreamSecurityRand() 
	{
		delete _in;
	}

	int64_t nextInt64() 
	{
		int64_t r;
		_in->read( reinterpret_cast<char*>( &r ), sizeof(r) );
		if ( _in->fail() ) 
		{
			abort();
		}
		return r;
	}

private:
	std::ifstream* _in;
};


MachineWithPid GenerateMachinePid(void);						//Generate machine and pid code. 
void GenerateUId(UidCode_t& sessionId);
void toHexLower(string& uid_str, const void* inRaw, int len);

// End of jack definition. 

/// yunfan modify end 

class CRefObject
{
public:
	CRefObject();
	virtual ~CRefObject();

	void SetLock(CLock* lock) { m_lock = lock; }
	void AddRef();
	void ReleaseRef();
	int getcount() {return m_refCount;}
private:
	int				m_refCount;
	CLock*	m_lock;
};

#define LOG_MODULE_IM         "IM"


extern CSLog g_imlog;

// Add By ZhangYuanhao 2015-01-14 For log show the file name not the full path + filename
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#if defined(_WIN32) || defined(_WIN64)
#define log(fmt, ...)  g_imlog.Info("<%s>\t<%d>\t<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define log(fmt, args...)  g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#endif
#define DbgLog(fmt, args...)  g_imlog.Debug("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define InfoLog(fmt, args...)  g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define ErrLog(fmt, args...)  g_imlog.Error("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define WarnLog(fmt, args...)  g_imlog.Warn("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)

//#define log(fmt, ...)  g_imlog.Info("<%s>\t<%d>\t<%s>,"+fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);
int is_file_exist(const char *file_path); 
string BaseVersion(void);


class CStrExplode
{
public:
	CStrExplode(char* str, char seperator);
	virtual ~CStrExplode();

	uint32_t GetItemCnt() { return m_item_cnt; }
	char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
	uint32_t	m_item_cnt;
	char** 		m_item_list;
};

char* replaceStr(char* pSrc, char oldChar, char newChar);
string int2string(uint32_t user_id);
uint32_t string2int(const string& value);
void replace_mark(string& str, string& new_value, uint32_t& begin_pos);
void replace_mark(string& str, uint32_t new_value, uint32_t& begin_pos);

void writePid();
bool IsPidExist(const char* sPid);
bool IsValidUid(const char* sSessionId,int nMaxLen);
int  SetSystemLimit(int nTotalOpenFiles, unsigned int nStackSize);
inline unsigned char toHex(const unsigned char &x);
inline unsigned char fromHex(const unsigned char &x);
string URLEncode(const string &sIn);
string URLDecode(const string &sIn);


int64_t get_file_size(const char *path);
const char*  memfind(const char *src_str,size_t src_len, const char *sub_str, size_t sub_len, bool flag = true);

string getuuid();
string uuid2str(uuid_t uid);
void str2uuid(string& str,uuid_t uid );
void freeuuid(uuid_t uid);
int compareuuid(uuid_t uid1, uuid_t uid2);
unsigned int BKDRHash(const char* str);

void string_replace(string& strBig, const string &strsrc, const string &strdst);
// sql语句中字符串过滤
string SqlFilter(string source);


#endif
