/* luna_queue.h */
#ifndef LUNA_QUEUE_H
#define LUNA_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#ifndef LUNA_ASSERT
#include <assert.h>
#define LUNA_ASSERT		assert
#endif

struct queue {
	uint32_t **ring;
	uint32_t   end;
	uint32_t   head;
	uint32_t   tail;
	uint32_t   nfree;
};

void luna_queue_init(struct queue *queue, uint32_t *ring[], uint32_t len);

uint32_t *luna_queue_pop(struct queue *queue);
void luna_queue_push(struct queue *queue, uint32_t *data);

bool luna_queue_is_empty(struct queue *queue);
bool luna_queue_is_full(struct queue *queue);

#endif

#define LUNA_QUEUE_IMPLEMENTATION
#ifdef LUNA_QUEUE_IMPLEMENTATION

void luna_queue_init(struct queue *queue, uint32_t *ring[], uint32_t len)
{
	LUNA_ASSERT(queue);
	LUNA_ASSERT(ring);
	LUNA_ASSERT(len >= 2);

	queue->head  = 0;
	queue->tail  = 0;
	queue->end   = len;
	queue->nfree = len;
	queue->ring  = &ring[0];
}

uint32_t *luna_queue_pop(struct queue *queue)
{
	LUNA_ASSERT(queue);

	uint32_t *data = 0;
	if (queue->nfree != queue->end) {
		data = queue->ring[queue->tail];
		if (queue->tail == 0) {
			queue->tail = queue->end;
		}
		--queue->tail;

		++queue->nfree;
	}
	return data;
}

void luna_queue_push(struct queue *queue, uint32_t *data)
{
	LUNA_ASSERT(queue);
	LUNA_ASSERT(data);

	LUNA_ASSERT(queue->nfree != 0);
	queue->ring[queue->head] = data;
	if (queue->head == 0) {
		queue->head = queue->end;
	}
	--queue->head;

	--queue->nfree;
}

bool luna_queue_is_empty(struct queue *queue)
{
	LUNA_ASSERT(queue);
	return queue->nfree == queue->end;
}

bool luna_queue_is_full(struct queue *queue)
{
	LUNA_ASSERT(queue);
	return queue->nfree == 0;
}

#endif

