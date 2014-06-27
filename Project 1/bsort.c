#include<stdio.h>
#include<stdlib.h>

int main()
{
	int array[10] = {9,8,7,6,5,4,3,2,1,0};
	int d, j, swap = 0;
	for (d = 0 ; d < 9; d++)
	{
		if (array[d] > array[d+1]) /* For decreasing order use < */
		{
			swap       = array[d];
			array[d]   = array[d+1];
			array[d+1] = swap;
		}
		for (j = 0 ; j < 10; j++)
		{
			printf("%d ", array[j]);
		}
		printf("\n");
	}
	
	return 0;
}
