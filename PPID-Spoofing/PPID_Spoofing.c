#include <Windows.h>
#include <stdio.h>


// disable error 4996 (caused by sprint)
#pragma warning (disable:4996)


#define TARGET_PROCESS		"RuntimeBroker.exe -Embedding"


BOOL CreatePPIDSpoofedProcess(IN HANDLE hParentProc, IN LPCSTR lpProcName, OUT DWORD* dwProcId, OUT HANDLE* hProcess, OUT HANDLE* hThread) {

	CHAR lpPath[MAX_PATH * 2];			
	CHAR CurrentDirectory[MAX_PATH];	
	CHAR WinDir[MAX_PATH];						

	SIZE_T sThreadAttrSize = NULL;
	LPPROC_THREAD_ATTRIBUTE_LIST pThreadAttrListAddr = NULL;

	STARTUPINFOEXA SiEx = { 0 };
	PROCESS_INFORMATION Pi = { 0 };


	RtlSecureZeroMemory(&SiEx, sizeof(STARTUPINFOEX));
	RtlSecureZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));

	SiEx.StartupInfo.cb = sizeof(STARTUPINFO);

	if (!GetEnvironmentVariableA("WINDIR", WinDir, MAX_PATH)) {

		printf("[!] GetEnvironmentVariableA Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	sprintf(lpPath, "%s\\System32\\%s", WinDir, lpProcName);

	sprintf(CurrentDirectory, "%s\\System32\\", WinDir);

	// get the size of the attribute list
	InitializeProcThreadAttributeList(NULL, 1, NULL, &sThreadAttrSize);

	pThreadAttrListAddr = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sThreadAttrSize);

	if (pThreadAttrListAddr == NULL) {
		printf("[!] HeapAlloc Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	if (!InitializeProcThreadAttributeList(pThreadAttrListAddr, 1, NULL, &sThreadAttrSize)) {
		printf("[!] InitializeProcThreadAttributeList Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	if(!UpdateProcThreadAttribute(pThreadAttrListAddr, NULL, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProc, sizeof(HANDLE), NULL, NULL)){

		printf("[!] UpdateProcThreadAttribute Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	SiEx.lpAttributeList = pThreadAttrListAddr;


	printf("[i] Running : \"%s\" ... ", lpPath);

	if (!CreateProcessA(
		NULL,
		lpPath,
		NULL,
		NULL,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT,
		NULL,
		CurrentDirectory,
		&SiEx.StartupInfo,
		&Pi
	)) {
		printf("[!] CreateProcessA Failed with Error : %d \n", GetLastError());
		return FALSE;
	}

	printf("[+] DONE \n");

	*dwProcId = Pi.dwProcessId;
	*hProcess = Pi.hProcess;
	*hThread = Pi.hThread;

	DeleteProcThreadAttributeList(pThreadAttrListAddr);
	CloseHandle(hParentProc);

	if (dwProcId != NULL || hProcess != NULL || hThread != NULL) {
		return TRUE;
	}

	return FALSE;
}


int main(int argc, char* argv[]) {

	HANDLE hParentProcess = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	DWORD dwParentPID = atoi(argv[1]);
	DWORD dwProcessId = NULL;

	if (argc < 2) {
		printf("[!] Missing \"Parent Process Id\" Argument \n");
		return -1;
	}

	hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwParentPID);

	if (hParentProcess == NULL) {
		printf("[!] OpenProcess Failed with Error : %d \n", GetLastError());
		return -1;
	}

	if (!CreatePPIDSpoofedProcess(hParentProcess, TARGET_PROCESS, dwProcessId, hProcess, hThread)) {

		return -1;
	}
	printf("[i] Target Process Created With Pid : %d \n", dwProcessId);


	printf("[#] Press <Enter> To Quit ... ");
	getchar();
	CloseHandle(hProcess);
	CloseHandle(hThread);

	return 0;


}