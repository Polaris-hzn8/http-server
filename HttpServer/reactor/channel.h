/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 13 Oct 2024 23:07:29 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include <stdbool.h>

typedef int(*handle_func)(void* arg);

typedef struct channel_st
{
	int				fd_;
	int				events_;
	handle_func		read_call_back_;	//���ص�
	handle_func		write_call_back_;	//д�ص�
	void*			arg_;
}CHANNEL, *PCHANNEL;

enum ChannelEvent
{
	CE_TIME_OUT	= 0x01,
	CE_READ_EVENT	= 0x02,
	CE_WRITE_EVENT = 0x04
};

PCHANNEL channel_init(int fd, int events, handle_func read_call_back, handle_func write_call_back, void* arg);

bool channel_set_event_writable(PCHANNEL channel, bool b_enable);
bool channel_get_event_writable(PCHANNEL channel);

