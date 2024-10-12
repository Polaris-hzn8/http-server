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

int send_http_response_head(int cfd, int statu, const char* descr, const char* type, int length)
{
	char buff[4096] = { 0 };
	sprintf(buff, "http/1.1 &d &s\r\n", statu, descr);
	sprintf(buff + strlen(buff), "content-type: %s\r\n", type);
	sprintf(buff + strlen(buff), "content-length: %d\r\n\r\n", length);

	send(cfd, buff, strlen(buff), 0);
	return 0;
}

int send_http_response_body_file(const char* file_name, int cfd)
{
	int fd = open(file_name, O_RDONLY);
	if (fd <= 0) {
		perror("send_file_info open fd <= 0");
		return -1;
	}

	//while (1) {
	//	char buff[1024] = { 0 };
	//	int len = read(fd, buff, sizeof(buff));
	//	if (len > 0) {
	//		send(cfd, buff, len, 0);
	//		usleep(10);
	//	} else if (len == 0) {
	//		break;
	//	} else {
	//		perror("send_file_info read");
	//		return -1;
	//	}
	//}
	
	struct stat st;
	int ret = stat(file_name, &st);
	if (ret < 0) {
		perror("stat failed");
		return -1;
	}

	int file_size = st.st_size;

	off_t offset = 0;
	while (offset < file_size) {
		int ret = sendfile(cfd, fd, &offset, file_size - offset);
		LOGOUT("sendfile ret value[%d].\n", ret);
		if (ret == -1) {
			if (errno == EAGAIN)
				LOGOUT("sendfile no data prepared.\n");
			else
				perror("sendfile failed");
		}
	}

	close(fd);

	return 0;
}

int send_http_response_body_directory(const char* dir_name, int cfd)
{
	LOGOUT("send_http_response_body_directory dir_name[%s]\n", dir_name);

	char buff[4096] = { 0 };
	sprintf(buff, "<html><head><title>%s</title></head><body><table>", dir_name);

	struct dirent** name_list = NULL;
	int name_count = scandir(dir_name, &name_list, NULL, alphasort);
	for (int i = 0; i < name_count; ++i) {
		struct stat st;
		char item_name_impact[4096] = {0};
		char* item_name = name_list[i]->d_name;
		sprintf(item_name_impact, "%s/%s", dir_name, item_name);
		stat(item_name_impact, &st);
		if (S_ISDIR(st.st_mode)) {	//目录项
			sprintf(buff + strlen(buff), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", item_name, item_name, st.st_size);
		} else {					//文件项
			sprintf(buff + strlen(buff), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", item_name, item_name, st.st_size);
		}

		send(cfd, buff, strlen(buff), 0);
		memset(buff, 0, sizeof(buff));
		free(name_list[i]);
	}

	sprintf(buff, "</table></body></html>");
	send(cfd, buff, strlen(buff), 0);
	memset(buff, 0, sizeof(buff));
	free(name_list);
	return 0;
}

const char* get_file_type(const char* name)
{
	// a.jpg a.mp4 a.html
	const char* dot = strrchr(name, '.');//strrchar从右向左查字符串
	if (dot == NULL)
		return "text/plain; charset=utf-8";
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void decode_str(char* output, char* input)
{
	while (*input != '\0') {
		if (input[0] == '%'
			&& isxdigit(input[1])
			&& isxdigit(input[2])) {
			*output = hex_to_dec(input[1]) * 16 + hex_to_dec(input[2]);
			input += 3;
		} else {
			*output = *input;
			++input;
		}
		++output;
	}
	*output = '\0';
}

int hex_to_dec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}
