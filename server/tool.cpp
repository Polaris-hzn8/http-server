/*************************************************************************
	> File Name: tool.cpp
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Thu 15 Jun 2023 06:47:52 PM CST
 ************************************************************************/

#include "tool.h"


/// @brief 通过scanDir获取到文件的名字 再组装发送给客户端
/// @param filename 发送文件的名字 
/// @param cfd 用于通信的文件描述符 
/// @return int
int mySendFile(const char *filename, int cfd) {
	//1.打开文件
	int fd = open(filename, O_RDONLY); assert(fd > 0);

	//2.从文件中读取数据 并将数据发送给客户端
	// while (1) {
	// 	char buff[1024];
	// 	int len = read(fd, buff, sizeof(buff));
	// 	if (len > 0) {
	// 		/* 正常读取文件并发送 */
	// 		send(cfd, buff, len, 0);
	// 		usleep(10);//控制发送端发送数据的速率
	// 	} else if (len == 0) {
	// 		/* 文件读取结束退出循环 */
	// 		break;
	// 	} else {
	// 		/* 异常读取文件 输出错误信息 */
	// 		perror("sendFile:read");
	// 	}
	// }
	/* 直接调用linux提供的系统函数sendfile进行文件发送更高效 */
	/* 效率更高 且能够减少内存的拷贝 数据块有内存区到用户区的拷贝过程 */
	int size = lseek(fd, 0, SEEK_END);//利用lseek求文件的大小
	sendfile(cfd, fd, NULL, size);

	return 0;
}


/// @brief 发送目录数据 
/// @param dirname 发送目录的路径 
/// @param cfd 用于通信的文件描述符 
/// @return int
int mySendDir(const char *dirname, int cfd) {

	
	
	return 0;
}





