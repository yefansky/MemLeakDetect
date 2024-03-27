#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)
#include <Windows.h>
#include <DbgHelp.h>

class DebugSymbolMgr
{
private:
	const char* m_cpszSearchPath	= nullptr;
	HANDLE      m_hProcess			= nullptr;

public:
	struct CallInfo
	{
		char szFile[MAX_PATH] = "";
		int nLineNum = 0;
		char szFunc[64] = "";
	};

private:
	void Init(HANDLE hProcess, const char* cpszSearchPath);
public:
	DebugSymbolMgr(HANDLE hProcess, const char* cpszSearchPath);
	~DebugSymbolMgr();

	void SetSearchPath(const char* cpszSearchPath);
	void Refresh();

	static bool GetCallInfo(
		_Out_ char szFile[], size_t uFileBufferSize,
		_Out_ int* pnLineNum,
		_Out_ char szFunc[], size_t uFuncBufferSize,
		int nIgnoreLayers = 1, const char* cpszSymbolPaths = nullptr
	);
};
#endif // _MSC_VER && _DEBUG