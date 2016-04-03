#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct node {
	void *data;
	struct node *next;
};

struct queue {
	struct node *head;
	struct node *tail;
};

struct work_queue {
	struct work_queue *returns;
	struct queue *queue;
	
	void *(*func)(void *);
	
	pthread_t thread;
	
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	
	volatile int cancel;
};

void *pop (struct queue *q);

int push (struct queue *q, void *data);

void write_queue (struct work_queue *q, void *data);

void *read_queue (struct work_queue *q);

void *worker (void *parent);

void init_queue (struct work_queue **qp, void *(*func)(void *));

void cancel_queue (struct work_queue *q);