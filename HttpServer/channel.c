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
	channel->m_fd = fd;
	channel->m_events = events;
	channel->m_read_call_back = read_call_back;
	channel->m_write_call_back = write_call_back;
	channel->m_arg = arg;
	return channel;
}

bool set_write_event_enable(PCHANNEL channel, bool b_enable)
{
	if (b_enable)
		channel->m_events |= WRITE_EVENT;
	else
		channel->m_events = channel->m_events & ~WRITE_EVENT;

	return true;
}

bool get_write_event_enable(PCHANNEL channel)
{
	return channel->m_events & WRITE_EVENT;
}
