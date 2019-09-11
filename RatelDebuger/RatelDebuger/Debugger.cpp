#include "Debugger.hpp"

void AddToLog(HWND dlgHdl, std::string str)
{

	HWND dlgItem = GetDlgItem(dlgHdl, EDIT_INFO);

	int oldLength = GetWindowTextLength(dlgItem) + 1;
	char * oldText = new char[oldLength];
	GetWindowText(dlgItem, oldText, oldLength);
	std::string temp(oldText);
	temp += str + "\r\n";
	SetWindowText(dlgItem, temp.c_str());
	delete oldText;
}

void LogDebugException(HWND dlgHdl, const LPDEBUG_EVENT debugEv, std::string formatStr)
{
	char buff[500];
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), formatStr.c_str(),
		debugEv->u.Exception.ExceptionRecord.ExceptionAddress,
		debugEv->u.Exception.ExceptionRecord.ExceptionCode);

	std::string str(buff);
	
	AddToLog(dlgHdl, str);
}

void ProcessDebugException(HWND dlgHdl, const LPDEBUG_EVENT debugEv)
{
	if (debugEv->u.Exception.dwFirstChance == 1)
		LogDebugException(dlgHdl, debugEv, "first chance exception at 0x%08x, exception-code: 0x%08x");
	else
		LogDebugException(dlgHdl, debugEv, "second chance exception at 0x%08x, exception-code: 0x%08x");

}

void ProcessDebugEvent(HWND dlgHdl, const LPDEBUG_EVENT debugEv)
{

}

std::string ProcessStackOverflow(const LPDEBUG_EVENT debugEv)
{
	return "Stack Overflow exception";
}

std::string EnterDebugLoop(HWND dlgHdl, const LPDEBUG_EVENT debugEv)
{
	DWORD dwContinueStatus = DBG_CONTINUE; // exception continuation 

	for (;;)
	{
		WaitForDebugEvent(debugEv, INFINITE);

		switch (debugEv->dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:
			// Process the exception code. When handling 
			// exceptions, remember to set the continuation 
			// status parameter (dwContinueStatus). This value 
			// is used by the ContinueDebugEvent function. 

			switch (debugEv->u.Exception.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				ProcessDebugException(dlgHdl, debugEv);
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case EXCEPTION_BREAKPOINT:
				ProcessDebugException(dlgHdl, debugEv);
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT:
				ProcessDebugException(dlgHdl, debugEv);
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case EXCEPTION_SINGLE_STEP:
				ProcessDebugException(dlgHdl, debugEv);
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case DBG_CONTROL_C:
				ProcessDebugException(dlgHdl, debugEv);
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case EXCEPTION_STACK_OVERFLOW:
				LogDebugException(dlgHdl, debugEv, "Exception at 0x%08x, exception-code: 0x%08x");
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			case STATUS_STACK_BUFFER_OVERRUN:
				LogDebugException(dlgHdl, debugEv, "Exception at 0x%08x, exception-code: 0x%08x");
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;

			default:
				ProcessDebugException(dlgHdl, debugEv);

				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				break;
			}

			break;

		case CREATE_THREAD_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Create thread id ") + std::to_string(debugEv->dwThreadId));
			break;

		case CREATE_PROCESS_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Create process id ") + std::to_string(debugEv->dwProcessId));

			break;

		case EXIT_THREAD_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Exit thread id ") + std::to_string(debugEv->dwThreadId));

			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Exit process id ") + std::to_string(debugEv->dwProcessId));

			break;

		case LOAD_DLL_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Load library with address ") + std::to_string((int)debugEv->u.LoadDll.lpBaseOfDll));

			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			AddToLog(dlgHdl, std::string("Unload library with address ") + std::to_string((int)debugEv->u.UnloadDll.lpBaseOfDll));
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			AddToLog(dlgHdl, std::string("Debug string : ") + std::string(debugEv->u.DebugString.lpDebugStringData));
			break;

		case RIP_EVENT:
			AddToLog(dlgHdl, std::string("Rip error ") + std::to_string(debugEv->u.RipInfo.dwError));

			break;
		}
		ContinueDebugEvent(debugEv->dwProcessId,
			debugEv->dwThreadId,
			dwContinueStatus);
	}
}

void OpenProcessByName(HWND dlgHdl, std::string fileName)
{
	if (fileName == "")
		throw std::runtime_error("No exe to debug");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(fileName.c_str(), NULL, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
		throw std::runtime_error("Failed open process");

	DEBUG_EVENT debug_event = { 0 };

	EnterDebugLoop(dlgHdl, &debug_event);
}