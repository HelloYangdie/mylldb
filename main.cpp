#include <iostream>
#include <stdlib.h>
#include "test/test.h"


int main()
{
	//SkipListTest();

#if defined(_MSC_VER)
	std::cout << "OS_WIN" << " " << "COMPILER_MSVC" << std::endl;
#else
	std::cout << " not " << "OS_WIN" << " " << "COMPILER_MSVC" << std::endl;
#endif

	system("pause");
	return 0;
}