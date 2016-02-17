class Solution {
public:
	int singleNumber(int A[], int n) 
	{
		int in[65536] = {0};

		for (int i = 0; i < n; ++i)
		{
			int valu = A[i] + 32768;
			in[valu] += 1;
		}
		for (int i = 0; i < 65536; ++i)
		{
			if (in[i]  == 1)
			{
				return i - 32768;
			}
		}

		return 0;
	}
};