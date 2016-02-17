// onvert-sorted-array-to-binary-search-tree.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
using namespace std;

struct TreeNode {
	int val;
	TreeNode *left;
	TreeNode *right;
	TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution {
public:
    TreeNode *sortedArrayToBST(vector<int> &num) 
    {
        if (num.size() == 0)
        {
			return NULL;
        }

		if (num.size() == 1)
		{
			TreeNode* anode = new TreeNode(num[0]);
			return anode;
		}

		int middle = num.size()/2;

		//int m = *(num.begin()+middle);

		TreeNode* mn = new TreeNode(num[middle]);

        vector<int> l;
		vector<int> r;

		l.assign(num.begin(), num.begin() + middle  );
		r.assign(num.begin() + middle + 1, num.end() );

		TreeNode* ln = sortedArrayToBST(l);
		TreeNode* rn = sortedArrayToBST(r);

		mn->left = ln;
		mn->right = rn;

		return mn;

    }
};

int _tmain(int argc, _TCHAR* argv[])
{
	vector<int> a;
	a.push_back(1);
	a.push_back(2);
	a.push_back(3);
	Solution so;
	so.sortedArrayToBST(a);
	return 0;
}

