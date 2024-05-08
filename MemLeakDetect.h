#pragma once

#define _USE_MEM_LEAK_DETECT

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_USE_MEM_LEAK_DETECT)

#define WIN32_LEAN_AND_MEAN // 避免包含不必要的 Windows 头文件
//#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <string>
#include <map>
#include <stdlib.h>
#include <iostream>
#include <dbghelp.h>
#include <mutex>
#include <assert.h>

class MemoryLeakDetect
{
	// 使用原生的 new 操作符
	void* operator new(size_t size) {
		return ::operator new(size);
	}

	// 使用原生的 delete 操作符
	void operator delete(void* ptr) noexcept {
		::operator delete(ptr);
	}

public:
	static MemoryLeakDetect* GetInstance();
	static bool IsProcessing();

	~MemoryLeakDetect()
	{
		Dump();
	}
	
	void Register(void* pvPointer, size_t uSize, const char* cpszFile, int nLineNum, const char* cpszFunc);
	void Unregister(void* pvPointer);

	void Dump();
	void MarkGlobal();
private:
	struct BlockInfo
	{
		bool bIsGlobal = false;
		std::string strFile;
		int nLineNum = -1;
		std::string strFuncName;
		size_t uSize = 0;
	};
	std::map<const void*, BlockInfo> m_infoMap;
	volatile static bool s_bProcessing;

	static std::mutex s_symbolMutex;
};

void* operator new(size_t uSize);
void* operator new(size_t uSize, int nCount);

void operator delete(void* pvPointer);

void* _DETECT_LEAK_malloc(size_t uSize, const char* cpszFile, int nLine, const char* cpszFunc);
void _DETECT_LEAK_delete(void* pvPointer);

#define malloc(_SIZE) _DETECT_LEAK_malloc(_SIZE, __FILE__, __LINE__, __FUNCTION__)
#define free(_POINTER) _DETECT_LEAK_delete(_POINTER)


struct MemoryLeakDetectGuard
{
	MemoryLeakDetect* pDetect = nullptr;

	MemoryLeakDetectGuard()
	{
		pDetect = MemoryLeakDetect::GetInstance();
		pDetect->MarkGlobal();
	}

	~MemoryLeakDetectGuard()
	{
		pDetect->Dump();
	}

};

#define MEMORY_LEAK_DETECT_INIT() static MemoryLeakDetectGuard _MemoryLeakDetectGuard;

#endif // _MSVC_ && _DEBUG