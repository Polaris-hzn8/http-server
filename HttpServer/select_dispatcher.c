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
	fd_set	r_set_;
	fd_set	w_set_;
}SELECTDATA, *PSELECTDATA;

static void* d_select_init();
static int d_select_add(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_remove(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_modify(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_select_dispatch(PEVENTLOOP event_loop, int timeout);
static int d_select_clear(PEVENTLOOP event_loop);

static void d_select_fdset_set(PCHANNEL channel, PSELECTDATA data);
static void d_select_fdset_cln(PCHANNEL channel, PSELECTDATA data);

DISPATCHER g_select_dispatcher =
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
	FD_ZERO(&data->r_set_);
	FD_ZERO(&data->w_set_);
	return data;
}

static int d_select_add(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)malloc(sizeof(SELECTDATA));
	if (channel->fd_ >= SELECT_FD_MAX_NUM)
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
	PSELECTDATA data = (PSELECTDATA)event_loop->dispatcher_;
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
	PSELECTDATA data = (PSELECTDATA)event_loop->dispatcher_;
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;

	fd_set rdset = data->r_set_;
	fd_set wrset = data->w_set_;
	int count = select(SELECT_FD_MAX_NUM, &rdset, &wrset, NULL, &val);
	if (count == -1) {
		perror("select");
		exit(0);
	}

	for (int i = 0; i < SELECT_FD_MAX_NUM; ++i) {
		if (FD_ISSET(i, &rdset))
		{
			/*文件描述符读事件*/
			event_tackle_active_fd(event_loop, i, READ_EVENT);
		}
		if (FD_ISSET(i, &wrset))
		{
			/*文件描述符写事件*/
			event_tackle_active_fd(event_loop, i, WRITE_EVENT);
		}
	}

	return 0;
}

static int d_select_clear(PEVENTLOOP event_loop)
{
	PSELECTDATA data = (PSELECTDATA)event_loop->dispatcher_;
	free(data);
	return 0;
}

static void d_select_fdset_set(PCHANNEL channel, PSELECTDATA data)
{
	if (channel->events_ & READ_EVENT)
		FD_SET(channel->fd_, &data->r_set_);
	if (channel->events_ & WRITE_EVENT)
		FD_SET(channel->fd_, &data->w_set_);
}

static void d_select_fdset_cln(PCHANNEL channel, PSELECTDATA data)
{
	if (channel->events_ & READ_EVENT)
		FD_CLR(channel->fd_, &data->r_set_);
	if (channel->events_ & WRITE_EVENT)
		FD_CLR(channel->fd_, &data->w_set_);
}

