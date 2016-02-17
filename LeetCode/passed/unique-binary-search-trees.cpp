class Solution {
public:
	int numTrees(int n) 
	{
		return numTrees(1,n);

	}

	int numTrees(int a, int b)
	{
		if (a >= b)
		{
			return 1;
		}
		int total = 0;
		for (int i = a; i <= b; i++)
		{
			total += numTrees(a,i-1) * numTrees(i+1,b);
		}
		return total;
	}

};

