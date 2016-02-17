class Solution {
public:
	int singleNumber(int A[], int n) {
		int ones = 0, twos = 0, xthrees = 0;
		for(int i = 0; i < n; ++i) 
		{
			int tmp = (ones & A[i]);

			twos |= (ones & A[i]); // 记录出现两次， 与上一次比较，两次为1

			ones ^= A[i]; // 记录出现一次，两次变0

			int tmp3 = ones & twos;
			int tmp2 = ~(ones & twos);

			xthrees = ~(ones & twos);   // 计算一个mark，如果 twos、ones都为1，意味着
			//  出现了3次， 就应该把twos、ones清零了
			ones &= xthrees;			

			twos &= xthrees;

		}

		return ones;
	}
};