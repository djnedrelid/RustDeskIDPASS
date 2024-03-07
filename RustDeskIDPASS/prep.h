﻿#pragma once
#include <TlHelp32.h>

// Forberedelsesklasse for prosess som skal hackes.
class hC1
{
	public:
	HANDLE __HandleProcess;
	PROCESSENTRY32W __GameProcess;
	uintptr_t ProcDllBasePtr = 0x0;
	uintptr_t ProcDllEndPtr = 0x0;
	uintptr_t ProcBasePtr = 0x0;
	uintptr_t ProcEndPtr = 0x0;


	//
	//	Funksjon for å hente PID til målprosess.
	//
	private:
	bool FindProcessName(
		const wchar_t* __ProcessName,
		PROCESSENTRY32W* pEntry
	) {
		PROCESSENTRY32W __ProcessEntry = {0};
		__ProcessEntry.dwSize = sizeof(PROCESSENTRY32W);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(
			TH32CS_SNAPPROCESS,
			0
		);

		if(hSnapshot == INVALID_HANDLE_VALUE)
			return false;

		if(!Process32FirstW(hSnapshot, &__ProcessEntry)) {
			CloseHandle(hSnapshot);
			return false;
		}

		int pcount = 0;
		do {
			if(!_wcsicmp(__ProcessEntry.szExeFile, __ProcessName))
			{
				// Det er 2 prosesser med samme navn, 
				// men ser ut som målprosess alltid er først.
				memcpy(
					(void*)pEntry,
					(void*)&__ProcessEntry,
					sizeof(PROCESSENTRY32W)
				);
				CloseHandle(hSnapshot);
				return true;
			}

		} while(Process32NextW(hSnapshot, &__ProcessEntry));

		CloseHandle(hSnapshot);
		return false;
	}

	//
	//	Funksjon for debug-privilegier for oss selv. 
	//	Gir optimale tilgangsrettigheter mot målprosess.
	//
	void runSetDebugPrivs()
	{
		HANDLE __HandleProcess = GetCurrentProcess(), __HandleToken;
		TOKEN_PRIVILEGES priv;
		LUID __LUID;

		OpenProcessToken(
			__HandleProcess,
			TOKEN_ADJUST_PRIVILEGES,
			&__HandleToken
		);

		LookupPrivilegeValue(0, L"SeDebugPrivilege", &__LUID);
		priv.PrivilegeCount = 1;
		priv.Privileges[0].Luid = __LUID;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(__HandleToken, false, &priv, 0, 0, 0);

		CloseHandle(__HandleToken);
		CloseHandle(__HandleProcess);
	}

	//
	//	Funksjon for henting av minnepekere til moduler.
	//	Brukes som entrypoints for offsets vi har funnet.
	//
	public:
	uintptr_t GetModuleNamePointer(
		const wchar_t* LPSTRModuleName,
		DWORD __DwordProcessId,
		uintptr_t* EndAddrPtr
	) {
		MODULEENTRY32W lpModuleEntry = {0};
		lpModuleEntry.dwSize = sizeof(MODULEENTRY32W);
		HANDLE hSnapShot = CreateToolhelp32Snapshot(
			TH32CS_SNAPMODULE,
			__DwordProcessId
		);

		if(!hSnapShot)
			return 0x0;

		lpModuleEntry.dwSize = sizeof(lpModuleEntry);
		BOOL __RunModule = Module32FirstW(hSnapShot, &lpModuleEntry);

		while (__RunModule) {

			if (!_wcsicmp(lpModuleEntry.szModule, LPSTRModuleName)) {
			
				// Regn ut sluttadresse som søk kan stoppe på.
				*EndAddrPtr = (uintptr_t)lpModuleEntry.modBaseAddr +
					(uintptr_t)lpModuleEntry.modBaseSize;

				return (uintptr_t)lpModuleEntry.modBaseAddr;
			}
			__RunModule = Module32NextW(hSnapShot, &lpModuleEntry);
		}
		
		CloseHandle(hSnapShot);
		return 0x0;
	}

	//
	//	Funksjon for klargjøring av prosessminne.
	//	Kjøres en enkelt gang under oppstart og kaster evt. et unntak 
	//	for videre feilhåndtering og avslutting av programmet.
	//
	void PrepProcess(const wchar_t* ExeProcessFileName) {

		// Gi oss selv debug rettigheter.
		// Mest aktuelt for system-prosesser.
		runSetDebugPrivs();

		// Prosessinfo.
		if (!FindProcessName(
			ExeProcessFileName,
			&__GameProcess
		)) {
			throw 1;
			return;
		}

		// HANDLE til hovedprosess.
		if ((__HandleProcess = OpenProcess(
			SYNCHRONIZE | 
			PROCESS_VM_READ |
			PROCESS_VM_WRITE |
			PROCESS_VM_OPERATION |
			PROCESS_QUERY_INFORMATION,
			false,
			__GameProcess.th32ProcessID
		)) == NULL) {
			throw 1;
			return;
		}

		// Peker til baseadresse til hovedprosess.
		if ((ProcBasePtr = GetModuleNamePointer(
			ExeProcessFileName,
			__GameProcess.th32ProcessID,
			&ProcEndPtr
		)) == 0x0) {
			CloseHandle(__HandleProcess);
			throw 1;
			return;
		}

		//
		// Peker til baseadresse til spesifik DLL etter behov.
		//
		if ((ProcDllBasePtr = GetModuleNamePointer(
			L"librustdesk.dll",
			__GameProcess.th32ProcessID,
			&ProcDllEndPtr
		)) == 0x0) {
			CloseHandle(__HandleProcess);
			throw 1;
			return;
		}
	}
};

hC1 hProc;