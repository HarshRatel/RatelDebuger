#pragma once
#include <Windows.h>
#include <string>

#include "resource.h"

std::string message = "";

class Debugger
{
public:
	Debugger();
	Debugger(std::string fileName);
	
	void OpenProcess();

	~Debugger();

private:
	std::string ProcessStackOverflow(const LPDEBUG_EVENT debugEv);
	std::string EnterDebugLoop(const LPDEBUG_EVENT debugEv);

	std::string _fileName = "";
};

