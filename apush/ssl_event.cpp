/**
 *  
 * ssl_event.cpp
 */


#include "ssl_event.h"
#include "ssl_socket.h"

#define MIN_TIMER_DURATION	100	// 100 miliseconds
#define MAX_EVENT_NUMS 1024*10*10*5

CSslEventDispatch* CSslEventDispatch::m_pSslEventDispatch = NULL;

CSslEventDispatch::CSslEventDispatch()
{
    running = false;
	m_epfd = epoll_create( MAX_EVENT_NUMS);
	if (m_epfd == -1)
	{
		printf("epoll_create failed\n");
	}

	m_event_state = EVENT_NONE;
}

CSslEventDispatch::~CSslEventDispatch()
{
	m_event_state = EVENT_NONE;
}


static CSslEventDispatch *g_pSslEventDispatch;

CSslEventDispatch* CSslEventDispatch::Instance()
{
	if (g_pSslEventDispatch == NULL)
	{
		g_pSslEventDispatch = new CSslEventDispatch();
	}

	return g_pSslEventDispatch;
}

void CSslEventDispatch::AddEvent(CSslSocket *sslSocket, uint64_t interval)
{
	NOTUSED_ARG(interval);
	CAutoLock lock(&m_lockAddEvent);

	if (sslSocket->GetSocket() > 0)
	{
		_AddEvent(sslSocket->GetSocket());
	}
}

void CSslEventDispatch::_AddEvent(SOCKET fd)
{

	//printf("ep add event\n");
	struct epoll_event ev;
	//ev.events =  EPOLLIN | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

	ev.events =  EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	//ev.events = EPOLLOUT | EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	//ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0)
	{
		printf("epoll_ctl() failed, errno=%d", errno);
	}

	m_event_state = EVENT_READABLE | EVENT_WRITEABLE;
}


void CSslEventDispatch::RemoveEvent(CSslSocket *sslSocket, uint8_t socket_event)
{
	CAutoLock lock(&m_lockRemoveEvent);
	if (sslSocket->GetSocket() > 0)
	{
		_RemoveEvent(sslSocket->GetSocket(), socket_event);
	}
}

void CSslEventDispatch::_RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	NOTUSED_ARG(socket_event);
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) != 0)
	{
		printf("epoll_ctl failed, errno=%d", errno);
	}
}


int CSslEventDispatch::AddEpollOut(CSslSocket *sslSocket)
{

	printf("add AddEpollOut\n");
	if (!sslSocket)
	{
		ErrLog("sslSocket is null");
		return -1;
	}

	SOCKET fd = sslSocket->GetSocket();

	struct epoll_event event;
	if(m_epfd <= 0 || fd <= 0)
	{
		ErrLog("m_epfd or fd <= 0");
		return -1;
	}

	event.data.fd = fd;
	event.events = EPOLLOUT | EPOLLIN | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

	if(0 != epoll_ctl(m_epfd,EPOLL_CTL_MOD,fd,&event))
	{
		int ierrno = errno;
		ErrLog("socket:%d, epoll_ctl", fd);
		return ierrno;
	}

	return 0;
}

int CSslEventDispatch::DelEpollOut(CSslSocket *sslSocket)
{
	printf("del DelEpollOut\n");
	if (!sslSocket)
	{
		ErrLog("sslSocket is null");
		return -1;
	}

	SOCKET fd = sslSocket->GetSocket();

	struct epoll_event event;
	if(m_epfd <= 0 || fd <= 0)
	{
		ErrLog("m_EpFd or fd <= 0");
		return -1;
	}

	event.events =  EPOLLIN | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	if(0 != epoll_ctl(m_epfd,EPOLL_CTL_MOD,fd,&event))
	{
		int ierrno = errno;
		ErrLog("socket:%d, epoll_ctl", fd);
		return ierrno;
	}

	return 0;
}

void CSslEventDispatch::StartDispatch(uint32_t wait_timeout)
{
	struct epoll_event *events = NULL;
	int nfds = 0;

    events = new struct epoll_event[MAX_EVENT_NUMS];
    if (!events) 
        return;
    if(running)
        return;
    running = true;
    
	while (running)
	{
		static uint64_t curr_tick = get_tick_count();
		nfds = epoll_wait(m_epfd, events, MAX_EVENT_NUMS, wait_timeout);
		for (int i = 0; i < nfds; i++)
		{
			int ev_fd = events[i].data.fd;
			CSslSocket* pSocket = FindSocket(ev_fd);

			if (!pSocket)
				continue;

			//_UpdateHeartBeat(pSocket);
            
            //Commit by zhfu @2015-02-28
            #ifdef EPOLLRDHUP
            if (events[i].events & EPOLLRDHUP)
            {
                printf("On Peer Close, socket=%d\n", ev_fd);
                pSocket->OnClose();
            }
            #endif
            // Commit End

			if (events[i].events & EPOLLIN)
			{
				printf("OnRead, socket=%d\n", ev_fd);
				pSocket->OnRead();
			}

			if (events[i].events & EPOLLOUT)
			{
				printf("OnWrite, socket=%d\n", ev_fd);
				pSocket->OnWrite();
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				printf("OnClose, socket=%d\n", ev_fd);
				pSocket->OnClose();
			}

			pSocket->ReleaseRef();
		}


        if (get_tick_count() > (curr_tick + 1000))
        {                                        
            //_CheckHeartBeat();                 
        }                                        
        //_CheckLoop();                          

	}
    delete events;
}

void CSslEventDispatch::StopDispatch()
{
    running = false;
}

