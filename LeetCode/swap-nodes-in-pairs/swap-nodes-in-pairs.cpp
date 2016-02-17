// swap-nodes-in-pairs.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/*
Given a linked list, 
swap every two adjacent nodes and return its head.

For example,
Given 1->2->3->4, you should return the list as 2->1->4->3.

Your algorithm should use only constant space. 
You may not modify the values in the list,
only nodes itself can be changed.
*/


struct ListNode {
	int val;
	ListNode *next;
	ListNode(int x) : val(x), next(NULL) {}
 };

class Solution {
public:
    ListNode *swapPairs(ListNode *head) 
	{
		if (head == NULL)return head;
		if (head->next == NULL)return head;
        
		ListNode* tmp = head;
		ListNode* twoNext = head->next->next;
		head = head->next;
		head->next = tmp;
		head->next->next = twoNext;

		if (head->next->next == NULL)return head;

		ListNode* sa = swapPairs(head->next->next);
		head->next->next = sa;

		return head;
    }
};


int _tmain(int argc, _TCHAR* argv[])
{
	ListNode* a = new ListNode(1);
	ListNode* b = new ListNode(2);
	ListNode* c = new ListNode(3);
	ListNode* d = new ListNode(4);
	a->next = b;
	b->next = c;
	c->next = d;
	Solution so;
	ListNode* ret = so.swapPairs(a);
	return 0;
}

