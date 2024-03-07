#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include "errors.h"
#include "prep.h"
#include "PMI.h"

//
//  ID og Pass henter for 64-bit portable-qs versjon 1.2.3+39 startet med admin rettigheter for elevering.
//

int main()
{
	SetConsoleTitleW(L"RustDeskIDPASS");
	setlocale(LC_ALL,"en_US.UTF-8"); 
	SetConsoleOutputCP(CP_UTF8); 

	// Forbered prosess for debugging.
	try {
		hProc.PrepProcess(L"rustdesk.exe");
	} catch (...) {
		GetError("PrepProcess");
	}

	// librustdesk.dll offsets.
	uintptr_t ID_dll_offset = hProc.ProcDllBasePtr + 0x178C060;
	uintptr_t PASS_dll_offset = hProc.ProcDllBasePtr + 0x178C148;
	std::vector<int> id_offsets = { 0x20 };
	std::vector<int> pass_offsets = { 0x20 };

	// Les verdier.
	LesPassID(ID_dll_offset, "ID", id_offsets);
	LesPassID(PASS_dll_offset, "PASS", pass_offsets);

	return 0;
}
