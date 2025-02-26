#include "emu6502.h"
#include <exception>
#include <map>
#include <fstream>
#include <iostream>
#include <exception>
#include <iomanip>


using namespace Emu;

emu6502::emu6502()
	: m_cpu(), m_addrVal(0x00), m_addrRel(0x00)
{
	m_bus[RESET_VECTOR] = 0X00;
	m_bus[RESET_VECTOR + 1] = 0X10;

	Byte hi = m_bus[RESET_VECTOR + 1];
	Byte lo = m_bus[RESET_VECTOR];
	m_cpu.p = Word((hi << 8) | lo);

	// initialize the lookup table of opcodes
	using a = emu6502;
	m_lookup =
	{
		{ "BRK", &a::BRK, &a::IMM, 7 },{ "ORA", &a::ORA, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::ZP0, 3 },{ "ASL", &a::ASL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHP", &a::PHP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::IMM, 2 },{ "ASL", &a::ASL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABS, 4 },{ "ASL", &a::ASL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BPL", &a::BPL, &a::REL, 2 },{ "ORA", &a::ORA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ZPX, 4 },{ "ASL", &a::ASL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLC", &a::CLC, &a::IMP, 2 },{ "ORA", &a::ORA, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABX, 4 },{ "ASL", &a::ASL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "JSR", &a::JSR, &a::ABS, 6 },{ "AND", &a::AND, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "BIT", &a::BIT, &a::ZP0, 3 },{ "AND", &a::AND, &a::ZP0, 3 },{ "ROL", &a::ROL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLP", &a::PLP, &a::IMP, 4 },{ "AND", &a::AND, &a::IMM, 2 },{ "ROL", &a::ROL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "BIT", &a::BIT, &a::ABS, 4 },{ "AND", &a::AND, &a::ABS, 4 },{ "ROL", &a::ROL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BMI", &a::BMI, &a::REL, 2 },{ "AND", &a::AND, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ZPX, 4 },{ "ROL", &a::ROL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEC", &a::SEC, &a::IMP, 2 },{ "AND", &a::AND, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ABX, 4 },{ "ROL", &a::ROL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTI", &a::RTI, &a::IMP, 6 },{ "EOR", &a::EOR, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "EOR", &a::EOR, &a::ZP0, 3 },{ "LSR", &a::LSR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHA", &a::PHA, &a::IMP, 3 },{ "EOR", &a::EOR, &a::IMM, 2 },{ "LSR", &a::LSR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::ABS, 3 },{ "EOR", &a::EOR, &a::ABS, 4 },{ "LSR", &a::LSR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVC", &a::BVC, &a::REL, 2 },{ "EOR", &a::EOR, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ZPX, 4 },{ "LSR", &a::LSR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLI", &a::CLI, &a::IMP, 2 },{ "EOR", &a::EOR, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ABX, 4 },{ "LSR", &a::LSR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTS", &a::RTS, &a::IMP, 6 },{ "ADC", &a::ADC, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ADC", &a::ADC, &a::ZP0, 3 },{ "ROR", &a::ROR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLA", &a::PLA, &a::IMP, 4 },{ "ADC", &a::ADC, &a::IMM, 2 },{ "ROR", &a::ROR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::IND, 5 },{ "ADC", &a::ADC, &a::ABS, 4 },{ "ROR", &a::ROR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVS", &a::BVS, &a::REL, 2 },{ "ADC", &a::ADC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ZPX, 4 },{ "ROR", &a::ROR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEI", &a::SEI, &a::IMP, 2 },{ "ADC", &a::ADC, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ABX, 4 },{ "ROR", &a::ROR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "???", &a::NOP, &a::IMP, 2 },{ "STA", &a::STA, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZP0, 3 },{ "STA", &a::STA, &a::ZP0, 3 },{ "STX", &a::STX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "DEY", &a::DEY, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 2 },{ "TXA", &a::TXA, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "STY", &a::STY, &a::ABS, 4 },{ "STA", &a::STA, &a::ABS, 4 },{ "STX", &a::STX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCC", &a::BCC, &a::REL, 2 },{ "STA", &a::STA, &a::IZY, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZPX, 4 },{ "STA", &a::STA, &a::ZPX, 4 },{ "STX", &a::STX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "TYA", &a::TYA, &a::IMP, 2 },{ "STA", &a::STA, &a::ABY, 5 },{ "TXS", &a::TXS, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::NOP, &a::IMP, 5 },{ "STA", &a::STA, &a::ABX, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::XXX, &a::IMP, 5 },
		{ "LDY", &a::LDY, &a::IMM, 2 },{ "LDA", &a::LDA, &a::IZX, 6 },{ "LDX", &a::LDX, &a::IMM, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "LDY", &a::LDY, &a::ZP0, 3 },{ "LDA", &a::LDA, &a::ZP0, 3 },{ "LDX", &a::LDX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "TAY", &a::TAY, &a::IMP, 2 },{ "LDA", &a::LDA, &a::IMM, 2 },{ "TAX", &a::TAX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "LDY", &a::LDY, &a::ABS, 4 },{ "LDA", &a::LDA, &a::ABS, 4 },{ "LDX", &a::LDX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCS", &a::BCS, &a::REL, 2 },{ "LDA", &a::LDA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "LDY", &a::LDY, &a::ZPX, 4 },{ "LDA", &a::LDA, &a::ZPX, 4 },{ "LDX", &a::LDX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "CLV", &a::CLV, &a::IMP, 2 },{ "LDA", &a::LDA, &a::ABY, 4 },{ "TSX", &a::TSX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 4 },{ "LDY", &a::LDY, &a::ABX, 4 },{ "LDA", &a::LDA, &a::ABX, 4 },{ "LDX", &a::LDX, &a::ABY, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "CPY", &a::CPY, &a::IMM, 2 },{ "CMP", &a::CMP, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPY", &a::CPY, &a::ZP0, 3 },{ "CMP", &a::CMP, &a::ZP0, 3 },{ "DEC", &a::DEC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INY", &a::INY, &a::IMP, 2 },{ "CMP", &a::CMP, &a::IMM, 2 },{ "DEX", &a::DEX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "CPY", &a::CPY, &a::ABS, 4 },{ "CMP", &a::CMP, &a::ABS, 4 },{ "DEC", &a::DEC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BNE", &a::BNE, &a::REL, 2 },{ "CMP", &a::CMP, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ZPX, 4 },{ "DEC", &a::DEC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLD", &a::CLD, &a::IMP, 2 },{ "CMP", &a::CMP, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ABX, 4 },{ "DEC", &a::DEC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "CPX", &a::CPX, &a::IMM, 2 },{ "SBC", &a::SBC, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPX", &a::CPX, &a::ZP0, 3 },{ "SBC", &a::SBC, &a::ZP0, 3 },{ "INC", &a::INC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INX", &a::INX, &a::IMP, 2 },{ "SBC", &a::SBC, &a::IMM, 2 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::SBC, &a::IMP, 2 },{ "CPX", &a::CPX, &a::ABS, 4 },{ "SBC", &a::SBC, &a::ABS, 4 },{ "INC", &a::INC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BEQ", &a::BEQ, &a::REL, 2 },{ "SBC", &a::SBC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ZPX, 4 },{ "INC", &a::INC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SED", &a::SED, &a::IMP, 2 },{ "SBC", &a::SBC, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ABX, 4 },{ "INC", &a::INC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 }
	};
}

/** Getter functions */
const CPU& emu6502::getCPU() const
{
	return m_cpu;
}

Byte* emu6502::getBus() 
{
	return m_bus;
}

int emu6502::getAddressRelative() const
{
	return static_cast<int>(m_addrRel.getCopy());
}

std::string_view emu6502::getInstructionName() const
{
	return m_instruction.mnemonic;
}

std::string_view emu6502::getAddressName() const
{
	return ADDRESS_STRINGS[static_cast<size_t>(m_lastAddressMode)];
}

int emu6502::getAddressValue() const
{
	return static_cast<int>(m_addrVal.getCopy());
}

Byte emu6502::getCycles() const
{
	return m_instruction.cycles;
}

/** Operational functions **/
void emu6502::clock()
{
	DEBUG_OUT("Clock");
	if (m_instruction.cycles == 0)
	{
		Byte opcode = m_bus[m_cpu.p++];

		m_instruction = m_lookup[opcode];
		(this->*m_instruction.addr)();
		(this->*m_instruction.exec)();
		//DEBUG_OUT(*this);
	}
	--m_instruction.cycles;
}

void emu6502::reset()
{
	Byte hi = m_bus[RESET_VECTOR + 1];
	Byte lo = m_bus[RESET_VECTOR];
	m_cpu.p = Word((hi << 8) | lo);
	m_cpu.a = 0x00;
	m_cpu.x = 0x00;
	m_cpu.y = 0x00;
	m_cpu.s = STACK_TOP;
}

void emu6502::irq()
{
	DEBUG_OUT("IRQ");
	busWrite(m_cpu.s--, m_cpu.p.getCopy() >> 8);			// hi byte of stack pointer
	busWrite(m_cpu.s--, m_cpu.p.getCopy() & 0XFF);			// lo byte of stack pointer
	m_cpu.flags.SetBit(Flags::BREAK);						// set break flag
	busWrite(m_cpu.s--, m_cpu.flags.getCopy());				// push flags with disable bit set

	m_cpu.flags.SetBit(Flags::INTERRUPT_DISABLE);

	Byte hi = m_bus[IRQ_VECTOR + 1];
	Byte lo = m_bus[IRQ_VECTOR];
	m_cpu.p = Word((hi << 8) | lo);
}

void emu6502::nmi()
{
	DEBUG_OUT("NMI");
	m_bus[m_cpu.s--] = m_cpu.p.getCopy() >> 8;				// hi byte of stack pointer
	m_bus[m_cpu.s--] = m_cpu.p.getCopy() & 0XFF;			// lo byte of stack pointer
	m_cpu.flags.ClearBit(Flags::BREAK);						// clear break flag
	m_bus[m_cpu.s--] = m_cpu.flags.getCopy();				// push flags with disable bit set
		
	m_cpu.flags.SetBit(Flags::INTERRUPT_DISABLE);

	Byte hi = m_bus[NMI_VECTOR + 1];
	Byte lo = m_bus[NMI_VECTOR];
	m_cpu.p = Word((hi << 8) | lo);
}

Byte emu6502::fetch()
{
	Byte byte = m_bus[m_cpu.p++];
	return byte;
}

size_t emu6502::fetch_and_execute()
{
	Byte opcode = m_bus[m_cpu.p++];

	m_instruction = m_lookup[opcode];
	(this->*m_instruction.addr)();
	(this->*m_instruction.exec)();
	return 0;
}

// Sipmly reads a text file of byte sized hexadecimal values
int emu6502::loadProgram(const char* fname, const Word& addr)
{
	size_t counter = addr;
	std::ifstream ifs(fname, std::ios::binary);
	std::string hex;

	if (ifs.fail()) return PROGRAM_LOAD_FAILURE;

	try
	{
		while (ifs >> hex)
			m_bus[counter++] = std::stoi(hex, nullptr, 16);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
		std::cerr << "\n\tShould be a text file containing the machine code in hex e.g. A2 00 ...";
		return PROGRAM_LOAD_FAILURE;
	}
	ifs.close();
	return PROGRAM_LOAD_SUCCESSFULL;
}

// Same as loadProgram but if the line starts with a memory address it is ignored e.g. FF00: a9 4c......
int Emu::emu6502::loadProgram2(const char* fname, const Word& addr)
{
	size_t counter = addr;
	std::ifstream ifs(fname, std::ios::in);
	std::string value;

	if (ifs.fail()) return PROGRAM_LOAD_FAILURE;

	try 
	{
		while (ifs >> value)
		{
			if (value.size() == 2)
			{
				m_bus[counter++] = std::stoi(value, nullptr, 16);
			}
		}
	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
	}

	return 0;
}

// loads a binary rom
int emu6502::loadProgramHex(const char* fname, const Word& addr)
{
	size_t counter = addr;
	std::ifstream ifs(fname, std::ios::binary);
	Byte hex;

	if (ifs.fail()) return PROGRAM_LOAD_FAILURE;

	try
	{
		while (ifs >> hex)
			m_bus[counter++] = hex;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
		std::cerr << "Error while reading hex file" << std::endl;
	}
	ifs.close();
	return PROGRAM_LOAD_SUCCESSFULL;
}

// Strips any leading memory addresses and creates a new file that can be loaded with loadProgram or loadProgram2
int Emu::emu6502::denatureHexText(const char* fname, const char* fname_new)
{
	std::ifstream ifs(fname, std::ios::in);
	std::ofstream ofs(fname_new, std::ios::out);
	std::string value;

	while (ifs >> value)
		if (value.length() == 2)
			ofs << value << ' ';


	return 0;
}

void emu6502::busWrite(const Word& addr, const Byte& val)
{
	DEBUG_OUT("Writing " << static_cast<int>(val) << " to: " << std::hex << static_cast<int>(addr));
	m_bus[addr] = val;
}
Byte emu6502::busRead(const Word& addr)
{
	DEBUG_OUT("Reading " << static_cast<int>(m_bus[addr]) << " from: " << std::hex << static_cast<int>(addr));
	return m_bus[addr];
}

void emu6502::setProgramCounter(const size_t& p)
{
	m_cpu.p = static_cast<Word>(p);
}

/*			START
* memory addressing functions */

// Implied
// This is the addressing mode for instruction like clear decimal flag and set carry flag
Byte emu6502::IMP()
{
	DEBUG_OUT("Implied"); 
	return static_cast<Byte>(Address_Mode::IMP);
}

// Immediate addressing
Byte emu6502::IMM()
{
	DEBUG_OUT("Immediate");
	m_addrVal = fetch();
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::IMM);
}

// Zero page addressing
Byte emu6502::ZP0()
{
	DEBUG_OUT("Zero page");
	m_addrVal = fetch();
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	DEBUG_OUT("\t\tValue at address: " << static_cast<int>(m_bus[static_cast<Word>(m_addrVal.getCopy())]));
	return static_cast<Byte>(Address_Mode::ZP0);
}

// Zero page addressing with x offset
Byte emu6502::ZPX()
{
	DEBUG_OUT("Zero page X");
	m_addrVal = fetch() + m_cpu.x.getCopy();
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::ZPX);
}

// Zero page addressing with y offset
Byte emu6502::ZPY()
{
	DEBUG_OUT("Zero page Y");
	m_addrVal = fetch() + m_cpu.y.getCopy();
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::ZPY);;
}

// Relative addressing
Byte emu6502::REL()
{
	DEBUG_OUT("Relative");
	m_addrRel = fetch();
	m_addrVal = m_addrRel.getCopy();
	DEBUG_OUT("\tAddrRel: " << static_cast<S_Byte>(m_addrRel.getCopy()));
	return static_cast<Byte>(Address_Mode::REL);
}

// Absolute addressing
Byte emu6502::ABS()
{
	DEBUG_OUT("Absolute");
	Word lo = fetch();
	Word hi = fetch();

	m_addrVal = Word((hi << 8) | lo);
	DEBUG_OUT("\tAddr: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::ABS);
}

// Absolute addressing with x offset
Byte emu6502::ABX()
{
	DEBUG_OUT("Absolute X");
	Word lo = fetch();
	Word hi = fetch();

	m_addrVal = Word((hi << 8) | lo) + static_cast<Word>(m_cpu.x.getCopy());
	if (m_addrVal.AND(0xFF00) != (hi << 8)) ++m_instruction.cycles;
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::ABX);
}

// Absolute addressing with y offset
Byte emu6502::ABY()
{
	DEBUG_OUT("Absolute Y");
	Word lo = fetch();
	Word hi = fetch();

	m_addrVal = Word((hi << 8) | lo) + static_cast<Word>(m_cpu.y.getCopy());
	if (m_addrVal.AND(0xFF00) != (hi << 8)) ++m_instruction.cycles;
	DEBUG_OUT("\tAddrVal: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::ABY);
}

// Indirect address mode, read low byte first, then hi byte
// Indirect addressing mode sets the program counter the memory address
// stored at the memory address given. e.g. lda (1000), get the low byte of the address
// at 1000 and the hi byte at 1001 
// Due to a hardware bug, if the indirect address were at 0x01FF, the high byte is read from 0x0100 instaed of 0x0200
Byte emu6502::IND()  // Indirection is only used by JMP
{
	DEBUG_OUT("Indirect");

	// Fetch the pointer (low and high bytes) from the program counter
	Byte ptr_lo = fetch();
	Byte ptr_hi = fetch();

	// Form the full pointer address
	Word ptr = Word((ptr_hi << 8) | ptr_lo);

	// Page boundary hardware bug
	if (ptr_lo == 0xFF)
	{
		m_addrVal = static_cast<Word>((m_bus[ptr & 0xFF00] << 8) | m_bus[ptr]);
	}
	else
	{
		m_addrVal = static_cast<Word>((m_bus[ptr + 1] << 8) | m_bus[ptr]);
	}

	DEBUG_OUT("\tJump Address: " << std::hex << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::IND);
}


// Indirect addressing with x offset
Byte emu6502::IZX()  // There is typically an address table held at the address location. So it's an address that stores other addresses
{
	DEBUG_OUT("Indirect X");
	Byte ptr = fetch();
	Byte x = m_cpu.x.getCopy();
	Byte lo = busRead((ptr + x)     & 0xFF);
	Byte hi = busRead((ptr + x + 1) & 0xFF);
	m_addrVal = Word((hi << 8) | lo);

	DEBUG_OUT("\tValue at addr: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::IZX);
}

// Indirect addressing with y offset
// Supplied 8 bit address indexes into zero page where the 16 bit address resides
Byte emu6502::IZY()
{
	DEBUG_OUT("Indirect Y");

	Byte ptr = fetch();  // Fetch zero-page pointer
	Byte y = m_cpu.y.getCopy();   // Get Y register

	// Fetch the low and high bytes of the base address
	Byte lo = busRead(ptr)     & 0xFF;
	Byte hi = busRead(ptr + 1) & 0xFF;
	m_addrVal = Word((hi << 8) | lo) + m_cpu.y.getCopy();

	if (m_addrVal.AND(0xFF00) != (hi << 8)) ++m_instruction.cycles;

	DEBUG_OUT("\tValue at addr: " << m_addrVal.getCopy());
	return static_cast<Byte>(Address_Mode::IZY);
}


/*			START
* instruction definitions */

// add with carry
// overflow = ~(a ^ arg) & (a ^ sum) & 0x80
Byte emu6502::ADC()
{
	DEBUG_OUT("ADC");

	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());	// check if it's immeidate addressing mode or not

	Word result = m_cpu.a.getCopy() + m_addrVal.getCopy() + m_cpu.flags.CheckBit(Flags::CARRY);

	checkFlag(~(m_cpu.a.getCopy() ^ m_addrVal.getCopy()) & (m_cpu.a.getCopy() ^ result) & 0x80, Flags::O_FLOW);

	m_cpu.a = result & 0xFF;

	checkFlag(result > 255, Flags::CARRY);
	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// Subtract with carry
// same as adc once you invert the operand
Byte emu6502::SBC()
{
	DEBUG_OUT("SBC");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_addrVal.Xor(0xFF);

	Word result = m_cpu.a.getCopy() + m_addrVal.getCopy() + m_cpu.flags.CheckBit(Flags::CARRY);

	checkFlag(~(m_cpu.a.getCopy() ^ m_addrVal.getCopy()) & (m_cpu.a.getCopy() ^ result) & 0x80, Flags::O_FLOW);
	m_cpu.a = result & 0xFF;

	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	checkFlag(result & 0xFF00, Flags::CARRY);

	return 0x00;
}

// Compare: basically just a substraction and sets bits
Byte emu6502::CMP()
{
	DEBUG_OUT("CMP");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	Byte result = m_cpu.a.getCopy() - m_addrVal.getCopy();

	DEBUG_OUT("\tResult: " << static_cast<int>(result));
	checkFlag(m_cpu.a.getCopy() >= m_addrVal.getCopy(), Flags::CARRY);
	checkFlag(result == 0, Flags::ZERO);
	checkFlag(Bits<Byte>::CheckBit(result, LastBit<Byte>), Flags::NEGATIVE);

	return 0;
}

// Compare x register
Byte emu6502::CPX()
{
	DEBUG_OUT("CPX");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());
	Byte result = m_cpu.x.getCopy() - m_addrVal.getCopy();

	DEBUG_OUT("\tResult: " << (int)result);
	checkFlag(m_cpu.x.getCopy() >= m_addrVal.getCopy(), Flags::CARRY);
	checkFlag(result == 0, Flags::ZERO);
	checkFlag(Bits<Byte>::CheckBit(result, LastBit<Byte>), Flags::NEGATIVE);

	return 0;
}

// Compare y register
Byte emu6502::CPY()
{
	DEBUG_OUT("CPY");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = m_bus[static_cast<Word>(m_addrVal.getCopy())];
	Byte result = m_cpu.y.getCopy() - m_addrVal.getCopy();

	DEBUG_OUT("\tResult: " << (int)result);
	checkFlag(m_cpu.y.getCopy() >= m_addrVal.getCopy(), Flags::CARRY);
	checkFlag(result == 0, Flags::ZERO);
	checkFlag(Bits<Byte>::CheckBit(result, LastBit<Byte>), Flags::NEGATIVE);

	return 0;
}

// Test if a bit is a set. Essentially performs accumulator & m_addrVal and only stores the result
Byte emu6502::BIT()
{
	DEBUG_OUT("BIT");
	Bits<Byte> value = busRead(m_addrVal.getCopy());
	Bits<Byte> result = m_cpu.a.getCopy() & value.getCopy();

	checkFlag(result.getCopy() == 0, Flags::ZERO);
	checkFlag(value.CheckBit((size_t)LastBit<Byte>), Flags::NEGATIVE);
	checkFlag(value.CheckBit((size_t)LastBit<Byte> - 1), Flags::O_FLOW);

	//std::cout << "Flags after: " << Bits<Byte>::ToBinaryString(m_cpu.flags.getCopy()) << std::endl;
	return 0x00;
}

// Logical and
Byte emu6502::AND()
{
	DEBUG_OUT("AND");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_cpu.a.And(static_cast<Byte>(m_addrVal.getCopy()));
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);

	return 0x00;
}

// a |= m_addrVal
Byte emu6502::ORA()
{
	DEBUG_OUT("ORA");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_cpu.a.Or(m_addrVal.getCopy());
	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// XOR accumulator
Byte emu6502::EOR()
{
	DEBUG_OUT("XOR");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_cpu.a.Xor(m_addrVal.getCopy());
	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// Branch Carry Clear
Byte emu6502::BCC()
{
	DEBUG_OUT("BCC\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (!(m_cpu.flags.CheckBit(Flags::CARRY)))
	{
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch Carry Set
Byte emu6502::BCS()
{
	DEBUG_OUT("BCS\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (m_cpu.flags.CheckBit(Flags::CARRY))
	{
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch if equal
Byte emu6502::BEQ()
{
	// if(! m_cpu.flags.CheckBit(Flags::ZERO) m_cpu.p = m_cpu.p.get() + static_cast<S_Byte>(m_addrRel.get());
	DEBUG_OUT("BEQ\n\tOffset: " << static_cast<int>(static_cast<S_Byte>(m_addrRel.getCopy())));
	if (m_cpu.flags.CheckBit(Flags::ZERO))
	{
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch if negative
Byte emu6502::BMI()
{
	DEBUG_OUT("BMI (Branch if negative)\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (m_cpu.flags.CheckBit(Flags::NEGATIVE))
	{
		//std::cout << "Negative\n";
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch if ZERO flag is 0
Byte emu6502::BNE()
{
	// if(! m_cpu.flags.CheckBit(Flags::ZERO) m_cpu.p = m_cpu.p.get() + static_cast<S_Byte>(m_addrRel.get());
	DEBUG_OUT("BNE\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (! (m_cpu.flags.CheckBit(Flags::ZERO)))
	{
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch on Plus
Byte emu6502::BPL()
{
	DEBUG_OUT("BPL\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (!(m_cpu.flags.CheckBit(Flags::NEGATIVE)))
	{
		++m_instruction.cycles;						// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Branch if overflow clear
Byte emu6502::BVC()
{
	DEBUG_OUT("BVC\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (!(m_cpu.flags.CheckBit(Flags::O_FLOW)))
	{
		++m_instruction.cycles;											// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Break if overflow set
Byte emu6502::BVS()
{
	DEBUG_OUT("BVS\n\tOffset: " << static_cast<int>(m_addrRel.getCopy()));
	if (m_cpu.flags.CheckBit(Flags::O_FLOW))
	{
		++m_instruction.cycles;											// +1 cycle for successful branch
		m_cpu.p = m_cpu.p.getCopy() + static_cast<S_Byte>(m_addrRel.getCopy());
	}
	return 0x00;
}

// Pushes program counter on stack, hi byte first
// Sets break flag, pushes flags
// Sets interrupt disable flags
// loads interrupt vector from 0xFFFF and 0xFFFE
Byte emu6502::BRK()
{
	DEBUG_OUT("BRK");
	m_bus[m_cpu.s--] = static_cast<Byte>(m_cpu.p.getCopy() >> 8);			// hi byte of stack pointer
	m_bus[m_cpu.s--] = static_cast<Byte>(m_cpu.p.getCopy() & 0XFF);			// lo byte of stack pointer
	m_cpu.flags.SetBit(Flags::BREAK);										// set break flag
	m_bus[m_cpu.s--] = m_cpu.flags.getCopy();								// push flags
	m_cpu.flags.ClearBit(Flags::BREAK);										// clear break flag
	m_cpu.p = Word(m_bus[IRQ_VECTOR + 1] | m_bus[IRQ_VECTOR]);				// load address at irq vector

	return 0x00;
}

// Clear carry
Byte emu6502::CLC()
{
	DEBUG_OUT("CLC");
	m_cpu.flags.ClearBit(Flags::CARRY);

	return 0x00;
}

// Clear decimal mode
Byte emu6502::CLD()
{
	DEBUG_OUT("CLD");
	m_cpu.flags.ClearBit(Flags::DECIMALE_MODE);

	return 0x00;
}

// Clear interrupt disable 
Byte emu6502::CLI()
{
	DEBUG_OUT("CLI");
	m_cpu.flags.ClearBit(Flags::INTERRUPT_DISABLE);

	return 0x00;
}

// Clear Overflow 
Byte emu6502::CLV()
{
	DEBUG_OUT("CLV");
	m_cpu.flags.ClearBit(Flags::O_FLOW);

	return 0x00;
}

// Set carry
Byte emu6502::SEC()
{
	DEBUG_OUT("SEC");
	m_cpu.flags.SetBit(Flags::CARRY);

	return 0x00;
}

// Set decimal
Byte emu6502::SED()
{
	DEBUG_OUT("SED");
	m_cpu.flags.SetBit(Flags::DECIMALE_MODE);

	return 0x00;
}

// Set interrupt
Byte emu6502::SEI()
{
	DEBUG_OUT("SEI");
	m_cpu.flags.SetBit(Flags::INTERRUPT_DISABLE);

	return 0x00;
}

// Decrement memory
Byte emu6502::DEC()
{
	DEBUG_OUT("DEC");
	Byte value = --(m_bus[(Word)m_addrVal.get()]);
	checkFlag(value == 0, Flags::ZERO);
	checkFlag(Bits<Byte>::CheckBit(value, LastBit<Byte>), Flags::NEGATIVE);
	
	return 0x00;
}

// Increment memory
Byte emu6502::INC()
{
	DEBUG_OUT("INC");
	Byte value = ++(m_bus[(Word)m_addrVal.get()]);
	checkFlag(value == 0, Flags::ZERO);
	checkFlag(Bits<Byte>::CheckBit(value, LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Decrement x register
Byte emu6502::DEX()
{
	DEBUG_OUT("DEX");
	--m_cpu.x;
	checkFlag(m_cpu.x.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.x.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	
	return 0x00;
}

// Increment x register
Byte emu6502::INX()
{
	DEBUG_OUT("INX");
	++m_cpu.x;
	checkFlag(m_cpu.x.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.x.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Decrement y register
Byte emu6502::DEY()
{
	DEBUG_OUT("DEY");
	--m_cpu.y;
	checkFlag(m_cpu.y.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.y.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Increment y register
Byte emu6502::INY()
{
	DEBUG_OUT("INY");
	++m_cpu.y;
	checkFlag(m_cpu.y.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.y.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// jump
Byte emu6502::JMP()
{
	DEBUG_OUT("JMP");
	m_cpu.p = static_cast<Word>(m_addrVal.getCopy());
	return 0x00;
}

// jump with return, pushes program counter onto stack first
Byte emu6502::JSR()
{
	DEBUG_OUT("JSR");
	m_bus[m_cpu.s--] = Byte(m_cpu.p.getCopy() >> 8);			// hi byte of program counter
	m_bus[m_cpu.s--] = Byte(m_cpu.p.getCopy() & 0XFF);			// lo byte of program counter
	m_cpu.p = static_cast<Word>(m_addrVal.getCopy());
	return 0x00;
}

// load accumulator
Byte emu6502::LDA()
{
	DEBUG_OUT("LDA");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());
	
	m_cpu.a = static_cast<Byte>(m_addrVal.getCopy());
	DEBUG_OUT("\t" << static_cast<int>(m_cpu.y.getCopy()) << "\t" << m_cpu.y.ToBinaryString());

	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// load x register
Byte emu6502::LDX()
{
	DEBUG_OUT("LDX");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_cpu.x = static_cast<Byte>(m_addrVal.getCopy());
	DEBUG_OUT("\t" << static_cast<int>(m_cpu.x.getCopy()) << "\t" << m_cpu.x.ToBinaryString());
	// Check Zero bit
	checkFlag(m_cpu.x.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.x.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Load y register
Byte emu6502::LDY()
{
	DEBUG_OUT("LDY");
	// First check if it was immediate addressing, then m_addrVal has value, otherwise value is at m_bus[m_addrVal]
	if (m_instruction.addr != &emu6502::IMM) m_addrVal = busRead(m_addrVal.getCopy());

	m_cpu.y = m_addrVal.getCopy();
	DEBUG_OUT("\t" << static_cast<int>(m_cpu.y.getCopy()) << "\t" << m_cpu.y.ToBinaryString());
	checkFlag(m_cpu.y.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.y.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// arithmatic shift left
// Differes from rotate left in that the carry first bit is always zero
Byte emu6502::ASL()
{
	DEBUG_OUT("ASL");
	Bits<Byte> result = 0;
	if (m_instruction.addr == &emu6502::IMP)
	{
		checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::CARRY);
		m_cpu.a.Las();
		result = m_cpu.a.getCopy();
	}
	else
	{
		Word address = static_cast<Word>(m_addrVal.getCopy());					 // We're operating on m_bus[m_addrVal] which has to be cast to a Word. Make using it easier and clearer
		checkFlag(Bits<Byte>::CheckBit(m_bus[address], LastBit<Byte>), Flags::CARRY);
		Bits<Byte>::Las(m_bus[address]);
		result = m_bus[address];
	}

	checkFlag(result.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	checkFlag(result.getCopy() == 0, Flags::ZERO);

	return 0x00;
}

// Logical shift right
// First bit is moved into carry, and carry and is NOT moved into the last bit
Byte emu6502::LSR()
{
	DEBUG_OUT("LSR");
	Bits<Byte> result = 0;	// It can operate on the accumulator or memory and we need to check the result to set the flags

	// LSR can work in address modes; IMP, ZPX, ZP0, ABS, ABX. If it's implied it operates on the accumulator
	if (m_instruction.addr == &emu6502::IMP)
	{
		checkFlag(m_cpu.a.CheckBit(1), Flags::CARRY);
		m_cpu.a.Ras();
		result = m_cpu.a.getCopy();
	}
	// Otherwise we're operating on memory
	else
	{
		Word address = static_cast<Word>(m_addrVal.getCopy());		 // We're operating on m_bus[m_addrVal] which has to be cast to a Word. Make using it easier and clearer
		checkFlag(Bits<Byte>::CheckBit(m_bus[address], 1), Flags::CARRY);
		Bits<Byte>::Ras(m_bus[address]);
		result = m_bus[address];
	}
	// Check Zero and Negitve now on result since it could be accumulator or memory
	checkFlag(result.getCopy() == 0, Flags::ZERO);
	checkFlag(result.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// Rotate left (only once)
// Carry bit gets set to first bit and last bit becomes carry bit
Byte emu6502::ROL()
{
	DEBUG_OUT("ROL");
	Bits<Byte> result = 0;
	bool oldCarry = m_cpu.flags.CheckBit(Flags::CARRY);
	bool willCarry = false;

	//std::cout << "ROL\t";

	// ROL can work in address modes; IMP, ZPX, ZP0, ABS, ABX. If it's implied it operates on the accumulator
	if (m_instruction.addr == &emu6502::IMP)
	{
		//std::cout << "Imp\n";
		willCarry = m_cpu.a.CheckBit(LastBit<Byte>);
		m_cpu.a.Las();
		if (oldCarry) m_cpu.a.SetBit(1);

		checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::CARRY);
		result = m_cpu.a.getCopy();
	}
	else
	{
		Word address = static_cast<Word>(m_addrVal.getCopy());					 // We're operating on m_bus[m_addrVal] which has to be cast to a Word. Make using it easier and clearer
		//std::cout << "Addr: " << std::hex << static_cast<int>(address) << std::endl;
		willCarry = Bits<Byte>::CheckBit(m_bus[address], LastBit<Byte>); // If the last bit is set, then a shift left results in a carry. Check it before we shift

		Bits<Byte>::Las(m_bus[address]);
		if (oldCarry)
		{
			Bits<Byte>::SetBit(m_bus[address], 1);
		}
		// If the first bit was set, then a shift right results in a carry
		result = m_bus[address];
	}
	checkFlag(willCarry, Flags::CARRY);
	//m_addrVal = result.get();
	// Check Zero and Negitve now on result since it could be accumulator or memory
	checkFlag(result.getCopy() == 0, Flags::ZERO);
	checkFlag(result.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// Rotate right
Byte emu6502::ROR()
{
	DEBUG_OUT("ROR");
	Bits<Byte> result = 0;
	bool oldCarry = m_cpu.flags.CheckBit(Flags::CARRY);
	bool willCarry = false;

	// ROr can work in address modes; IMP, ZPX, ZP0, ABS, ABX. If it's implied it operates on the accumulator
	if (m_instruction.addr == &emu6502::IMP)
	{
		willCarry = m_cpu.a.CheckBit(1);

		m_cpu.a.Ras();
		if (oldCarry) result.SetBit(LastBit<Byte>);
		result = m_cpu.a.getCopy();
	}
	else
	{
		Word address = static_cast<Word>(m_addrVal.getCopy());		 // We're operating on m_bus[m_addrVal] which has to be cast to a Word. Make using it easier and clearer
		willCarry = Bits<Byte>::CheckBit(m_bus[address], 1); // If the first bit is set, then a shift right results in a carry. Check it before we shift

		Bits<Byte>::Ras(m_bus[address]);
		if (oldCarry) Bits<Byte>::SetBit(m_bus[address], LastBit<Byte>);
		result = m_bus[address];
	}
	checkFlag(willCarry, Flags::CARRY);
	// Check Zero and Negitve now on result since it could be accumulator or memory
	checkFlag(result.getCopy() == 0, Flags::ZERO);
	checkFlag(result.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Push accumulator
Byte emu6502::PHA()
{
	DEBUG_OUT("PHA");
	m_bus[m_cpu.s--] = m_cpu.a.getCopy();
	return 0x00;
}

// Push processor state (FLAGS)
Byte emu6502::PHP()
{
	DEBUG_OUT("PHP");
	m_bus[m_cpu.s--] = m_cpu.flags.getCopy();
	return 0x00;
}

// Pull accumulator
Byte emu6502::PLA()
{
	DEBUG_OUT("PLA");
	m_cpu.a = m_bus[++m_cpu.s];
	return 0x00;
}

// Pull processor state
Byte emu6502::PLP()
{
	DEBUG_OUT("PLP");
	m_cpu.flags = m_bus[++m_cpu.s];
	return 0x00;
}

// return
Byte emu6502::RTI()
{
	DEBUG_OUT("RTI");
	m_cpu.flags = busRead(++m_cpu.s);
	Byte lo = busRead(++m_cpu.s);
	Byte hi = busRead(++m_cpu.s);

	m_cpu.p = Word((hi << 8) | lo);

	return 0x00;
}

// Return from subroutine
Byte emu6502::RTS()
{
	DEBUG_OUT("RTS");
	Byte lo = busRead(++m_cpu.s);
	Byte hi = busRead(++m_cpu.s);
	m_cpu.p = Word((hi << 8) | lo);


	return 0x00;
}

// Store Accumulator
Byte emu6502::STA()
{
	DEBUG_OUT("STA");
	busWrite(m_addrVal.getCopy(), m_cpu.a.getCopy());

	return 0x00;
}

// store y register
Byte emu6502::STY()
{
	DEBUG_OUT("STY");
	busWrite(m_addrVal.getCopy(), m_cpu.y.getCopy());

	return 0x00;
}

// store x register
Byte emu6502::STX()
{
	DEBUG_OUT("STX");
	busWrite(m_addrVal.getCopy(), m_cpu.x.getCopy());

	return 0x00;
}

// Transfer stack ponter to x
Byte emu6502::TSX()
{
	DEBUG_OUT("TSX");
	m_cpu.x = m_bus[++m_cpu.s];

	checkFlag(m_cpu.x.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.x.CheckBit(LastBit<Byte>), Flags::NEGATIVE);

	return 0x00;
}

// Transfer X to stack ponter
Byte emu6502::TXS()
{
	DEBUG_OUT("TXS");
	m_bus[m_cpu.s--] = m_cpu.x.getCopy();

	return 0x00;
}

// Transfer a to x
Byte emu6502::TAX()
{
	DEBUG_OUT("TAX");
	m_cpu.x = m_cpu.a.getCopy();
	//m_addrVal = m_cpu.a.get();

	checkFlag(m_cpu.x.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.x.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Transfer X to A
Byte emu6502::TXA()
{
	DEBUG_OUT("TXA");
	m_cpu.a = m_cpu.x.getCopy();
	//m_addrVal = m_cpu.x.get();

	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Transfer A to Y
Byte emu6502::TAY()
{
	DEBUG_OUT("TAY");
	m_cpu.y = m_cpu.a.getCopy();
	//m_addrVal = m_cpu.a.get();

	checkFlag(m_cpu.y.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.y.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// Transfer Y to A
Byte emu6502::TYA()
{
	DEBUG_OUT("TYA");
	m_cpu.a = m_cpu.y.getCopy();
	//m_addrVal = m_cpu.y.get();

	checkFlag(m_cpu.a.getCopy() == 0, Flags::ZERO);
	checkFlag(m_cpu.a.CheckBit(LastBit<Byte>), Flags::NEGATIVE);
	return 0x00;
}

// just kills 2 clock cycles
Byte emu6502::NOP()
{
	DEBUG_OUT("NOP");
	return 0x00;
}

// Any undefined opcode
Byte emu6502::XXX()
{
	DEBUG_OUT("XXX");
	return 0X01;
}


/* EXTRAS */
// If the condition is met (true), the flag will be set, otherwise the flag is cleared. 
void emu6502::checkFlag(bool condition, const Flags& flag)
{
	if (condition)
		m_cpu.flags.SetBit(flag);
	else
		m_cpu.flags.ClearBit(flag);
}

void emu6502::printMemoryRange(const size_t& start, const size_t& end) const
{
	std::cout << std::setfill('0') << std::hex;
	size_t count = 1;
	for (size_t i = start; i <= end; ++i)
	{
		std::cout << std::setw(2) << static_cast<int>(m_bus[i]) << ' ';
		if (count > 1 && count % 32 == 0) std::cout << '\n';
		++count;
	}
}

/*@Todo: 
	Fix this
*/
void emu6502::disassembleText(const char* fname)
{
	struct AsmLine
	{
		size_t byte = 0;
		std::string line;
	};
	std::vector<AsmLine> lines;
	std::map<size_t, std::string> labels; // Store labels separately
	size_t currentByte = 0;
	size_t labelCount = 0;
	std::stringstream ss;
	bool branch = false;

	size_t counter = USER_PROGRAM;
	std::ofstream ofs;
	Instruction instr;
	Word oldPC = m_cpu.p.getCopy();

	Byte opcode = 0x00;
	Word value = 0x0000;
	size_t lineNumber = 0;

	m_cpu.p = USER_PROGRAM;

	do
	{
		opcode = fetch();
		currentByte = 1;
		value = 0;
		instr = m_lookup[opcode];

		if (instr.addr != &emu6502::IMP)
		{
			if (instr.addr == &emu6502::ABS || instr.addr == &emu6502::ABX || instr.addr == &emu6502::ABY ||
				instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
			{
				Byte lo = fetch();
				Byte hi = fetch();
				value = (hi << 8) | lo;
				currentByte = 3;
			}
			else
			{
				value = fetch();
				currentByte = 2;
			}
		}

		if (instr.exec == &emu6502::BNE || instr.exec == &emu6502::BEQ || instr.exec == &emu6502::BCC || instr.exec == &emu6502::BMI ||
			instr.exec == &emu6502::BCS || instr.exec == &emu6502::BPL || instr.exec == &emu6502::BVC || instr.exec == &emu6502::BVS)
		{
			int offset = static_cast<int>(S_Byte(value));
			size_t targetAddress = counter + offset + currentByte;
			labels[targetAddress] = "\t\t   l" + std::to_string(labelCount) + ":\n";
			branch = true;
		}

		ss << std::setbase(10) << std::setfill('0') << std::setw(6) << lineNumber << '\t'
			<< std::setw(2) << std::setbase(16) << static_cast<int>(opcode) << '\t'
			<< std::setw(4) << std::setfill('0') << value << '\t'
			<< instr.mnemonic << std::setfill(' ');

		if (instr.addr != &emu6502::IMP)
		{
			ss << '\t';
			if (instr.addr == &emu6502::IMM)
				ss << "#$" << std::setw(2) << std::setfill('0');
			else if (!branch)
				ss << "$";

			if (instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
				ss << "(";

			if (branch)
			{
				ss << "l" << labelCount++;
			}
			else
			{
				ss << value;
			}

			if (instr.addr == &emu6502::ABX || instr.addr == &emu6502::ABY || instr.addr == &emu6502::ZPX || instr.addr == &emu6502::ZPY)
			{
				ss << ((instr.addr == &emu6502::ZPX || instr.addr == &emu6502::ABX) ? ",X" : ",Y");
			}

			if (instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
				ss << ")";
		}
		ss << std::endl;

		lines.push_back({ currentByte, ss.str() });
		++lineNumber;
		ss.str("");
		currentByte = 0;
		branch = false;

	} while (instr.mnemonic != "BRK");

	ofs.open("asm.asm", std::ios::out);
	ofs << std::setw(6) << "Line #\t" << std::setw(2) << "Op\t" << std::setw(4) << "Value\t" << "Mnemonic\t" << std::endl;
	ofs << "---------------------------------------------\n";

	size_t currentAddress = USER_PROGRAM;
	for (const auto& line : lines)
	{
		if (labels.count(currentAddress))
		{
			ofs << labels[currentAddress];
		}
		ofs << line.line;
		currentAddress += line.byte;
	}

	ofs.close();
	m_cpu.p = oldPC;
}
	/*
	struct AsmLine
	{
		size_t byte = 0;
		std::string line;
	};
	std::vector<AsmLine> lines(0xFFFF);
	size_t currentByte = 0;
	size_t labelCount = 0;
	std::string str;
	bool branch = false;

	size_t counter = USER_PROGRAM;
	std::ofstream ofs;
	std::stringstream ss;
	std::string hex;
	Instruction instr;
	Word oldPC = m_cpu.p.get();

	Byte opcode = 0x00;
	Word value = 0x0000;
	size_t lineNumber = 0;

	m_cpu.p = USER_PROGRAM;


	do
	{
		opcode = fetch();
		currentByte = 1;
		value = 0;
		instr = m_lookup[opcode];

		// Read the instruction and the necessary bytes of the addressing mode
		if (instr.addr != &emu6502::IMP)
		{
			if (instr.addr == &emu6502::ABS || instr.addr == &emu6502::ABX || instr.addr == &emu6502::ABY ||
				instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
			{
				Byte lo = fetch();
				Byte hi = fetch();
				value = (hi << 8) | lo;
				currentByte = 3;
			}
			else
			{
				value = fetch();
				currentByte = 2;
			}
		}

		// If the instruction is a branch instruction we need to insert a label at the line specified by the offset
		if (instr.exec == &emu6502::BNE || instr.exec == &emu6502::BEQ || instr.exec == &emu6502::BCC || instr.exec == &emu6502::BMI ||
			instr.exec == &emu6502::BCS || instr.exec == &emu6502::BPL || instr.exec == &emu6502::BVC || instr.exec == &emu6502::BVS)
		{
			size_t totalBytesMoved = currentByte;
			branch = true;
			int offset = static_cast<int>(S_Byte(value));			// this is how many BYTES back to put it
			ss << "\t\t   l" << labelCount << ":\n";					// generate the label
			// starting at end of vector, 
			// while we haven't moved back to the offset position in vector determined by bytes of each line, 
			// decrement the iterator and add to total bytes moved
			//auto i = lines.end();
			//while (totalBytesMoved < std::abs(offset) - 1)
			//{
			//	totalBytesMoved += i->byte;
			//	if (offset < 0)
			//		i.operator--();
			//	else
			//		++i;
			//}
			if (offset > 0)
			{
				lines.insert(lines.begin() + currentByte + offset, AsmLine { 0, ss.str() });
			}
			else
			{
				lines.insert(lines.begin() + currentByte - offset, AsmLine{ 0, ss.str() });
			}
			//lines.insert(offset > 0 ? lines.begin() + currentByte + offset : lines.begin() + currentByte - offset, {0, ss.str()});
			// Don't update labelCount yet because we use it again
			ss.str("");
		}

		// Populate all the information for the line the be output to the file
		ss << std::setbase(10) << std::setfill('0') << std::setw(6) << lineNumber << '\t'	// line number
			<< std::setw(2) << std::setbase(16) << static_cast<int>(opcode) << '\t'			// opcode
			<< std::setw(4) << std::setfill('0') << value << '\t' 							// value
			<< instr.mnemonic << std::setfill(' ');											// mnemonic

		// Check if it requires #, $, (), ,X or ,Y and print value
		if (instr.addr != &emu6502::IMP)													// If it's implied then there's nothing left
		{
			ss << '\t';
			if (instr.addr == &emu6502::IMM)
				ss << "#$" << std::setw(2) << std::setfill('0');
			else
				if (!branch)			// If it's a branch we don't a $ because we're printing a label
					ss << "$";
			// If it's an indirect addressing mode we need parenthesis 
			if (instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
				ss << "(";

			// If the instruction was a branch then print the label to branch to, otherwise print the value
			if (branch)
			{
				ss << "l" << labelCount;
				++labelCount;
			}
			else
			{
				ss << value;
			}

			// If it's an addressing mode that uses an X or Y register offset then it needs to print ,(X)(Y)
			if (instr.addr == &emu6502::ABX || instr.addr == &emu6502::ABY || instr.addr == &emu6502::ZPX || instr.addr == &emu6502::ZPY)
			{
				if (instr.addr == &emu6502::ZPX || instr.addr == &emu6502::ABX)
					ss << ",X";
				else
					ss << ",Y";
			}

			// Then again if it was an indirect addressing mode add the ending parenth
			if (instr.addr == &emu6502::IND || instr.addr == &emu6502::IZX || instr.addr == &emu6502::IZY)
				ss << ")";
		}
		ss << std::endl;

		// Add the line to the lines vector, increase lineNumber, 0 currentByte and clear the string stream
		lines.push_back({ currentByte, ss.str() });
		++lineNumber;
		ss.str("");
		currentByte = 0;
		branch = false;

	} while (instr.mnemonic != "BRK");

	// Open the file, print the header, and then print each line in the vector
	ofs.open("asm.asm", std::ios::out);
	ofs << std::setw(6) << "Line #\t" << std::setw(2) << "Op\t" << std::setw(4) << "Value\t" << "Mnemonic\t" << std::endl;
	ofs << "---------------------------------------------\n";
	for (const auto& line : lines)
		ofs << line.line;
	ofs.close();
	// Restore the program counter
	m_cpu.p = oldPC;*/
//}
