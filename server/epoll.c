/*************************************************************************
	> File Name: epoll.c
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:23:41 PM CST
 ************************************************************************/

#include "epoll.h"
#include "head.h"


/**
 * @brief 
 * 
 * @param fd 用于监听的文件描述符 
 * @param epollfd epoll_create创建的文件描述符
 * @return int 
 */
int acceptClient(int fd, int epollfd) {
	int connectfd;
	// 1.建立连接 NULL 传出参数用于保存客户端的ip与端口信息 addrsock_in类型 
	if ((connectfd = accept(fd, NULL, NULL)) == -1) return -1;

	//2.将文件描述符设置为非阻塞
	int flag = fcntl(connectfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(connectfd, F_SETFL, flag);

	//3.将文件描述符添加到epoll模型中
	struct epoll_event ev;
	ev.data.fd = connectfd;
	ev.events = EPOLLIN | EPOLLET;
	/* 此时epoll的工作模式为边缘非阻塞工作模式 */
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connectfd, &ev) == -1) {
		perror("epoll_ctl");
		return -1;
	}
	return 0;
}


/**
 * @brief 
 * 
 * @param listenfd epoll监听的文件描述符
 */
int epollRun(int listenfd) {
	//1.创建epoll实例（创建epoll红黑树的根节点）
	int epollfd;
	if ((epollfd = epoll_create(1)) == -1) {
		perror("epoll_create");
		return -1;
	}

	//2.将用户监听的文件描述符 添加到epoll树上
	struct epoll_event ev;
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
		perror("epoll_ctl");
		return -1;
	}

	//3.检测文件描述符的动态
	struct epoll_event events[MAXEVENTS];
	int size = sizeof(events) / sizeof(struct epoll_event);
	while (1) {
		int nfd = epoll_wait(epollfd, events, size, -1);
		for (int i = 0; i < nfd; ++i) {
			int fd = events[i].data.fd;
			if (fd == listenfd) {
				/* case1 : 发生变化的是 listenfd 表明有新的客户端连接到服务器 需要建立新的连接 */
				acceptClient(listenfd, epollfd);
			} else  {
				/* case2 : 发生变化的不是 listenfd 表明是已经建立连接的客户端需要进行 数据通信操作 */
				/* 主要负责进行对端的数据接收（进行数据读取EPOLLIN） 且接受的数据为HTTP请求的格式 */
				/* 请求行 请求头 空行 请求的数据块 */
				/* POST请求 数据块中含有内容 */
				/* GET请求 数据块中没有内容 */
				


			}
		}
	}
}

