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
#include <sys/epoll.h>
#include "dispatcher.h"

#define EPOLL_EVENT_MAX_NUM 520

typedef struct epoll_data_st
{
	int					m_epfd;
	struct epoll_event* m_events;
}EPOLLDATA, * PEPOLLDATA;

static void* d_epoll_init();
static int d_epoll_add(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_epoll_remove(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_epoll_modify(PCHANNEL channel, PEVENTLOOP event_loop);
static int d_epoll_dispatch(PEVENTLOOP event_loop, int timeout);
static int d_epoll_clear(PEVENTLOOP event_loop);

static int d_epoll_ctl(PCHANNEL channel, PEVENTLOOP event_loop, int op);

DISPATCHER epoll_dispatcher =
{
	d_epoll_init,
	d_epoll_add,
	d_epoll_remove,
	d_epoll_modify,
	d_epoll_dispatch,
	d_epoll_clear
};

static void* d_epoll_init()
{
	PEPOLLDATA data = (PEPOLLDATA)malloc(sizeof(EPOLLDATA));
	int epfd = epoll_create(1);
	if (epfd == -1) {
		perror("epoll_create");
		exit(0);
	}
	data->m_epfd = epfd;
	data->m_events = (struct epoll_event*)calloc(EPOLL_EVENT_MAX_NUM, sizeof(struct epoll_event));
	return data;
}

static int d_epoll_add(PCHANNEL channel, PEVENTLOOP event_loop)
{
	return d_epoll_ctl(channel, event_loop, EPOLL_CTL_ADD);
}

static int d_epoll_remove(PCHANNEL channel, PEVENTLOOP event_loop)
{
	return d_epoll_ctl(channel, event_loop, EPOLL_CTL_DEL);
}

static int d_epoll_modify(PCHANNEL channel, PEVENTLOOP event_loop)
{
	return d_epoll_ctl(channel, event_loop, EPOLL_CTL_MOD);
}

int d_epoll_ctl(PCHANNEL channel, PEVENTLOOP event_loop, int op)
{
	PEPOLLDATA data = (PEPOLLDATA)event_loop->m_dispatcher;
	struct epoll_event ev;
	ev.data.fd = channel->m_fd;
	int events = 0;
	if (channel->m_events & READ_EVENT)
		events |= EPOLLIN;
	if (channel->m_events & WRITE_EVENT)
		events |= EPOLLOUT;
	ev.events = events;
	int ret = epoll_ctl(data->m_epfd, op, channel->m_fd, &ev);
	return ret;
}

/**
 * \brief ���ĺ��� ���¼����д���
 * \param event_loop
 * \param timeout
 * \return 
 */
static int d_epoll_dispatch(PEVENTLOOP event_loop, int timeout)
{
	PEPOLLDATA data = (PEPOLLDATA)event_loop->m_dispatcher;
	int count = epoll_wait(data->m_epfd, data->m_events, EPOLL_EVENT_MAX_NUM, timeout * 1000);
	for (int i = 0; i < count; ++i) {
		int fd = data->m_events[i].data.fd;
		int events = data->m_events[i].events;
		if (events & EPOLLERR || events & EPOLLHUP) {
			/*���ӶϿ�*/
			//d_epoll_remove(channel, event_loop);
			continue;
		}
		if (events & EPOLLIN)
		{
			/*�ļ����������¼�*/
			continue;
		}
		if (events & EPOLLOUT)
		{
			/*�ļ�������д�¼�*/
			continue;
		}
	}
	return 0;
}

static int d_epoll_clear(PEVENTLOOP event_loop)
{
	PEPOLLDATA data = (PEPOLLDATA)event_loop->m_dispatcher;
	free(data->m_events);
	close(data->m_epfd);
	free(data);
	return 0;
}

