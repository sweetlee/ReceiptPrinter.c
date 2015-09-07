/*
 * bytebuf.c
 *
 *  Created on: Jan 30, 2014
 *      Author: jeroen
 */


#include "bytebuf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int bytebuf_min_capacity = 1024;

bytebuf* bytebuf_create(void) {
	bytebuf* buf = malloc(sizeof(bytebuf));
	if(buf) {
		buf->size = 0;
        buf->block_size = 0;
		buf->head = NULL;
		buf->tail = NULL;
	}
	return buf;
}

bytebuf* bytebuf_create_with_data(char* data, int length) {
	bytebuf* buf = bytebuf_create();
	if(buf) {
		bytebuf_append_bytes(buf, data, length);
	}
	return buf;
}

static int bytebuf_block_size(int desired) {
	return desired > bytebuf_min_capacity ? desired : bytebuf_min_capacity;
}

int bytebuf_append_bytes_internal(bytebuf* buf, const void* data, int length, int new_block) {
	int original_length = length;

	/* If we have a tail-block with room to spare, copy as much data as possible into it */
	if(!new_block && buf->tail && buf->tail->end < buf->tail->capacity) {
		int available = buf->tail->capacity - buf->tail->end;
        
        if (buf->block_size == 0 || (buf->block_size > 0 && available >= length)) {
            int copy_length = available < length ? available : length;
            memcpy(buf->tail->data + buf->tail->end, data, copy_length);
            buf->tail->size += copy_length;
            buf->tail->end += copy_length;
            data += copy_length;
            length -= copy_length;
            buf->size += copy_length;
        }
	}
    
    if (buf->block_size > 0 && length > buf->block_size) {
        // Error !
        return 0;
    }

	/*
	 *  If there's still data left, copy it to a new block, which will become the new
	 *  tail-block.
	 */
	if(length) {
		bytebuf_block* block = malloc(sizeof(bytebuf_block));
		if(block) {
            int desired = (buf->block_size > 0) ? buf->block_size : length;
			int capacity = bytebuf_block_size(desired);
			block->data = malloc(capacity);
			if(block->data) {
				block->capacity = capacity;
                block->size = length;
				block->start = 0;
				block->end = length;
				block->next = NULL;
				memcpy(block->data, data, length);
				buf->size += length;

				if(buf->tail) {
					buf->tail->next = block;
					buf->tail = block;
				} else {
					buf->tail = buf->head = block;
				}
				length = 0;
			} else {
				free(block);
			}
		}
	}
    
    //printf("BUF add size: %i\n", buf->size);
	return original_length - length;
}

int bytebuf_append_bytes(bytebuf* buf, const void* data, int length) {
    return bytebuf_append_bytes_internal(buf, data, length, 0);
}

int bytebuf_append_bytes_new_block(bytebuf* buf, const void* data, int length) {
    return bytebuf_append_bytes_internal(buf, data, length, 1);
}

int bytebuf_append_bytes_no_copy(bytebuf* buf, char* data, int length) {
	bytebuf_block* block = malloc(sizeof(bytebuf_block));
	if(block) {
		block->capacity = length;
		block->start = 0;
		block->end = length;
		block->data = data;
		block->next = NULL;
		if(buf->tail) {
			buf->tail->next = block;
			buf->tail = block;
		} else {
			buf->head = buf->tail = block;
		}
		buf->size += length;
		return length;
	} else {
		return 0;
	}
}

int bytebuf_copy_head(bytebuf* buf, char* dest, int length) {
	int offset = 0;

	if(length > buf->size) {
		length = buf->size;
	}

	if(length) {
		bytebuf_block* block = buf->head;
		while(offset < length) {
			int available = block->end - block->start;
			if(available > length-offset) {
				available = length-offset;
			}
			memcpy(dest + offset, buf->head->data + buf->head->start, available);
			offset += available;
			block = block->next;
		}
	}

	return length;
}

static int bytebuf_remove_head_internal(bytebuf* buf, int length, char* dest) {
	int original_length;
	if(length > buf->size) {
		length = buf->size;
	}

	original_length = length;

	if(length) {
		bytebuf_block* block = buf->head;
		while(length) {
			bytebuf_block* next = block->next;
			int available = block->end - block->start;
			if(available > length) {
				available = length;
			}
			if(dest) {
				memcpy(dest, block->data + block->start, available);
				dest += available;
			}
			block->start += available;
			if(block->start == block->end) {
				free(block->data);
				free(block);
				buf->head = next;
			}
			length -= available;
			buf->size -= available;
			block = next;
		}
	}

	if(!buf->head) {
		buf->tail = NULL;
	}

	return original_length;
}

static int bytebuf_remove_head_block_internal(bytebuf* buf, int length, char* dest) {
    int original_length;
    if(length > buf->size) {
        length = buf->size;
    }
    
    original_length = length;
    
    if(length) {
        bytebuf_block* block = buf->head;
        bytebuf_block* next = block->next;
        int available = block->end - block->start;
        if(available > length) {
            available = length;
        }
        if(dest) {
            memcpy(dest, block->data + block->start, available);
            dest += available;
            buf->size -= available;
        }

        free(block->data);
        free(block);
        buf->head = next;
    }
    
    if(!buf->head) {
        buf->tail = NULL;
    }
    
    
    
    return original_length;
}

int bytebuf_take_head(bytebuf* buf, char* dest, int length) {
    //printf("BUF bytebuf_take_head size: %i - %i\n", buf->size, length);
	return bytebuf_remove_head_internal(buf, length, dest);
}

int bytebuf_take_head_block(bytebuf* buf, char* dest, int length) {
    //printf("BUF bytebuf_take_head_block size: %i - %i\n", buf->size, length);
    return bytebuf_remove_head_block_internal(buf, length, dest);
}

int bytebuf_remove_head(bytebuf* buf, int length) {
	return bytebuf_remove_head_internal(buf, length, NULL);
}

int bytebuf_size(bytebuf* buf) {
    //printf("BUF size: %i\n", buf->size);
	return buf->size;
}

int bytebuf_next_block_size(bytebuf* buf) {
    bytebuf_block* next = buf->head;
    if (!next) {
        return 0;
    }
    return next->size;
}

void bytebuf_clear(bytebuf* buf) {
	while(buf->head) {
		bytebuf_block* next = buf->head->next;
		free(buf->head->data);
		free(buf->head);
		buf->head = next;
	}
	buf->tail = NULL;
	buf->size = 0;
}

char* bytebuf_convert_to_bytes(bytebuf* buf, int* length) {
	char* bytes = malloc(buf->size);
	if(bytes) {
		char* dst = bytes;
		bytebuf_block* block = buf->head;
		if (length) *length = buf->size;
		while(block) {
			bytebuf_block* next = block->next;
			memcpy(dst, block->data + block->start, block->end - block->start);
			dst += (block->end - block->start);
			free(block->data);
			free(block);
			block = next;
		}
		free(buf);
		return bytes;
	} else {
		*length = -1;
		return NULL;
	}
}

/* Return 1 if found in `block` at `offset` (which must be >= start), 0 otherwise */
static int bytebuf_find_bytes_here(bytebuf_block* block, int offset, char* bytes, int bytes_length) {
	int l;
	for(l=0; l<bytes_length; l++) {
		if(!block || block->data[offset] != bytes[l]) {
			return 0;
		}
		if(++offset >= block->end) {
			block = block->next;
			offset = block ? block->start : 0;
		}
	}
	return 1;
}

int bytebuf_find_bytes(bytebuf* buf, char* bytes, int bytes_length) {
	int pos = 0;
	bytebuf_block* scan = buf->head;

	while(scan) {
		int l=0;
		for(l=scan->start; l<scan->end; l++) {
			if(bytebuf_find_bytes_here(scan, l, bytes, bytes_length)) {
				return pos;
			}
			pos++;
		}
		scan = scan->next;
	}

	return -1;
}

void bytebuf_dispose(bytebuf* buf) {
	bytebuf_block* block = buf->head;

	while(block) {
		bytebuf_block* next = block->next;
		free(block->data);
		free(block);
		block = next;
	}

	free(buf);
}

void bytebuf_dump(bytebuf* buf, int hex) {
    int len = bytebuf_size(buf);
    int len_out = (hex) ? len : len + 1;
    char *str = malloc(len_out);
    bytebuf_copy_head(buf, str, len);
    str[len_out] = 0;
    
    if (hex) {
        
        for (int i = 0; i < len_out; i++) {
            printf(" %02x", ((unsigned char *) str) [i]);
        }

        printf("\n");
    } else {
        printf("%s\n", str);
    }
    free(str);
}
