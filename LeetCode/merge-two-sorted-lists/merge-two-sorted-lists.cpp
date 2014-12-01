// merge-two-sorted-lists.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/*

Merge two sorted linked lists and return it as a new list. 
The new list should be made by splicing together the nodes of the first two lists.

*/



 struct ListNode {
     int val;
     ListNode *next;
     ListNode(int x) : val(x), next(NULL) {}
 };
 
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


int _tmain(int argc, _TCHAR* argv[])
{

	ListNode* a1 = new ListNode(1);
	ListNode* a2 = new ListNode(2);
	ListNode* a3 = new ListNode(3);
	ListNode* a4 = new ListNode(4);


	a1->next = a3;

	a2->next = a4;


	Solution so;
	ListNode* aa =  so.mergeTwoLists(a1,a2);


	return 0;
}

