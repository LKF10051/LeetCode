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
    vector<int> inorderTraversal(TreeNode *root) 
	{
		vector<int> ret;
		inorderTraversal(root,ret);
		return ret;
    }

	void inorderTraversal(TreeNode *root, vector<int>& v) 
	{
		if (root == NULL){
			return;
		}
		inorderTraversal(root->left,v);
		v.push_back(root->val);
		inorderTraversal(root->right,v);


	}
};