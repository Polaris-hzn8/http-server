/*************************************************************************
	> File Name: epoll.h
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:23:36 PM CST
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#include "head.h"

//与客户端建立连接
int acceptClient(int lfd, int epollfd);
//接受并处理http消息
int recvHttpRequest(int cfd, int epollfd);
//启动epoll
int epollRun(int lfd);

#endif
