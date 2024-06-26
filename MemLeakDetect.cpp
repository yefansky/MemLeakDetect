#include "MemLeakDetect.h"
#include "DebugSymbolMgr.h"

#undef malloc
#undef free

MemoryLeakDetect* MemoryLeakDetect::GetInstance()
{
	static std::mutex s_mutex;

	MemoryLeakDetect* pResult = nullptr;

	s_mutex.lock();

	static MemoryLeakDetect s_Instance;

	pResult = &s_Instance;

	s_mutex.unlock();

	return pResult;
}

void MemoryLeakDetect::Register(void* pvPointer, size_t uSize, const char* cpszFile, int nLineNum, const char* cpszFunc)
{
	m_symbolMutex.lock();

	BlockInfo info;
	info.strFile = cpszFile;
	info.nLineNum = nLineNum;
	info.strFuncName = cpszFunc;
	info.uSize = uSize;

	m_infoMap.emplace(pvPointer, info);

	m_symbolMutex.unlock();
}

void MemoryLeakDetect::Unregister(void* pvPointer)
{
	m_symbolMutex.lock();

	m_infoMap.erase(pvPointer);

	m_symbolMutex.unlock();
}

// 函数用于获取当前时间的字符串表示
static void GetCurrentTimeAsString(char* pszBuffer, size_t uBufferSize) {
	time_t now;
	struct tm* pLocalTime;

	// 获取当前时间
	time(&now);
	pLocalTime = localtime(&now);

	// 构建日期时间字符串
	strftime(pszBuffer, uBufferSize, "%Y-%m-%d-%H-%M-%S", pLocalTime);
}

// 函数用于获取当前可执行文件名
static void GetExeName(char* pszBuffer, size_t uBufferSize) 
{
#ifdef _WIN32
	GetModuleFileNameA(NULL, pszBuffer, (DWORD)uBufferSize);
	// 从路径中提取文件名
	char* pLastBackslash = strrchr(pszBuffer, '\\');
#else
	ssize_t len = readlink("/proc/self/exe", pszBuffer, uBufferSize - 1);
	if (len != -1) 
		pszBuffer[len] = '\0';
	// 从路径中提取文件名
	char* pLastBackslash = strrchr(pszBuffer, '/');
#endif

	if (pLastBackslash != NULL) 
	{
		// 移动文件名到缓冲区起始位置
		memmove(pszBuffer, pLastBackslash + 1, strlen(pLastBackslash + 1) + 1);
	}

	// 去掉扩展名
	char* pDot = strrchr(pszBuffer, '.');
	if (pDot != NULL) 
		*pDot = '\0';

	pszBuffer[uBufferSize - 1] = '\0';
}

static void GetLogFileName(char* pszBuffer, size_t uBufferName)
{
	char szExeName[128];
	char szTime[128];

	GetExeName(szExeName, sizeof(szExeName));
	GetCurrentTimeAsString(szTime, sizeof(szTime));

	snprintf(pszBuffer, uBufferName, "MEMLEAK_DETECT_%s_%s.log", szExeName, szTime);
	pszBuffer[uBufferName - 1] = '\0';
}

void MemoryLeakDetect::Dump()
{
	m_symbolMutex.lock();

	char szFileName[128];
	GetLogFileName(szFileName, sizeof(szFileName));
	FILE* pfFile = fopen(szFileName, "w+");

	for (auto& rInfoPair : m_infoMap)
	{
		const void* pvPointer = rInfoPair.first;
		auto& rInfo = rInfoPair.second;

		if (rInfo.bIsGlobal)
			continue;

		printf("MemoryLeakDetect: address:%p size:%lld, path:%s, line:%d, func:%s\n",
			pvPointer, rInfo.uSize, rInfo.strFile.c_str(), rInfo.nLineNum, rInfo.strFuncName.c_str()
		);

		if (pfFile)
			fprintf(pfFile, "MemoryLeakDetect: address:%p size:%lld, path:%s, line:%d, func:%s\n",
				pvPointer, rInfo.uSize, rInfo.strFile.c_str(), rInfo.nLineNum, rInfo.strFuncName.c_str()
			);
	}

	if (pfFile)
		fclose(pfFile);

	m_symbolMutex.unlock();
}

void MemoryLeakDetect::MarkGlobal()
{
	for (auto& rInfoPair : m_infoMap)
	{
		auto& rInfo = rInfoPair.second;
		rInfo.bIsGlobal = true;
	}
}

void* operator new(size_t uSize)
{
	void* pvPointer = malloc(uSize);
	if (pvPointer)
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
	if (pvPointer)
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
	assert(pvPointer);
	
	MemoryLeakDetect::GetInstance()->Unregister(pvPointer);
	free(pvPointer);
}

void* _DETECT_LEAK_malloc(size_t uSize, const char* cpszFile, int nLine, const char* cpszFunc)
{
	void* pvPointer = malloc(uSize);
	if (pvPointer)
		MemoryLeakDetect::GetInstance()->Register(pvPointer, uSize, cpszFile, nLine, cpszFunc);
	return pvPointer;
}

void _DETECT_LEAK_delete(void* pvPointer)
{
	assert(pvPointer);
	MemoryLeakDetect::GetInstance()->Unregister(pvPointer);
	free(pvPointer);
}
