#pragma 

//
//	Funksjon som leser bytes fra ønsket addr.  
//
bool ReadMem(
	bool AddBaseAddr,
	uintptr_t ReadAddr,
	unsigned char* ReadBuf,
	SIZE_T BufSize
) {
	try {
		// Skal det brukes offset fra baseadresse?
		if(AddBaseAddr)
			ReadAddr = hProc.ProcBasePtr + ReadAddr;

		if(ReadProcessMemory(
			hProc.__HandleProcess,
			(void*)ReadAddr,
			ReadBuf,
			BufSize,
			NULL
		) == 0) {
			GetError("RPM returnerte 0.");
		}

		return true;

	} catch(...) {
		GetError("ReadMem");
	}
}


//
//  Hovedfunksjon for lesing av verdier.
//
void LesPassID(uintptr_t IDPASS, const char* Hva, std::vector<int>& offsets)
{
	//
	//	Finn første adresse.
	//
	//std::cout << u8"Finner første adresse til " << Hva << ": ";
	unsigned char ReadAddrBuf[8] = {0};
	if (ReadMem(false, IDPASS, ReadAddrBuf, 8)) 
		std::memcpy(&IDPASS, ReadAddrBuf, 8);
	//std::cout << std::hex << IDPASS;
	//std::cout << "\n";

    // Les offsets.
	//std::cout << u8"Leser offsets til " << Hva << ": ";
	for (int n=0; n<offsets.size(); n++) 
		if (ReadMem(false, IDPASS + offsets[n], ReadAddrBuf, 8)) 
			std::memcpy(&IDPASS, ReadAddrBuf, 8);
	//std::cout << std::hex << IDPASS;
	//std::cout << "\n";

	// Les verdien.
	std::cout << u8"Leser verdien til " << Hva << ": ";
	unsigned char passid[128] = {0};

	// Justering av lengde som leses...
	// Vanligvis er passord 6 og ID maks 10.
	if (!memcmp(Hva, "ID", 3)) {
		ReadMem(false, IDPASS, passid, 10);

		// ID kan være kortere enn 10 tall.
		// Sjekk tall innefor UTF-8 0x30 - 0x39.
		for (int a=0; a<10; a++) {
			bool gyldighex = false;
			for (int b=0x30; b<=0x39; b++) {
				if (passid[a] == b)
					gyldighex = true;
			}
			if (!gyldighex)
				passid[a] = '\0';
		}

	} else if (!memcmp(Hva, "PASS", 5)) {
		ReadMem(false, IDPASS, passid, 6);
	}

	// Returner verdi.
	std::cout << passid << "\n";
}