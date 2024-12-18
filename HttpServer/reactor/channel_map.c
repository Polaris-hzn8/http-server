/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 13 Oct 2024 23:34:36 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include "channel_map.h"

PCHANNELMAP channel_map_init(int size)
{
	PCHANNELMAP channel_map = (PCHANNELMAP)malloc(sizeof(CHANNELMAP));
	channel_map->size_ = size;
	channel_map->list_ = (CHANNEL**)malloc(size * sizeof(CHANNEL*));
	return channel_map;
}

bool channel_map_uninit(PCHANNELMAP channel_map)
{
	if (channel_map) {
		for (int i = 0; i < channel_map->size_; ++i) {
			PCHANNEL channel = channel_map->list_[i];
			if (channel)
				free(channel);
		}
		free(channel_map->list_);
		channel_map->list_ = NULL;
	}
	channel_map->size_ = 0;
	return true;
}

bool channel_map_resize(PCHANNELMAP channel_map, int size, int unit_size)
{
	int list_size = channel_map->size_;
	if (list_size < size) {
		int new_size;
		while (new_size < size)
			new_size *= 2;
		CHANNEL** new_list = realloc(channel_map->list_, new_size * unit_size);
		if (new_list) {
			channel_map->list_ = new_list;
			channel_map->size_ = new_size;
			memset(&new_list[new_size], 0, (new_size - list_size) * unit_size);
			return true;
		}
	}
	return false;
}
