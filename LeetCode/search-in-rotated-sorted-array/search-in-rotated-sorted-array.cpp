class Solution {
public:
	int search(vector<int>& nums, int target) 
	{
		if (nums.size() == 0)
		{
			return -1;
		}

		int ret = -1;
		int ibegin = 0;
		int iend = nums.size() - 1;
		int im = (ibegin + iend)/2;



		bool isrunning = true;

		while (isrunning)
		{
			if (ibegin == iend && target != nums[ibegin])
			{
				return -1;
			}

			im = (ibegin + iend)/2;
			if (nums[im] == target)
			{
				ret = im;
				return ret;
			}



			if (target > nums[iend] && target < nums[ibegin])
			{
				return -1;
			}

			if (nums[ibegin] == target)
			{
				ret = ibegin;
				return ret;
			}
			if (nums[iend] == target)
			{
				ret = iend;
				return ret;
			}

			if (im == ibegin || im == iend )
			{

				return -1;
			}



			if (target > nums[im] && target < nums[iend]) // nums[iend] > target > nums[im] 
			{
				if (ibegin == im)
				{
					ibegin += 1;
				}
				else
				{
					ibegin = im;
				}
			}

			if (target > nums[im] && target > nums[iend] && nums[im] > nums[iend]) 
			{
				if (ibegin == im)
				{
					ibegin += 1;
				}
				else
				{
					ibegin = im;
				}
				continue;
			}

			if (target > nums[im] && target > nums[iend] && nums[im] < nums[iend]) 
			{
				if (iend == im)
				{
					iend -= 1;
				}
				else
				{
					iend = im;
				}
				continue;
			}

			if (target < nums[im] && target > nums[ibegin]) // nums[iend] > target > nums[im] 
			{
				if (iend == im)
				{
					iend -= 1;
				}
				else
				{
					iend = im;
				}
				continue;
			}

			if (target < nums[im] && target < nums[ibegin] && nums[im] < nums[ibegin]) 
			{
				if (iend == im)
				{
					iend -= 1;
				}
				else
				{
					iend = im;
				}
				continue;
			}

			if (target < nums[im] && target < nums[ibegin] && nums[im] > nums[ibegin]) 
			{
				if (ibegin == im)
				{
					ibegin += 1;
				}
				else
				{
					ibegin = im;
				}
				continue;
			}


		}

		return 0;

	}
};
