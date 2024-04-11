#include "myStack.h"
#include "gtest/gtest.h"
 
class stackTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
  }
 
  virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
  }
};
 
 int t[] = {1, 3, 2, 43, 432, 45, 65, 2, 54};

TEST_F(stackTest,pushOnce){
    Stack stack;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[0]));
    EXPECT_EQ(1,stack.numElem());
}

TEST_F(stackTest,pushMultiple){
    Stack stack;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[0]));
    EXPECT_EQ(0,stack.push(&t[1]));
    EXPECT_EQ(0,stack.push(&t[3]));
    EXPECT_EQ(0,stack.push(&t[2]));
    EXPECT_EQ(0,stack.push(&t[6]));
    EXPECT_EQ(5,stack.numElem());
}

TEST_F(stackTest,pushNULL){
    Stack stack;
    EXPECT_EQ(-1,stack.push(0));
    EXPECT_EQ(0 ,stack.numElem());
}

TEST_F(stackTest,popOnce){
    Stack stack;
     int data;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[3]));
    EXPECT_EQ(0,stack.push(&t[7]));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(1,stack.numElem());
}

TEST_F(stackTest,popMultiple){
    Stack stack;
     int data;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[0]));
    EXPECT_EQ(0,stack.push(&t[1]));
    EXPECT_EQ(0,stack.push(&t[3]));
    EXPECT_EQ(0,stack.push(&t[2]));
    EXPECT_EQ(0,stack.push(&t[6]));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(2,stack.numElem());
}

TEST_F(stackTest,popTooMuch){
    Stack stack;
    int data;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[2]));
    EXPECT_EQ(0,stack.push(&t[6]));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(0,stack.pop(&object));
    EXPECT_EQ(-1,stack.pop(&object));
    EXPECT_EQ(0,stack.numElem());
}

TEST_F(stackTest,emptyOneElementInStack){
    Stack stack;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[8]));
    EXPECT_EQ(1,stack.numElem());
    EXPECT_EQ(0,stack.empty());
    EXPECT_EQ(0,stack.numElem());
}

TEST_F(stackTest,emptyMultipleElementInStack){
    Stack stack;
    EXPECT_EQ(sizeof(int),stack.assignSize(sizeof(int)));
    EXPECT_EQ(0,stack.push(&t[6]));
    EXPECT_EQ(0,stack.push(&t[4]));
    EXPECT_EQ(0,stack.push(&t[8]));
    EXPECT_EQ(0,stack.push(&t[2]));
    EXPECT_EQ(4,stack.numElem());
    EXPECT_EQ(0,stack.empty());
    EXPECT_EQ(0,stack.numElem());
}

TEST_F(stackTest,assignSizeTooSmall){
    Stack stack;
    EXPECT_EQ(-1,stack.assignSize(0));
}
