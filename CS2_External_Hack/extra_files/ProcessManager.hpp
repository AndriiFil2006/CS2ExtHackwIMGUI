#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <Tlhelp32.h>
#include <atlconv.h>
#define _is_invalid(v) if(v==NULL) return false
#define _is_invalid(v,n) if(v==NULL) return n

/*
	@Liv github.com/TKazer
*/

/// <summary>
/// 进程状态码
/// </summary>
enum StatusCode
{
	SUCCEED,
	FAILE_PROCESSID,
	FAILE_HPROCESS,
	FAILE_MODULE,
};

/// <summary>
/// 进程管理
/// </summary>
class ProcessManager 
{
private:

	bool   Attached = false;

public:

	HANDLE hProcess = 0;
	DWORD  ProcessID = 0;
	DWORD64  ModuleAddress = 0;

public:
	~ProcessManager()
	{
		//if (hProcess)
			//CloseHandle(hProcess);
	}


	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		_is_invalid(hProcess,false);
		_is_invalid(ProcessID, false);

		if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(Address), &Value, Size, 0))
			return true;
		return false;
	}

	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value)
	{
		_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(Address), &Value, sizeof(ReadType), 0))
			return true;
		return false;
	}

	/// <summary>
	/// 写入进程内存
	/// </summary>
	/// <typeparam name="ReadType">写入类型</typeparam>
	/// <param name="Address">写入地址</param>
	/// <param name="Value">写入数据</param>
	/// <param name="Size">写入大小</param>
	/// <returns>是否写入成功</returns>
	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (WriteProcessMemory(hProcess, reinterpret_cast<LPCVOID>(Address), &Value, Size, 0))
			return true;
		return false;
	}

	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value)
	{
		_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(Address), &Value, sizeof(ReadType), 0))
			return true;
		return false;
	}

	/// <summary>
	/// 特征码搜索
	/// </summary>
	/// <param name="Signature">特征码</param>
	/// <param name="StartAddress">起始地址</param>
	/// <param name="EndAddress">结束地址</param>
	/// <returns>匹配特征结果</returns>

	DWORD64 TraceAddress(DWORD64 BaseAddress, std::vector<DWORD> Offsets)
	{
		_is_invalid(hProcess, 0);
		_is_invalid(ProcessID, 0);
		DWORD64 Address = 0;

		if (Offsets.size() == 0)
			return BaseAddress;

		if (!ReadMemory<DWORD64>(BaseAddress, Address))
			return 0;

		for (int i = 0; i < Offsets.size() - 1; i++)
		{
			if (!ReadMemory<DWORD64>(Address + Offsets[i], Address))
				return 0;
		}
		return Address == 0 ? 0 : Address + Offsets[Offsets.size() - 1];
	}


public:

	DWORD GetProcessID(std::string ProcessName)
	{
		PROCESSENTRY32 ProcessInfoPE;
		ProcessInfoPE.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(15, 0);
		Process32First(hSnapshot, &ProcessInfoPE);
		USES_CONVERSION;
		do {
			if (strcmp(W2A(ProcessInfoPE.szExeFile), ProcessName.c_str()) == 0)
			{
				CloseHandle(hSnapshot);
				return ProcessInfoPE.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &ProcessInfoPE));
		CloseHandle(hSnapshot);
		return 0;
	}

	HMODULE GetProcessModuleHandle(DWORD processID, std::string ModuleName)
	{
		MODULEENTRY32 ModuleInfoPE;
		ModuleInfoPE.dwSize = sizeof(MODULEENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
		Module32First(hSnapshot, &ModuleInfoPE);
		USES_CONVERSION;
		do {
			if (strcmp(W2A(ModuleInfoPE.szModule), ModuleName.c_str()) == 0)
			{
				CloseHandle(hSnapshot);
				return ModuleInfoPE.hModule;
			}
		} while (Module32Next(hSnapshot, &ModuleInfoPE));
		CloseHandle(hSnapshot);
		return 0;
	}

}ProcessMgr;
