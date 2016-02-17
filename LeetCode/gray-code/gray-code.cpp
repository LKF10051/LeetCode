// gray-code.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>

using namespace std;


/*
The gray code is a binary numeral system where two successive values differ in only one bit.

Given a non-negative integer n representing the total number of bits in the code, print the sequence of gray code. A gray code sequence must begin with 0.

For example, given n = 2, return [0,1,3,2]. Its gray code sequence is:

00 - 0
01 - 1
11 - 3
10 - 2
Note:
For a given n, a gray code sequence is not uniquely defined.

For example, [0,2,3,1] is also a valid gray code sequence according to the above definition.

For now, the judge is able to judge based on one instance of gray code sequence. Sorry about that.
	*/


/*

class Solution {
public:
	vector<int> grayCode(int n) 
	{
		int len = 1<<n;

		vector<int> ret;
		if (n = 0)
		{
			return ret;
		}
		vector<int> restV;
		ret.push_back(0);
		for (int i = 1; i <len; i++)
		{
			int sz = ret.size();
			if (isGray(ret[sz - 1], i))
			{
				ret.push_back(i);
			}
// 			else if (isGray(ret[0], i))
// 			{
// 				ret.insert(ret.begin(),i);
// 			}
			else
			{
				bool isIn = false;

				for (int j = 0; j <sz - 1; j++)
				{	
					if (isGray(ret[j], i) && isGray(ret[j+1], i))
					{
						ret.insert(ret.begin(),i);
						isIn = true;
						break;
					}
				}

				if (!isIn)
				{
					restV.push_back(i);
				}
			}
		}

		while(restV.size() != 0)
		{
			restV = forInsert(restV, ret);
		}

		return ret;
	}


	bool isGray(int lhs, int rhs)
	{
		int k = lhs^rhs;
		if (k == 1)
		{
			return true;
		}
		int sizeofint = sizeof(int);
		int weiyi = 1;
		for (int i = 0; i < sizeofint; i++)
		{
			int check = i<<weiyi;
			if(k == check)
			{
				return true;
			}
		}
		return false;
	}

	vector<int> forInsert(vector<int>& lst, vector<int>& in)
	{
		vector<int> ret;
		
		int lstSize = lst.size();
		for (int j = 0; j <lstSize; j++)
		{	
			bool isfind = false;
			int sz = in.size();
			if (isGray(in[sz - 1], lst[j] ))
			{
				in.push_back(lst[j] );
				continue;
			}
			for (int i = 0; i < sz-1; i++)
			{
				if (isGray(in[i], lst[j]) && isGray(in[i+1], lst[j]))
				{
					in.insert(in.begin()+i,lst[j]);
					break;
				}
			}
			if (!isfind)
			{
				ret.push_back(lst[j]);
			}
		}
		return ret;
	}
};

*/
/*
class Solution {
public:
	vector<int> grayCode(int n) {
		vector<int> v;
		//n = 1<<n;

		int x =0;   
		v.push_back(x); 
		for(int i=0; i<n; i++){
			int len = v.size();
			for (int j=0; j<len; j++){
				x = v[j]<<1;
				if (j%2==0){
					v.push_back(x);
					v.push_back(x+1);
				}else{
					v.push_back(x+1);
					v.push_back(x);
				}
			}
			v.erase(v.begin(), v.begin()+len);
		}

		return v;
	}
};
*/


class Solution {
public:
	vector<int> grayCode(int n) 
	{
		vector<int> ret;
		if (n == 0)
		{
			ret.push_back(0);
			return ret;
		}
		vector<int> tmp = grayCode(n-1);
		int addNum = 1 << (n-1);
		ret.assign(tmp.begin(), tmp.end());

		for (int i = tmp.size()-1; i >=0; i--)
		{
			ret.push_back(addNum +tmp[i]);
		}

		return ret;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{

	Solution so;
	so.grayCode(2);


	return 0;
}

