// unique-paths-ii.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
using namespace std;
/*
Follow up for "Unique Paths":

Now consider if some obstacles are added to the grids. How many unique paths would there be?

An obstacle and empty space is marked as 1 and 0 respectively in the grid.

For example,
There is one obstacle in the middle of a 3x3 grid as illustrated below.

[
[0,0,0],
[0,1,0],
[0,0,0]
]
The total number of unique paths is 2.

Note: m and n will be at most 100.
*/

class Solution {
public:
	int uniquePathsWithObstacles(vector<vector<int> > &obstacleGrid) 
	{
		int mSize = obstacleGrid.size();
		if (mSize == 0)
		{
			return 0;
		}
		int nSize = obstacleGrid[0].size();

		int* matrix = new int[nSize* mSize];


		for (int i = 0; i < mSize ; i++)
		{
			for (int j = 0; j < nSize; j++)
			{
				int pos = nSize*i + j;
				if (obstacleGrid[i][j] == 1)
				{
					matrix[pos] = 0;
					continue;
				}

				int posTop = nSize*(i-1) + j;
				int posLeft = nSize*i + j - 1;

				if (i == 0 && j == 0)
				{
					matrix[pos] = 1;
					continue;
				}

				if (i == 0)
				{
					matrix[pos] = 0 + matrix[posLeft];
					continue;
				}

				if (j == 0)
				{
					matrix[pos] = matrix[posTop] + 0;
					continue;
				}

				matrix[pos] = matrix[posTop] + matrix[posLeft];
			}
		}

		int ret = matrix[nSize* mSize-1];
		delete matrix;
		return ret;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	vector<vector<int>> in;
	vector<int> in2(1,0);
	in.push_back(in2);


	Solution so;
	so.uniquePathsWithObstacles(in);
	return 0;
}

