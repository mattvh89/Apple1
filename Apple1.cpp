#include "Apple1.h"
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <ctime>

Emu::Apple1::Apple1()
    : m_cursorPos{ 0, 0 }, m_stdInHandle(NULL), m_stdOutHandle(NULL), m_running(true), m_onStartup(true)
{
    for (Word i = 0; i < 0xFFFF; ++i) m_cpu.getBus()[i] = 0xFF;
    //m_cpu.denatureHexText("roms/wozmon.txt", "roms/wozmon1.txt");
    //m_cpu.denatureHexText("roms/basic.txt", "roms/basic1.txt");
	m_cpu.loadProgram2("roms/basic1.txt", BASIC_ENTRY);
	m_cpu.loadProgram2("roms/wozaci.txt", WOZACI_ENTRY);
	m_cpu.loadProgram2("roms/wozmon1.txt", WOZMON_ENTRY);
    m_cpu.setProgramCounter(WOZMON_ENTRY);

    m_stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
    m_stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(m_stdInHandle, &mode);
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT); // Disable Enter buffering & echo
    mode &= ~ENABLE_PROCESSED_INPUT; // Prevent special key processing (Ctrl+C, etc.)
    SetConsoleMode(m_stdInHandle, mode);

    // Hide the cursor (Apple 1 had no blinking cursor)
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(m_stdOutHandle, &cci);
    cci.bVisible = false;
    SetConsoleCursorInfo(m_stdOutHandle, &cci);

    // Set console screen buffer size to match Apple 1 (40x24)
    COORD bufferSize = { SCREEN_CHAR_WIDTH, SCREEN_CHAR_HEIGHT };
    SetConsoleScreenBufferSize(m_stdOutHandle, bufferSize);

    // Set window size to 40x24 (Apple 1 display size)
    SMALL_RECT windowSize = { 0, 0, 39, 23 };
    SetConsoleWindowInfo(m_stdOutHandle, TRUE, &windowSize);

    // Set text color to green (Apple 1 used monochrome green monitors)
    SetConsoleTextAttribute(m_stdOutHandle, FOREGROUND_GREEN);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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
    LARGE_INTEGER frequency, start, now;
    Byte clockCount = 0;
    bool cursorFlag = false;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

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
        if (elapsed >= 0.5) // half a second has passed
        {
            if (cursorFlag) std::cout << ' ';
            else            std::cout << "@";
            cursorFlag = !cursorFlag;
            SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);

            // Reset the start time.
            QueryPerformanceCounter(&start);
        }

        // read input, execute, and output
        m_cpu.clock();
        ++clockCount;

        // Check for remaining cycles to run MMIO operations
        if (m_cpu.getCycles() == 0)
            this->mmioRegisterMonitor();

        // Wait if too many cycles have been processed (1 MHz timing)
        // The clock runs at 1 MHz, so each cycle should take 1 microsecond (1 µs)
        // If you've executed too many cycles, we adjust the sleep duration to maintain proper speed
        elapsed = static_cast<double>(now.QuadPart - start.QuadPart) / frequency.QuadPart;
        if (elapsed < 1.0 && clockCount >= 1000)
        {
            // If 1000 cycles have passed, it means 1 millisecond has passed
            // Sleep the difference between elapsed time and 1ms
            double sleepDuration = 1.0 - elapsed;
            if (sleepDuration > 0)
                Sleep(static_cast<DWORD>(sleepDuration * 1000));  // Convert seconds to milliseconds

            // Reset the clock count
            clockCount = 0;
        }
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
                m_cursorPos.X = 0;
                m_cursorPos.Y = 0;
                SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);
            }
            else
            if(vKey == VK_F2)                                                               // Reset button
            {
                m_onStartup = false;
                m_cpu.reset();
                // Clear all of the ram
                for (Word i = 0; i < 0xFFFF; ++i) m_cpu.getBus()[i] = 0xFF;
                // Reload the programs incase values were overwritten.
                // It should return the same state as when it is turned on and reset
                m_cpu.loadProgram2("roms/basic1.txt", BASIC_ENTRY);
                m_cpu.loadProgram2("roms/wozaci.txt", WOZACI_ENTRY);
                m_cpu.loadProgram2("roms/wozmon1.txt", WOZMON_ENTRY);
                m_cpu.setProgramCounter(WOZMON_ENTRY);
                m_cursorPos.X = 0;
                m_cursorPos.Y = 0;
                SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);
            }
            else
            if(vKey == VK_F3)                                                               // Quit button
            {
                m_running = false;
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

        std::cout << ' ';
        SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);

        if (outputChar == CR)
        {
            std::cout << std::endl;
            if (++m_cursorPos.Y > SCREEN_CHAR_HEIGHT) m_cursorPos.Y = 0;
            m_cursorPos.X = 0;
        }
        else     
        if (outputChar >= 32 and outputChar <= 126)
        {
            std::cout << outputChar;
            if (++m_cursorPos.X > SCREEN_CHAR_WIDTH) m_cursorPos.X = 0;
        }
        SetConsoleCursorPosition(m_stdOutHandle, m_cursorPos);
 
        m_cpu.busWrite(DISPLAY_OUTPUT_REGISTER,  0x00);
        Bits<Byte>::ClearBit(m_cpu.getBus()[KEYBOARD_CNTRL_REGISTER], LastBit<Byte>);
    }
}
