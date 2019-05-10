#include "wsBase.h"
#include "userMgr.h"

bool CWsBase::init(uint32_t listen_port,uint32_t work_size)
{	
	//init WS server
	try {
        // Set logging settings
        m_wsSocket.set_access_channels(websocketpp::log::alevel::all);
        m_wsSocket.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        m_wsSocket.init_asio();

        // Register our message handler
        m_wsSocket.set_message_handler(bind(&CWsBase::on_message,this,&m_wsSocket,::_1,::_2));

		m_wsSocket.set_close_handler(bind(&CWsBase::on_close,this,::_1));
		m_wsSocket.set_open_handler(bind(&CWsBase::on_open,this,::_1));
		
        // Listen on port 9002
        m_wsSocket.listen(listen_port);

        // Start the server accept loop
        m_wsSocket.start_accept();

    } 
	catch (websocketpp::exception const & e) 
	{
        ErrLog("WS init wsServer failed,errmsg[%s]",e.what());
		return false;
    } 
	catch (...) 
	{
		ErrLog("WS init wsServer failed,catch unknown exception");
		return false;
	}

	//init process threadpool
	if(0 != m_taskThreadPool.Init(work_size))
	{	
		ErrLog("WS init process threadpool failed");
		return false;
	}
	
	return true;

}

bool CWsBase::send(websocketpp::connection_hdl hdl,const string& msg)
{
	try {
		DbgLog("send msg[%s]",msg.c_str());
        m_wsSocket.send(hdl, msg, websocketpp::frame::opcode::text);
    }
	catch (websocketpp::exception const & e) 
	{
    	ErrLog("send %s faild exception[%s]",msg.c_str(),e.what());
        return false;
    }

	return true;
}

bool CWsBase::run()
{
	 // Start the ASIO io_service run loop
	 try {
     		m_wsSocket.run();
	 }
	 catch (websocketpp::exception const & e) 
	{
        ErrLog("wsServer run exception[%s]",e.what());
		return false;
    } 
	catch (...) 
	{
		return false;
	}
	
	 return true;
}

void CWsBase::on_open(websocketpp::connection_hdl hdl)
{
	DbgLog("hdl:%p,connet", hdl.lock().get());
}

void CWsBase::on_close(websocketpp::connection_hdl hdl)
{
	DbgLog("hdl:%p,close by peer", hdl.lock().get());
}

void CWsBase::close(websocketpp::connection_hdl hdl,const string& reason)
{
	DbgLog("close hdl:%p",hdl.lock().get());

	m_wsSocket.close(hdl, websocketpp::close::status::normal,reason);
}



