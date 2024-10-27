/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 27 Oct 2024 13:37:27 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include "workthread.h"

int work_thread_init(PWORK_THREAD work_thread, int idx)
{
	work_thread->thread_id_ = 0;
	sprintf(work_thread->thread_name_, "sub_thread_%d", idx);
	pthread_mutex_init(&work_thread->mutex_, NULL);
	pthread_cond_init(&work_thread->cond_, NULL);
	work_thread->event_loop_ = NULL;
	return 0;
}

void* work_thread_run_cb(void* arg)
{
	PWORK_THREAD work_thread = (PWORK_THREAD)arg;

	pthread_mutex_lock(&work_thread->mutex_);
	work_thread->event_loop_ = event_loop_init_ex(work_thread->thread_name_);
	pthread_mutex_unlock(&work_thread->mutex_);

	pthread_cond_signal(&work_thread->cond_);

	event_loop_run(work_thread->event_loop_);
	return 0;
}

int work_thread_run(PWORK_THREAD work_thread)
{
	pthread_create(&work_thread->thread_id_, NULL, work_thread_run_cb, work_thread);

	//block main thread until event_loop init succeed.
	pthread_mutex_lock(&work_thread->mutex_);
	while (work_thread->event_loop_ == NULL) {
		pthread_cond_wait(&work_thread->cond_, &work_thread->mutex_);
	}
	pthread_mutex_unlock(&work_thread->mutex_);
	return 0;
}


