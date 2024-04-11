#include <string.h>

#include "linkedlist.h"
#include "unity.h"

#define MY_RUN_TEST(func) RUN_TEST(func, 0)


void setUp(void)
{
    // This is run before EACH test
}

void tearDown(void)
{
  // This is run after EACH test
}


/***************************************************/
/* 				TEST CASES ADD FIRST			   */
/***************************************************/
void test_add_first_no_head(void)
{
	ITEM *list = NULL;
	int ret = 0;
	ret = add_first(&list, 1);
	TEST_ASSERT_EQUAL(0,ret);
	TEST_ASSERT_EQUAL(list->value, 1);
	clean(list);
}

void test_add_first_head(void)
{
	ITEM *list = NULL;
	int ret = add_first(&list, 1);
	
	ret = add_first(&list, 2);
	TEST_ASSERT_EQUAL(0,ret);
	TEST_ASSERT_EQUAL(list->next->value, 1);
	TEST_ASSERT_EQUAL(list->value, 2);
	clean(list);
}

void test_add_first_head_multiple(void)
{
	ITEM *list = NULL;
	int ret = add_first(&list, 1);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_first(&list, 2);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_first(&list, 5);
	TEST_ASSERT_EQUAL(0,ret);
	
	TEST_ASSERT_EQUAL(list->next->next->value, 1);
	TEST_ASSERT_EQUAL(list->next->value, 2);
	TEST_ASSERT_EQUAL(list->value, 5);
	clean(list);
}
	
void test_add_first_null(void)
{
	ITEM *list = NULL;
	int ret = add_last(NULL, 0);
	TEST_ASSERT_EQUAL(-1, ret);
	clean(list);	
}


/***************************************************/
/* 				TEST CASES ADD LAST				   */
/***************************************************/
void test_add_last_no_head(void)
{
	ITEM*list = NULL;
	int ret = add_last(&list, 5);
	
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL(5, list->value);
	clean(list);
}

void test_add_last_head(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);

	ret = add_last(&list, 2);
	TEST_ASSERT_EQUAL(0,ret);
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(2, list->next->value);
	clean(list);
}

void test_add_last_head_multiple(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_last(&list, 2);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_last(&list, 3);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_last(&list, 4);
	TEST_ASSERT_EQUAL(0,ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(2, list->next->value);
	TEST_ASSERT_EQUAL(3, list->next->next->value);
	TEST_ASSERT_EQUAL(4, list->next->next->next->value);
	TEST_ASSERT_NULL(list->next->next->next->next);
	clean(list);
	
}

void test_add_last_null(void)
{
	ITEM *list = NULL;
	int ret = add_last(NULL, 0);
	TEST_ASSERT_EQUAL(-1, ret);
	clean(list);	
}


/***************************************************/
/* 				TEST CASES ADD AFTER			   */
/***************************************************/
void test_add_after_head_start(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	ret = add_last(&list,2);
	
	ret = add_after(list, list, 3);
	TEST_ASSERT_EQUAL(0,ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(3, list->next->value);
	TEST_ASSERT_EQUAL(2, list->next->next->value);
}

void test_add_after_head_end(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	ret = add_last(&list,2);
	
	ret = add_after(list, list->next, 3);
	TEST_ASSERT_EQUAL(0,ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(2, list->next->value);
	TEST_ASSERT_EQUAL(3, list->next->next->value);
	TEST_ASSERT_NULL(list->next->next->next);
}

void test_add_after_head_multiple(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	ret = add_last(&list,2);
	
	ret = add_after(list, list->next, 3);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_after(list, list->next, 4);
	TEST_ASSERT_EQUAL(0,ret);
	
	ret = add_after(list, list, 5);
	TEST_ASSERT_EQUAL(0,ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(5, list->next->value);
	TEST_ASSERT_EQUAL(2, list->next->next->value);
	TEST_ASSERT_EQUAL(4, list->next->next->next->value);
	TEST_ASSERT_EQUAL(3, list->next->next->next->next->value);
}

void test_add_after_no_head(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	ret = add_last(&list,2);
	
	ret = add_after(NULL, list->next, 3);
	TEST_ASSERT_EQUAL(-1,ret);
}

void test_add_after_c_item_null(void)
{
	ITEM *list = NULL;
	int ret = add_last(&list, 1);
	ret = add_last(&list,2);
	
	ret = add_after(list, NULL, 3);
	TEST_ASSERT_EQUAL(-1,ret);
}


/***************************************************/
/* 				TEST CASES REM FIRST			   */
/***************************************************/
void test_rem_first_head(void)
{
	ITEM *list = NULL;
	add_last(&list, 1);
	add_last(&list, 2);
	
	int ret = rem_first(&list);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL(2, list->value);
	clean(list);	
}
void test_rem_first_head_multiple(void)
{
	ITEM *list = NULL;
	add_last(&list, 1);
	add_last(&list, 3);
	add_last(&list, 2);
	add_last(&list, 4);
	
	int ret = rem_first(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	ret = rem_first(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	ret = rem_first(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	TEST_ASSERT_EQUAL(4, list->value);
	clean(list);	
}

void test_rem_first_null(void)
{
	ITEM *list = NULL;
	
	int ret = rem_first(&list);
	TEST_ASSERT_EQUAL(-1, ret);
	clean(list);	
}


/***************************************************/
/* 				TEST CASES REM LAST				   */
/***************************************************/
void test_rem_last_head(void)
{
	ITEM *list = NULL;
	add_last(&list, 1);
	add_last(&list, 2);
	
	int ret = rem_last(&list);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_NULL(list->next);
	clean(list);	
}

void test_rem_last_head_multiple(void)
{
	ITEM *list = NULL;
	add_last(&list, 1);
	add_last(&list, 3);
	add_last(&list, 2);
	add_last(&list, 4);
	
	int ret = rem_last(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(3, list->next->value);
	TEST_ASSERT_EQUAL(2, list->next->next->value);
	TEST_ASSERT_NULL(list->next->next->next);
	
	ret = rem_last(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(3, list->next->value);
	TEST_ASSERT_NULL(list->next->next)
	
	ret = rem_last(&list);
	TEST_ASSERT_EQUAL(0, ret);
	
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_NULL(list->next);
	clean(list);	
}

void test_rem_last_null(void)
{
	ITEM *list = NULL;
	
	int ret = rem_last(&list);
	TEST_ASSERT_EQUAL(-1, ret);
	clean(list);	
}

/***************************************************/
/* 				TEST CASES REM AFTER			   */
/***************************************************/
void test_rem_after_head(void)
{
	ITEM *list = NULL;
	add_last(&list, 1);
	add_last(&list, 2);
	add_last(&list, 3);
	
	int ret = rem_after(list, list);
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL(1, list->value);
	TEST_ASSERT_EQUAL(3, list->next->value);
	TEST_ASSERT_NULL(list->next->next);
	
	clean(list);	
}

void test_rem_after_no_head(void)
{
	ITEM *list = NULL;
	
	int ret = rem_after(NULL, list);
	TEST_ASSERT_EQUAL(-1, ret);	
	clean(list);	
}

void test_rem_after_c_item_not_found(void)
{
	ITEM *list = NULL;
	
	int ret = rem_after(list, list);
	TEST_ASSERT_EQUAL(-1, ret);	
	clean(list);	
}

void test_rem_after_c_item_last(void)
{
	ITEM *list = NULL;
	
	int ret = rem_after(list, list);
	TEST_ASSERT_EQUAL(-1, ret);	
	clean(list);	
}



int main (int argc, char * argv[])
{
    UnityBegin();

	// Testing Add First
    MY_RUN_TEST(test_add_first_no_head);
    MY_RUN_TEST(test_add_first_head);
    MY_RUN_TEST(test_add_first_head_multiple);
   	MY_RUN_TEST(test_add_first_null);
    
    // Testing Add Last
    MY_RUN_TEST(test_add_last_no_head);
	MY_RUN_TEST(test_add_last_head);
	MY_RUN_TEST(test_add_last_head_multiple);
	MY_RUN_TEST(test_add_last_null);
		
	// Testing Add After
	MY_RUN_TEST(test_add_after_head_start);
	MY_RUN_TEST(test_add_after_head_end);
	MY_RUN_TEST(test_add_after_head_multiple);
	MY_RUN_TEST(test_add_after_no_head);
	MY_RUN_TEST(test_add_after_c_item_null);
	
	// Testing Rem First
	MY_RUN_TEST(test_rem_first_head);
	MY_RUN_TEST(test_rem_first_head_multiple);
	MY_RUN_TEST(test_rem_first_null);
		
	// Testing Rem Last
	MY_RUN_TEST(test_rem_last_head);
	MY_RUN_TEST(test_rem_last_head_multiple);
	MY_RUN_TEST(test_rem_last_null);
		
	// Testing Rem After
	MY_RUN_TEST(test_rem_after_head);
	MY_RUN_TEST(test_rem_after_no_head);
	MY_RUN_TEST(test_rem_after_c_item_not_found);
	MY_RUN_TEST(test_rem_after_c_item_last);
		
    return UnityEnd();
}
