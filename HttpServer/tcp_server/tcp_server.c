/*
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

#include "tcp_server.h"
#include <arpa/inet.h>

PLISTENER listener_init(unsigned short port)
{
	PLISTENER listener = (PLISTENER)malloc(sizeof(LISTENER));

	// socket
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket failed");
		return NULL;
	}

	// 端口复用
	int reuse = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret == -1) {
		perror("setsockopt failed");
		return NULL;
	}

	// 绑定
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;//监听所有地址
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("bind failed");
		return NULL;
	}

	// 监听
	ret = listen(lfd, 128);
	if (ret == -1) {
		perror("listen failed");
		return NULL;
	}

	listener->lfd_ = lfd;
	listener->port_ = port;

	return listener;
}

PTCP_SERVER tcp_server_init(unsigned short port, int thread_num)
{
	PTCP_SERVER tcp_server = (PTCP_SERVER)malloc(sizeof(TCP_SERVER));

	tcp_server->listener_ = listener_init(port);
	tcp_server->event_loop_ = event_loop_init();
	tcp_server->thread_num_ = thread_num;
	tcp_server->thread_pool_ = thread_pool_init(tcp_server->event_loop_, thread_num);

	return tcp_server;
}

int tcp_server_run(PTCP_SERVER tcp_server)
{
	// 启动线程池
	thread_pool_run(tcp_server->thread_pool_);

	// 添加检测的任务
	PCHANNEL channel = channel_init(tcp_server->listener_->lfd_, CE_READ_EVENT, accept_connection, NULL, tcp_server);
	event_loop_task_add(tcp_server->event_loop_, channel, CN_ADD);

	// 启动反应堆模型
	event_loop_run(tcp_server->event_loop_);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
int accept_connection(void* arg)
{
	PTCP_SERVER tcp_server = (PTCP_SERVER)arg;

	// 建立连接
	int cfd = accept(tcp_server->listener_->lfd_, NULL, NULL);

	// 读写事件交由子线程处理
	PEVENTLOOP event_loop = get_event_loop_from_workthread(tcp_server->thread_pool_);

	//TODO

	return 0;
}


