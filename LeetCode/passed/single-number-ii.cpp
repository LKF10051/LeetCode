class Solution {
public:
	int singleNumber(int A[], int n) {
		int ones = 0, twos = 0, xthrees = 0;
		for(int i = 0; i < n; ++i) 
		{
			int tmp = (ones & A[i]);

			twos |= (ones & A[i]); // ��¼�������Σ� ����һ�αȽϣ�����Ϊ1

			ones ^= A[i]; // ��¼����һ�Σ����α�0

			int tmp3 = ones & twos;
			int tmp2 = ~(ones & twos);

			xthrees = ~(ones & twos);   // ����һ��mark����� twos��ones��Ϊ1����ζ��
			//  ������3�Σ� ��Ӧ�ð�twos��ones������
			ones &= xthrees;			

			twos &= xthrees;

		}

		return ones;
	}
};