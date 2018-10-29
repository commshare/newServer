/**
 *  
 * ssl_event.h 
 */


#ifndef __SSL_EVENT_H__
#define __SSL_EVENT_H__


#include "ostype.h"
#include "util.h"

#include "lock.h"

class CSslSocket;

enum {
	SOCKET_READ		= 0x1,
	SOCKET_WRITE	= 0x2,
	SOCKET_EXCEP	= 0x4,
	SOCKET_ALL		= 0x7
};


enum EVENT_STATE{
	EVENT_NONE		= 0x0,
	EVENT_WRITEABLE = 0x1,
	EVENT_READABLE	= 0x2,
};

enum
{
	SOCKET_STATE_IDLE = 0x0,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_SSL_ERR,
	SOCKET_STATE_SSL_SUCCESS,
	SOCKET_STATE_CLOSING,
};

class CSslEventDispatch
{
public:
	CSslEventDispatch();

	virtual ~CSslEventDispatch();

	void AddEvent(CSslSocket *sslSocket, uint64_t interval);
	void RemoveEvent(CSslSocket *sslSocket, uint8_t socket_event);

	void StartDispatch(uint32_t wait_timeout = 100);
    void StopDispatch();
    
    bool isRunning() {return running;}

	static CSslEventDispatch* Instance();

	int GetEventState(){return m_event_state;}

	int AddEpollOut(CSslSocket *sslSocket);
	int DelEpollOut(CSslSocket *sslSocket);

private:

	void _AddEvent(SOCKET fd);
	void _RemoveEvent(SOCKET fd, uint8_t socket_event);

private:

	CLock				m_lockAddEvent;
	CLock				m_lockRemoveEvent;

	int		m_epfd;
	static CSslEventDispatch* m_pSslEventDispatch;
    bool running;
	int 			m_event_state;
};



#endif
