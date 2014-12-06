// sort-colors.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/*
	
	Given an array with n objects colored red, 
	white or blue, sort them so that objects of the same color are adjacent, 
	with the colors in the order red, white and blue.

	Here, we will use the integers 0, 1, and 2 to represent the color red, 
	white, and blue respectively.

	Note:
	You are not suppose to use the library's sort function for this problem.
	*/


class Solution {
public:
	void sortColors(int A[], int n) 
	{
		if (n == 0)return;
		int pos = 0;

		int red = 0;
		int white = 1;
		int blue = 2;

		int nowColor = red;
		pos = SortColor(pos, n, A, nowColor);
		nowColor = white;
		pos = SortColor(pos, n, A, nowColor);
		nowColor = blue;
		pos = SortColor(pos, n, A, nowColor);

	}

	int SortColor( int pos, int n, int * A, int nowColor ) 
	{
		for (int i = pos; i < n;i++)
		{
			if (A[i] == nowColor)
			{
				if (pos != i)
				{
					swap(A[i],A[pos]);
				}
				pos++;
			}
		}	return pos;
	}

	void swap(int &a, int &b)
	{
		int tmp = b;
		b = a;
		a = tmp;
	}
};


int _tmain(int argc, _TCHAR* argv[])
{

	int A[9] = {0,1,2,0,1,2,0,1,2};

	Solution so;
	so.sortColors(A,9);




	return 0;
}

