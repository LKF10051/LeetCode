class Solution {
public:
	int reverse(int x) 
	{
		if (x == 0 || x == -2147483648 || x == 2147483647){
			return 0;
		}

		if (x<0){
			return -reverse(-x);
		}

		static const int chang = 9;

		int ten = 10;
		int in[chang + 1] = {0};
		int wei = 0;
		unsigned int chushu = 1;
		for (int i =chang;i>=0;i--)
		{
			chushu = 1;
			for (int j =0;j<i;j++)
			{
				chushu *= ten;
			}
			int yushu = x/chushu;
			if (yushu != 0 || wei !=0)
			{
				in[wei] = yushu;
				wei++;
			}
			x = x-yushu*chushu;
		}

		int ret = 0;
		unsigned int chengshu = 1;
		for (int i =wei -1;i>=0;i--)
		{
			chengshu = 1;
			for (int j =0;j<i;j++)
			{
				chengshu *= ten;
			}

			ret += in[i] *chengshu;
			if (ret < 0)
			{
				return 0;
			}

		}

		return ret;
	}
};