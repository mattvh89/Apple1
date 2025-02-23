/*
Author: Matthew Van Helden
Date:   2/23/2025

	I was inspired by Javidx9 (One Lone Coder) who made an nes emulator. He created the opcode vector table, and as soon as I saw that i decided
to make my own 6502 emulator, using his opcode table as the template. Superficially it might look fairly similar, but it is fundamentally different and unique.
*/

#pragma once
#include "Bit.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>
#include <assert.h>
#include "Debug.h"

// Define reserved regions in memory
#define STACK_TOP    0x01FF	// storing using the post decrement operator, retreiving uses the pre incremenet operator
#define STACK_BOTTOM 0x0100
#define IRQ_VECTOR   0XFFFE	// IRQ vector is FFFE and FFFF
#define RESET_VECTOR 0xFFFC // FFFC and FFFD
#define NMI_VECTOR   0XFFFA // FFFA and FFFB
#define USER_PROGRAM 0XFF00

// Declare everything in a namespace
namespace Emu {

	const int PROGRAM_LOAD_SUCCESSFULL = 0;
	const int PROGRAM_LOAD_FAILURE = 1;

	// Make it easy to determine which addressing mode we used. Now the addressing mode function can return which address mode it was
	// with an Address_Mode enum that matches it's index in the ADDRESS_STRINGS array
	enum Address_Mode
	{
		IMP, IMM, REL, ZP0, ZPX, ZPY, ABS, ABX, ABY, IND, IZX, IZY
	};
	const std::string ADDRESS_STRINGS[] = { "Implied", "Immediate", "Relative", "Zero Page", "Zero X", "Zero Y",
											"Absolute", "Absolute X", "Absolute Y", "Indirect", "Indrect X", "Indrect Y" };

	// enumerate flags 1-8 to be used with the bit class that expects an index to the bit starting at 1. (First bit, second bit, etc)
	enum Flags
	{
		CARRY = 1, ZERO, INTERRUPT_DISABLE, DECIMALE_MODE,
		BREAK, UNUSED, O_FLOW, NEGATIVE
	};

	// CPU structures. Consists of 4 Byte registers; accumulator, x, y, and flags. 
	//							   2 Words registers; stack pointer and program counter
	struct CPU
	{
		Bits<Byte> a;			// accumulator
		Bits<Byte> x;			// index register x
		Bits<Byte> y;			// index register y
		Bits<Byte> flags;		// flags register
		Bits<Word> p;			// program counter
		Bits<Word> s;			// stack pointer

		CPU() { this->reset(); }

		// Set reigsters and flags to 0. Set program counter to 0x1000 for now because we don't have access to the bus
		// emu6502::reset() will have set it properly from RESET_VECTOR
		void reset()
		{
			a = 0x00;
			x = 0x00;
			y = 0x00;
			p = USER_PROGRAM;
			s = STACK_TOP;
			flags = 0x00;
			flags.SetBit(Flags::UNUSED);
		}
	};

	/* Class declaration */
	class emu6502
	{
	public:
											emu6502					();																	// Initialization

// The main functionality
					void					reset					();																	// Reset the state of the processor (set program counter to address at RESET_VECTOR

					void					clock					();																	// Executes an instruction while counting down the clock cycles

					void					irq						();																	// Interrupt request

					void					nmi						();																	// Non maskable interrupt

					size_t					fetch_and_execute		();																	// Fetch the opcode and memory mode value and execute it

					void					busWrite				(const Word& addr, 
																	 const Byte& val);

					Byte					busRead					(const Word& addr);



// Mostly for the menu output
					Byte*					getBus					();

		const		CPU&					getCPU					()										const;						// Get a constant reference to the cpu

					std::string_view		getInstructionName		()										const;						// Get the instruction mnemonic

					std::string_view		getAddressName			()										const;						// Get the address name from ADDRESS_NAMES indexed with an Address_Mode enum

					int						getAddressValue			()										const;						// Get m_addrVal. Usually contains a memory address addressing mode was IMM

					int						getAddressRelative		()										const;						// If the addressming mode requires a relative offset like for BNE it's stored at m_addrRel

					Byte					getCycles				()										const;						// Gets the number of cycles for that instruction and addressing mode  plus branches and pages boundary crossings



// More so just for debugging right now
					Byte					fetch					();																	// Used for disassembly. Fetches a single from m_bus[m_cpu.p++]

					void					setProgramCounter		(const size_t& p = 0);												// explictly sets the program counter

					int						loadProgram				(const char* fname,													// Loads a text file of hexadecimal machine code
																	 const Word& addr = USER_PROGRAM);

					int						loadProgram2			(const char* fname,													// loads a text file of hexadecimal predicated the address is should be at e.g. E000: AA 58 E0 etc....
																	 const Word& addr = USER_PROGRAM);

					int						loadProgramHex			(const char* fname,
																	 const Word& addr = USER_PROGRAM);

					int						denatureHexText			(const char* fname, 
																	 const char* fname_new);											// Takes a text file containing hexadecimal instructions and removing any addresses that may start the line, e.g. E0000: A2 08 5D ....

					void					disassembleText			(const char* fname);												// Disassembles a text file of machine code in assembly. Saved in a file asm.asm

	// Output operations
					void					printMemoryRange		(const size_t& start, 
																	 const size_t& end)						const;

	/* Define an Instruction strcture */
	private:
		struct Instruction
		{
			std::string_view mnemonic;			// the name of the opcode
			Byte(emu6502::* exec)() = nullptr;	// function pointer to the instruction
			Byte(emu6502::* addr)() = nullptr;	// function pointer to the address mode
			Byte cycles = 0;					// the amount of the cycles
		};
//
//
/* Member variables to emu6502 */
	private:
		std::vector<Instruction> m_lookup;			// Table of opcodes correctly indexed to their hex value
		CPU                      m_cpu;				// CPU
		Instruction              m_instruction;		// keep track of the current instruction, mostly for the cycles variable but also to check addressing mode for m_addrVal
		Bits<DWord>				 m_addrVal;			// Used to get the value for the instruction. It is the next byte if it's IMM otherwise it's an address
		Bits<Byte>				 m_addrRel;			// Used for relative offsets
		Bits<Byte>				 m_value;			// the value at a memory address after calling an addressing function
		Byte					 m_bus[64 * 1024];	// Memory of size 0xFFFF
		Address_Mode			 m_lastAddressMode; // Keep track of the last address mode used for debugging purposes

/* Private helper functions to check the status of flags and clear or set them accordingly */
	private:
		void checkFlag(bool condition, const Flags& flag);

/* CPU Addressing modes */
	private:
		Byte IMP();	Byte IMM();
		Byte ZP0();	Byte ZPX();
		Byte ZPY();	Byte REL();
		Byte ABS();	Byte ABX();
		Byte ABY();	Byte IND();
		Byte IZX();	Byte IZY();

/* CPU Instructions */
	private:
		Byte ADC();	Byte AND();	Byte ASL();	Byte BCC();
		Byte BCS();	Byte BEQ();	Byte BIT();	Byte BMI();
		Byte BNE();	Byte BPL();	Byte BRK();	Byte BVC();
		Byte BVS();	Byte CLC();	Byte CLD();	Byte CLI();
		Byte CLV();	Byte CMP();	Byte CPX();	Byte CPY();
		Byte DEC();	Byte DEX();	Byte DEY();	Byte EOR();
		Byte INC();	Byte INX();	Byte INY();	Byte JMP();
		Byte JSR();	Byte LDA();	Byte LDX();	Byte LDY();
		Byte LSR();	Byte NOP();	Byte ORA();	Byte PHA();
		Byte PHP();	Byte PLA();	Byte PLP();	Byte ROL();
		Byte ROR();	Byte RTI();	Byte RTS();	Byte SBC();
		Byte SEC();	Byte SED();	Byte SEI();	Byte STA();
		Byte STX();	Byte STY();	Byte TAX();	Byte TAY();
		Byte TSX();	Byte TXA();	Byte TXS();	Byte TYA();
		Byte XXX();
	};

}