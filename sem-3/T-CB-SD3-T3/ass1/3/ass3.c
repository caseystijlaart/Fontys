#include <stdio.h>
#include <time.h>
#include <string.h>
/* 3rd part of Assignment 1 SD */

static void alg1(int *arr, int n)
{
	// selection sort
	int pos, safe;

  for (int i = 0; i < (n - 1); i++) 
  {
    pos = i;

    for (int j = i + 1; j < n; j++)
    {
      if (arr[pos] > arr[j])
        pos = j;
    }
    
    
    if (pos != i)
    {
	// swapping position
      safe = arr[i];
      arr[i] = arr[pos];
      arr[pos] = safe;
    }
  }
}

static void alg2(int *arr, int n)
{
	// bubble sort
	int storeVal;
	for (int i = 0; i < n; i++)
		{
			for (int j = i + 1; j < n; j++)
			{
				if(arr[i] > arr[j])
				{
					// swapping position
					storeVal = arr[i];
					arr[i] = arr[j];
					arr[j] = storeVal;
				}
			}
		}	
}

static void alg3(int *arr, int n)
{
	// insertion sort
	int safe, j;
	for(int i = 1; i < n; i++)
	{
		safe = arr[i];
		j = i - 1;
		
		while (safe < arr[j] && j >= 0 )
		{
			arr[j+1] = arr[j];
			j = j-1;
		}
		arr[j + 1] = safe;
	}		
}

static void read_in_arrays_to_sort(int *arr1, int *arr2, int *arr3, int n1, int n2, int n3)
{
	for(int i=0; i<n1; i++)
	{
		scanf("%d", &arr1[i]);
	}
	for(int i=0; i<n2; i++)
	{
		scanf("%d", &arr2[i]);
	}
	for(int i=0; i<n3; i++)
	{
		scanf("%d", &arr3[i]);
	}

}

static void write_to_file(int *arr1, int *arr2, int *arr3, int n1, int n2, int n3, char *filename)
{
	FILE *f= fopen(filename, "w");
	if(f==NULL)
	{
		printf("Error in creating output file %s", filename);
		return;
	}
	for(int i=0; i<n1; i++)
	{
		fprintf(f, "%d ", arr1[i]);
	};
	fprintf(f, "\n");
	for(int i=0; i<n2; i++)
	{
		fprintf(f, "%d ", arr2[i]);
	}
	fprintf(f, "\n");
	for(int i=0; i<n3; i++)
	{
		fprintf(f, "%d ", arr3[i]);
	}
	fclose(f);

}

static void execute_algorithm(int *arr, int n, void (*sort_alg)())
{
	clock_t t;
	double time_taken;
	t = clock();
	sort_alg(arr,n);
	t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Time needed to sort %d elements was %f seconds\n", n, time_taken);
}

static void copy_input_arrays(int *arr1, int *arr2, int *arr3, int *sarr1, int *sarr2, int *sarr3, int n1, int n2, int n3)
{
	memcpy(arr1, sarr1, n1*sizeof(int));
	memcpy(arr2, sarr2, n2*sizeof(int));
	memcpy(arr3, sarr3, n3*sizeof(int));
}

int main() {
	/* Don't touch main */
  
	const int n1 = 10;
	const int n2 = 1000;
	const int n3 = 10000;
	int arr1[n1];
	int arr2[n2];
	int arr3[n3];
	int sarr1[n1];
	int sarr2[n2];
	int sarr3[n3];

	read_in_arrays_to_sort(sarr1, sarr2, sarr3, n1, n2, n3);
	copy_input_arrays(arr1, arr2, arr3, sarr1, sarr2, sarr3, n1, n2, n3);	

	printf("Execution of the first alg1 algorithm\n");

	execute_algorithm(arr1, n1, alg1);
	execute_algorithm(arr2, n2, alg1);
	execute_algorithm(arr3, n3, alg1);
	write_to_file(arr1, arr2, arr3, n1, n2, n3, "out_alg1");

	copy_input_arrays(arr1, arr2, arr3, sarr1, sarr2, sarr3, n1, n2, n3);	

	printf("Execution of the second alg2 algorithm\n");

	execute_algorithm(arr1, n1, alg2);
	execute_algorithm(arr2, n2, alg2);
	execute_algorithm(arr3, n3, alg2);
	write_to_file(arr1, arr2, arr3, n1, n2, n3, "out_alg2");

	copy_input_arrays(arr1, arr2, arr3, sarr1, sarr2, sarr3, n1, n2, n3);	

	printf("Execution of the third alg3 algorithm\n");

	execute_algorithm(arr1, n1, alg3);
	execute_algorithm(arr2, n2, alg3);
	execute_algorithm(arr3, n3, alg3);
	write_to_file(arr1, arr2, arr3, n1, n2, n3, "out_alg3");
	return 0;
}
