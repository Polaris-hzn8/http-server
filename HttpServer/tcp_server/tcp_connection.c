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

#include "tcp_connection.h"

int connection_read_process(void* args)
{
	PTCP_CONNECTION connection = (PTCP_CONNECTION)args;
	// 接收数据
	int ret = buffer_read_from_socket(connection->_buffer_r, connection->_channel->fd_);
	if (ret > 0) {
		/* 接收到客户端数据请求 */


	} else {
		/* 连接断开 */


	}
	return 0;
}

PTCP_CONNECTION tcp_connection_init(int cfd, PEVENTLOOP event_loop)
{
	PTCP_CONNECTION connection = (PTCP_CONNECTION)malloc(sizeof(TCP_CONNECTION));

	sprintf(connection->name_, "connection-%d", cfd);
	connection->_event_loop = event_loop;
	connection->_buffer_r = buffer_init(10 * 1024);//10k
	connection->_buffer_w = buffer_init(10 * 1024);
	connection->_channel = channel_init(cfd, CE_READ_EVENT, connection_read_process, NULL, connection);

	event_loop_task_add(event_loop, connection->_channel, CN_ADD);

	return connection;
}
