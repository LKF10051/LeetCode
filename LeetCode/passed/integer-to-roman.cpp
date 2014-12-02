class Solution {
public:
	string intToRoman(int num) 
	{
		string ret;
		int mark = 0;

		while(true)
		{
			string PJ;
			string s1;
			string s2;
			string s3;


			if (mark == 0)
			{
				s1 = "I";
				s2 = "V";
				s3 = "X";
			}
			if (mark == 1)
			{
				s1 = "X";
				s2 = "L";
				s3 = "C";
			}
			if (mark == 2)
			{
				s1 = "C";
				s2 = "D";
				s3 = "M";
			}
			if (mark == 3)
			{
				s1 = "M";
			}



			int last = num %10;
			num = num/10;

			if (last==0 && num ==0)
			{
				break;
			}

			if (last <= 3)
			{
				for (int i = 0; i < last; i++)
				{
					PJ+=s1;
				}
			}
			else if (last == 4)
			{
				PJ = s1+s2;
			}
			else if (last == 5)
			{
				PJ = s2;
			}
			else if (last <= 8)
			{
				PJ += s2;
				for (int i = 0; i < last-5; i++)
				{
					PJ+=s1;
				}

			}
			else // 9
			{
				PJ = s1 + s3;
			}

			ret = PJ + ret;



			mark++;
		}
		return ret;
	}
};