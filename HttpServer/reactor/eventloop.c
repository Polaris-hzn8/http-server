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
#include <unistd.h>
#include "eventloop.h"

void task_list_wake_up(PEVENTLOOP event_loop)
{
	const char* wake_up_msg = "anything.";
	write(event_loop->socket_pair_[0], wake_up_msg, strlen(wake_up_msg));
}

int read_local_message(void* arg)
{
	PEVENTLOOP event_loop = (PEVENTLOOP)(arg);
	char buff[1024] = { 0 };
	read(event_loop->socket_pair_[1], buff, sizeof(buff));
	return 0;
}

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

	//socket_pair_[0] socket_pair_[1]
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, event_loop->socket_pair_);
	if (ret == -1) {
		perror("socketpair");
		exit(0);
	}

	PCHANNEL channel = channel_init(event_loop->socket_pair_[1], CE_READ_EVENT, read_local_message, NULL, event_loop);
	event_loop_task_add(event_loop, channel, CN_ADD);

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
	while (!event_loop->is_running_) {
		dispatcher->dispatch(event_loop, time_out);
		event_loop_task_process(event_loop);
	}

	return 0;
}

int event_tackle_active_fd(PEVENTLOOP event_loop, int act_fd, int act_event)
{
	if (act_fd < 0 || event_loop == NULL)
		return -1;

	PCHANNEL channel = event_loop->channel_map_->list_[act_fd];

	if (channel->fd_ != act_fd)
		return -2;

	if (act_event & CE_READ_EVENT)
		channel->read_call_back_(channel->arg_);

	if (act_event & CE_WRITE_EVENT)
		channel->write_call_back_(channel->arg_);

	return 0;
}

int event_loop_task_add(PEVENTLOOP event_loop, PCHANNEL channel, int type)
{
	pthread_mutex_lock(&event_loop->mutex_);

	PCHANNEL_NODE channel_node = (PCHANNEL_NODE)malloc(sizeof(CHANNEL_NODE));

	channel_node->channel_ = channel;
	channel_node->type_ = type;
	channel_node->next_ = NULL;
	if (event_loop->tasklist_head_ == NULL) {
		event_loop->tasklist_head_ = channel_node;
		event_loop->tasklist_tail_ = channel_node;
	} else {
		event_loop->tasklist_tail_->next_ = channel_node;
		event_loop->tasklist_tail_ = channel_node;
	}

	pthread_mutex_unlock(&event_loop->mutex_);

	/**
	 * 结点处理
	 * 1.任务添加可以是当前线程执行、也可以是主线程添加
	 *	修改fd的事件：由子线程发起并由子线程处理
	 *	添加新的fd：和新客户端建立连接，由主线程发起并执行
	 * 2.主线程不进行结点的处理
	 */
	if (event_loop->thread_id_ = pthread_self()) {
		/* 子线程 */
		event_loop_task_process(event_loop);
	} else {
		/*
		* 主线程
		* 通知子线程处理任务队列中的任务
		* 在检测的集合中添加可控制的fd 用于唤醒解除阻塞
		* 1.两个文件描述符
		* 2.两个文件描述符之间可以进行数据通信(单向即可)
		* 3.pipe/socketpair
		*/
		task_list_wake_up(event_loop);
	}

	return 0;
}

int event_loop_task_process(PEVENTLOOP event_loop)
{
	pthread_mutex_lock(&event_loop->mutex_);

	PCHANNEL_NODE head_channel = event_loop->tasklist_head_;
	while (head_channel != NULL) {
		PCHANNEL channel = head_channel->channel_;
		switch (head_channel->type_) {
			case CN_ADD: {
				event_loop_add(event_loop_add, channel);
			}
			break;
			case CN_DEL: {
				event_loop_rem(event_loop_add, channel);
			}
			break;
			case CN_MOD: {
				event_loop_mod(event_loop_add, channel);
			}
			break;
		}
		PCHANNEL_NODE del_channel = head_channel;
		head_channel = head_channel->next_;
		free(del_channel);
	}
	event_loop->tasklist_head_ = NULL;
	event_loop->tasklist_tail_ = NULL;
	
	pthread_mutex_unlock(&event_loop->mutex_);
	return 0;
}

int event_loop_add(PEVENTLOOP event_loop, PCHANNEL channel)
{
	int fd = channel->fd_;
	PCHANNELMAP channel_map = event_loop->channel_map_;
	if (fd >= channel_map->size_) {
		if (!channel_map_resize(channel_map, fd, sizeof(PCHANNEL))) {
			perror("channel_map_resize");
			return -1;
		}
	}

	if (channel_map->list_[fd] == NULL) {
		channel_map->list_[fd] = channel;
		event_loop->dispatcher_->add(channel, event_loop);
	}
	return 0;
}

int event_loop_rem(PEVENTLOOP event_loop, PCHANNEL channel)
{
	int fd = channel->fd_;
	PCHANNELMAP channel_map = event_loop->channel_map_;
	if (fd >= channel_map->size_ || channel_map->list_[fd] == NULL) {
		LOGOUT("fd[%d] not in channel_map, remove from event_loop failed.", fd);
		return -1;
	}
	int ret = event_loop->dispatcher_->remove(channel, event_loop);
	return ret;
}

int event_loop_mod(PEVENTLOOP event_loop, PCHANNEL channel)
{
	int fd = channel->fd_;
	PCHANNELMAP channel_map = event_loop->channel_map_;
	if (fd >= channel_map->size_ || channel_map->list_[fd] == NULL) {
		LOGOUT("fd[%d] not in channel_map, modify from event_loop failed.", fd);
		return -1;
	}
	int ret = event_loop->dispatcher_->modify(channel, event_loop);
	return ret;
}

int event_loop_channel_distroy(PEVENTLOOP event_loop, PCHANNEL channel)
{
	int fd = channel->fd_;
	event_loop->channel_map_->list_[fd] = NULL;
	free(channel);
	close(fd);
	return 0;
}

