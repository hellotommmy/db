/* queue.h: queue implemented with a circular, doubly-linked list with a
   sentinel */

#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    struct node *tail, *next;
} queue_t;

typedef struct node {
    struct node *next,*prev;
} node_t;

/* Initialize a queue */
void queue_init(queue_t * queue);

/* Remove and return the item at the front of the queue Return NULL if the
   queue is empty */
node_t *queue_get(node_t * queue);

/* Add item to the back of the queue */
void queue_put(queue_t * queue, node_t * item);

/* Determine if the queue is empty.
 * Returns 1 if the queue is empty.
 * Returns 0 otherwise.
 */
int queue_empty(node_t *queue);

/* Returns the first item in the queue
 * Returns NULL if the queue is empty
 */
node_t *queue_first(node_t *queue);

/* A comparison function should return:
 *  1 if a is less-than-or-equal-to b;
 *  0 otherwise.
 */
typedef int (*node_lte)(node_t *a, node_t *b);

/* Insert this item /elt/ into the queue /q/
 * in ascending order with the less-than-or-equal-to
 * inequality /comp/.
 * If /q/ was sorted (w.r.t. /comp/) before the
 * call, then /q/ is sorted after the call.
 * This is the simple linear-time algorithm.
 */
void queue_put_sort(node_t *q, node_t *item, node_lte comp);

#endif                          /* QUEUE_H */

