/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Tue 08 Oct 2024 00:28:16 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include "server.h"

#define LOGOUT(fmt, ...) do { printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)

#define RES_PAGE_NOT_FOUNUD "404.html"

typedef struct thread_params_st
{
	int fd;
	int epfd;
	pthread_t tid;
}THREAD_PARAMS, *PTHREAD_PARAMS;

int init_listen_fd(unsigned short port)
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket failed");
		return -1;
	}

	int reuse = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret == -1) {
		perror("setsockopt failed");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;//监听所有地址
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("bind failed");
		return -1;
	}

	ret = listen(lfd, 128);
	if (ret == -1) {
		perror("listen failed");
		return -1;
	}

	return lfd;
}

int epoll_run(int lfd)
{
	int epfd = epoll_create(1);
	if (epfd == -1) {
		perror("epoll_create failed");
		return -1;
	}

	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if (ret == -1) {
		perror("epoll_ctl failed");
		return -1;
	}

	struct epoll_event evs[1024];
	int size = sizeof(evs) / sizeof(struct epoll_event);
	while (1) {
		int num = epoll_wait(epfd, evs, size, -1);
		for (int i = 0; i < num; ++i) {
			int fd = evs[i].data.fd;
			THREAD_PARAMS* p_params = (THREAD_PARAMS*)malloc(sizeof(THREAD_PARAMS));
			p_params->fd = fd;
			p_params->epfd = epfd;
			if (fd == lfd) {
				/* 连接建立 */
				pthread_create(&p_params->tid, NULL, thread_accept_connect, p_params);
			} else {
				/* 数据收发 */
				pthread_create(&p_params->tid, NULL, thread_recv_http_request, p_params);
			}
		}
	}

	return 0;
}

void* thread_accept_connect(void* arg)
{
	THREAD_PARAMS* params = (THREAD_PARAMS*)arg;
	int lfd = params->fd;
	int epfd = params->epfd;

	int cfd = accept(lfd, NULL, NULL);
	if (cfd == -1) {
		perror("accept failed");
		return;
	}

	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	struct epoll_event ev;
	ev.data.fd = cfd;
	ev.events = EPOLLIN | EPOLLET;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
	if (ret == -1) {
		perror("epoll_ctl failed");
		return;
	}

	LOGOUT("thread_accept_connect end thread_id[%d] client[%d] connected.\n", params->tid, cfd);
	free(params);

	return;
}

void* thread_recv_http_request(void* arg)
{
	THREAD_PARAMS* params = (THREAD_PARAMS*)arg;
	int cfd = params->fd;
	int epfd = params->epfd;

	int idx = 0;
	int len = 0;
	char buff[4096] = { 0 };
	char buff_seg[1024] = { 0 };

	while ((len = recv(cfd, buff_seg, sizeof(buff_seg), 0)) > 0) {
		if (idx + len < sizeof(buff))
			memcpy(buff + idx, buff_seg, len);
		idx += len;
	}

	if (len == -1 && errno == EAGAIN) {
		/* 请求解析 */
		char* req_head = strstr(buff, "\r\n");
		int req_head_len = req_head - buff;
		buff[req_head_len] = '\0';
		LOGOUT("parse request from client[%d] buff[%s]\n", cfd, buff);
		parse_request_line(buff, cfd);
	} else if (len == 0) {
		/* 连接断开 */
		LOGOUT("client[%d] disconnecte.\n", cfd);
		epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
		close(cfd);
	} else {
		perror("recv failed");
		return;
	}

	LOGOUT("thread_recv_http_request end thread_id[%d] client cfd[%d].\n", params->tid, cfd);
	free(params);

	return;
}

int parse_request_line(const char* line, int cfd)
{
	char req_path[2048] = { 0 };
	char req_method[1024] = { 0 };
	sscanf(line, "%[^ ] %[^ ]", req_method, req_path);
	if (strcasecmp(req_method, "get") != 0)
		return -1;
	
	decode_str(req_path, req_path);

	char* req_file = NULL;
	if (strcmp(req_path, "/") == 0)
		req_file = "./";//folder
	else
		req_file = req_path + 1;//file

	LOGOUT("parse_request_line req_method[%s] req_file[%s]\n", req_method, req_file);

	struct stat st;
	int ret = stat(req_file, &st);
	if (ret == -1) {
		//404 no found
		send_http_response_head(cfd, 404, "Not Found", get_file_type(".html"), -1);
		send_http_response_body_file(RES_PAGE_NOT_FOUNUD, cfd);
		return 0;
	}

	if (S_ISDIR(st.st_mode)) {
		//将目录内容返回
		send_http_response_head(cfd, 200, "OK", get_file_type(".html"), -1);
		send_http_response_body_directory(req_file, cfd);
	} else {
		//将文件内容返回
		send_http_response_head(cfd, 200, "OK", get_file_type(req_file), st.st_size);
		send_http_response_body_file(req_file, cfd);
	}

	return 0;
}