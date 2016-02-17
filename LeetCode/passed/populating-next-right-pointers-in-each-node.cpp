/**
 * Definition for binary tree with next pointer.
 * struct TreeLinkNode {
 *  int val;
 *  TreeLinkNode *left, *right, *next;
 *  TreeLinkNode(int x) : val(x), left(NULL), right(NULL), next(NULL) {}
 * };
 */

class Solution {
public:
    void connect(TreeLinkNode *root) 
	{
        if (root == NULL)
        {
			return;
        }

		if (root->left == NULL)
		{
			return;
		}

		root->left->next = root->right;

		connectLeftToRight(root->left,root->right);
		connect(root->left);
		connect(root->right);
    }


	void connectLeftToRight(TreeLinkNode *left,TreeLinkNode *right)
	{
		if (left->right == NULL)
		{
			return;
		}
		TreeLinkNode* mr = left;
		TreeLinkNode* ml = right;
		while(true)
		{
			if (mr->right == NULL)
			{
				break;
			}
			mr = mr->right;
			ml = ml->left;
			mr->next = ml;
		}

		mr->next = ml;
	}

};