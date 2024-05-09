#pragma once
#include "TestCharacter.h"
#include <list>
#include <time.h>
#include "MemLeakDetect.h"

class TestWorld
{
public:
	std::list<TestCharacter*> m_characterList;

	void Init()
	{
		for (int i = 0; i < 100; i++)
		{
			auto pCharacter = new TestCharacter();
			m_characterList.push_back(pCharacter);
		}
	}

	void UnInit()
	{
		for (auto pCharacter : m_characterList)
			delete pCharacter;
		m_characterList.clear();
	}

	void Activate()
	{
		time_t nNextTime = time(NULL);
		int nFrame = 0;

		while (true)
		{
			time_t nNow = time(NULL);

			if (nNow < nNextTime)
			{
				continue;
			}

			nNextTime += 1;

			nFrame++;
#ifdef _USE_MEM_LEAK_DETECT
			if (nFrame % 10 == 1)
				MemoryLeakDetect::GetInstance()->Dump();
#endif
		}
	}
};

