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
    ListNode *deleteDuplicates(ListNode *head) 
	{
		if (head == NULL)
		{
			return head;
		}



		ListNode* next = deleteDuplicates(head->next);
		
		if (next == NULL)
		{
			return head;
		}


		if (next->val == head->val)
		{
			head->next = next->next;
			//delete next;
		}

		return head;
        
    }
};