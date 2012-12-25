#include "main.h"
DWORD err = 0;

LONGLONG SizeFromName(LPCWSTR szFileName) // Returns a file's size from its filename, returns a LONGLONG, in case you have a LARGE LARGE file :)
{
	LARGE_INTEGER fileSize = {0};
	HANDLE hfile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Get a handle on the file

	if (hfile==INVALID_HANDLE_VALUE) //if something went wrong ...
	{
		err = GetLastError();
		dprintf(L"[-] Invalid file handle! CreateFile() returned : %08x\n", err);
		//CloseHandle(hfile);
		return -1;
	}

	if(!GetFileSizeEx(hfile,&fileSize)) // Get the size from the file handle
	{
		err = GetLastError();
		dprintf(L"[-] Error getting file size! GetFileSizeEx() returned : %08x\n", err);
		//CloseHandle(hfile);
		return -1;
	}

	//CloseHandle(hfile); // this will ALWAYS throw an exception if run under a debugger, but good higene if run under "production"
	return fileSize.QuadPart; //LARGE_INTEGER is a sruct, QuadPart is the filesize in a 64bit digit... which should cover all file sizes "That's for files >4GB" 
} 

DWORD CopyFileToBuffer(LPCWSTR szFileName, BYTE** buffer)
{
	// get file size...
	dprintf(L"[*] Copying file \"%s\" to buffer...\n", szFileName);
	LONGLONG size = 0;
	size = SizeFromName(szFileName);
	if (size == -1)
	{
		return -1;
	}
	else {
		dprintf(L"[*] Size of \"%s\" is \"%d\" bytes.\n", szFileName, size);
	}

	// Allocate memory ...
	dprintf(L"[*] Trying to VirtualAlloc \"%d + 5\" bytes of data ... the extra 5 are for ASM voodoo\n", size);
	*buffer = (BYTE*)VirtualAlloc(0, (SIZE_T)size + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (buffer == NULL)
	{
		err = GetLastError();
		dprintf(L"[-] Failed to allocate memory! VirtualAlloc() returned : %08x\n", err);
		return -1;
	}
	dprintf(L"[*] Success! \"%d + 5\" bytes of data allocated.\n", size);

	// reading file content into buffer...
	HANDLE hfile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Get a handle on the file

	if (hfile==INVALID_HANDLE_VALUE) //if something went wrong ...
	{
		err = GetLastError();
		dprintf(L"[-] Invalid file handle! CreateFile() returned : %08x\n", err);
		//CloseHandle(hfile);
		return -1;
	}

	if( FALSE == ReadFile(hfile, *buffer, (DWORD)size-1, NULL, NULL) )
	{
		printf("Terminal failure: Unable to read from file.\n");
		//CloseHandle(hfile);
		return 0;
	}
	return (DWORD)size;
}

int PatchString(BYTE* buffer, const char* cOriginal, const int index, const int NoOfBytes)
{
	/*
	DWORD* pOffset = nullptr;
	wchar_t UNIcOriginal[1024] = {0}, UNIcReplacement[1024] = {0};

	if(AnsiToUnicode(cOriginal, UNIcOriginal)	&& AnsiToUnicode	(cReplacement, UNIcReplacement))
	{
	dprintf(L"[*] Patching buffer: \"%s\" -> \"%s\"...\n", UNIcOriginal, UNIcReplacement);
	}


	//pOffset = strstr(buffer, cOriginal); //strstr doesn't work on binary data ... :/
	if (pOffset == NULL || pOffset == nullptr)
	{
	dprintf(L"[-] Couldn't get the offset of \"%s\" ... it's either not there, or something screwed up.", cOriginal);
	return 1;
	}
	else {
	dprintf(L"[*] Found \"%s\" at offset \"0x%08x\".", UNIcOriginal, pOffset);
	}
	*/
	int counter = 0;
	for(int i = index; i < (index + NoOfBytes); i++)
	{
		if(counter == NoOfBytes) break;
		buffer[i] = cOriginal[counter];
		counter++;
	}
	return 0;
}

DWORD binstrstr(BYTE * buff1, int lenbuff1, BYTE * buff2, int lenbuff2)  // shamelessly ripped from http://forums.devshed.com/c-programming-42/binary-strstr-395935.html , thanks "AlejandroVarela"
{ 
	if (! buff1)                return FALSE; 
	if (! buff2)                return FALSE; 
	if (lenbuff1 == 0)            return FALSE; 
	if (lenbuff2 == 0)            return FALSE; 
	if (lenbuff1 < lenbuff2)            return FALSE; 

	for (int i = 0; i <= (lenbuff1 - lenbuff2); ++ i) 
	{
		if (memcmp(buff1 + i, buff2, lenbuff2-1) == 0)
		{
			return i; 
		}
	}

	return FALSE; 
}  
bool AnsiToUnicode(const char* ascii, wchar_t* unicode)
{
	size_t len = strlen(ascii);
	if(len < 1024)
	{
		int result = MultiByteToWideChar(CP_OEMCP, 0, ascii, -1, unicode, len + 1);
		return TRUE;
	}
	else return FALSE;
}

