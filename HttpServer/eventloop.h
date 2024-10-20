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

#pragma once

#include <pthread.h>
#include "channel.h"
#include "channel_map.h"
#include "dispatcher.h"

extern DISPATCHER g_epoll_dispatcher;
extern DISPATCHER g_poll_dispatcher;
extern DISPATCHER g_select_dispatcher;

enum NodeType
{
	CN_ADD,
	CN_DEL,
	CN_MOD
};

typedef struct channel_node_st
{
	int				type_;
	PCHANNEL		channel_;
	PCHANNEL_NODE	next_;
}CHANNEL_NODE, *PCHANNEL_NODE;

typedef struct eventloop_st
{
	bool		is_running_;
	PDISPATCHER dispatcher_;
	void*		dispatcher_data_;
	//任务队列
	PCHANNEL_NODE tasklist_head_;
	PCHANNEL_NODE tasklist_tail_;
	//记录对应关系
	PCHANNELMAP channel_map_;
	//线程信息
	pthread_t	thread_id_;
	char		thread_name_[32];
	//任务队列互斥锁
	pthread_mutex_t mutex_;
}EVENTLOOP, *PEVENTLOOP;

PEVENTLOOP event_loop_init();
PEVENTLOOP event_loop_init_ex(const char* thread_name);

int event_loop_run(PEVENTLOOP event_loop);
int event_tackle_active_fd(PEVENTLOOP event_loop, int act_fd, int act_event);
