// find-minimum-in-rotated-sorted-array.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
using namespace std;

/*
Suppose a sorted array is rotated at some pivot unknown to you beforehand.

	(i.e., 0 1 2 4 5 6 7 might become 4 5 6 7 0 1 2).

	Find the minimum element.

	You may assume no duplicate exists in the array.

	*/

class Solution {
public:
	int findMin(vector<int> &num) 
	{
		if (num.empty())
		{
			return 0;
		}
		 
		int mini = num[0];
		vector<int>::iterator it = num.begin();
		for (; it != num.end(); it++)
		{
			if (mini > (*it))
			{
				mini = *it;
			}
		}
		return mini;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	vector<int> in;
	in.push_back(0);
	Solution so;
	so.findMin(in);
	return 0;
}

