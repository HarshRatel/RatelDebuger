#include "Debugger.hpp"



Debugger::Debugger()
{
}

Debugger::Debugger(std::string fileName) : _fileName(fileName)
{
}

void Debugger::OpenProcess()
{
	if (_fileName == "")
		throw std::runtime_error("No exe to debug");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(_fileName.c_str(), NULL, NULL, NULL, FALSE,
						DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
		throw std::runtime_error("Failed open process");

	DEBUG_EVENT debug_event = { 0 };

	EnterDebugLoop(&debug_event);
}


Debugger::~Debugger()
{
}


std::string Debugger::ProcessStackOverflow(const LPDEBUG_EVENT debugEv)
{
	return "Stack Overflow exception";
}

std::string Debugger::EnterDebugLoop(const LPDEBUG_EVENT debugEv)
{
	DWORD dwContinueStatus = DBG_CONTINUE; // exception continuation 

	for (;;)
	{
		// Wait for a debugging event to occur. The second parameter indicates
		// that the function does not return until a debugging event occurs. 

		WaitForDebugEvent(debugEv, INFINITE);

		// Process the debugging event code. 

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
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			case EXCEPTION_BREAKPOINT:
				// First chance: Display the current 
				// instruction and register values. 
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			case EXCEPTION_SINGLE_STEP:
				// First chance: Update the display of the 
				// current instruction and register values. 
				break;

			case DBG_CONTROL_C:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			case EXCEPTION_STACK_OVERFLOW:
				message = ProcessStackOverflow(debugEv);
				break;

			default:
				// Handle other exceptions. 
				break;
			}

			break;
		}

		// Resume executing the thread that reported the debugging event. 

		ContinueDebugEvent(debugEv->dwProcessId,
			debugEv->dwThreadId,
			dwContinueStatus);
	}
}
