/**
 * Definition for binary tree
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
    bool isSameTree(TreeNode *p, TreeNode *q) 
	{
		if (p == NULL || q == NULL)
		{
			if (p != q)
			{
				return false;
			}
			return true;
		}
		if (q->val != p->val)
		{
			return false;
		}
		if (!isSameTree(p->left, q->left))
		{
			return false;
		}
		if (!isSameTree(p->right, q->right))
		{
			return false;
		}
		return true;
    }
};