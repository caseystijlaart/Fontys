# Makefile for queue from 2 stacks library using shared library from mystack

CC = gcc 
LD = gcc
CPPFLAGS = -g -DDEBUG=0
UNITY_FOLDER=./Unity
CFLAGS =  -fPIC -g -I../mystack/product -Iproduct -I$(UNITY_FOLDER)
LD_SHARE_FLAGS = -shared  
LDFLAGS = -Wl,-rpath,.
RM = rm -f  
TARGET_LIB = libmystack.so 
TEST_TARGET=myqueue_test

SRCS = ../mystack/product/mystack.c 
OBJS = $(SRCS:.c=.o)
QUE_SRCS = product/myqueue.c
TEST_SRCS = test/myqueue_test.c $(UNITY_FOLDER)/unity.c
QUE_OBJS = $(QUE_SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o) $(QUE_OBJS)

.PHONY: all clean 
mystack: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(LD) $(LD_SHARE_FLAGS) -o $@ $^

test : $(TARGET_LIB) $(TEST_OBJS)
	$(LD) $(LDFLAGS) -o $(TEST_TARGET) $^ -L. -lmystack
	@./$(TEST_TARGET)

clean:
	-$(RM) $(TARGET_LIB) $(OBJS) $(QUE_OBJS) $(TEST_OBJS) $(TEST_TARGET)
