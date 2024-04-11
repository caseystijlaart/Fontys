#include <string.h>
#include <stdio.h>

#include "mystack.h"
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

void run_my_test()
{
	printf("Please specify your tests\n");
}

void test_stack_create(void)
{
	size_t t = 1;
	StackMeta_t *stack = mystack_create(t);
	TEST_ASSERT_NULL(stack->stack)
}

void test_stack_push_element_array(void)
{
	size_t t = 1;
	int arr[] = {1};
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
}

void test_stack_push_multiple_element_array(void)
{
	size_t t = 1;
	int arr[] = {1, 2, 3};
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	
	ret = mystack_push(stack, &arr[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next);
	
	ret = mystack_push(stack, &arr[2]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next->next);
}

void test_stack_push_obj_null(void)
{
	size_t t = 1;
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, NULL);
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_stack_push_stack_null(void)
{
	int arr[] = {1};
	StackMeta_t *stack = NULL;
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_stack_pop_one(void)
{
	size_t t = 1;
	int arr[] = {1, 2};
	StackObject_t *object = NULL;
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	
	ret = mystack_push(stack, &arr[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next);
	
	ret = mystack_pop(stack, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	TEST_ASSERT_NULL(stack->stack->next);
	
}

void test_stack_pop_multiple_stack_empty(void)
{
	size_t t = 1;
	int arr[] = {1, 2};
	StackObject_t *object = NULL;
	StackObject_t *object1 = NULL;
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	
	ret = mystack_push(stack, &arr[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next);
	
	ret = mystack_pop(stack, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	TEST_ASSERT_NULL(stack->stack->next);
	
	ret = mystack_pop(stack, object1);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NULL(stack->stack);
}

void test_stack_pop_multiple_stack_halfway_empty(void)
{
	size_t t = 1;
	int arr[] = {1};
	StackObject_t *object = NULL;
	StackObject_t *object1 = NULL;
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	
	ret = mystack_pop(stack, object);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NULL(stack->stack);
	
	ret = mystack_pop(stack, object1);
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_stack_pop_stack_null(void)
{
	StackObject_t *object = NULL;
	StackMeta_t *stack = NULL;
	
	int ret = mystack_pop(stack, object);
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_stack_one_numelem(void)
{
	size_t t = 1;
	int arr[] = {1};
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	TEST_ASSERT_EQUAL(stack->numelem, 1);
}

void test_stack_multiple_numelem(void)
{
	size_t t = 1;
	int arr[] = {1,2,3};
	
	StackMeta_t *stack = mystack_create(t);
	int ret = mystack_push(stack, &arr[0]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack);
	
	ret = mystack_push(stack, &arr[1]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next);
	
	ret = mystack_push(stack, &arr[2]);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_NOT_NULL(stack->stack->next->next);
	
	TEST_ASSERT_EQUAL(stack->numelem, 3);
}


int main (int argc, char * argv[]) {
	UnityBegin();
	MY_RUN_TEST(run_my_test);
	MY_RUN_TEST(test_stack_create);

	MY_RUN_TEST(test_stack_push_element_array);
	MY_RUN_TEST(test_stack_push_multiple_element_array);
	MY_RUN_TEST(test_stack_push_obj_null);
	MY_RUN_TEST(test_stack_push_stack_null);
	
	MY_RUN_TEST(test_stack_pop_one);
	MY_RUN_TEST(test_stack_pop_multiple_stack_empty);
	MY_RUN_TEST(test_stack_pop_multiple_stack_halfway_empty);
	MY_RUN_TEST(test_stack_pop_stack_null);
	
	MY_RUN_TEST(test_stack_one_numelem);
	MY_RUN_TEST(test_stack_multiple_numelem);
	
	
	
	return UnityEnd();
}
