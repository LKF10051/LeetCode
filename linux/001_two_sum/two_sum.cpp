#include <string>
#include <vector>
#include <unordered_map>
using namespace std;


vector<int> twoSum1(vector<int>& numbers, int target) 
{
    unordered_map<int, int> m;
    vector<int> result;
    for(size_t i=0; i<numbers.size(); i++)
    {
	// not found the second one
	if (m.find(numbers[i])==m.end() ) 
	{ 
	    // store the first one poisition into the second one's key
	    m[target - numbers[i]] = i; 
	}
	else 
	{ 
	    // found the second one
	    result.push_back(m[numbers[i]]);
	    result.push_back(i);
	    break;
	}
    }

    return result;
}


int main()
{ 
    vector<int> nums = {1,2,3,4,5,6};
    vector<int> ret = twoSum1(nums, 6);
    
    return 0;
}
