#include <stdio.h>
#include <time.h>
/* 2nd part of Assignment 1 SD */


int main() {
	/* this is code to measure the time of your algorithm, please don't touch it */

    	clock_t t; 
	double time_taken;

    	t = clock(); 
	
	int storeVal, sizeList, amount, list[1000];
	int sumMin = 0, sumMax = 0;
		

		scanf("%d", &sizeList);
		
		for ( int i = 0; i < sizeList; ++i)
		{
			scanf("%d", &list[i]);
		}
		
		scanf("%d", &amount);
		
	// sorting the array of numbers
		for (int i = 0; i < sizeList; i++)
		{
			for (int j = i + 1; j < sizeList; j++)
			{
				if(list[i] > list[j])
				{
					// switching the numbers if the number on i is bigger then place j.
					// storing the val before changing the position. 
					storeVal = list[i];

					// switching the actual position of the numbers. 
					list[i] = list[j];
					list[j] = storeVal;
				}
			}
		}
		

	

	for (int i = 0; i < amount; i++)
	{
		sumMin +=list[i];
	}
	

	for (int i = (sizeList - amount); i < sizeList ; i++) 
	{
		sumMax += list[i];
	}
	
	
	int outcome= sumMax - sumMin;

	/* implement her your code e.g. like this 
	 * outcome = get_difference_between_min_and_max_sum_of_k_elems( choose your own arguments );
	 */


	/* this is the (performance) test code, please don't touch it */

	t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("difference between minimum and maximum sum of K elements is %d\n", outcome);
#ifdef TEST_TIME
	printf("time needed was %f seconds\n", time_taken);
#endif
	return 0;
}
