#include "stdafx.h"

#if defined(_MSC_VER) && defined(_DEBUG)

#include "DebugSymbolMgr.h"
#include <mutex>

#pragma comment(lib, "Dbghelp.lib")

void DebugSymbolMgr::Init(HANDLE hProcess, const char* cpszSearchPath)
{
	m_cpszSearchPath = cpszSearchPath;
	m_hProcess = hProcess;
	SymInitialize(hProcess, cpszSearchPath, FALSE);
	//SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG);
}

DebugSymbolMgr::DebugSymbolMgr(HANDLE hProcess, const char* cpszSearchPath)
{
	Init(hProcess, cpszSearchPath);
}

DebugSymbolMgr::~DebugSymbolMgr()
{
	if (m_hProcess)
		SymCleanup(m_hProcess);
}

void DebugSymbolMgr::SetSearchPath(const char* cpszSearchPath)
{
	if (cpszSearchPath != m_cpszSearchPath)
	{
		m_cpszSearchPath = cpszSearchPath;
		if (m_hProcess)
		{
			SymCleanup(m_hProcess);
			Init(m_hProcess, cpszSearchPath);
		}
	}
}

void DebugSymbolMgr::Refresh()
{
	if (m_hProcess)
		SymRefreshModuleList(m_hProcess);
}

bool DebugSymbolMgr::GetCallInfo(
	_Out_ char szFile[], size_t uFileBufferSize,
	_Out_ int* pnLineNum,
	_Out_ char szFunc[], size_t uFuncBufferSize,
	int nIgnoreLayers/* = 1*/, const char* cpszSymbolPaths/* = nullptr*/
)
{
	BOOL                bRetCode = false;
	static const int    s_cnMaxNameLen = 64;
	void* pvStack[16];
	int                 nFrames = 0;
	HANDLE              hCurrentProcess = GetCurrentProcess();
	static DebugSymbolMgr    s_SymbolMgr(hCurrentProcess, cpszSymbolPaths);
	static std::mutex   s_symbolMutex;

	std::lock_guard<std::mutex> lock(s_symbolMutex);

	s_SymbolMgr.SetSearchPath(cpszSymbolPaths);

	nFrames = CaptureStackBackTrace(nIgnoreLayers, _countof(pvStack), pvStack, NULL);
	if (nFrames < 1)
		return false;

	for (int i = 0; i < nFrames; i++)
	{
		DWORD64         ullAddress = (DWORD64)(pvStack[i]);
		DWORD64         ullDisplacementSym = 0;
		BYTE            byBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO    pSymbol = (PSYMBOL_INFO)byBuffer;
		DWORD           dwDisplacementLine = 0;
		IMAGEHLP_LINE64 Line;

		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		bRetCode = SymFromAddr(hCurrentProcess, ullAddress, &ullDisplacementSym, pSymbol);

		if (!bRetCode)
		{
			s_SymbolMgr.Refresh();
			bRetCode = SymFromAddr(hCurrentProcess, ullAddress, &ullDisplacementSym, pSymbol);
		}

		if (SymGetLineFromAddr64(hCurrentProcess, ullAddress, &dwDisplacementLine, &Line))
		{
			if (strstr(Line.FileName, "Microsoft"))
				continue;

			if (strstr(Line.FileName, "\\crt\\"))
				continue;

			if (szFile && uFileBufferSize > 0)
				strncpy(szFile, Line.FileName, uFileBufferSize);
			if (szFunc && uFuncBufferSize)
				strncpy(szFunc, pSymbol->Name, uFuncBufferSize);
			*pnLineNum = Line.LineNumber;

			return true;
		}
	}

	// 	SymCleanup(process);

	return false;
}

#endif  // _MSC_VER  && _DEBUG