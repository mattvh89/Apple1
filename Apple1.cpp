#include "Apple1.h"
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <ctime>
#include <chrono>
#include <thread>

Emu::Apple1::Apple1()
    : m_cursorPos{ 0, 0 }, m_stdInHandle(NULL), m_stdOutHandle(NULL), m_running(true), m_onStartup(true), m_throttled(true)
{
    //for (Word i = 0; i < 0xFFFF; ++i) m_cpu.getBus()[i] = 0x0F;
    //m_cpu.denatureHexText("roms/wozmon.txt", "roms/wozmon1.txt");
    //m_cpu.denatureHexText("roms/basic.txt", "roms/basic1.txt");
    //m_cpu.denatureHexText("roms/wozaci.txt", "roms/wozaci1.txt");
	m_cpu.loadProgram2("roms/basic1.txt", BASIC_ENTRY);
	m_cpu.loadProgram2("roms/wozaci1.txt", WOZACI_ENTRY);
	m_cpu.loadProgram2("roms/wozmon1.txt", WOZMON_ENTRY);
    m_cpu.setProgramCounter(WOZMON_ENTRY);

    m_stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
    m_stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(m_stdInHandle, &mode);
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT); // Disable Enter buffering & echo
    mode &= ~ENABLE_PROCESSED_INPUT; // Prevent special key processing (Ctrl+C, etc.)
    SetConsoleMode(m_stdInHandle, mode);

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    if (GetCurrentConsoleFontEx(m_stdOutHandle, FALSE, &cfi))
    {
        cfi.dwFontSize.X = 24;  // Width of the font (try adjusting these values)
        cfi.dwFontSize.Y = 24; // Height of the font (try adjusting these values)
        wcscpy_s(cfi.FaceName, L"Lucida Console");  // Set the font to Lucida Console
        SetCurrentConsoleFontEx(m_stdOutHandle, FALSE, &cfi);
    }

    // Hide the cursor (Apple 1 had no blinking cursor)
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(m_stdOutHandle, &cci);
    cci.bVisible = false;
    SetConsoleCursorInfo(m_stdOutHandle, &cci);

    // Set console screen buffer size to match Apple 1 (40x24)
    COORD bufferSize;// = { SCREEN_CHAR_WIDTH, SCREEN_CHAR_HEIGHT };
    bufferSize.X = SCREEN_CHAR_WIDTH;
    bufferSize.Y = SCREEN_CHAR_HEIGHT;
    SetConsoleScreenBufferSize(m_stdOutHandle, bufferSize);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(m_stdOutHandle, &csbi);

    // Set window size to 40x24 (Apple 1 display size)
    SMALL_RECT windowSize = { 0, 0, csbi.dwSize.X, csbi.dwSize.Y};
    SetConsoleWindowInfo(m_stdOutHandle, TRUE, &windowSize);

    // Set text color to green (Apple 1 used monochrome green monitors)
    SetConsoleTextAttribute(m_stdOutHandle, FOREGROUND_GREEN);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // The apple 1 starts with random values of the display buffer, I'm just going to simulate this because there's nothing
    // added by emulating this buffer. It's essentially the buffer of the console.
    for (size_t y = 0; y < SCREEN_CHAR_HEIGHT; ++y)
    {
        for (size_t x = 0; x < SCREEN_CHAR_WIDTH; ++x)
        {
            // Generate a random printable ASCII character (uppercase letters, numbers, and some symbols)
            char randomChar = static_cast<char>((std::rand() % 32) + 0x40); // Roughly simulates Apple 1 video memory noise

            // Write to console at position (x, y)
            COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
            SetConsoleCursorPosition(m_stdOutHandle, pos);
            std::cout << randomChar;
        }
    }
}

int Emu::Apple1::run()
{
    LARGE_INTEGER frequency, start, now, displayFlagStart, displayFrequency;
    Byte clockCount = 0;
    bool cursorFlag = false, displayFlag = false;

    QueryPerformanceFrequency(&displayFrequency);
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    QueryPerformanceCounter(&displayFlagStart);

    while (m_running)
    {
        
        // Need to check for input first so we can reset after start up
        this->readKeyboard();
        // if the apple1 has been started but not reset yet it can't do anything
        if (m_onStartup)
        {
            Sleep(10);
            continue;
        }
        // Flash cursor
        QueryPerformanceCounter(&now);
        double elapsed = static_cast<double>(now.QuadPart - start.QuadPart) / frequency.QuadPart;
        double displayFlagElapsed = static_cast<double>(now.QuadPart - displayFlagStart.QuadPart) / displayFrequency.QuadPart;

        if (displayFlagElapsed > .01)
        {
            if (displayFlag) Bits<Byte>::SetBit(m_cpu.getBus()[DISPLAY_OUTPUT_REGISTER], LastBit<Byte>);
            else Bits<Byte>::ClearBit(m_cpu.getBus()[DISPLAY_OUTPUT_REGISTER], LastBit<Byte>);
            displayFlag = !displayFlag;
            QueryPerformanceCounter(&displayFlagStart);
        }
        

        if (elapsed >= 0.5) // half a second has passed
        {
            if (cursorFlag) std::cout << ' ';
            else            std::cout << "@";
            cursorFlag = !cursorFlag;
            SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);

            if (clockCount > 500) Sleep((elapsed - clockCount) * 1000);

            // Reset the start time.
            QueryPerformanceCounter(&start);
        }

        // read input, execute, and output
        m_cpu.clock();
        ++clockCount;

        // Check for remaining cycles to run MMIO operations
        if (m_cpu.getCycles() == 0)
            this->mmioRegisterMonitor();
    }

    return 0;
}

char Emu::Apple1::readKeyboard()
{
    INPUT_RECORD ir;
    DWORD readCount;
    
    if (PeekConsoleInput(m_stdInHandle, &ir, 1, &readCount) && readCount > 0)
    {
        ReadConsoleInput(m_stdInHandle, &ir, 1, &readCount);
        CHAR key = ir.Event.KeyEvent.uChar.AsciiChar & 0x7F;
        WORD vKey = ir.Event.KeyEvent.wVirtualKeyCode;
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
        {
            if (vKey == VK_F1)                                                              // Clear screen button
            {
                system("cls");
            }
            else
            if(vKey == VK_F2)                                                               // Reset button
            {                                                                               // The reset button on the Apple 1 does not clear the ram.
                std::cout << ' ';                                                           // clear the cursor if it's there or it will be left on the screen
                m_onStartup = false;                                                        // if this is the first time starting, this will stop the program blocking
                m_cpu.reset();                                                              // reset the cpu
                m_cpu.loadProgram2("roms/basic1.txt", BASIC_ENTRY);                         // restore the programs incase they were over written
                m_cpu.loadProgram2("roms/wozaci1.txt", WOZACI_ENTRY);
                m_cpu.loadProgram2("roms/wozmon1.txt", WOZMON_ENTRY);
                // program counter is successfully reset from the reset vector set by the wozmon, 
                // so calling m_cpu.reset() properly sets the program counter. Now just reset the cursor
                m_cursorPos.X = 0;
                m_cursorPos.Y = 0;
                SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);
            }
            else
            if(vKey == VK_F3)                                                               // Quit button
            {
                m_running = false;
            }
            else
            if(vKey == VK_F4)
            {
                m_throttled = !m_throttled;
            }

            // We have to write the key to the keyboard input register with the last bit set.
            m_cpu.busWrite(KEYBOARD_INPUT_REGISTER, static_cast<Byte>(std::toupper(key)) | 0x80);

            // Set the keyboard control register so the wozmon knows there's a key ready to be read
            Bits<Byte>::SetBit(m_cpu.getBus()[KEYBOARD_CNTRL_REGISTER], LastBit<Byte>);
        }
    }
    return 0x00;
}

void Emu::Apple1::mmioRegisterMonitor()
{
    if (m_cpu.getInstructionName() == "STA" and m_cpu.getAddressValue() == DISPLAY_OUTPUT_REGISTER)
    {
        char outputChar = std::toupper(static_cast<char>(m_cpu.busRead(DISPLAY_OUTPUT_REGISTER) & 0x7F));

        //std::cout << ' ';
        //SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);

        if (outputChar == CR)
        {
            std::cout << std::endl;
            if (++m_cursorPos.Y >= SCREEN_CHAR_HEIGHT) m_cursorPos.Y = 0;
            m_cursorPos.X = 0;
        }
        else     
        if (outputChar >= 32 and outputChar <= 126)
        {
            std::cout << outputChar;
            if (++m_cursorPos.X > SCREEN_CHAR_WIDTH - 1) m_cursorPos.X = 0;
        }
        SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);
 
        m_cpu.busWrite(DISPLAY_OUTPUT_REGISTER,  0x00);
        Bits<Byte>::ClearBit(m_cpu.getBus()[KEYBOARD_CNTRL_REGISTER], LastBit<Byte>);
    }
}
