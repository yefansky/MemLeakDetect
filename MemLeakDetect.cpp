#include "stdafx.h"

#ifdef _USE_MEM_LEAK_DETECT

#include "MemLeakDetect.h"

#undef malloc
#undef free

bool MemoryLeakDetect::s_bProcessing = false;
std::mutex MemoryLeakDetect::s_symbolMutex;

MemoryLeakDetect* MemoryLeakDetect::GetInstance()
{
	s_symbolMutex.lock();

	static MemoryLeakDetect* s_pInstance = nullptr;

	if (!s_pInstance)
	{
		s_bProcessing = true;
		s_pInstance = new MemoryLeakDetect();
		s_bProcessing = false;
	}

	s_symbolMutex.unlock();

	return s_pInstance;
}

bool MemoryLeakDetect::IsProcessing()
{
	return s_bProcessing;
}

void MemoryLeakDetect::Register(void* pvPointer, size_t uSize, const char* cpszFile, int nLineNum, const char* cpszFunc)
{
	s_symbolMutex.lock();

	s_bProcessing = true;

	BlockInfo info;
	info.strFile = cpszFile;
	info.nLineNum = nLineNum;
	info.strFuncName = cpszFunc;
	info.uSize = uSize;

	m_infoMap.emplace(pvPointer, info);
	s_bProcessing = false;

	s_symbolMutex.unlock();
}

void MemoryLeakDetect::Unregister(void* pvPointer)
{
	s_symbolMutex.lock();

	s_bProcessing = true;
	m_infoMap.erase(pvPointer);
	s_bProcessing = false;

	s_symbolMutex.unlock();
}

void MemoryLeakDetect::Dump()
{
	s_symbolMutex.lock();

	s_bProcessing = true;
	for (auto& rInfoPair : m_infoMap)
	{
		const void* pvPointer = rInfoPair.first;
		auto& rInfo = rInfoPair.second;
		printf("MemoryLeakDetect: address:%p size:%lld, path:%s, line:%d, func:%s\n",
			pvPointer, rInfo.uSize, rInfo.strFile.c_str(), rInfo.nLineNum, rInfo.strFuncName.c_str()
		);
	}
	s_bProcessing = false;

	s_symbolMutex.unlock();
}

void* operator new(size_t uSize)
{
	void* pvPointer = malloc(uSize);
	if (pvPointer && !MemoryLeakDetect::IsProcessing())
	{
		DebugSymbolMgr::CallInfo callInfo;
		DebugSymbolMgr::GetCallInfo(
			callInfo.szFile, sizeof(callInfo.szFile),
			&callInfo.nLineNum,
			callInfo.szFunc, sizeof(callInfo.szFunc),
			2
		);
		MemoryLeakDetect::GetInstance()->Register(pvPointer, uSize, callInfo.szFile, callInfo.nLineNum, callInfo.szFunc);
	}

	return pvPointer;
}

void* operator new(size_t uSize, int nCount)
{
	uSize *= nCount;
	void* pvPointer = malloc(uSize);
	if (pvPointer && !MemoryLeakDetect::IsProcessing())
	{
		DebugSymbolMgr::CallInfo callInfo;
		DebugSymbolMgr::GetCallInfo(
			callInfo.szFile, sizeof(callInfo.szFile),
			&callInfo.nLineNum,
			callInfo.szFunc, sizeof(callInfo.szFunc),
			2
		);
		MemoryLeakDetect::GetInstance()->Register(pvPointer, uSize, callInfo.szFile, callInfo.nLineNum, callInfo.szFunc);
	}
	return pvPointer;
}

void operator delete(void* pvPointer)
{
	if (pvPointer && !MemoryLeakDetect::IsProcessing())
		MemoryLeakDetect::GetInstance()->Unregister(pvPointer);
	free(pvPointer);
}

void* _DETECT_LEAK_malloc(size_t uSize, const char* cpszFile, int nLine, const char* cpszFunc)
{
	void* pvPointer = malloc(uSize);
	if (pvPointer && !MemoryLeakDetect::IsProcessing())
		MemoryLeakDetect::GetInstance()->Register(pvPointer, uSize, cpszFile, nLine, cpszFunc);
	return pvPointer;
}

void _DETECT_LEAK_delete(void* pvPointer)
{
	if (pvPointer && !MemoryLeakDetect::IsProcessing())
		MemoryLeakDetect::GetInstance()->Unregister(pvPointer);
	free(pvPointer);
}

#endif // _USE_MEM_LEAK_DETECT