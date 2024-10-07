/*************************************************************************
	> File Name: epoll.c
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:23:41 PM CST
 ************************************************************************/

#include "server.h"
#include "head.h"


/**
 * @brief 接受并处理客户端创立的新连接
 * 
 * @param lfd 用于监听的文件描述符 
 * @param epollfd epoll_create创建的文件描述符
 * @return int 
 */
int acceptClient(int lfd, int epollfd) {
	int cfd;
	// 1.建立连接 NULL 传出参数用于保存客户端的ip与端口信息 addrsock_in类型 
	if ((cfd = accept(lfd, NULL, NULL)) == -1) return -1;

	//2.将文件描述符设置为非阻塞
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//3.将文件描述符添加到epoll模型中
	struct epoll_event ev;
	ev.data.fd = cfd;
	ev.events = EPOLLIN | EPOLLET;
	/* 此时epoll的工作模式为边缘非阻塞工作模式 */
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1) {
		perror("epoll_ctl");
		return -1;
	}
	return 0;
}

/**
 * @brief 接受并处理http消息（http请求解析）
 * 将客户端发送的数据都接受 并写入到本地
 * @param cfd 用于通信的文件描述符
 * @param epollfd 若客户端发送的数据已发送完毕 并断开了连接 则需要将通信的fd文件描述符从 epoll树上删除
 * @return int 
 */
int recvHttpRequest(int cfd, int epollfd) {
	int index = 0;//当前已经写入消息的长度
	char buff[1024] = {0};//临时缓存
	char dest[4096] = {0};//数据的最终存储位置
	int len = 0;//每次接受到消息数据的长度
	/* 开始接受并处理http请求的消息 */
	/* 由于用于通信的文件描述符connectfd已经被设置为了 边缘非阻塞模式 则poll检测到文件描述符对应的读事件之后 只会进行一次通知 */
	/* 因此需要在得到epoll通知之后 一次性将所有的数据进行读取完成 */
	while ((len = recv(cfd, buff, sizeof(buff), 0)) > 0) {
		if (index + len < sizeof(dest)) memcpy(dest + index, buff, len);//只需要接受4096个大小的数据
		else break;
		index += len;
	}
	
	//判断数据是否被正常接受完毕
	/* 如果cfd被设置为阻塞 则当数据读取完毕之后 当前的线程会阻塞在recv函数的位置上 */
	/* 如果cfd被设置为非阻塞 则当数据读取完毕之后 当前的线程不会阻塞在recv函数的位置上 会继续读取 */
	/* 如果cfd套接字对应的读缓冲区中已经没有数据 recv再继续读取数据则会返回 -1  */
	/* 即recv操作在数据读取完毕 与 数据读取失败的两种情况下都会返回 -1 */
	if (len == -1 && errno == EAGAIN) {
		/* 数据正常读取完毕 则开始解析http请求行 */


	} else if (len == 0) {
		/* recv返回值为0 客户端断开了连接 */
		epoll_ctl(epollfd, EPOLL_CTL_DEL, cfd, NULL);
		close(cfd);
	} else {
		/* 其他异常读取操作导致 数据读取提前结束 */
		perror("recv");
	}
	return 0;
}


/**
 * @brief 
 * 启动epoll进行文件描述符监听
 * @param lfd epoll监听的文件描述符
 */
int epollRun(int lfd) {
	//1.创建epoll实例（创建epoll红黑树的根节点）
	int epollfd;
	if ((epollfd = epoll_create(1)) == -1) {
		perror("epoll_create");
		return -1;
	}

	//2.将用户监听的文件描述符 添加到epoll树上
	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lfd, &ev) == -1) {
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
			if (fd == lfd) {
				/* case1 : 发生变化的是 listenfd 表明有新的客户端连接到服务器 需要建立新的连接 */
				acceptClient(lfd, epollfd);
			} else  {
				/* case2 : 发生变化的不是 listenfd 表明是已经建立连接的客户端需要进行 数据通信操作 */
				/* 主要负责进行对端的数据接收（ 只进行进行数据读取操作!~ EPOLLIN ） 且接受的数据为HTTP请求的格式 */
				/* 请求行 请求头 空行 请求的数据块 */
				/* POST请求 数据块中含有内容 */
				/* GET请求 数据块中没有内容 */
				recvHttpRequest(fd, epollfd);
			}
		}
	}
}

