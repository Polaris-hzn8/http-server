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
#include "thread_pool.h"
#include "../reactor/eventloop.h"

PTHREAD_POOL thread_pool_init(PEVENTLOOP main_loop, int thread_num)
{
	PTHREAD_POOL thread_pool = (PTHREAD_POOL)malloc(sizeof(THREAD_POLL));

	thread_pool->idx_ = 0;
	thread_pool->is_running_ = false;
	thread_pool->thread_num_ = thread_num;
	thread_pool->main_loop_ = main_loop;
	thread_pool->work_threads_ = (PWORK_THREAD)malloc(sizeof(WORK_THREAD) * thread_num);

	return thread_pool;
}

int thread_pool_run(PTHREAD_POOL thread_pool)
{
	if (thread_pool == NULL || thread_pool->is_running_) {
		LOGOUT("thread_pool is running already, or thread_pool NULL.");
		return -1;
	}

	if (thread_pool->main_loop_->thread_id_ != pthread_self()) {
		LOGOUT("only main thread event_loop can run thread_pool.");
		return -1;
	}

	thread_pool->is_running_ = true;

	int thread_num = thread_pool->thread_num_;
	if (thread_num) {
		for (int i = 0; i < thread_num; ++i) {
			PWORK_THREAD work_thread_tmp = thread_pool->work_threads_[i];
			work_thread_init(&work_thread_tmp, i);
			work_thread_run(&work_thread_tmp);
		}
	}

	return 0;
}

PEVENTLOOP get_event_loop_from_workthread(PTHREAD_POOL thread_pool)
{
	if (!thread_pool->is_running_) {
		LOGOUT("thread_pool is not running. get event_loop failed.");
		return NULL;
	}

	if (thread_pool->main_loop_->thread_id_ != pthread_self()) {
		LOGOUT("only main thread can get event_loop from thread_pool.");
		return NULL;
	}

	PEVENTLOOP event_loop = thread_pool->main_loop_;

	int idx = thread_pool->idx_;
	if (thread_pool->thread_num_) {
		event_loop = thread_pool->work_threads_[idx]->event_loop_;
		thread_pool->idx_ = (idx + 1) % idx;
	}

	return event_loop;
}
