ASSIGNMENT=mystack
MYSTACK_TEST=main_test

UNITY_FOLDER=./Unity
INC_DIRS=-Iproduct
TEST_INC_DIRS=$(INC_DIRS) -I$(UNITY_FOLDER)

SHARED_FILES=product/mystack.c

MYSTACK_FILES=$(SHARED_FILES) product/main.c

MYSTACK_TEST_FILES=$(SHARED_FILES) \
	           $(UNITY_FOLDER)/unity.c \
	           test/mystack_test.c

HEADER_FILES=product/*.h

CC=gcc

SYMBOLS=-g -O0 -std=c99 -Wall -Werror -pedantic
#SYMBOLS=-g  -O0 -std=c99
TEST_SYMBOLS=$(SYMBOLS) -DTEST

.PHONY: clean test mystack

all: $(ASSIGNMENT)

$(ASSIGNMENT): Makefile $(MYSTACK_FILES)  $(HEADER_FILES)
	$(CC) $(INC_DIRS) $(SYMBOLS) $(MYSTACK_FILES) -o $(ASSIGNMENT)

$(MYSTACK_TEST): Makefile $(MYSTACK_TEST_FILES)  $(HEADER_FILES)
	$(CC) $(TEST_INC_DIRS) $(TEST_SYMBOLS) $(MYSTACK_TEST_FILES) -o $(MYSTACK_TEST)

clean:
	@rm -f $(ASSIGNMENT) $(MYSTACK_TEST)

test: $(MYSTACK_TEST)
	  @./$(MYSTACK_TEST)

testrun: $(TEST)
		@groovy $(UNITY_FOLDER)/testrun.groovy
