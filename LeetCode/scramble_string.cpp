bool isScramble(string s1, string s2)
{
	int len1 = s1.size();
	int len2 = s2.size();
	if (len1 != len2) return false;
	if (len1 == 1 && len2 == 1)	return s1 == s2;
	vector<vector<bool>> matrix1;
	vector<vector<vector<bool>>> matrix_all;

	for (size_t i = 0; i < len1; i++)
	{
		vector<bool> aa;
		for (size_t j = 0; j < len1; j++)
		{
			aa.push_back(s1[i] == s2[j]);
		}
		matrix1.push_back(aa);
	}
	matrix_all.push_back(matrix1);

	vector<vector<bool>> matrix_matrix_new;


	for (size_t i = 1; i < len1; i++)
	{
		int tmp_len = i + 1;
		vector<vector<bool>> matrix_matrix_new;
		for (size_t j = 0; j < len1 - i; j++)
		{
			vector<bool> aline;
			for (size_t k = 0; k < len1 - i; k++)
			{
				bool block_stat = false;
				int limit = (i + 1) / 2;
				for (size_t y = 0; y < limit; y++)
				{
					int x = i - 1 - y;
					int xx = x + 1;
					int yy = y + 1;
					vector<vector<bool>>& a1 = matrix_all[y];
					vector<vector<bool>>& a2 = matrix_all[x];
					if (a2[j][k] && a1[j + xx][k + xx])
					{
						block_stat = true;
						break;
					}

					if (a1[j][k] && a2[j + yy][k + yy])
					{
						block_stat = true;
						break;
					}

					if (j + xx < a1.size() && k + yy< a2.size())
					{
						if (a2[j][k + yy] && a1[j + xx][k])
						{
							block_stat = true;
							break;
						}
					}

					if (k + xx < a1.size() && j + yy < a2.size())
					{
						if (a1[j][k + xx] && a2[j + yy][k])
						{
							block_stat = true;
							break;
						}
					}
				}
				aline.push_back(block_stat);
			}
			matrix_matrix_new.push_back(aline);
		}
		matrix_all.push_back(matrix_matrix_new);
	}

	bool ret = matrix_all[matrix_all.size() - 1][0][0];
	return ret;
}