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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "dispatcher.h"

#define SELECT_FD_MAX_NUM 1024

typedef struct select_data_st
{
	fd_set	m_r_set;
	fd_set	m_w_set;
}SELECTDATA, *PSELECTDATA;

static void* d_select_init();
static int d_select_add(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_remove(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_modify(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_dispatch(PEVENTLOOP event_loop, int timeout);
static int d_select_clear(PEVENTLOOP event_loop);

static void d_select_fdset_set(PCHANNEL channel, PSELECTDATA data);
static void d_select_fdset_cln(PCHANNEL channel, PSELECTDATA data);

DISPATCHER epoll_dispatcher =
{
	d_select_init,
	d_select_add,
	d_select_remove,
	d_select_modify,
	d_select_dispatch,
	d_select_clear
};

static void* d_select_init()
{
	PSELECTDATA data = (PSELECTDATA)malloc(sizeof(SELECTDATA));
	FD_ZERO(&data->m_r_set);
	FD_ZERO(&data->m_w_set);
	return data;
}

static int d_select_add(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)malloc(sizeof(SELECTDATA));
	if (channel->m_fd >= SELECT_FD_MAX_NUM)
		return -1;
	d_select_fdset_set(channel, data);
	return 0;
}

static int d_select_remove(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)malloc(sizeof(SELECTDATA));
	d_select_fdset_cln(channel, data);
	return 0;
}

static int d_select_modify(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)event_loop->m_dispatcher;
	d_select_fdset_set(channel, data);
	d_select_fdset_cln(channel, data);
	return 0;
}

/**
 * \brief 核心函数 对事件进行处理
 * \param event_loop
 * \param timeout
 * \return
 */
static int d_select_dispatch(PEVENTLOOP event_loop, int timeout)
{
	PSELECTDATA data = (PSELECTDATA)event_loop->m_dispatcher;
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;

	fd_set rdset = data->m_r_set;
	fd_set wrset = data->m_w_set;
	int count = select(SELECT_FD_MAX_NUM, &rdset, &wrset, NULL, &val);
	if (count == -1) {
		perror("select");
		exit(0);
	}

	for (int i = 0; i < SELECT_FD_MAX_NUM; ++i) {
		if (FD_ISSET(i, &rdset))
		{
			/*文件描述符读事件*/
			continue;
		}
		if (FD_ISSET(i, &wrset))
		{
			/*文件描述符写事件*/
			continue;
		}
	}

	return 0;
}

static int d_select_clear(PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)event_loop->m_dispatcher;
	free(data);
	return 0;
}

static void d_select_fdset_set(PCHANNEL channel, PSELECTDATA data)
{
	if (channel->m_events & READ_EVENT)
		FD_SET(channel->m_fd, &data->m_r_set);
	if (channel->m_events & WRITE_EVENT)
		FD_SET(channel->m_fd, &data->m_w_set);
}

static void d_select_fdset_cln(PCHANNEL channel, PSELECTDATA data)
{
	if (channel->m_events & READ_EVENT)
		FD_CLR(channel->m_fd, &data->m_r_set);
	if (channel->m_events & WRITE_EVENT)
		FD_CLR(channel->m_fd, &data->m_w_set);
}

