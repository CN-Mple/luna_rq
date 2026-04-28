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

#define LUNA_ALIGN_UP(x, align)		(((x) + (align) - 1) & ~((align) - 1))
#define LUNA_ALIGN_DOWN(x, align)	((x) & ~((align) - 1))

struct rq {
	uint8_t  *buffer;
	uint32_t  capacity;
	uint32_t  elem_size;
	uint8_t  *base;
	uint8_t  *w;
	uint8_t  *r;
	uint8_t  *end;
};

void luna_rq_init(struct rq *rq, uint8_t *buffer, uint32_t capacity, uint32_t size);

bool luna_rq_pop(struct rq *rq, uint8_t *data);
bool luna_rq_push(struct rq *rq, const uint8_t *data);

bool luna_rq_is_empty(const struct rq *rq);
bool luna_rq_is_full(const struct rq *rq);

#endif

#ifdef LUNA_RQ_IMPLEMENTATION

void luna_rq_init(struct rq *rq, uint8_t *buffer, uint32_t capacity, uint32_t size)
{
	LUNA_ASSERT(rq);
	LUNA_ASSERT(buffer);
	LUNA_ASSERT(size > 0);
	LUNA_ASSERT(capacity > 0);
	LUNA_ASSERT(size <= capacity);

	rq->buffer      = buffer;
	rq->capacity    = capacity;
	rq->elem_size   = LUNA_ALIGN_UP(size, sizeof(uintptr_t));

	rq->base        = LUNA_ALIGN_UP((uintptr_t)buffer, sizeof(uintptr_t));
	rq->w = rq->r   = rq->base;
	uint32_t number = (buffer + capacity - rq->base) / rq->elem_size;
	rq->end         = rq->base + rq->elem_size * number;
}

bool luna_rq_pop(struct rq *rq, uint8_t *data)
{
	LUNA_ASSERT(rq);
	LUNA_ASSERT(data);

	if (luna_rq_is_empty(rq)) {
		return false;
	}

	uint8_t *src = rq->r;
	memcpy(data, src, rq->elem_size);

	rq->r += rq->elem_size;
	if (rq->r >= rq->end) {
		rq->r = rq->base;
	}
	return true;
}

bool luna_rq_push(struct rq *rq, const uint8_t *data)
{
	LUNA_ASSERT(rq);
	LUNA_ASSERT(data);

	if (luna_rq_is_full(rq)) {
		return false;
	}
	uint8_t *dst = rq->w;
	memcpy(dst, data, rq->elem_size);

	rq->w += rq->elem_size;
	if (rq->w >= rq->end) {
		rq->w = rq->base;
	}
	return true;
}

bool luna_rq_is_empty(const struct rq *rq)
{
	LUNA_ASSERT(rq);
	return rq->r == rq->w;
}

bool luna_rq_is_full(const struct rq *rq)
{
	LUNA_ASSERT(rq);
	uint8_t *w = rq->w + rq->elem_size;
	if (w >= rq->end) {
		w = rq->base;
	}
	return w == rq->r;
}

#endif

