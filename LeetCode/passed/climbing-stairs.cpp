class Solution {
public:


	vector<int> m_vi;

	int climbStairs(int n) 
	{

		if (n == 0)
		{
			return 1;
		}

		for (int i = 0; i<= n; i++)
		{
			if (i == 0)
			{
				m_vi.push_back(1);
				continue;
			}

			if (i == 1)
			{
				m_vi.push_back(1);
				continue;
			}
			int in = m_vi[i-1]+m_vi[i-2];
			m_vi.push_back(in);
		}


		return m_vi[n];





		/*


		if (n == 0)
		{
			return 1;
		}

		if (n == 1)
		{
			return 1;
		}

		int ret = climbStairs(n-1) + climbStairs(n-2);
		return ret;

		*/
	}

};