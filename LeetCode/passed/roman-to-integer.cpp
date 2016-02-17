class Solution {
public:
	int romanToInt(string s) 
	{
		int ret = 0;
		int sz = s.size();
		vector<string> vs;
		vector<int> vi;
		vector<int> vm;

		for (int i =0  ; i < sz; i++)
		{
			string num = s.substr(i,1);
			//vs.push_back();

			if (num == "I")
				vi.push_back(1);
			if (num == "V")
				vi.push_back(5);
			if (num == "X")
				vi.push_back(10);
			if (num == "L")
				vi.push_back(50);
			if (num == "C")
				vi.push_back(100);
			if (num == "D")
				vi.push_back(500);
			if (num == "M")
				vi.push_back(1000);
		}

		int vPre = 0;
		for (int i =0  ; i < sz; i++)
		{
			if (i == 0)
			{
				vPre = vi[i];
				continue;
			}
			if (vi[i] > vPre)
			{
				vm.push_back(0);
			}
			else
			{
				vm.push_back(1);
			}
			vPre = vi[i];
		}


		for (int i =0  ; i < sz; i++)
		{
			if (i == sz - 1)
			{
				ret += vi[i];
			}
			else
			{
				if (vm[i] == 0)
				{
					ret -= vi[i];
				}
				else
				{
					ret += vi[i];
				}
			}
		}


		return ret;
	}
};