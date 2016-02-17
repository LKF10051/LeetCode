class Solution {
public:
	int maxSubArray(int A[], int n) 
	{
		int mv= 0;
		int sum=0;


		if (n == 0)
		{
			return mv;
		}


		for (int i = 0; i < n; i++)
		{
			sum += A[i];
			if (i == 0)
			{
				mv = sum;
			}

			if (sum < 0)
			{
				if (sum > mv)
				{
					mv = sum;
				}
				sum = 0;
				continue;
			}

			if (sum > mv)
			{
				mv = sum;
			}

		}
		return mv;
	}
};