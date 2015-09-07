/*
 * bytebuf.h
 *
 *  Created on: Jan 30, 2014
 *      Author: jeroen
 */

#ifndef BYTEBUF_H_
#define BYTEBUF_H_

typedef struct bytebuf_block_t {
	char* data;
    int size;
    int capacity; /* Number of bytes allocated in 'data' */
	int start;     /* Start of the region currently used */
	int end;	   /* End (exclusive) of the region currently used */
	               /* The block is fully used when start == 0 and end == capacity */
	struct bytebuf_block_t* next; /* Pointing towards tail */
} bytebuf_block;

typedef struct {
	int size;
    int block_size;
	bytebuf_block* head;
	bytebuf_block* tail;
} bytebuf;

/*
 * Returns NULL if not enough memory is available.
 */
bytebuf* bytebuf_create(void);

bytebuf* bytebuf_create_with_data(char* data, int length);

/*
 * Returns the actual number of bytes appended. If no memory could be
 * allocated, this will be less than length.
 */
int bytebuf_append_bytes(bytebuf* buf, const void* data, int length);
int bytebuf_append_bytes_new_block(bytebuf* buf, const void* data, int length);

/*
 * Appends the given bytes to the bytebuf, but does not copy them. Therefore,
 * the given memory must not be deallocated - it is deallocated automatically
 * when the bytebuf is disposed.
 * Returns the number of bytes appended, or 0 if no memory could be allocated
 * (in which case `data` _must_ be deallocated by the caller, since it could not
 * be incorporated in the bytebuf).
 */
int bytebuf_append_bytes_no_copy(bytebuf* buf, char* data, int length);

/*
 * Returns the actual number of bytes copied. This may be less than length
 * if not enough data is available.
 */
int bytebuf_copy_head(bytebuf* buf, char* dest, int length);

/*
 * Returns the actual number of bytes taken. This may be less than length
 * if not enough data is available.
 */
int bytebuf_take_head(bytebuf* buf, char* dest, int length);

int bytebuf_take_head_block(bytebuf* buf, char* dest, int length);

/*
 * Returns the actual number of bytes removed. This may be less than length
 * if not enough data is available.
 */
int bytebuf_remove_head(bytebuf* buf, int length);

/*
 * Returns the total number of bytes in the buffer.
 */
int bytebuf_size(bytebuf* buf);

int bytebuf_next_block_size(bytebuf* buf);

/// Clears the buffer
void bytebuf_clear(bytebuf* buf);

char* bytebuf_convert_to_bytes(bytebuf* buf, int* length);

/*
 * Returns the (zero-based) index of the first occurrence of `bytes` in `buf`,
 * or -1 if the bytes cannot be found.
 */
int bytebuf_find_bytes(bytebuf* buf, char* bytes, int bytes_length);

void bytebuf_dispose(bytebuf* buf);

// For debug
void bytebuf_dump(bytebuf* buf, int hex);

#endif /* BYTEBUF_H_ */
