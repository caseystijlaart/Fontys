#include <stdio.h>
#include <time.h>
/* 1st part of Assignment 1 SD */


int main() {
	/* this is code to measure the time of your algorithm, please don't touch it */

		clock_t t; 
		double time_taken;

    	t = clock(); 
    	
 
		int sizeList, a, repeats, numbers[10000], safe;

		
		// size of list
		scanf("%d", &sizeList);
		
		
		// input numbers
		for (int i = 0; i < sizeList; ++i)
		{
			scanf("%d", &numbers[i]);
		}
		
		//amount of repeats
		scanf("%d", &repeats);
		
		
		for (int i = 0; i < sizeList; i++)
		{
			for (int j = i + 1; j < sizeList; ++j)
			{
				if(numbers[i] > numbers[j])
				{
					a = numbers[i];
					numbers[i] = numbers[j];
					numbers[j] = a;
				}
			}
		}
		
		int prev = numbers[0];
		int count = 1;
		
		for (int i = 1; i < sizeList; i++)
		{
			if (numbers[i] == prev)
			{
				count++;
				safe = numbers[i];
				
			} else {
				if(count == repeats)
				{ 
					break;
				}
				prev = numbers[i];
				count = 1;
			}
		}
		

	/* implement her your code e.g. like this 
	 * outcome = get_minimum_number_with_k_occurences( choose your own arguments );
	 */

	int outcome = safe;

	/* this is the (performance) test code, please don't touch it */
	
	t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("minimum number of K occurences is %d\n", outcome);
#ifdef TEST_TIME
	printf("time needed was %f seconds\n", time_taken);
#endif
	return 0;
}
