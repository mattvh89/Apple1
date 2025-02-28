#pragma once
#include "emu6502.h"
#include <Windows.h>

#define KEYBOARD_INPUT_REGISTER 0xD010
#define KEYBOARD_CNTRL_REGISTER 0xD011
#define DISPLAY_OUTPUT_REGISTER 0xD012

#define WOZMON_ENTRY			0xFF00
#define BASIC_ENTRY				0xE000		// this is really the A1 assembler
#define ASM_ENTRY				0X9000
#define WOZACI_ENTRY			0xC100

#define SCREEN_CHAR_WIDTH  40
#define SCREEN_CHAR_HEIGHT 24

#define ESC 0x1B
#define CR  0x0D
#define TAB '\t'
#define BS  0X08

#define LOG_FILE "logs/log.dat"

//namespace Apple1_Title
//{
//	const char* A1_LINE1 = "      /\\        ---   ---  |      ----  \n";
//	const char* A1_LINE2 = "     /  \\      |   \\ |   \\ |     |      \n";
//	const char* A1_LINE3 = "    /    \\     |---/ |---/ |     |      \n";
//	const char* A1_LINE4 = "   /------\\    |     |     |     |---   \n";
//	const char* A1_LINE5 = "  /        \\   |     |     |     |      \n";
//	const char* A1_LINE6 = " /          \\  |     |     |     |      \n";
//	const char* A1_LINE7 = "/            \\ |     |     |____ |____  \n";
//	const char* A1_LINE8 = "                  ONE                   \n";
//	const char* A1_LINE9 = "0000000000000000000000000000000000000000\n";
//}

namespace Emu
{

class Apple1
{
public:
									Apple1								();

			int						run									();

protected:
			char					readKeyboard						();

			void					mmioRegisterMonitor					();

			bool					saveState							();

private:
	Emu::emu6502 m_cpu;
	COORD		 m_cursorPos;
	HANDLE		 m_stdOutHandle, 
				 m_stdInHandle;
	bool		 m_running,
				 m_onStartup,
				 m_throttled;
};

}
