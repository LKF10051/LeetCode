class Solution {
public:

	vector<string> m_vs;
	int totalNQueens(int n) 
	{
		vector<vector<string> >  ret;
		vector<string>  clean = inti(n);
		for (int j =0; j < n; j++) // ÐÐ
		{
			vector<string> vs  = clean;
			setblock(ret,vs,0,j,n);
		}
		return ret.size();
	}

	void setblock(vector<vector<string> >& total,vector<string> vs , int i,int j, int n)
	{
		vs[i].replace(j,1,"Q");
		setMark(vs,i,j,n);
		if (i + 1 == n)
		{
			total.push_back(vs);
			return ;
		}
		vector<int> nextLine = isClean(vs[i + 1], n);
		int num = nextLine.size();
		if (num == 0)
			return ;

		for (int k = 0; k < num; k++)
		{
			setblock(total ,vs, i+1, nextLine[k] ,n);
		}
	}

	void setMark(vector<string>& vs , int i,int j, int n)
	{
		for (int k =0; k < n; k++) 
		{
			if (k == j)
				continue;

			vs[i].replace(k,1,".");
		}

		for (int k =0; k < n; k++) 
		{
			if (k == i)
				continue;

			vs[k].replace(j,1,".");
		}

		for (int k =1; k < n; k++)
		{
			if (i + k >= n)
				continue;
			if (j - k < 0)
				continue;

			vs[i + k].replace(j - k,1,".");
		}

		for (int k =1; k < n; k++)
		{
			if (i + k >= n)
				continue;
			if (j + k >= n)
				continue;

			vs[i + k].replace(j + k,1,".");
		}
	}

	vector<int> isClean(string& s, int n)
	{
		vector<int> ret ;
		for (int i =0; i < n; i++)
		{
			string sub = s.substr(i,1);
			if (sub == " ")
				ret.push_back(i);

		}
		return ret;
	}

	vector<string>  inti(int n) 
	{
		vector<string> v1;
		v1.reserve(n);
		string s = "";
		for (int i =0; i < n; i++)
		{
			s += " ";
		}

		for (int i =0; i < n; i++)
		{
			v1.push_back(s);
		}
		return v1;
	}
};