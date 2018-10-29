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
	:m_waitTime(100), running(false)
{
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

/*
static CSslEventDispatch *g_pSslEventDispatch;

CSslEventDispatch* CSslEventDispatch::Instance()
{
	if (g_pSslEventDispatch == NULL)
	{
		g_pSslEventDispatch = new CSslEventDispatch();
	}

	return g_pSslEventDispatch;
}
*/

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

	InfoLog("ep add event socket:%d\n", fd);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

	//ev.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0)
	{
		ErrLog("epoll_ctl() failed, errno=%d", errno);
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

void CSslEventDispatch::StartDispatch(uint32_t wait_timeout)
{
	m_waitTime = wait_timeout;
	if (running)
	{
		InfoLog("sslEventDispatch %p already running, set waitTime to %d", this, m_waitTime);
		return;
	}
	StartThread();
}

void CSslEventDispatch::StopDispatch()
{
    running = false;
}

void CSslEventDispatch::OnThreadRun(void)
{
	struct epoll_event *events = NULL;
	int nfds = 0;

	events = new struct epoll_event[MAX_EVENT_NUMS];
	if (!events)
		return;
	running = true;
	InfoLog("sslEventDispatch %p start success", this);
	while (running)
	{
		//static uint64_t curr_tick = get_tick_count();
		nfds = epoll_wait(m_epfd, events, MAX_EVENT_NUMS, m_waitTime);
		for (int i = 0; i < nfds; i++)
		{
			int ev_fd = events[i].data.fd;
			CSslSocket* pSocket = FindSocket(ev_fd);

			if (!pSocket)
				continue;

#ifdef EPOLLRDHUP
			if (events[i].events & EPOLLRDHUP)
			{
				InfoLog("On Peer Close, socket=%d\n", ev_fd);
				pSocket->OnClose();
			}
#endif

			if (events[i].events & EPOLLIN)
			{
				//InfoLog("OnRead, socket=%d\n", ev_fd);
				pSocket->OnRead();
			}

			if (events[i].events & EPOLLOUT)
			{
				//InfoLog("OnWrite, socket=%d\n", ev_fd);
				pSocket->OnWrite();
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				InfoLog("OnClose, socket=%d\n", ev_fd);
				pSocket->OnClose();
			}

		}

	}
	delete events;
}

