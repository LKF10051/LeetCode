class Solution {
public:
	int searchInsert(int A[], int n, int target) 
	{

		for (int i = 0; i < n; i++)
		{
			if (A[i] == target)
			{
				return i;
			}
			if (A[i] >= target)
			{
				// 				int tmp = i-1;
				// 				if (tmp < 0)
				// 				{
				// 					tmp = 0;
				// 				}
				// 				return tmp;
				return i;
			}

		}
		return n;
	}
};