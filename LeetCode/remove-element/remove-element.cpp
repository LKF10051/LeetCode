// remove-element.cpp : 定义控制台应用程序的入口点。
//

/*
Given an array and a value, remove all instances of that value in place and return the new length.

The order of elements can be changed. It doesn't matter what you leave beyond the new length.
*/

#include "stdafx.h"

class Solution {
public:
    int removeElement(int A[], int n, int elem) 
	{
		if (n == 0) return 0;

		for (int i = 0; i < n;)
		{
			if (A[i] == elem)
			{
				A[i] = A[n-1];
				n -= 1;
			}
			else
			{
				 i++;
			}
		}
        return n;
    }
};

int _tmain(int argc, _TCHAR* argv[])
{
	int a[3] = {1,2,3};
	Solution so;
	so.removeElement(a,3,2);


	return 0;
}

