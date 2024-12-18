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
#include <sys/socket.h>
#include "channel.h"
#include "channel_map.h"
#include "dispatcher.h"

#define LOGOUT(fmt, ...) do { printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)

// 前向声明解决循环依赖
typedef struct dispatcher_st DISPATCHER, * PDISPATCHER;
// 前向声明解决自我依赖
typedef struct channel_node_st CHANNEL_NODE, * PCHANNEL_NODE;

enum CN_TYPE
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
	//任务队列 用于修改文件描述符集合
	PCHANNEL_NODE tasklist_head_;
	PCHANNEL_NODE tasklist_tail_;
	//记录对应关系
	PCHANNELMAP channel_map_;
	//线程信息
	pthread_t	thread_id_;
	char		thread_name_[32];
	//任务队列互斥锁
	pthread_mutex_t mutex_;
	//本地通信fd
	int			socket_pair_[2];
}EVENTLOOP, *PEVENTLOOP;

extern DISPATCHER g_epoll_dispatcher;
extern DISPATCHER g_poll_dispatcher;
extern DISPATCHER g_select_dispatcher;

PEVENTLOOP event_loop_init();
PEVENTLOOP event_loop_init_ex(const char* thread_name);

int event_loop_run(PEVENTLOOP event_loop);
int event_tackle_active_fd(PEVENTLOOP event_loop, int act_fd, int act_event);
int event_loop_task_add(PEVENTLOOP event_loop, PCHANNEL channel, int type);

int event_loop_process_task(PEVENTLOOP event_loop);
int event_loop_add(PEVENTLOOP event_loop, PCHANNEL channel);
int event_loop_rem(PEVENTLOOP event_loop, PCHANNEL channel);
int event_loop_mod(PEVENTLOOP event_loop, PCHANNEL channel);

int event_loop_channel_distroy(PEVENTLOOP event_loop, PCHANNEL channel);