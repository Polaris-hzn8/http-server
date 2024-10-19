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

#include <string.h>
#include <assert.h>
#include "eventloop.h"

PEVENTLOOP event_loop_init()
{
	return event_loop_init_ex(NULL);
}

PEVENTLOOP event_loop_init_ex(const char* thread_name)
{
	PEVENTLOOP event_loop = (PEVENTLOOP)malloc(sizeof(EVENTLOOP));

	event_loop->is_running_ = false;
	event_loop->dispatcher_ = &g_epoll_dispatcher;
	//event_loop->dispatcher_ = &g_poll_dispatcher;
	//event_loop->dispatcher_ = &g_select_dispatcher;
	event_loop->dispatcher_data_ = event_loop->dispatcher_->init();

	event_loop->tasklist_head_ = NULL;
	event_loop->tasklist_tail_ = NULL;

	int size = 128;
	event_loop->channel_map_ = channel_map_init(size);

	event_loop->thread_id_ = pthread_self();
	strcpy(event_loop->thread_name_, (thread_name == NULL) ? "main_thread" : thread_name);

	pthread_mutex_init(&event_loop->mutex_, NULL);

	return event_loop;
}

int event_loop_run(PEVENTLOOP event_loop)
{
	assert(event_loop != NULL);

	if (event_loop->thread_id_ != pthread_self())
		return -1;

	/* 取出事件分发和检测模型 */
	PDISPATCHER dispatcher = event_loop->dispatcher_;

	/* 循环进行事件处理 */
	int time_out = 3;
	while (!event_loop->is_running_)
		dispatcher->dispatch(event_loop, time_out);

	return 0;
}

int tackle_active_fd(PEVENTLOOP event_loop, int act_fd, int act_event)
{




	return 0;
}

