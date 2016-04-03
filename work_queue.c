#include "work_queue.h"

void *pop (struct queue *q)
{
	struct node *tmp;
	void *retn;
	
	if (q->head == NULL)
		return NULL;
	
	retn = q->head->data;
	tmp = q->head;
	q->head = q->head->next;
	
	free(tmp);
	
	return retn;
}

int push (struct queue *q, void *data)
{
	struct node *new;
	
	if ((new = malloc(sizeof(struct node))) == NULL)
		return -1;
	
	new->data = data;
	new->next = NULL;
	
	if (q->tail != NULL)
		q->tail->next = new;
	q->tail = new;
	
	if (q->head == NULL)
		q->head = new;
	
	return 0;
}

void write_queue (struct work_queue *q, void *data)
{
	if (q->cancel)
		return;
	
	pthread_mutex_lock(&q->mutex);	
	
	push(q->queue, data);
	
	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);
}

void *read_queue (struct work_queue *q)
{
	void *retn;
	
	pthread_mutex_lock(&q->mutex);
	
	while (q->queue->head == NULL && !q->cancel)
		pthread_cond_wait(&q->cond, &q->mutex);
	
   	if (q->cancel)	
		retn = NULL;
   	else  		
		retn = pop(q->queue);
   	
   	pthread_mutex_unlock(&q->mutex);
   	
   	return retn;
}

void *worker (void *parent)
{
	struct work_queue *q = parent;
	void *data, *retn;
	
	while ((data = read_queue(q)) && !q->cancel) {
		if ((retn = q->func(data)) != NULL) {
			write_queue(q->returns, retn);
		}
	}
	
	return NULL;
}

void init_queue (struct work_queue **qp, void *(*func)(void *))
{
	*qp = malloc(sizeof(struct work_queue));
	
	(*qp)->cancel = 0;
	(*qp)->func = func;
	(*qp)->queue = calloc(sizeof(struct queue), 1);
	
	if (func) 
		init_queue(&(*qp)->returns, NULL);
	
	pthread_mutex_init(&(*qp)->mutex, NULL);
	pthread_cond_init(&(*qp)->cond, NULL);
}


void cancel_queue (struct work_queue *q)
{
	pthread_mutex_lock(&q->mutex);
	
	q->cancel = 1;
	while (pop(q->queue) != NULL)
		;
	
	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);
	
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
	
	pthread_join(q->thread, NULL);
	
	free(q);
}
