// majority-element.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include <map>

using namespace std;
/*

Given an array of size n, find the majority element. The majority element is the element that appears more than ⌊ n/2 ⌋ times.

You may assume that the array is non-empty and the majority element always exist in the array.
*/

class Solution {
public:
	int majorityElement(vector<int> &num) 
	{
		map<int,int> amap;
		int nSize = num.size();
		for (int i = 0; i < nSize; i++)
		{
			int val = num[i];
			if (amap.find(val) == amap.end())
			{
				amap[val] = 1;
				continue;
			}

			amap[val]++;
		}

		map<int,int>::iterator it;
		for (it = amap.begin();it != amap.end(); it++)
		{
			if (it->second >(nSize/2) )
			{
				return it->first;
			}
		}

		return 0;
	}
};


// Moore Voting Algorithm
// Refer to: 
// http://www.cs.utexas.edu/~moore/best-ideas/mjrty/index.html
int majorityElement(vector<int> &num) {
	int majority;
	int cnt = 0;
	for(int i=0; i<num.size(); i++){
		if ( cnt ==0 ){
			majority = num[i];
			cnt++;
		}else{
			majority == num[i] ? cnt++ : cnt --;
			if (cnt >= num.size()/2+1) return majority;
		}
	}
	return majority;

}


int _tmain(int argc, _TCHAR* argv[])
{
	Solution so;
	int a[] = {1,1,1,2,2,3,1,3,3};
	vector<int> num(a,a + sizeof(a)/sizeof(a[0]));
	so.majorityElement(num);
	majorityElement(num);
	return 0;
}

