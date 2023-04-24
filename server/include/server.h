/*************************************************************************
	> File Name: epoll.h
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:23:36 PM CST
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#include "head.h"

int acceptClient(int lfd, int epollfd);//与客户端建立连接
int recvHttpRequest(int cfd, int epollfd);//接受并处理http消息
int parseHttpRequestLine(const char* line, int cfd);//在接受到http消息之后 进行具体的解析操作
int epollRun(int lfd);//启动epoll

#endif
