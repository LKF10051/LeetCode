class Solution {
public:
	int maxProfit(vector<int> &prices) 
	{
		if (prices.size() <= 1)
		{
			return 0;
		}

		vector<int> v2;
		typedef vector<int>::iterator vitor;
		vitor it = prices.begin();
		int mv = 0;
		for (;it != prices.end(); it++)
		{
			vitor it2 = it;
			it2++;
			if (it2 == prices.end())
			{
				break;
			}

			v2.push_back(*it2-*it);
		}

		it = v2.begin();
		int sum = 0;
		for (;it != v2.end(); it++)
		{
			sum += *it;
			if (sum < 0)
			{
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
