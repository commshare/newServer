#include<uv.h>
#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
//#define DEFAULT_PORT 2800
#define DEFAULT_PORT 80
#define BUF_LEN 1024
using namespace std;
//void (*uv_connect_cb)(uv_connect_t* req, int status)
//
//            uv_connect_t
//
typedef struct {
	uv_write_t req;
	uv_buf_t buf;

} write_req_t;

void write_callback(uv_write_t* req, int status){   
	std::cout << "==============================" << status <<"===========================" << std::endl;		
	if(status >= 0) {                                                                      
		std::cout << "write successfully" <<endl;                                          
	}else {                                                                                

		std::cout << "write faile"<< endl;                                                 
	}                                                                                      
	if(req) {                                                                              
		free(((write_req_t*)req)->buf.base);                                               
		free(req);                                                                         
		//req = nullptr;                                                                   
		req = NULL;                                                                
	}
};

int writeData(uv_connect_t * conn = NULL) {
	if (conn == NULL ) 
	{
		return 0;
	}

	uv_stream_t* dest = (uv_stream_t*)conn->handle;
//	if(uv_is_active((const uv_handle_t*)dest)) {
//			
//		std::cout << "======================" << std::endl;
//	} else {
//		std::cout << "connect lost..." << std::endl;
//		return 0;
//	}

	char buf[] = {"hello server, I'm client"};
//	char buf[BUF_LEN] = {0};
//	memset((char*)&buf, 'Y', BUF_LEN);
	write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
//	uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
	req->buf = uv_buf_init((char*) malloc(sizeof(buf)), sizeof(buf));
	memcpy(req->buf.base, buf, sizeof(buf));
	std::cout << "ready data = " << std::string(req->buf.base, req->buf.len).c_str() << endl;

//	auto CallBack =  [](uv_write_t* req, int status){   
//		std::cout << "==============================" << status <<"===========================" << std::endl;		
//		if(status >= 0) {                                                                      
//			std::cout << "write successfully" <<endl;                                          
//		}else {                                                                                
//
//			std::cout << "write faile"<< endl;                                                 
//		}                                                                                      
//		if(req) {                                                                              
//			free(((write_req_t*)req)->buf.base);                                               
//			free(req);                                                                         
//			//req = nullptr;                                                                   
//			req = NULL;                                                                
//		}
//	};
//	uv_write((uv_write_t*) req, (uv_stream_t*)dest, &req->buf, 1, (uv_write_cb)CallBack);
	uv_write((uv_write_t*) req, (uv_stream_t*)dest, &req->buf, 1, (uv_write_cb)write_callback);
//	uv_write((uv_write_t*) req, (uv_stream_t*)dest, &req->buf, 1, [](uv_write_t* req, int status){
//			if(status >= 0) {
//				std::cout << "write successfully" <<endl;
//			}else { 
//
//				std::cout << "write faile"<< endl;
//			}
//			if(req) {
//				free(((write_req_t*)req)->buf.base);
//				free(req);
//				//req = nullptr;
//				req = NULL;
//			}
//	});




	return 1;

}

void on_connect(uv_connect_t* conn, int status) {
	if(status == 0) {
		cout << "connect successfull" <<endl;
		while (1) {
			//here send data loop
			int ret = writeData(conn);
			if(ret == 0) 
			{
				break;
			}
			usleep(1000*1000);
		}
		

	} else {
		cout << "connect error: " << uv_strerror(status)<< endl;
	}
}

int main(int argc, char* argv[]) {

	uv_loop_t *loop = uv_default_loop();
	uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

	uv_tcp_init(loop, socket);

	uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

	struct sockaddr_in dest;
	//uv_ip4_addr("127.0.0.1", DEFAULT_PORT, &dest);
	uv_ip4_addr("192.168.1.55", DEFAULT_PORT, &dest);
	//uv_ip4_addr("mytest.com", DEFAULT_PORT, &dest);

	uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);
	return 	uv_run(loop, UV_RUN_DEFAULT);	
//	while(true) {
//		sleep(1);
//	}
}
