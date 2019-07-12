#include<uv.h>
//#include<memory.h>
#include<stdlib.h>
#include<iostream>
#define DEFAULT_PORT 2800
#define DEFAULT_BACKLOG 128
uv_loop_t *loop = NULL;

using namespace std;
//void (*uv_connection_cb)(uv_stream_t* server, int status)
//void (*uv_alloc_cb)(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
//void (*uv_read_cb)(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void echo_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
	 std::cout << "recv message = " << string(buf->base, nread) << std::endl;
}


void on_new_connection(uv_stream_t* server, int status) {
	if (status < 0) {
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		// error!
		return;
	}

	uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, client);
	if (uv_accept(server, (uv_stream_t*) client) == 0) {
		uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
	}

}


int main() {
	loop = uv_default_loop();

	uv_tcp_t server;
	uv_tcp_init(loop, &server);
	
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
	cout << "start server listen on 0.0.0.0:"<< DEFAULT_PORT << endl;

	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	return uv_run(loop, UV_RUN_DEFAULT);
}
