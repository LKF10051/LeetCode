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
    int maxDepth(TreeNode *root) 
	{
		int dep;

		if (root == NULL)
		{
			return 0;
		}
		if (root->left == NULL && root->right == NULL)
		{
			return 1;
		}
		if (root->left == NULL && root->right != NULL)
		{
			return maxDepth(root->right)+1;
		}

		if (root->left != NULL && root->right == NULL)
		{
			return maxDepth(root->left)+1;
		}

		int Lret =  maxDepth(root->left);
		int Rret =  maxDepth(root->right);
		if (Lret >Rret )
		{
			return Lret + 1;
		}
		return Rret + 1;
    }
};