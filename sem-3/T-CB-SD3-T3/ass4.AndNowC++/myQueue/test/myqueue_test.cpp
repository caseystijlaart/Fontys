#include "myQueue.h"
#include "myStack.h"
#include "gtest/gtest.h"
 
class queueTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
  }
 
  virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
  }
};
 
 int t[] = {1, 3, 2, 43, 432, 45, 65, 2, 54};

TEST_F(queueTest,enqueueOnce){
    Queue queue;
    EXPECT_EQ(0,queue.enqueue(&t[0]));
}

TEST_F(queueTest,enqueueMultiple){
    Queue queue;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[0]));
    EXPECT_EQ(0,queue.enqueue(&t[1]));
    EXPECT_EQ(0,queue.enqueue(&t[3]));
    EXPECT_EQ(0,queue.enqueue(&t[2]));
    EXPECT_EQ(0,queue.enqueue(&t[6]));
    
}

TEST_F(queueTest,enqueueNULL){
    Queue queue;
    EXPECT_EQ(-1,queue.enqueue(0));
}

TEST_F(queueTest,dequeueOnce){
    Queue queue;
    StackObject_t *object = NULL;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[3]));
    EXPECT_EQ(0,queue.enqueue(&t[7]));
    EXPECT_EQ(0,queue.dequeue(object));
}

TEST_F(queueTest,dequeueMultiple){
    Queue queue;
    StackObject_t *object = NULL;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[0]));
    EXPECT_EQ(0,queue.enqueue(&t[1]));
    EXPECT_EQ(0,queue.enqueue(&t[3]));
    EXPECT_EQ(0,queue.enqueue(&t[2]));
    EXPECT_EQ(0,queue.enqueue(&t[6]));
    EXPECT_EQ(0,queue.dequeue(object));
    EXPECT_EQ(0,queue.dequeue(object));
    EXPECT_EQ(0,queue.dequeue(object));
}

TEST_F(queueTest,dequeueTooMuch){
    Queue queue;
    StackObject_t *object = NULL;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[2]));
    EXPECT_EQ(0,queue.enqueue(&t[6]));
    EXPECT_EQ(0,queue.dequeue(object));
    EXPECT_EQ(0,queue.dequeue(object));
    EXPECT_EQ(-1,queue.dequeue(object));
}

TEST_F(queueTest,destroyOneElementInqueue){
    Queue queue;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[8]));
    EXPECT_EQ(0,queue.destroy());
}

TEST_F(queueTest,destroyMultipleElementInqueue){
    Queue queue;
    EXPECT_EQ(sizeof(uint8_t),queue.assignSize(sizeof(uint8_t)));
    EXPECT_EQ(0,queue.enqueue(&t[6]));
    EXPECT_EQ(0,queue.enqueue(&t[4]));
    EXPECT_EQ(0,queue.enqueue(&t[8]));
    EXPECT_EQ(0,queue.enqueue(&t[2]));
    EXPECT_EQ(0,queue.destroy());
}

TEST_F(queueTest,assignSizeTooSmall){
    Queue queue;
    EXPECT_EQ(-1,queue.assignSize(-1));
}
