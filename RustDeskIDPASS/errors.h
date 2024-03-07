#pragma once

//
//	Generell feilhåndterer av exceptions o.l. i programmet.
//
void GetError(std::string lpszFunction)
{
	int err = GetLastError();
	std::string lpDisplayBuf;
	char* lpMsgBuf;

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0,
		NULL);

	lpDisplayBuf.append("(" + std::to_string(err) + ") ");
	lpDisplayBuf.append(lpMsgBuf);

	std::string TotalMessage;
	TotalMessage.append(lpszFunction + "\n\nSystem:\n" + lpDisplayBuf);

	std::cout << "Lesing av ID og/eller PASS feilet.\nBe kunde manuelt om info.\n\n" << TotalMessage.c_str();
	exit(1);
}
