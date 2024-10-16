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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/poll.h>
#include "dispatcher.h"

#define POLL_EVENT_MAX_NUM 1024

typedef struct epoll_data_st
{
	int				m_nfds;
	struct pollfd	m_fds[POLL_EVENT_MAX_NUM];
}POLLDATA, * PPOLLDATA;

static void* d_poll_init();
static int d_poll_add(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_poll_remove(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_poll_modify(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_poll_dispatch(PEVENTLOOP event_loop, int timeout);
static int d_poll_clear(PEVENTLOOP event_loop);

DISPATCHER epoll_dispatcher =
{
	d_poll_init,
	d_poll_add,
	d_poll_remove,
	d_poll_modify,
	d_poll_dispatch,
	d_poll_clear
};

static void* d_poll_init()
{
	PPOLLDATA data = (PPOLLDATA)malloc(sizeof(POLLDATA));
	data->m_nfds = 0;
	for (int i = 0; i < POLL_EVENT_MAX_NUM; ++i) {
		data->m_fds[i].fd = -1;
		data->m_fds[i].events = 0;
		data->m_fds[i].revents = 0;
	}
	return data;
}

static int d_poll_add(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PPOLLDATA data = (PPOLLDATA)event_loop->m_dispatcher;
	int events = 0;
	if (channel->m_events & READ_EVENT)
		events |= POLLIN;
	if (channel->m_events & WRITE_EVENT)
		events |= POLLOUT;

	int i = 0;
	for (; i < POLL_EVENT_MAX_NUM; ++i) {
		if (data->m_fds[i].fd == -1) {
			data->m_fds[i].fd = channel->m_fd;
			data->m_fds[i].events = events;
			//data->m_nfds = max(i, data->m_nfds);
			data->m_nfds = (i > data->m_nfds) ? i : data->m_nfds;
			break;
		}
	}

	if (i > POLL_EVENT_MAX_NUM) {
		return -1;
	}

	return 0;
}

static int d_poll_remove(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PPOLLDATA data = (PPOLLDATA)event_loop->m_dispatcher;

	int i = 0;
	for (; i < POLL_EVENT_MAX_NUM; ++i) {
		if (data->m_fds[i].fd == channel->m_fd) {
			data->m_fds[i].fd = -1;
			data->m_fds[i].events = 0;
			data->m_fds[i].revents = 0;
			break;
		}
	}

	if (i > POLL_EVENT_MAX_NUM) {
		return -1;
	}

	return 0;
}

static int d_poll_modify(PCHANNEL channel, PEVENTLOOP event_loop)
{
	PPOLLDATA data = (PPOLLDATA)event_loop->m_dispatcher;
	int events = 0;
	if (channel->m_events & READ_EVENT)
		events |= POLLIN;
	if (channel->m_events & WRITE_EVENT)
		events |= POLLOUT;

	int i = 0;
	for (; i < POLL_EVENT_MAX_NUM; ++i) {
		if (data->m_fds[i].fd == channel->m_fd) {
			data->m_fds[i].events = events;
			break;
		}
	}

	if (i > POLL_EVENT_MAX_NUM) {
		return -1;
	}

	return 0;
}

/**
 * \brief 核心函数 对事件进行处理
 * \param event_loop
 * \param timeout
 * \return
 */
static int d_poll_dispatch(PEVENTLOOP event_loop, int timeout)
{
	PPOLLDATA data = (PPOLLDATA)event_loop->m_dispatcher;
	int count = poll(data->m_fds, data->m_nfds + 1, timeout * 1000);
	if (count == -1) {
		perror("poll");
		exit(0);
	}

	for (int i = 0; i <= data->m_nfds; ++i) {
		if (data->m_fds[i].fd == -1) {
			continue;
		}
		int revents = data->m_fds[i].revents;
		if (revents & POLLIN)
		{
			/*文件描述符读事件*/
			continue;
		}
		if (revents & POLLOUT)
		{
			/*文件描述符写事件*/
			continue;
		}
	}

	return 0;
}

static int d_poll_clear(PEVENTLOOP event_loop)
{
	PPOLLDATA data = (PPOLLDATA)event_loop->m_dispatcher;
	free(data);
	return 0;
}

