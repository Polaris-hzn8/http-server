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

#pragma once

#include <pthread.h>
#include "../reactor/eventloop.h"

typedef struct work_thread_st {
	pthread_t thread_id_;
	char thread_name_[1024];
	pthread_mutex_t mutex_;
	pthread_cond_t cond_;
	PEVENTLOOP event_loop_;
} WORK_THREAD, *PWORK_THREAD;

int work_thread_init(PWORK_THREAD work_thread, int idx);
int work_thread_run(PWORK_THREAD work_thread);
