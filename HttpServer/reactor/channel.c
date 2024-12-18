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

#include "channel.h"

PCHANNEL channel_init(int fd, int events, handle_func read_call_back, handle_func write_call_back, void* arg)
{
	PCHANNEL channel = (PCHANNEL)malloc(sizeof(CHANNEL));
	channel->fd_ = fd;
	channel->events_ = events;
	channel->read_call_back_ = read_call_back;
	channel->write_call_back_ = write_call_back;
	channel->arg_ = arg;
	return channel;
}

bool channel_set_event_writable(PCHANNEL channel, bool b_enable)
{
	if (b_enable)
		channel->events_ |= CE_WRITE_EVENT;
	else
		channel->events_ = channel->events_ & ~CE_WRITE_EVENT;

	return true;
}

bool channel_get_event_writable(PCHANNEL channel)
{
	return channel->events_ & CE_WRITE_EVENT;
}
