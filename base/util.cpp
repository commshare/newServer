#include "util.h"
#include <sstream>
using namespace std;

CSLog g_imlog = CSLog(LOG_MODULE_IM);

CRefObject::CRefObject()
{
	m_lock = NULL;
	m_refCount = 1;
}

CRefObject::~CRefObject()
{

}

void CRefObject::AddRef()
{
	if (m_lock)
	{
		m_lock->lock();
		m_refCount++;
		m_lock->unlock();
	}
	else
	{
		m_refCount++;
	}
}

void CRefObject::ReleaseRef()
{
	if (m_lock)
	{
		m_lock->lock();
		m_refCount--;    
		//m_refCount = (bClose) ? 0 : m_refCount-1;
		if (m_refCount == 0)
		{
			m_lock->unlock();
			delete this;
			return;
		}
		m_lock->unlock();
	}
	else
	{
		m_refCount--;
		if (m_refCount == 0)
			delete this;
	}
}

string BaseVersion(void)
{
	string sVersion = BASE_VERSION ;

	sVersion += BASE_SUBVERSION;
	
	return sVersion;
}

uint64_t get_tick_count()
{
#ifdef _WIN32
	LARGE_INTEGER liCounter; 
	LARGE_INTEGER liCurrent;

	if (!QueryPerformanceFrequency(&liCounter))
		return GetTickCount();

	QueryPerformanceCounter(&liCurrent);
	return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
	struct timeval tval;
	uint64_t ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond)
{
#ifdef _WIN32
	Sleep(millisecond);
#else
	usleep(millisecond * 1000);
#endif
}

CStrExplode::CStrExplode(char* str, char seperator)
{
	m_item_cnt = 1;
	char* pos = str;
	while (*pos) {
		if (*pos == seperator) {
			m_item_cnt++;
		}

		pos++;
	}

	m_item_list = new char* [m_item_cnt];

	int idx = 0;
	char* start = pos = str;
	while (*pos) {
		if ( pos != start && *pos == seperator) {
			uint32_t len = pos - start;
			m_item_list[idx] = new char [len + 1];
			strncpy(m_item_list[idx], start, len);
			m_item_list[idx][len]  = '\0';
			idx++;

			start = pos + 1;
		}

		pos++;
	}

	uint32_t len = pos - start;
    if(len != 0)
    {
        m_item_list[idx] = new char [len + 1];
        strncpy(m_item_list[idx], start, len);
        m_item_list[idx][len]  = '\0';
    }
}

CStrExplode::~CStrExplode()
{
	for (uint32_t i = 0; i < m_item_cnt; i++) {
		delete [] m_item_list[i];
	}

	delete [] m_item_list;
}


SecurityRand* SecurityRand::create() {
	return new IfStreamSecurityRand( "/dev/urandom" );
}

char* replaceStr(char* pSrc, char oldChar, char newChar)
{
    if(NULL == pSrc)
    {
        return NULL;
    }
    
    char *pHead = pSrc;
    while (*pHead != '\0') {
        if(*pHead == oldChar)
        {
            *pHead = newChar;
        }
        ++pHead;
    }
    return pSrc;
}

string int2string(uint32_t user_id)
{
    stringstream ss;
    ss << user_id;
    return ss.str();
}

uint32_t string2int(const string& value)
{
    return (uint32_t)atoi(value.c_str());
}

// 由于被替换的内容可能包含?号，所以需要更新开始搜寻的位置信息来避免替换刚刚插入的?号
void replace_mark(string& str, string& new_value, uint32_t& begin_pos)
{
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) {
        return;
    }
    
    string prime_new_value = "'"+ new_value + "'";
    str.replace(pos, 1, prime_new_value);
    
    begin_pos = pos + prime_new_value.size();
}

void replace_mark(string& str, uint32_t new_value, uint32_t& begin_pos)
{
    stringstream ss;
    ss << new_value;
    
    string str_value = ss.str();
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) {
        return;
    }
    
    str.replace(pos, 1, str_value);
    begin_pos = pos + str_value.size();
}


void writePid()
{
	uint32_t curPid;
#ifdef _WIN32
	curPid = (uint32_t) GetCurrentProcess();
#else
	curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}

bool IsValidUid(const char* sUid,int nMaxLen)
{
	if(!sUid || !strlen(sUid))
		return false;
	
	const int len = strlen(sUid);
	
	if (len < 20 || len > nMaxLen) return false;

	int digitCount = 0;
	for (int i = 0; i < len; ++i)
	{
		const char& currentChar = sUid[i];
		if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z'))
		{
			continue;
		}
		else if (currentChar >= '0' && currentChar <= '9')
		{
			++digitCount;
		}
		else
		{
			return false;
		}
	}
	return digitCount > 0;
}

bool IsPidExist(const char* sPid)
{
	bool ret = false;
	int count = 0;
	char cmd[PIPE_BUF] = {0};
	char buf[PIPE_BUF] = {0}; 

	const char * p = strrchr(sPid, '/');
	if(p == NULL)
	{
		p = sPid;
	}
	else
	{
		++p;
	}
//	p = (p) ? ++p : sPid;
	
	//sprintf(cmd,"ps -ef | grep -w %s | grep -v grep | wc -l",p);
	sprintf(cmd,"ps -A | grep -w %s | grep -v grep | wc -l",p); //torry updated. 2017.11.17

	
	FILE *fp = popen(cmd, "r");	//Open pipe to execute shell command. 
	
	if( (fgets(buf,PIPE_BUF,fp))!= NULL ) 
	{
		count = atoi(buf); 
		if(count> 1) 
			ret = true;
	} 
	
	pclose(fp);
	return ret;

}

int  SetSystemLimit(int nTotalOpenFiles, unsigned int nStackSize)
{
	struct rlimit rlim;    
	if (getrlimit(RLIMIT_CORE, &rlim) != 0)    
	{        
		perror("getrlimit.");    
	}    
	else    
	{        
		int uid = geteuid();        
		seteuid(getuid()); 
		rlim_t v = RLIM_INFINITY;        
		rlim.rlim_cur = v;        
		if (setrlimit(RLIMIT_CORE, &rlim) < 0)        
		{            
			perror("setrlimit.");        
		}        
		else        
		{            
			getrlimit(RLIMIT_CORE, &rlim);        
		}        
		seteuid(uid);    
	}    

	struct rlimit flim;    
	flim.rlim_cur = nTotalOpenFiles;    
	flim.rlim_max = nTotalOpenFiles;    
	if (setrlimit(RLIMIT_NOFILE, &flim) < 0)    
	{        
		perror("setrlimit.");    
	}    
	else    
	{        
		//struct rlimit rfilelim;        
		if (getrlimit(RLIMIT_NOFILE, &flim) < 0)        
		{            
			perror("getrlimit.");        
		}        
		else        
		{
			DbgLog("soft file limit =%d,hard file limit = %d", flim.rlim_cur,flim.rlim_max);
		}    
	}    
	struct rlimit slim;

	if(!getrlimit(RLIMIT_STACK,&slim))
	{
		if(slim.rlim_cur < nStackSize)
		{
			slim.rlim_cur = nStackSize;
            if(setrlimit(RLIMIT_STACK, &slim)<0)
            {
            	perror("setrlimit.");
            }
		}
	}
	
	return 0;

}


inline unsigned char toHex(const unsigned char &x)
{
    return x > 9 ? x -10 + 'A': x + '0';
}

inline unsigned char fromHex(const unsigned char &x)
{
    return isdigit(x) ? x-'0' : x-'A'+10;
}

string URLEncode(const string &sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        unsigned char buf[4];
        memset( buf, 0, 4 );
        if( isalnum( (unsigned char)sIn[ix] ) )
        {
            buf[0] = sIn[ix];
        }
        //else if ( isspace( (unsigned char)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
        //{
        //    buf[0] = '+';
        //}
        else
        {
            buf[0] = '%';
            buf[1] = toHex( (unsigned char)sIn[ix] >> 4 );
            buf[2] = toHex( (unsigned char)sIn[ix] % 16);
        }
        sOut += (char *)buf;
    }
    return sOut;
}

string URLDecode(const string &sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        unsigned char ch = 0;
        if(sIn[ix]=='%')
        {
            ch = (fromHex(sIn[ix+1])<<4);
            ch |= fromHex(sIn[ix+2]);
            ix += 2;
        }
        else if(sIn[ix] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
    return sOut;
}

int is_file_exist(const char *file_path)
{
	if (file_path == nullptr)
		return -1;
	return access(file_path, F_OK);
}


int64_t get_file_size(const char *path)
{
    int64_t filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

const char*  memfind(const char *src_str,size_t src_len, const char *sub_str, size_t sub_len, bool flag)
{
    if(NULL == src_str || NULL == sub_str || src_len <= 0)
    {
        return NULL;
    }
    if(src_len < sub_len)
    {
        return NULL;
    }
    const char *p;
    if (sub_len == 0)
        sub_len = strlen(sub_str);
    if(src_len == sub_len)
    {
        if(0 == (memcmp(src_str, sub_str, src_len)))
        {
            return src_str;
        }
        else
        {
            return NULL;
        }
    }
    if(flag)
    {
        for (unsigned int i = 0; i < src_len - sub_len; i++)
        {
            p = src_str + i;
            if(0 == memcmp(p, sub_str, sub_len))
                return p;
        }
    }
    else
    {
        for ( int i = (src_len - sub_len) ; i >= 0;i--  )
        {
            p = src_str + i;
            if ( 0 == memcmp(  p,sub_str,sub_len ) )
                return p;
            
        }
    }
    return NULL;
}

string getuuid()
{
	uuid_t uid;
	uuid_generate(uid);
	char sz[36];
	uuid_unparse(uid,sz);
	string str;
	str.assign(sz,strlen(sz));
	return str;
}

int compareuuid(uuid_t uid1, uuid_t uid2)
{
	return uuid_compare(uid1,uid2);
}

string uuid2str(uuid_t uid)
{
	string str;
	if(!uuid_is_null(uid))
	{
		char sz[36]={0};
		uuid_unparse(uid, sz);
		str.assign(sz,strlen(sz));
	}
	return str;
}

void str2uuid(string& str,uuid_t uid)
{
	uuid_parse(str.c_str(),uid);
}

void freeuuid(uuid_t uid)
{
	if(!uuid_is_null(uid))
		uuid_clear(uid);
}
//Added by jack ,for generate machine and pid code . 
MachineWithPid GenerateMachinePid(void)
{
	std::shared_ptr<SecurityRand> sr( SecurityRand::create() );
	
	int64_t n = sr->nextInt64();
	MachineWithPid x = reinterpret_cast<MachineWithPid&>(n);

    unsigned pid = static_cast<unsigned>(getpid());
    x._pid ^= (unsigned short) pid;
    // when the pid is greater than 16 bits, let the high bits modulate the machine id field.
    unsigned short& rest = (unsigned short &) x._machineNumber[1];
    rest ^= pid >> 16;	

	return x;
}

void GenerateUId(UidCode_t& sessionId)
{
    static atomic_int inc = {static_cast<unsigned>(
            std::shared_ptr<SecurityRand>(SecurityRand::create())->nextInt64())};
	UidCode_t uid;
	unsigned t;
	unsigned char* T;
	int new_inc;

    t = (unsigned) time(0);
    T = (unsigned char *) &t;
    uid.Uid_Item._time[0] = T[3]; // big endian order 
    uid.Uid_Item._time[1] = T[2];
    uid.Uid_Item._time[2] = T[1];
    uid.Uid_Item._time[3] = T[0];

    uid.Uid_Item._machineWithPid = GenerateMachinePid();


    new_inc = inc++;
    T = (unsigned char *) &new_inc;
    uid.Uid_Item._inc[0] = T[2];
    uid.Uid_Item._inc[1] = T[1];
    uid.Uid_Item._inc[2] = T[0];
	uid.Uid_Item.code[UID_SIZE] = 0;

	sessionId = uid;
 }

void toHexLower(string& uid_str, const void* inRaw, int len) 
{
	static const char hexchars[] = "0123456789abcdef";

	//StringBuilder out;
	char out[30] = {0};
	const char* in = reinterpret_cast<const char*>(inRaw);
	for (int i=0; i<len; ++i) {
		char c = in[i];
		char hi = hexchars[(c & 0xF0) >> 4];
		char lo = hexchars[(c & 0x0F)];

		out[2*i]   = hi;
		out[2*i+1] = lo;
	}
	uid_str = out;
}

// BKDR Hash Function    
unsigned int BKDRHash(const char* str)
{
	unsigned int seed = 31;// 31 131 1313 13131 131313 etc..    
	unsigned int hash = 0;

	if(!str)
		return 0;
	
	while (*str)
	{
		hash = hash*seed + (*str++);
	} 
	return hash;
}


void string_replace(string& strBig, const string &strsrc, const string &strdst)
{
    std::string::size_type pos = 0;
    std::string::size_type srclen = strsrc.size();
    std::string::size_type dstlen = strdst.size();

    while( (pos=strBig.find(strsrc, pos)) != std::string::npos )
    {
        strBig.replace( pos, srclen, strdst );
        pos += dstlen;
    }
} 


string SqlFilter(string source)
{
    string_replace(source, "'", "''");
//	string_replace(source, "/", "//");
//	string_replace(source, "%", "/%");
    string_replace(source, ";", "；");
//    string_replace(source, "(", "（");
//    string_replace(source, ")", "）");
    return source;
}
// End of jack adding. 
