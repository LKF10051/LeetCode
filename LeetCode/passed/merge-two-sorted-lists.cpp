/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    ListNode *mergeTwoLists(ListNode *l1, ListNode *l2) 
	{
		ListNode* ret = NULL;
		ListNode* tmp = NULL;

		while (true)
		{
			if (l1 == NULL && l2 == NULL)
			{
				break;
			}
			if (l1 != NULL && l2 == NULL)
			{
				doLink(ret,tmp, l1);
				continue;
			}
			if (l1 == NULL && l2 != NULL)
			{
				doLink(ret,tmp, l2);
				continue;
			}

			if ( l1->val > l2->val )
			{
				doLink(ret,tmp, l2);
			}
			else
			{
				doLink(ret,tmp, l1);
			}
		}
		
		return ret;
    }

	void doLink(ListNode* &baseP, ListNode* &lhs, ListNode* &rhs)
	{
		if (lhs == NULL)
		{
			lhs = rhs;
			baseP = rhs;
			rhs = rhs->next;

		}
		else
		{
			lhs->next = rhs;
			rhs = rhs->next;
			lhs = lhs->next;
		}
	}
};