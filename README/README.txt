Author: Matthew Van Helden
Date: 02/23/2025
Project: Apple 1 emulator in C++
Contact: matt.van.helden@hotmail.com
------------------------------------------------------------------------------------------------------------------------------------------------

Thanks for checking out my Apple 1 emulator. This is just a simplified manual to get you up and running. Here are some of the basic functions:



The apple 1 starts with the screen buffer full of random values. 

F1 - to clear the screen
F2 - to reset the emulation

You should see a \ and the cursor (@) should drop down.

Typing in a hexadecimal memory address will print the value at that address, e.g. FF00 <ret> prints the value at that address, which should
be D8, because that is where the wozmon program is loaded. Put a dot '.' between memory address to print a range, e.g. FF00.FF0A <ret> should
print:
FF00: D8 58 A0 7F 8C 12 D0 A9
FF08: A7 8D 11

Input a memory address followed by a colon and values delimited by spaces to store values in memory:

0:A9 0 AA 20 EF FF E8 8A 4C 2 0 <RET>
This will store those opcodes at memory location 0000. This is the test program from the manual.

Type: R <ret>

And the program should start executing, continually printing all of the characters. Press F2 to reset and then F1 to clear the screen.
This will clear the program from memory

A1 ASM is stored at E000. Type:
E000R <ret> to enter the assembler.

Enter "NEW" to clear RAM
Enter "AUTO" to set up automatic lines/memory addressing
You can now enter 6502 assembly instructions and build a program.
ESC to stop editing
Enter: WOZMON <ret> to exit the assembler and go back to the WOZMON.

https://www.sbprojects.net/projects/apple1/download.php
This is where I obtained the ROMS, and also the manual. You should obviously download the manuals so you understand what you can do and how
to do it. There are a lot of little nuances to the Apple 1 and the A1 assembler.

I hope you enjoy!