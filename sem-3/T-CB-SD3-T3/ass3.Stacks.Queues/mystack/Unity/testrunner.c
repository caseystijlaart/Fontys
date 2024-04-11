#include "testrunner.h"
#include "unity.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>

// I rather dislike keeping line numbers updated, so I made my own macro to ditch the line number
#define MY_RUN_TEST(func) RUN_TEST(func, 0)


int runTestCases(TEST_CASE *cases, int numberOfCases) {
	UnityBegin();

	for(int i = 0; i < numberOfCases;i++) {
		TEST_CASE* test_case = &cases[i];
		printf("Running test: %s\n", test_case->name);
		MY_RUN_TEST(test_case->test_function);
	}

	return UnityEnd();
}

int runTestCase(TEST_CASE* c) {
	UnityBegin();

	printf("Running test: %s\n", c->name);
	MY_RUN_TEST(c->test_function);

	int result = UnityEnd();
	if(result == 0) {
		return c->points;
	}

	return 0;
}

void parseMainArgument(int argc, char * argv[],
                      TestCommand *command,
                      int* test_case_index)
{
  *command = TEST_CMD_NONE;

  if(argc >= 2) {
    char* cmd = argv[1];

    if(strcmp(cmd,CMD_INFO) == 0)
    {
      if(argc != 3) {
        printf("error: please provide index argument.\n");
        *command = TEST_CMD_ERROR;
        return;
      }
      *command = TEST_CMD_INFO;
      *test_case_index = atoi(argv[argc-1]);

    }
    else if(strcmp(cmd,CMD_SIZE) == 0)
    {
      *command = TEST_CMD_SIZE;
    }
    else if(strcmp(cmd,CMD_RUN) == 0)
    {
      if(argc != 3) {
        printf("error: please provide index argument.\n");
        *command = TEST_CMD_ERROR;
        return;
      }
      *command = TEST_CMD_RUN;
      *test_case_index = atoi(argv[2]);
    }
  }
}

int runTests(int argc, char * argv[], TEST_CASE *test_cases, int numberOfCases) {
  TestCommand command = TEST_CMD_NONE;
	int test_case_index = 0;

  parseMainArgument(argc, argv, &command, &test_case_index);

	if(command == TEST_CMD_INFO) {
			if(test_case_index > (numberOfCases-1) ) {
				printf("error: index out of range [0..%d].\n", numberOfCases-1);
			}
			else {
				TEST_CASE* test_case = &test_cases[test_case_index];
				printf("%s, %d\n", test_case->name, test_case->points);
			}
			return 0;
	}
	else if(command == TEST_CMD_SIZE)
	{
		printf("%d\n", numberOfCases);
		return 0;
	}
	else if(command == TEST_CMD_RUN)
	{
    if(test_case_index > (numberOfCases-1) ) {
      printf("error: index out of range [0..%d].\n", numberOfCases-1);
      return 0;
    }

		return runTestCase(&test_cases[test_case_index]);
	}
  else if(command == TEST_CMD_ERROR) {
    printf("Error occurred");
    return 1;
  }

  return runTestCases(test_cases, numberOfCases);
}
