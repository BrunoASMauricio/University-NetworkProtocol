
#include "data.h"

queue*
newQueue()
{
	queue* q = (queue*)malloc(sizeof(queue));
	q->size = 0;
	q->first = NULL;
	q->last= NULL;
	if (pthread_mutex_init(&(q->lock), NULL) != 0) {
        fatalErr("mutex init failed for outbound lock\n");
    }
	return q;
}

/*
 * The reason for using linked lists instead of arrays
 * Is to allow for prioritized packets to "jump ahead" easily
 * If the memory allocation proves to be too slow or non-deterministic,
 * a pool of pre-allocated queue_el can be implemented
 */
void
addToQueue(void* packet, int size, queue* q, int pr)
{
	queue_el* new_el = (queue_el*)malloc(sizeof(queue_el));
	queue_el *last_higher;
	queue_el *to_swap;

	new_el->packet = packet;
	new_el->packet_size = size;
	new_el->pr = pr;
	new_el->next_el = NULL;

	pthread_mutex_lock(&(q->lock));
	to_swap = q->first;
	last_higher = q->first;

	while (to_swap != NULL && to_swap->pr >= pr) {
		last_higher = to_swap;
		to_swap = (queue_el*)to_swap->next_el;
	}

	if (q->first == NULL) {
		q->first = new_el;
		q->last = new_el;
	} else {
		if (to_swap == last_higher) {
			new_el->next_el= last_higher;
			q->first = new_el;
		} else {
			if (to_swap == NULL) {
				q->last = new_el;
			}
			new_el->next_el = last_higher->next_el;
			last_higher->next_el = new_el;
		}
	}
	q->size += 1;
	pthread_mutex_unlock(&(q->lock));
}


void*
popFromQueue(int* size, queue* q)
{	
	queue_el* popped;
	void* buf;
	
	if (q->size == 0) {
		return 0;
	}

	pthread_mutex_lock(&(q->lock));
	popped = q->first;
	q->first = (queue_el*)q->first->next_el;
	q->size -= 1;
	pthread_mutex_unlock(&(q->lock));

	buf = popped->packet;
	*size = popped->packet_size;

	free(popped);

	return buf;
}

void
delQueue(queue* q)
{
	pthread_mutex_destroy(&(q->lock));
	free(q);
}




