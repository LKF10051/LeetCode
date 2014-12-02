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
    vector<int> preorderTraversal(TreeNode *root) 
	{
        vector<int> ret;
		preorderTraversal(root,ret);
		return ret;
    }

	void preorderTraversal(TreeNode *root, vector<int>& v) 
	{
		if (root == NULL){
			return;
		}
		v.push_back(root->val);
		preorderTraversal(root->left,v);
		preorderTraversal(root->right,v);
	}
};