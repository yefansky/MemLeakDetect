#pragma once

//#define _USE_MEM_LEAK_DETECT

// ʹ��˵����
// 1.�ڹ���Ԥ���������� _USE_MEM_LEAK_DETECT�� ������stdafx.h �������� #define _USE_MEM_LEAK_DETECT
// 2.��MemLeakDetect.h ����stdafx.h �����ϲ�
// 3.��main�����Ķ��˷��� MEMORY_LEAK_DETECT_INIT()
// 4.���� KGMemLeakDetect.lib ��̬��
/* ���磺
int main()
{
	MEMORY_LEAK_DETECT_INIT();

	...

}
*/

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_USE_MEM_LEAK_DETECT)

#define WIN32_LEAN_AND_MEAN // �����������Ҫ�� Windows ͷ�ļ�
//#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <string>
#include <map>
#include <stdlib.h>
#include <iostream>
#include <dbghelp.h>
#include <mutex>
#include <assert.h>
#include <memory>

class MemoryLeakDetect
{
private:
	// ʹ��ԭ���� new ������
	void* operator new(size_t uSize) 
	{
		return malloc(uSize);
	}

	void* operator new(size_t uSize, int nCount)
	{
		uSize *= nCount;
		return malloc(uSize);
	}

	// ʹ��ԭ���� delete ������
	void operator delete(void* pvPointer) noexcept 
	{
		free(pvPointer);
	}

	template <typename T>
	class _Allocator {
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// ���캯������������
		_Allocator() noexcept {}
		~_Allocator() noexcept {}

		// �������캯��
		template <typename U>
		_Allocator(const _Allocator<U>&) noexcept {}

		// �����ڴ�
		pointer allocate(size_type n) 
		{
			return static_cast<pointer>(operator new(n * sizeof(value_type)));
		}

		// �ͷ��ڴ�
		void deallocate(pointer p, size_type n)
		{
			UNREFERENCED_PARAMETER(n);
			operator delete(p);
		}

		// ������
		template <typename U, typename... Args>
		void construct(U* p, Args&&... args) 
		{
			new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
		}

		// ��������
		template <typename U>
		void destroy(U* p) 
		{
			p->~U();
		}

		// rebind����
		template <typename U>
		struct rebind 
		{
			using other = _Allocator<U>;
		};
	};

	using _string = std::basic_string<char, std::char_traits<char>, _Allocator<char>>;

	template<typename Key, typename T, typename Compare = std::less<Key>>
	using _map = std::map<Key, T, Compare, _Allocator<std::pair<const Key, T>>>;

	/*
	template<typename T, typename Compare = std::less<T>>
	using _set = std::set<T, Compare, _Allocator<T>>;

	template<typename T>
	using _vector = std::vector<T, _Allocator<T>>;

	template<typename T>
	using _list = std::list<T, _Allocator<T>>;

	template<typename T>
	using _deque = std::deque<T, _Allocator<T>>;
	*/
public:
	static MemoryLeakDetect* GetInstance();

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
		bool	bIsGlobal	= false;
		_string strFile;
		int		nLineNum	= -1;
		_string strFuncName;
		size_t	uSize		= 0;
	};
	_map<const void*, BlockInfo> m_infoMap;

	std::mutex m_symbolMutex;
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

#endif // _MSVC_ && _DEBUG

#ifdef _USE_MEM_LEAK_DETECT
	#define MEMORY_LEAK_DETECT_INIT()	\
	KMemory::UseCRTMalloc(true);		\
	KGMemoryUseCRTMalloc(true);			\
	static MemoryLeakDetectGuard _MemoryLeakDetectGuard;
#else
	#define MEMORY_LEAK_DETECT_INIT()
#endif