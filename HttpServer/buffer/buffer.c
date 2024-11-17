/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 27 Oct 2024 22:40:45 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include <memory.h>
#include <sys/uio.h>
#include "buffer.h"

PBUFFER buffer_init(int capacity)
{
	PBUFFER buffer = (PBUFFER)malloc(sizeof(BUFFER));
	if (buffer == NULL) {
		LOGOUT("buffer malloc failed.");
		return NULL;
	}

	buffer->data_ = (char*)malloc(sizeof(char) * capacity);
	buffer->capacity_ = capacity;
	buffer->read_pos_ = 0;
	buffer->write_pos_ = 0;

	memset(buffer->data_, 0, capacity);

	return buffer;
}

int buffer_destroy(PBUFFER buffer)
{
	if (buffer != NULL) {
		if (buffer->data_ != NULL) {
			free(buffer->data_);
			buffer->data_ = NULL;
		}
		free(buffer);
		buffer = NULL;
	}
	return 0;
}

int buffer_resize(PBUFFER buffer, int capacity)
{
	if (buffer_write_size_remain(buffer) >= capacity) {
		/* 内存够用 */
		LOGOUT("buffer write size is already over new capacity, no need to resize.");
	} else if (buffer_write_size_remain(buffer) + buffer->read_pos_ >= capacity) {
		/* 内存够用 需要合并 */
		int readable_size = buffer_read_size_remain(buffer);
		memcpy(buffer->data_, buffer->data_ + buffer->read_pos_, readable_size);
		buffer->read_pos_ = 0;
		buffer->write_pos_ = readable_size;
	} else {
		/* 内存不够 */
		void* p = realloc(buffer->data_, buffer->capacity_ + capacity);
		if (p == NULL) {
			LOGOUT("buffer_resize realloc failed, ret pointer is null.");
			return -1;
		}
		memset(p + buffer->capacity_, 0, capacity);
		buffer->data_ = p;
		buffer->capacity_ += capacity;
	}
	return 0;
}

int buffer_write_size_remain(PBUFFER buffer)
{
	return buffer->capacity_ - buffer->write_pos_;
}

int buffer_read_size_remain(PBUFFER buffer)
{
	return buffer->write_pos_ - buffer->read_pos_;
}

int buffer_write(PBUFFER buffer, const char* data, int size)
{
	if (buffer == NULL || data == NULL || size <= 0) {
		LOGOUT("buffer_write failed, invalid data.");
		return -1;
	}

	buffer_resize(buffer, size);

	memcpy(buffer->data_ + buffer->write_pos_, data, size);
	buffer->write_pos_ += size;

	return 0;
}

int buffer_write_ex(PBUFFER buffer, const char* data)
{
	int size = strlen(data);
	return buffer_write(buffer, data, size);
}

int buffer_read_from_socket(PBUFFER buffer, int fd)
{
	struct iovec vec[2];
	int writable_size = buffer_write_size_remain(buffer);
	vec[0].iov_base = buffer->data_ + buffer->write_pos_;
	vec[0].iov_len = writable_size;

	/* 临时存储 */
	char* tmpbuff = (char*)malloc(40960);
	vec[1].iov_base = buffer->data_ + buffer->write_pos_;
	vec[0].iov_len = 40960;

	int ret = readv(fd, vec, 2);
	if (ret == -1) {
		return -1;
	} else if (ret <= writable_size) {
		buffer->write_pos_ += ret;
	} else {
		buffer->write_pos_ = buffer->capacity_;
		buffer_write(buffer, tmpbuff, ret - writable_size);
	}

	free(tmpbuff);
	tmpbuff = NULL;

	return ret;
}

