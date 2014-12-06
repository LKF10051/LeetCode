// balanced-binary-tree.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"




struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution {
public:
    bool isBalanced(TreeNode *root) 
	{
		if (!root)
		{
			return true;
		}
		int left = maxDepth(root->left);
		int right = maxDepth(root->right);
        if (left - right > 1 || right - left> 1 )
        {
			return false;
        }

		if (!isBalanced(root->right))
		{
			return false;
		}
		if (!isBalanced(root->left))
		{
			return false;
		}

		return true;
    }

	int maxDepth(TreeNode* root)
	{
		if (root == NULL)
		{
			return 0;
		}

		int left = maxDepth(root->left);
		int right = maxDepth(root->right);

		if (left > right)
		{
			 return 1 + left;

		}
		else
		{
			return 1+ right;
		}
	}

};


int _tmain(int argc, _TCHAR* argv[])
{
	 TreeNode* a = new TreeNode(1);
	 TreeNode* b = new TreeNode(2);
	 TreeNode* c = new TreeNode(3);

	a->right = b;
	b->right = c;


	Solution so;
	so.isBalanced(a);

	return 0;
}

