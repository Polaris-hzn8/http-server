/*************************************************************************
	> File Name: epoll.h
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:23:36 PM CST
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#include "head.h"

int epollRun(int fd);//启动epoll
int acceptClient(int fd, int epollfd);//与客户端建立连接

#endif
