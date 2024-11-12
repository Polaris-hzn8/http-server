/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Mon 28 Oct 2024 08:35:58 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include "../reactor/eventloop.h"
#include "../thread/thread_pool.h"

typedef struct listener_st {
	int lfd_;
	unsigned short port_;
}LISTENER, *PLISTENER;

typedef struct tcp_server_st {
	int thread_num_;
	PTHREAD_POOL thread_pool_;
	PEVENTLOOP event_loop_;
	PLISTENER listener_;
}TCP_SERVER, *PTCP_SERVER;

PLISTENER listener_init(unsigned short port);

PTCP_SERVER tcp_server_init(unsigned short port, int thread_num);
int tcp_server_run(PTCP_SERVER tcp_server);

// tcp_server
int accept_connection(void *arg);
