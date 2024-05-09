#include "stdafx.h"
#include "MemLeakDetect.h"
#include <iostream>
#include "TestWorld.h"

class LeakTestClassA
{
	int nValue;
};

class LeakTestClassB
{
public:
	void Alloc();
};

void LeakTestClassB::Alloc()
{
	LeakTestClassA* pArray = new LeakTestClassA[32];
}

int main()
{
	/*
	char* pszMalloc = (char*)malloc(1024);
	char* pszNew = new char[1023];
	pszNew = new char[1021];

	LeakTestClassB B;

	B.Alloc();

	MemoryLeakDetect::GetInstance()->Dump();
	*/

	TestWorld* pWorld = new TestWorld();

	if (pWorld)
	{
		pWorld->Init();

		pWorld->Activate();

		pWorld->UnInit();
	}

	return 0;
}
