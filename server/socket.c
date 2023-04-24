/*************************************************************************
	> File Name: socket.c
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 12:18:56 PM CST
 ************************************************************************/

#include "head.h"
#include "socket.h"

int initListenFd(unsigned short port) {
	int lfd;
	//1.创建用于监听的套接字 AF_INET使用ipv4\6  SOCK_STREAM使用TCP流式协议
	if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

	//2.绑定套接字与结构体信息 ip地址与端口号
	int reuse = 1; setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int));//设置端口复用
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;//ipv4
	addr.sin_port = htons(port);//port 本地字节序转为网络字节序（大端）
	addr.sin_addr.s_addr = INADDR_ANY;//ip地址（大端）INADDR_ANY监听任何一个地址 0.0.0.0
	if (bind(lfd, (struct sockarr*)&addr, sizeof(addr)) < 0) return -1;

	//3.设置监听套接字 将主动套接字转为被动套接字
	if (listen(lfd, 128) < 0) return -1;// 128: 正在连接的连接序列有多少个 一次性能够最多接受多少个连接
	return lfd;
}
