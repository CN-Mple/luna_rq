/* luna_rq.h */
#ifndef LUNA_RQ_H
#define LUNA_RQ_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef LUNA_ASSERT
#include <assert.h>
#define LUNA_ASSERT			assert
#endif

#define LUNA_RQ_ALIGNMENT		(8U)
#define LUNA_RQ_ALIGN_UP(addr, align)	(((addr) + (align - 1U)) & ~(align - 1U))

struct luna_rq {
        uint8_t *buffer;
        size_t item_size;
        size_t w;
        size_t r;
        size_t capacity;
};

void luna_rq_init(struct luna_rq *rq, uint8_t *bufferfer, size_t bufferfer_size, size_t item_size);

bool luna_rq_is_empty(struct luna_rq *rq);
bool luna_rq_is_full(struct luna_rq *rq);

size_t luna_rq_get_used(struct luna_rq *rq);
size_t luna_rq_get_free(struct luna_rq *rq);
size_t luna_rq_capacity(struct luna_rq *rq);

size_t luna_rq_pop(struct luna_rq *rq, void *item);
size_t luna_rq_push(struct luna_rq *rq, const void *item);
size_t luna_rq_peek(struct luna_rq *rq, void *item);

size_t luna_rq_read(struct luna_rq *rq, void *items, size_t num);
size_t luna_rq_write(struct luna_rq *rq, const void *items, size_t num);

size_t luna_rq_drop(struct luna_rq *rq, size_t len);
size_t luna_rq_wipe(struct luna_rq *rq);

#endif

#ifdef LUNA_RQ_IMPLEMENTATION

void luna_rq_init(struct luna_rq *rq, uint8_t *buffer, size_t bufferfer_size, size_t item_size)
{
	LUNA_ASSERT(rq);
	LUNA_ASSERT(buffer);
	LUNA_ASSERT(item_size > 0U);

	uintptr_t buffer_origin = (uintptr_t)buffer;
	uintptr_t aligned_addr = LUNA_RQ_ALIGN_UP(buffer_origin, LUNA_RQ_ALIGNMENT);
	size_t offset = aligned_addr - buffer_origin;

	size_t avail_bytes = bufferfer_size - offset;
	LUNA_ASSERT(avail_bytes > 0U);

	LUNA_ASSERT((aligned_addr % item_size) == 0U);

	size_t capacity = avail_bytes / item_size;
	LUNA_ASSERT(capacity >= 2U);

	rq->buffer         = (uint8_t *)aligned_addr;
	rq->item_size   = item_size;
	rq->w           = 0U;
	rq->r           = 0U;
	rq->capacity    = capacity;
}

bool luna_rq_is_empty(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return rq->w == rq->r;
}

bool luna_rq_is_full(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return ((rq->w + 1) % rq->capacity) == rq->r;
}

size_t luna_rq_get_used(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return (rq->w - rq->r + rq->capacity) % rq->capacity;
}

size_t luna_rq_get_free(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return (rq->r - rq->w - 1 + rq->capacity) % rq->capacity;
}

size_t luna_rq_capacity(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return rq->capacity - 1;
}

size_t luna_rq_pop(struct luna_rq *rq, void *item)
{
        LUNA_ASSERT(rq);
        LUNA_ASSERT(item);
        if (luna_rq_is_empty(rq)) {
                return 0;
        }
        uint8_t *src = rq->buffer + rq->r * rq->item_size;
        memcpy(item, src, rq->item_size);
        rq->r = (rq->r + 1) % rq->capacity;
        return 1;
}

size_t luna_rq_push(struct luna_rq *rq, const void *item)
{
        LUNA_ASSERT(rq);
        LUNA_ASSERT(item);
        if (luna_rq_is_full(rq)) {
                return 0;
        }
        uint8_t *dst = rq->buffer + rq->w * rq->item_size;
        memcpy(dst, item, rq->item_size);
        rq->w = (rq->w + 1) % rq->capacity;
        return 1;
}

size_t luna_rq_peek(struct luna_rq *rq, void *item)
{
        LUNA_ASSERT(rq);
        LUNA_ASSERT(item);
        if (luna_rq_is_empty(rq)) {
                return 0;
        }
        uint8_t *src = rq->buffer + rq->r * rq->item_size;
        memcpy(item, src, rq->item_size);
        return 1;
}

size_t luna_rq_read(struct luna_rq *rq, void *items, size_t num)
{
        LUNA_ASSERT(rq);
        LUNA_ASSERT(items);
        if (num == 0)
                return 0;

        size_t used = luna_rq_get_used(rq);
        size_t read_num = num < used ? num : used;

        size_t remain = rq->capacity - rq->r;
        uint8_t *out = (uint8_t *)items;

        if (read_num <= remain) {
                memcpy(out, rq->buffer + rq->r * rq->item_size, read_num * rq->item_size);
        } else {
                memcpy(out, rq->buffer + rq->r * rq->item_size, remain * rq->item_size);
                memcpy(out + remain * rq->item_size, rq->buffer, (read_num - remain) * rq->item_size);
        }
        rq->r = (rq->r + read_num) % rq->capacity;
        return read_num;
}

size_t luna_rq_write(struct luna_rq *rq, const void *items, size_t num)
{
        LUNA_ASSERT(rq);
        LUNA_ASSERT(items);
        if (num == 0)
                return 0;

        size_t free = luna_rq_get_free(rq);
        size_t write_num = num < free ? num : free;
        size_t remain = rq->capacity - rq->w;
        const uint8_t *in = (const uint8_t *)items;

        if (write_num <= remain) {
                memcpy(rq->buffer + rq->w * rq->item_size, in, write_num * rq->item_size);
        } else {
                memcpy(rq->buffer + rq->w * rq->item_size, in, remain * rq->item_size);
                memcpy(rq->buffer, in + remain * rq->item_size, (write_num - remain) * rq->item_size);
        }
        rq->w = (rq->w + write_num) % rq->capacity;
        return write_num;
}

size_t luna_rq_drop(struct luna_rq *rq, size_t len)
{
        LUNA_ASSERT(rq);
        size_t used = luna_rq_get_used(rq);
        if (len > used)
                len = used;
        rq->r = (rq->r + len) % rq->capacity;
        return len;
}

size_t luna_rq_wipe(struct luna_rq *rq)
{
        LUNA_ASSERT(rq);
        return luna_rq_drop(rq, luna_rq_get_used(rq));
}

#endif
