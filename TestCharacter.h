#pragma once

#include <map>
#include <vector>

class TestCharacter
{
private:
	std::vector<int> m_skillList;
	std::map<int, int> m_representMap;

	char* m_pszName = nullptr;
	void* m_pvPayload = nullptr;

public:
	TestCharacter()
	{
		m_pszName = new char[32];

		m_pvPayload = new int[16];
	}
};

