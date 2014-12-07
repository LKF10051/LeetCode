// unique-paths.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
using namespace  std;
/*
A robot is located at the top-left corner of a m x n grid (marked 'Start' in the diagram below).

The robot can only move either down or right at any point in time. The robot is trying to reach the bottom-right corner of the grid (marked 'Finish' in the diagram below).

How many possible unique paths are there?


Above is a 3 x 7 grid. How many possible unique paths are there?

Note: m and n will be at most 100.
*/

/*
class Solution {
public:
	int uniquePaths(int m, int n) 
	{
		if (m == 1 && n!= 1)
		{
			return 1;
		}
		if (m != 1 && n == 1)
		{
			return 1;
		}
		if (m == 1 && n == 1)
		{
			return 0;
		}

		return uniquePaths(m,n-1) + uniquePaths(m-1,n);
	}
};
*/
class Solution {
public:
	int uniquePaths(int m, int n) {
		int* matrix = new int[m*n];
		for (int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
			{
				if(i==0 || j==0)
				{
					int a = i*n+j;
					matrix[i*n+j]=1;
				}
				else
				{
					int a = i*n+j;
					int b = (i-1)*n+j;
					int c = i*n+j-1;
					matrix[i*n+j] = matrix[(i-1)*n+j] + matrix[i*n+j-1];
				}
			}
		} 
		printMatrix(matrix, m, n);
		int u = matrix[m*n-1];
		delete[] matrix;
		return u;
	} 

	void printMatrix(int*a, int m, int n)
	{
		for (int i=0; i<m; i++){
			for (int j=0; j<n; j++){
				printf("%4d ", a[i*n+j]);
			}
			printf("\n");
		}
		printf("\n");
	}
};
int _tmain(int argc, _TCHAR* argv[])
{
	Solution so;
	int ret = so.uniquePaths(5,3);
	return 0;
}

