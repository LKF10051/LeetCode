// use hash to Simplify the problem,  O(n) time complexity
char* remove(char* s1, const char* s2)
{
	if(s1 == NULL || s2 == NULL)
	{ return s1; }

	if (*s1 == '/0' || *s2 == '/0')
	{ return s1; }

	int hashTable[256] = {0};
	int index = 0;

	// construct hash
	while(true)
	{
		if (*(s2 + index) == '\0')
		{
			break;
		}

		hashTable[(int) *(s2 + index)] = 1;
		index++;
	}

	index = 0;
	int writeIndex = 0;
	while (true)
	{
		if (*(s1 + index) == '\0')
		{
			break;
		}

		if ( hashTable[(int)*(s1 + index)] == 0)
		{
			s1[writeIndex]= *(s1 + index);
			writeIndex++;
		}
		index++;
	}

	s1[writeIndex]= '/0';

	return s1;
}
