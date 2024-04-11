ASSIGNMENT=mem_manager
ASSIGNMENT_TEST=$(ASSIGNMENT)_test

UNITY_FOLDER=./Unity
INC_DIRS=-Iproduct
TEST_INC_DIRS=$(INC_DIRS) -I$(UNITY_FOLDER)

SHARED_FILES=product/mem_manager.c 

ASSIGNMENT_FILES=$(SHARED_FILES) \
	      product/main.c

ASSIGNMENT_TEST_FILES=$(SHARED_FILES) \
	           $(UNITY_FOLDER)/unity.c \
	           test/mem_manager_test.c

HEADER_FILES=product/*.h

CC=gcc

SYMBOLS=-g  -O0 -std=c99 -Wall -Werror -pedantic
TEST_SYMBOLS=$(SYMBOLS) -DTEST

.PHONY: clean test $(ASSIGNMENT)

all: $(ASSIGNMENT)

$(ASSIGNMENT): Makefile $(ASSIGNMENT_FILES)  $(HEADER_FILES)
	$(CC) $(INC_DIRS) $(SYMBOLS) $(ASSIGNMENT_FILES) -o $(ASSIGNMENT)

$(ASSIGNMENT_TEST): Makefile $(ASSIGNMENT_TEST_FILES)  $(HEADER_FILES)
	$(CC) $(TEST_INC_DIRS) $(TEST_SYMBOLS) $(ASSIGNMENT_TEST_FILES) -o $(ASSIGNMENT_TEST)

clean:
	@rm -f $(ASSIGNMENT) $(ASSIGNMENT_TEST) out

test: $(ASSIGNMENT_TEST)
	  @./$(ASSIGNMENT_TEST)

testall: $(ASSIGNMENT)
	@./$(ASSIGNMENT) < input1.txt > out
	cmp correct1.txt out
	@./$(ASSIGNMENT) < input2.txt > out
	cmp correct2.txt out
	@./$(ASSIGNMENT) < input.txt > out
	cmp correct.txt out
