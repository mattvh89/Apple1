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
	m_cpu.loadProgram2("roms/basic1.txt", BASIC_ENTRY);                                                 // this is really the A1 assembler
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
    for (size_t y = 0; y <= SCREEN_CHAR_HEIGHT; ++y)
    {
        for (size_t x = 0; x < SCREEN_CHAR_WIDTH; ++x)
        {
            // Generate a random printable ASCII character (uppercase letters, numbers, and some symbols)
            char randomChar = static_cast<char>((std::rand() % (126 - 32) + 32)); // Roughly simulates Apple 1 video memory noise
            std::cout << randomChar;
        }
    }
    //std::cout << Apple1_Title::A1_LINE1 << Apple1_Title::A1_LINE2 << Apple1_Title::A1_LINE3 << Apple1_Title::A1_LINE4 << Apple1_Title::A1_LINE5 << Apple1_Title::A1_LINE6 << Apple1_Title::A1_LINE7 << Apple1_Title::A1_LINE8 << Apple1_Title::A1_LINE9;
}

int Emu::Apple1::run()
{
    LARGE_INTEGER frequency, start, now, displayFlagStart, displayFrequency;
    Byte clockCount = 0;
    bool cursorFlag = false;

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
        
        // Do timer stuff for Flashing the cursor and throttling the display rate
        QueryPerformanceCounter(&now);
        double elapsed = static_cast<double>(now.QuadPart - start.QuadPart) / frequency.QuadPart;
        double displayFlagElapsed = static_cast<double>(now.QuadPart - displayFlagStart.QuadPart) / displayFrequency.QuadPart;

        // The bottlneck of the Apple 1 was the monitor, so we can emulate the speed of monitor by constantly flipping the last bit in the display output register
        // This is how the Apple 1 monitor actually worked too so this is good emulation. F4 toggles this on and off. If off, we need to keep that bit cleared
        if (m_throttled)
        {   // the display ready flag bit should be read about 10x a minute, so flip it ever 5 hundreths of a second
            if (displayFlagElapsed > .05)
            {
                Bits<Byte>::ToggleBit(m_cpu.getBus()[DISPLAY_OUTPUT_REGISTER], LastBit<Byte>);  // Toggle the last bit of the display output register. This controls whether the monitor is available or not
                QueryPerformanceCounter(&displayFlagStart);
            }
        }
        else
            Bits<Byte>::ClearBit(m_cpu.getBus()[DISPLAY_OUTPUT_REGISTER], LastBit<Byte>);
        

        if (elapsed >= 0.5) // half a second has passed
        {
            if (cursorFlag) std::cout << ' ';
            else            std::cout << "@";
            cursorFlag = !cursorFlag;
            SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);

            // Reset the start time.
            QueryPerformanceCounter(&start);
        }

        /*@Todo:
         Need to slow the 6502 down to approximately 1000 clock cycles per second*/
        // read input, execute, and output
        m_cpu.clock();
        //++clockCount;

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
                m_cpu.loadProgram2("roms/wozmon1.txt", WOZMON_ENTRY);                       // program counter is successfully reset from the reset vector set by the wozmon, 
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
            m_cpu.busWrite(KEYBOARD_INPUT_REGISTER, static_cast<Byte>(std::toupper(key)) | 0x80);                   // We have to write the key to the keyboard input register with the last bit set.

            Bits<Byte>::SetBit(m_cpu.getBus()[KEYBOARD_CNTRL_REGISTER], LastBit<Byte>);                             // Set the keyboard control register so the wozmon knows there's a key ready to be read
        }
    }
    return 0x00;
}

void Emu::Apple1::mmioRegisterMonitor()
{
    if (m_cpu.getInstructionName() == "STA" and m_cpu.getAddressValue() == DISPLAY_OUTPUT_REGISTER)
    {
        char outputChar = std::toupper(static_cast<char>(m_cpu.busRead(DISPLAY_OUTPUT_REGISTER) & 0x7F));           // read the key from the display output register, but we don't want the last bit

        if (outputChar == CR)                                                                                       // if it's carriage return 0x8D
        {
            std::cout << ' ' << std::endl;                                                                          // erase a possible ghost @ cursor
            if (++m_cursorPos.Y >= SCREEN_CHAR_HEIGHT) m_cursorPos.Y = 0;                                           // make sure we're within height of the screen
            m_cursorPos.X = 0;                                                                                      // reset x coord
        }
        else     
        if (outputChar >= 32 and outputChar <= 126)                                                                 // else it's a printable character so print it
        {                                                                                                           // no need to erase the cursor because our character will
            std::cout << outputChar;
            if (++m_cursorPos.X > SCREEN_CHAR_WIDTH - 1) m_cursorPos.X = 0;                                         // make sure we're within the width of the screen
        }
        SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);                                                      // set the cursor the cursor pos
 
        Bits<Byte>::ClearBit(m_cpu.getBus()[KEYBOARD_CNTRL_REGISTER], LastBit<Byte>);                               // clear the key board control register so the wozmon knows the character has been processed
    }
}
