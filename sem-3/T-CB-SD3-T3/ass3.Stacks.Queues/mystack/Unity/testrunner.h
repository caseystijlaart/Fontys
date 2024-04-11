#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#define CMD_RUN ("run")
#define CMD_INFO ("info")
#define CMD_SIZE ("size")

typedef enum {
  TEST_CMD_NONE,
  TEST_CMD_ERROR,
  TEST_CMD_RUN,
  TEST_CMD_INFO,
  TEST_CMD_SIZE
} TestCommand;

typedef struct
{
	char* name;
	void (*test_function)(void);
	int points;
} TEST_CASE;

int runTests(int argc, char * argv[], TEST_CASE *cases, int numberOfCases);

int runTestCase(TEST_CASE* c);
int runTestCases(TEST_CASE *cases, int numberOfCases);
void parseMainArgument(int argc, char * argv[], TestCommand *command, int* test_case_index);

#endif
