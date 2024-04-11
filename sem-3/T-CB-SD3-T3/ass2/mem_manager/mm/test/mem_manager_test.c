#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem_manager.h"
#include "unity.h"

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)

void setUp(void)
{
}

void tearDown(void)
{
    // This is run after EACH test
}

void test_setting_time_hours(void)
{	

}

void test_show_list_null(void)
{
	ELEMENT *List = NULL;
	showList(List);
	
}
void test_show_list_one(void)
{
	ELEMENT *List = NULL;
	add_node(&List, 100, 1);
	showList(List);
}
    
void test_show_list_multiple(void)
{
	ELEMENT *List = NULL;
	add_node(&List, 80, 21);
	add_node(&List, 10, 1);
	showList(List);
}

void test_claim_memory_no_alloc_once(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 100, 1);
	
	int ret = claimMemory(&fList, &aList, 10);
	
	TEST_ASSERT_EQUAL(1, ret);
	TEST_ASSERT_EQUAL(90, fList->size);
	TEST_ASSERT_EQUAL(11, fList->addr);
	TEST_ASSERT_NULL(fList->next);
	
	TEST_ASSERT_EQUAL(1, aList->addr);
	TEST_ASSERT_EQUAL(10, aList->size);
	TEST_ASSERT_NULL(aList->next);
}

void test_claim_memory_no_alloc_multiple(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 100, 1);
	
	int ret = claimMemory(&fList, &aList, 10);
	TEST_ASSERT_EQUAL(1, ret);
	
	ret = claimMemory(&fList, &aList, 20);
	TEST_ASSERT_EQUAL(11, ret);
	
	TEST_ASSERT_EQUAL(70, fList->size);
	TEST_ASSERT_EQUAL(31, fList->addr);
	TEST_ASSERT_NULL(fList->next);
	
	TEST_ASSERT_EQUAL(1, aList->addr);
	TEST_ASSERT_EQUAL(10, aList->size);
	TEST_ASSERT_EQUAL(11, aList->next->addr);
	TEST_ASSERT_EQUAL(20, aList->next->size);
	TEST_ASSERT_NULL(aList->next->next);
}

void test_claim_memory_multiple_alloc(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 100, 1);
	
	int ret = claimMemory(&fList, &aList, 10);
	
	TEST_ASSERT_EQUAL(1, ret);
	TEST_ASSERT_EQUAL(90, fList->size);
	TEST_ASSERT_EQUAL(11, fList->addr);
	TEST_ASSERT_NULL(fList->next);
	
	TEST_ASSERT_EQUAL(1, aList->addr);
	TEST_ASSERT_EQUAL(10, aList->size);
	TEST_ASSERT_NULL(aList->next);
	
	ret = claimMemory(&fList, &aList, 10);
	TEST_ASSERT_EQUAL(11, ret);
	TEST_ASSERT_EQUAL(80, fList->size);
	TEST_ASSERT_EQUAL(21, fList->addr);
	TEST_ASSERT_NULL(fList->next);
	
	TEST_ASSERT_EQUAL(1, aList->addr);
	TEST_ASSERT_EQUAL(10, aList->size);
	TEST_ASSERT_EQUAL(11, aList->next->addr);
	TEST_ASSERT_EQUAL(10, aList->next->size);
	TEST_ASSERT_NULL(aList->next->next);
}

void test_claim_memory_no_free(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 10, 91);
	
	int ret = claimMemory(&fList, &aList, 15);
	
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_claim_memory_flist_null(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	int ret = claimMemory(&fList, &aList, 15);
	
	TEST_ASSERT_EQUAL(-1, ret);
}

void test_free_memory_one_alloc_one_flist(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&aList, 80, 21);
	add_node(&fList, 20, 1);
	
	int ret = freeMemory(&fList, &aList, 21);
	TEST_ASSERT_EQUAL(80, ret);

	TEST_ASSERT_EQUAL(100, fList->size);
	TEST_ASSERT_EQUAL(1, fList->addr);
	TEST_ASSERT_NULL(fList->next);
}

void test_free_memory_multiple_alloc_one_flist(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&aList, 80, 21);
	add_node(&aList, 10, 1);
	add_node(&fList, 10, 11);
	
	int ret = freeMemory(&fList, &aList, 1);
	TEST_ASSERT_EQUAL(10, ret);

	ret = freeMemory(&fList, &aList, 21);
	TEST_ASSERT_EQUAL(80, ret);
	
	TEST_ASSERT_EQUAL(100, fList->size);
	TEST_ASSERT_EQUAL(1, fList->addr);
	TEST_ASSERT_NULL(fList->next);
}

void test_free_memory_one_alloc_multiple_flist(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 20, 1);
	add_node(&fList, 60, 41);
	add_node(&aList, 20, 21);
	
	int ret = freeMemory(&fList, &aList, 21);
	TEST_ASSERT_EQUAL(20, ret);

	TEST_ASSERT_EQUAL(100, fList->size);
	TEST_ASSERT_EQUAL(1, fList->addr);
	TEST_ASSERT_NULL(fList->next);
}

void test_free_memory_multiple_alloc_multiple_flist_once(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 30, 71);
	add_node(&fList, 20, 41);
	add_node(&fList, 20, 1);
	add_node(&aList, 10, 61);
	add_node(&aList, 20, 21);
	
	int ret = freeMemory(&fList, &aList, 21);
	TEST_ASSERT_EQUAL(20, ret);
	
	TEST_ASSERT_EQUAL(60, fList->size);
	TEST_ASSERT_EQUAL(1, fList->addr);
	TEST_ASSERT_EQUAL(30, fList->next->size);
	TEST_ASSERT_EQUAL(71, fList->next->addr);
	TEST_ASSERT_NULL(fList->next->next);
}

void test_free_memory_multiple_alloc_multiple_flist_multiple(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 30, 71);
	add_node(&fList, 20, 41);
	add_node(&fList, 20, 1);
	add_node(&aList, 10, 61);
	add_node(&aList, 20, 21);
	
	int ret = freeMemory(&fList, &aList, 21);
	TEST_ASSERT_EQUAL(20, ret);
	
	ret = freeMemory(&fList, &aList, 61);
	TEST_ASSERT_EQUAL(10, ret);
	
	TEST_ASSERT_EQUAL(100, fList->size);
	TEST_ASSERT_EQUAL(1, fList->addr);
	TEST_ASSERT_NULL(fList->next);
}

void test_free_memory_alloc_null(void)
{
	ELEMENT *aList = NULL;
	ELEMENT *fList = NULL;
	
	add_node(&fList, 100, 1);
	
	int ret = freeMemory(&fList, &aList, 1);
	TEST_ASSERT_EQUAL(-1, ret);
}

int main (int argc, char * argv[])
{
    UnityBegin();

    MY_RUN_TEST(test_setting_time_hours);
    
    MY_RUN_TEST(test_show_list_null);
	MY_RUN_TEST(test_show_list_one);
	MY_RUN_TEST(test_show_list_multiple);
	MY_RUN_TEST(test_claim_memory_no_alloc_once);
	MY_RUN_TEST(test_claim_memory_no_alloc_multiple);
	MY_RUN_TEST(test_claim_memory_multiple_alloc);
	MY_RUN_TEST(test_claim_memory_no_free);
	MY_RUN_TEST(test_claim_memory_flist_null);
	MY_RUN_TEST(test_free_memory_one_alloc_one_flist);
	MY_RUN_TEST(test_free_memory_multiple_alloc_one_flist);
	MY_RUN_TEST(test_free_memory_one_alloc_multiple_flist);
	MY_RUN_TEST(test_free_memory_multiple_alloc_multiple_flist_once);
	MY_RUN_TEST(test_free_memory_multiple_alloc_multiple_flist_multiple);
	MY_RUN_TEST(test_free_memory_alloc_null);

    return UnityEnd();
}
