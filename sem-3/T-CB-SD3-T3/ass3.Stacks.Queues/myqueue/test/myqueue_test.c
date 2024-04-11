#include <string.h>
#include <stdio.h>
#include "mystack.h"
#include "myqueue.h"
#include "unity.h"
#include "testrunner.h"

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
}

void tearDown(void)
{
    // This is run after EACH test
}

void test_queue_create_delete(void)
{
}

void test_enqueue_dequeue(void)
{
}

void test_create_queue(void)
{
	int t = 1;
	QueueMeta_t *queue = myqueue_create(t);
	TEST_ASSERT_NULL(queue->stack_in->stack)
	TEST_ASSERT_NULL(queue->stack_out->stack)
}

void test_create_queue_size_too_small(void)
{
	int t = -10;
	QueueMeta_t *queue = myqueue_create(t);
	TEST_ASSERT_NULL(queue)
}

void test_queue_enqueue_once(void)
{
	int a = 1;
	int t[] = {1};
	QueueMeta_t *queue = myqueue_create(a);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	int ret = myqueue_enqueue(queue, &t[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
}

void test_queue_enqueue_multiple(void)
{
	int a = 1;
	int t[] = {1, 2, 3};
	QueueMeta_t *queue = myqueue_create(a);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	int ret = myqueue_enqueue(queue, &t[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_enqueue(queue, &t[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack->next);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_enqueue(queue, &t[2]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack->next->next);
	TEST_ASSERT_NULL(queue->stack_in->stack->next->next->next);
	TEST_ASSERT_NULL(queue->stack_out->stack);
}

void test_queue_enqueue_obj_null(void)
{
	int t = 1;
	QueueMeta_t *queue = myqueue_create(t);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack)
	int ret = myqueue_enqueue(queue, NULL);
	TEST_ASSERT_EQUAL(-1,ret);
}

void test_queue_dequeue_once(void)
{
	int a = 1;
	int t[] = {1, 2, 3};
	StackObject_t *object = NULL;
	
	QueueMeta_t *queue = myqueue_create(a);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	int ret = myqueue_enqueue(queue, &t[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
}

void test_queue_dequeue_multiple(void)
{
	int a = 1;
	int t[] = {1, 2, 3};
	StackObject_t *object = NULL;
	
	QueueMeta_t *queue = myqueue_create(a);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	int ret = myqueue_enqueue(queue, &t[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_enqueue(queue, &t[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack->next);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_out->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack->next);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	
	ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
}

void test_queue_dequeue_multiple_halfway_empty(void)
{
	int a = 1;
	int t[] = {1, 2, 3};
	StackObject_t *object = NULL;
	
	QueueMeta_t *queue = myqueue_create(a);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	int ret = myqueue_enqueue(queue, &t[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NULL(queue->stack_in->stack);
	TEST_ASSERT_NULL(queue->stack_out->stack);
	
	ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(-1, ret);	
}

void test_queue_dequence_queue_null(void)
{
	StackObject_t *object = NULL;
	QueueMeta_t *queue = NULL;
	int ret = myqueue_dequeue(queue, object);
	TEST_ASSERT_EQUAL(-1, ret);
}

int main () {
	UnityBegin();
	//MY_RUN_TEST(test_queue_create_delete);
	//MY_RUN_TEST(test_enqueue_dequeue);
	MY_RUN_TEST(test_create_queue);
	MY_RUN_TEST(test_create_queue_size_too_small);
	
	MY_RUN_TEST(test_queue_enqueue_once);
	MY_RUN_TEST(test_queue_enqueue_multiple);
	MY_RUN_TEST(test_queue_enqueue_obj_null);
	
	MY_RUN_TEST(test_queue_dequeue_once);
	MY_RUN_TEST(test_queue_dequeue_multiple);
	MY_RUN_TEST(test_queue_dequeue_multiple_halfway_empty);
	MY_RUN_TEST(test_queue_dequence_queue_null);
	
	return UnityEnd();
}
