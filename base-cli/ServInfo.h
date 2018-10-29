/*
 * ServInfo.h
 *
 *  Created on: 2013-7-19
 *      Author: ziteng@mogujie.com
 */

#ifndef SERVINFO_H_
#define SERVINFO_H_

#include "util.h"
#include "imconn.h"
#include "ConfigFileReader.h"
#include <vector>

#define MAX_RECONNECT_CNT	64
#define MIN_RECONNECT_CNT	4

typedef struct {
	string		server_ip;
	uint16_t	server_port;
	uint32_t	idle_cnt;
	uint32_t	reconnect_cnt;
	CImConn*	serv_conn;
} serv_info_t;

template <class T>
void serv_init(serv_info_t* server_list, uint32_t server_count)
{
	for (uint32_t i = 0; i < server_count; i++) {
		T* pConn = new T();
		pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
		server_list[i].serv_conn = pConn;
		server_list[i].idle_cnt = 0;
		server_list[i].reconnect_cnt = MIN_RECONNECT_CNT / 2;
	}
}

template <class T>
void serv_check_reconnect(serv_info_t* server_list, uint32_t server_count)
{
	T* pConn;
	for (uint32_t i = 0; i < server_count; i++) {
		pConn = (T*)server_list[i].serv_conn;
		if (!pConn) {
			server_list[i].idle_cnt++;
			if (server_list[i].idle_cnt >= server_list[i].reconnect_cnt) {
				pConn = new T();
				pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
				server_list[i].serv_conn = pConn;
			}
		}
	}
}

template <class T>
void serv_reset(serv_info_t* server_list, uint32_t server_count, uint32_t serv_idx)
{
	if (serv_idx >= server_count) {
		return;
	}

	server_list[serv_idx].serv_conn = NULL;
	server_list[serv_idx].idle_cnt = 0;
	server_list[serv_idx].reconnect_cnt *= 2;
	if (server_list[serv_idx].reconnect_cnt > MAX_RECONNECT_CNT) {
		server_list[serv_idx].reconnect_cnt = MIN_RECONNECT_CNT;
	}
}

#if 0
template <class T>
class serv_container{
public:
	serv_container() {}
	~serv_container(){}
	void init();
	void push(const serv_info_t& serv);
	void pop();
	void popall();
	void reset();
	void check_reconnect();
private:
	std::vector<serv_info_t> m_serv_list;
};

template <class T>
void serv_container<T>::push(const serv_info_t& serv) {
    m_serv_list.push_back(serv);
    check_reconnect();
}

template <class T>
void serv_container<T>::pop() {
    if (m_serv_list.size() > 0) {
	 auto itr = m_serv_list.begin()+m_serv_list.size()-1;
	 itr->serv_conn.Close();
        m_serv_list.erase(itr);
    	}
}

template <class T>
void serv_container<T>::popall() {
    for(auto &i : m_serv_list) {
        i.serv_conn.Close();
    }
    m_serv_list.clear();
}

template <class T>
void serv_container<T>::reset() {
    for(auto &i : m_serv_list) {
	i.serv_conn = NULL;
	i.idle_cnt = 0;
	i.reconnect_cnt *= 2;
	if (i.reconnect_cnt > MAX_RECONNECT_CNT) {
		i.reconnect_cnt = MIN_RECONNECT_CNT;
	}        
    }
}

template <class T>
void serv_container<T>::check_reconnect(void)
{
	T* pConn;
	size_t index = 0;
	for (auto &i : m_serv_list) {
		pConn = (T*)i.serv_conn;
		index++;
		if (!pConn) {
			i.idle_cnt++;
			if (i.idle_cnt >= i.reconnect_cnt) {
				pConn = new T();
				pConn->Connect(i.server_ip.c_str(), i.server_port, index);
				i.serv_conn = pConn;
			}
		}
	}
}

template <class T>
void serv_container<T>::init(void)
{
	for (auto &i : m_serv_list) {
		T* pConn = new T();
		pConn->Connect(i.server_ip.c_str(), i.server_port, i);
		i.serv_conn = pConn;
		i.idle_cnt = 0;
		i.reconnect_cnt = MIN_RECONNECT_CNT / 2;
	}
}
#endif

serv_info_t* read_server_config(CConfigFileReader* config_file, const char* server_ip_format,
		const char* server_port_format, uint32_t& server_count);


#endif /* SERVINFO_H_ */
