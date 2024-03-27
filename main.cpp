// MemLeakDetect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
