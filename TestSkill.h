#pragma once
#include <list>

class TestSkill
{
public:
	std::list<int> m_attributeList;

	TestSkill()
	{
		for (int i = 0; i < 10; i++)
			m_attributeList.push_back(i);
	}
};

