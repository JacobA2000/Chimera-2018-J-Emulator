/*
	CHIMERA 2018 J Microprocesor Emulator
	Computer and Network Systems
	UWE BRISTOL

	User Advice: 
	1. To run this system please use the included marking server.exe file.
	2. The debug information is currently disabled to allow the marking server to run much faster. 
	   To re-enable this information uncomment the printf lines in the emulate function.

	Project started: 20/01/2020
	Last revised: 27/02/2020 - Cleaned up code and added comments

	Author: Jacob Allen - 19003931
*/

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "19003931"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x80
#define FLAG_N  0x20
#define FLAG_V  0x10
#define FLAG_Z  0x08
#define FLAG_C  0x01
#define REGISTER_M  7
#define REGISTER_A	6
#define REGISTER_H	5
#define REGISTER_L	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0 

WORD IndexRegister;

BYTE Registers[8];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"BRA rel      ", 
"BCC rel      ", 
"BCS rel      ", 
"BNE rel      ", 
"BEQ rel      ", 
"BVC rel      ", 
"BVS rel      ", 
"BMI rel      ", 
"BPL rel      ", 
"BGE rel      ", 
"BLE rel      ", 
"BLS rel      ", 
"BHI rel      ", 
"ILLEGAL     ", 
"RTN impl     ", 
"ILLEGAL     ", 

"ST abs       ", 
"PSH  ,A      ", 
"POP A,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"CLC impl     ", 
"SEC impl     ", 
"CLI impl     ", 
"STI impl     ", 
"SEV impl     ", 
"CLV impl     ", 
"DEX impl     ", 
"INX impl     ", 
"NOP impl     ", 
"WAI impl     ", 
"ILLEGAL     ", 

"ST abs,X     ", 
"PSH  ,s      ", 
"POP s,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ADI  #       ", 
"SBI  #       ", 
"CPI  #       ", 
"ANI  #       ", 
"XRI  #       ", 
"MVI  #,B     ", 
"MVI  #,C     ", 
"MVI  #,D     ", 
"MVI  #,E     ", 
"MVI  #,L     ", 
"MVI  #,H     ", 

"ILLEGAL     ", 
"PSH  ,B      ", 
"POP B,       ", 
"JPR abs      ", 
"CCC abs      ", 
"CCS abs      ", 
"CNE abs      ", 
"CEQ abs      ", 
"CVC abs      ", 
"CVS abs      ", 
"CMI abs      ", 
"CPL abs      ", 
"CHI abs      ", 
"CLE abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

"ILLEGAL     ", 
"PSH  ,C      ", 
"POP C,       ", 
"TST abs      ", 
"INC abs      ", 
"DEC abs      ", 
"RCR abs      ", 
"RCL abs      ", 
"SAL abs      ", 
"ASR abs      ", 
"NOT abs      ", 
"ROL abs      ", 
"ROR abs      ", 
"ILLEGAL     ", 
"LDX  #       ", 
"LODS  #      ", 

"STOX abs     ", 
"PSH  ,D      ", 
"POP D,       ", 
"TST abs,X    ", 
"INC abs,X    ", 
"DEC abs,X    ", 
"RCR abs,X    ", 
"RCL abs,X    ", 
"SAL abs,X    ", 
"ASR abs,X    ", 
"NOT abs,X    ", 
"ROL abs,X    ", 
"ROR abs,X    ", 
"ILLEGAL     ", 
"LDX abs      ", 
"LODS abs     ", 

"STOX abs,X   ", 
"PSH  ,E      ", 
"POP E,       ", 
"TSTA A,A     ", 
"INCA A,A     ", 
"DECA A,A     ", 
"RCRA A,A     ", 
"RCLA A,A     ", 
"RCLA A,A     ",
"SALA A,A     ", 
"ASRA A,A     ", 
"NOTA A,A     ", 
"ROLA A,A     ", 
"RORA A,A     ", 
"ILLEGAL     ", 
"LDX abs,X    ", 
"LODS abs,X   ", 

"ILLEGAL     ", 
"PSH  ,L      ", 
"POP L,       ", 
"ILLEGAL     ", 
"TAS impl     ", 
"TSA impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"MOVE A,A     ", 
"MOVE B,A     ", 
"MOVE C,A     ", 
"MOVE D,A     ", 
"MOVE E,A     ", 
"MOVE L,A     ", 
"MOVE H,A     ", 
"MOVE M,A     ", 

"ILLEGAL     ", 
"PSH  ,H      ", 
"POP H,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"SWI impl     ", 
"RTI impl     ", 
"ILLEGAL     ", 
"MOVE A,B     ", 
"MOVE B,B     ", 
"MOVE C,B     ", 
"MOVE D,B     ", 
"MOVE E,B     ", 
"MOVE L,B     ", 
"MOVE H,B     ", 
"MOVE M,B     ", 

"ADC A,B      ", 
"SBC A,B      ", 
"CMP A,B      ", 
"IOR A,B      ", 
"AND A,B      ", 
"XOR A,B      ", 
"BT A,B       ", 
"ILLEGAL     ", 
"MOVE A,C     ", 
"MOVE B,C     ", 
"MOVE C,C     ", 
"MOVE D,C     ", 
"MOVE E,C     ", 
"MOVE L,C     ", 
"MOVE H,C     ", 
"MOVE M,C     ", 

"ADC A,C      ", 
"SBC A,C      ", 
"CMP A,C      ", 
"IOR A,C      ", 
"AND A,C      ", 
"XOR A,C      ", 
"BT A,C       ", 
"ILLEGAL     ", 
"MOVE A,D     ", 
"MOVE B,D     ", 
"MOVE C,D     ", 
"MOVE D,D     ", 
"MOVE E,D     ", 
"MOVE L,D     ", 
"MOVE H,D     ", 
"MOVE M,D     ", 

"ADC A,D      ", 
"SBC A,D      ", 
"CMP A,D      ", 
"IOR A,D      ", 
"AND A,D      ", 
"XOR A,D      ", 
"BT A,D       ", 
"LD  #        ", 
"MOVE A,E     ", 
"MOVE B,E     ", 
"MOVE C,E     ", 
"MOVE D,E     ", 
"MOVE E,E     ", 
"MOVE L,E     ", 
"MOVE H,E     ", 
"MOVE M,E     ", 

"ADC A,E      ", 
"SBC A,E      ", 
"CMP A,E      ", 
"IOR A,E      ", 
"AND A,E      ", 
"XOR A,E      ", 
"BT A,E       ", 
"LD abs       ", 
"MOVE A,L     ", 
"MOVE B,L     ", 
"MOVE C,L     ", 
"MOVE D,L     ", 
"MOVE E,L     ", 
"MOVE L,L     ", 
"MOVE H,L     ", 
"MOVE M,L     ", 

"ADC A,L      ", 
"SBC A,L      ", 
"CMP A,L      ", 
"IOR A,L      ", 
"AND A,L      ", 
"XOR A,L      ", 
"BT A,L       ", 
"LD abs,X     ", 
"MOVE A,H     ", 
"MOVE B,H     ", 
"MOVE C,H     ", 
"MOVE D,H     ", 
"MOVE E,H     ", 
"MOVE L,H     ", 
"MOVE H,H     ", 
"MOVE M,H     ", 

"ADC A,H      ", 
"SBC A,H      ", 
"CMP A,H      ", 
"IOR A,H      ", 
"AND A,H      ", 
"XOR A,H      ", 
"BT A,H       ", 
"ILLEGAL     ", 
"MOVE A,M     ", 
"MOVE B,M     ", 
"MOVE C,M     ", 
"MOVE D,M     ", 
"MOVE E,M     ", 
"MOVE L,M     ", 
"MOVE H,M     ", 
"MOVE -,-     ", 

"ADC A,M      ", 
"SBC A,M      ", 
"CMP A,M      ", 
"IOR A,M      ", 
"AND A,M      ", 
"XOR A,M      ", 
"BT A,M       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"JMP abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

/*
Function: set_flag_z
Description: used for setting the zero flag
Parameters: BYTE inReg : the register to test in order to set the flag.
Returns: Nothing (void)
Warnings: none
*/
void set_flag_z(BYTE inReg) {
	BYTE reg; 
	reg = inReg; 

	if (reg == 0x00)
	{ 
		Flags = Flags | FLAG_Z;
	}
	else 
	{ 
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

/*
Function: set_flag_n
Description: used for setting the negative flag
Parameters: BYTE inReg : the register to test in order to set the flag. 
Returns: Nothing (void)
Warnings: none
*/
void set_flag_n(BYTE inReg) {
	BYTE reg;
	reg = inReg;

	if ((reg & 0x80) == 0x80) // most significant bit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

/*
Function: set_flag_v
Description: used for setting the overflow flag
Parameters: BYTE in1 : the first parameter, BYTE in2 : the second parameter, BYTE in3 : the third parameter
Returns: Nothing (void)
Warnings: the second parameter should be passed in as a negative if a subtraction has occured
*/
void set_flag_v(BYTE in1, BYTE in2, BYTE out1) {
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;
	reg1in = in1;
	reg2in = in2;
	regOut = out1;
	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) //overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{
		Flags = Flags | FLAG_V;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}
}

/*
Function: getAddressAbs
Description: get the adress using the abs addressing mode
Parameters: none
Returns: WORD
Warnings: None
*/
WORD getAddressAbs() {
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;

	HB = fetch();
	LB = fetch();
	address += (WORD)((WORD)HB << 8) + LB;
	return address;
}

/*
Function: getAddressAbsX
Description: get the adress using the abs x addressing mode
Parameters: none
Returns: WORD
Warnings: None
*/
WORD getAddressAbsX() {
	WORD address = 0;

	address += IndexRegister;
	address += getAddressAbs();
	return address;
}

/*
Function: push
Description: Pushes a registers contents onto the top of the stack
Parameters: BYTE reg : the register to push
Returns: Nothing (void)
Warnings: None
*/
void push(BYTE reg) {
	if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
		//Store contents of a specified register into Memory at the address held in the stackpointer.
		Memory[StackPointer] = reg;
		StackPointer--;
	}
}

/*
Function: ADC_F
Description: Adds register to accumulator with carry
Parameters: BYTE reg : the register to add
Returns: Nothing (void)
Warnings: None
*/
void ADC_F(BYTE reg) {
	BYTE param1, param2;
	WORD temp_word;

	//Adding a specified registers contents to the accumulator
	param1 = Registers[REGISTER_A];
	param2 = reg;
	temp_word = (WORD)param1 + (WORD)param2;
	if ((Flags & FLAG_C) != 0) {
		temp_word++;
	}
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C; // Set carry flag
	}
	else {
		Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
	}
	set_flag_n(temp_word);
	set_flag_z(temp_word);
	set_flag_v(param1, param2, temp_word);
	Registers[REGISTER_A] = temp_word;
}

/*
Function: SBC_F
Description: Register subtracted from accumulator with carry
Parameters: BYTE reg : the register to subtract
Returns: Nothing (void)
Warnings: None
*/
void SBC_F(BYTE reg) {
	BYTE param1, param2;
	WORD temp_word;

	//Subtracting a specified registers contents to the accumulator
	param1 = Registers[REGISTER_A];
	param2 = reg;
	temp_word = (WORD)param1 - (WORD)param2;
	if ((Flags & FLAG_C) != 0) {
		temp_word--;
	}
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C; // Set carry flag
	}
	else {
		Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
	}
	set_flag_n(temp_word);
	set_flag_z(temp_word);
	set_flag_v(param1, -param2, temp_word);
	Registers[REGISTER_A] = temp_word;
}

/*
Function: CMP_F
Description: Used to compare registers with the accumulator
Parameters: BYTE reg : the register to compare with accumulator;
Returns: Nothing (void)
Warnings: None
*/
void CMP_F(BYTE reg) {
	BYTE param1, param2;
	WORD temp_word;

	//Comparing a specified registers contents to the accumulator via the subtraction process.
	param1 = Registers[REGISTER_A];
	param2 = reg;
	temp_word = (WORD)param1 - (WORD)param2;
	if (temp_word >= 0x100) {
		Flags = Flags | FLAG_C; // Set carry flag
	}
	else {
		Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
	}
	set_flag_n(temp_word);
	set_flag_z(temp_word);
	set_flag_v(param1, -param2, temp_word);
}

/*
Function: IOR_F
Description: Inclusive OR between the accumulator and a register 
Parameters: BYTE reg : the register to use in the IOR;
Returns: Nothing (void)
Warnings: None
*/
void IOR_F(BYTE reg) {
	WORD temp_word;

	//Performing an inclusive or between a specified registers contents and the accumulator
	temp_word = (WORD)Registers[REGISTER_A] | (WORD)reg;

	set_flag_n(temp_word);
	set_flag_z(temp_word);

	Registers[REGISTER_A] = (BYTE)temp_word;

	Flags = Flags & (0xFF - FLAG_V);
}

/*
Function: AND_F
Description: AND between the accumulator and a register
Parameters: BYTE reg : the register to use in the AND;
Returns: Nothing (void)
Warnings: None
*/
void AND_F(BYTE reg) {
	WORD temp_word;

	//Performing an and or between a specified registers contents and the accumulator
	temp_word = (WORD)Registers[REGISTER_A] & (WORD)reg;

	set_flag_n(temp_word);
	set_flag_z(temp_word);

	Registers[REGISTER_A] = (BYTE)temp_word;
}

/*
Function: XOR_F
Description: XOR between the accumulator and a register
Parameters: BYTE reg : the register to use in the XOR;
Returns: Nothing (void)
Warnings: None
*/
void XOR_F(BYTE reg) {
	WORD temp_word;

	//Performing an exclusive or between a specified registers contents and the accumulator
	temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)reg;

	set_flag_n(temp_word);
	set_flag_z(temp_word);

	Registers[REGISTER_A] = (BYTE)temp_word;
}

/*
Function: BT_F
Description: bit test between the accumulator and a register
Parameters: BYTE reg : the register to use in the bit test;
Returns: Nothing (void)
Warnings: None
*/
void BT_F(BYTE reg) {
	WORD temp_word;

	//Performs a bit test between a specified registers contents and the accumulator, using the and operation.
	temp_word = (WORD)Registers[REGISTER_A] & (WORD)reg;

	set_flag_n(temp_word);
	set_flag_z(temp_word);
}

void Group_1(BYTE opcode){
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word;
	BYTE param1, param2;
	BYTE saved_flags;
	WORD offset;

	//FLAGS FOR BRANCH AND CALL OPERATIONS
	BYTE NF, VF, CF, ZF;

	#pragma region Setting Flag Bytes
	if ((Flags & FLAG_N) == 0)
	{
		NF = 0;
	}
	else {
		NF = 1;
	}

	if ((Flags & FLAG_V) == 0)
	{
		VF = 0;
	}
	else {
		VF = 1;
	}

	if ((Flags & FLAG_C) == 0)
	{
		CF = 0;
	}
	else {
		CF = 1;
	}

	if ((Flags & FLAG_Z) == 0)
	{
		ZF = 0;
	}
	else {
		ZF = 1;
	}
	#pragma endregion

	switch(opcode) {
		
		/*
		OPERATOR: LD
		DESC: LOAD MEMORY INTO ACCUMULATOR 
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region LD
		case 0xB7: //LD #
			//Put memory into the accumulator using the data variable
			data = fetch();
			Registers[REGISTER_A] = data;
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0xC7: //LD Abs
			//Put memory found at a specific address into the accumulator
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE) {
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0xD7: //LD Abs,X
			//Put memory found at a specific address into the accumulator
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE) {
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
			break;
	#pragma endregion

		/*
		OPERATOR: ST
		DESC: STORES ACCUMULATOR INTO MEMORY
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ST
		case 0x10: //ST Abs
			//Store contents of the accumulator at a specifc address in memory
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x20: //ST Abs,X
			//Store contents of the accumulator at a specifc address in memory
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: ADC
		DESC: REGISTER ADDED TO ACCUMULATOR WITH CARRY
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ADC
		case 0x90: //ADC A,B
			ADC_F(Registers[REGISTER_B]);
			break;

		case 0xA0: //ADC A,C
			ADC_F(Registers[REGISTER_C]);
			break;

		case 0xB0: //ADC A,D
			ADC_F(Registers[REGISTER_D]);
			break;

		case 0xC0: //ADC A,E
			ADC_F(Registers[REGISTER_E]);
			break;

		case 0xD0: //ADC A,L
			ADC_F(Registers[REGISTER_L]);
			break;

		case 0xE0: //ADC A,H
			ADC_F(Registers[REGISTER_H]);
			break;

		case 0xF0: //ADC A,M
			address = (WORD)Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			ADC_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: SBC
		DESC: Register subtracted to Accumulator with Carry
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SBC
		case 0x91: //SBC A,B
			SBC_F(Registers[REGISTER_B]);
			break;

		case 0xA1: //SBC A,C
			SBC_F(Registers[REGISTER_C]);
			break;

		case 0xB1: //SBC A,D
			SBC_F(Registers[REGISTER_D]);
			break;

		case 0xC1: //SBC A,E
			SBC_F(Registers[REGISTER_E]);
			break;

		case 0xD1: //SBC A,L
			SBC_F(Registers[REGISTER_L]);
			break;

		case 0xE1: //SBC A,H
			SBC_F(Registers[REGISTER_H]);
			break;

		case 0xF1: //SBC A,M
			address = (WORD)Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			SBC_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: CMP
		DESC: REGISTER COMPARED TO ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CMP
		case 0x92: //CMP A,B
			CMP_F(Registers[REGISTER_B]);
			break;

		case 0xA2: //CMP A,C
			CMP_F(Registers[REGISTER_C]);
			break;

		case 0xB2: //CMP A,D
			CMP_F(Registers[REGISTER_D]);
			break;

		case 0xC2: //CMP A,E
			CMP_F(Registers[REGISTER_E]);
			break;

		case 0xD2: //CMP A,L
			CMP_F(Registers[REGISTER_L]);
			break;

		case 0xE2: //CMP A,H
			CMP_F(Registers[REGISTER_H]);
			break;

		case 0xF2: //CMP A,M
			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			CMP_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: IOR
		DESC: Register bitwise inclusive or with Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region IOR
		case 0x93: //IOR A,B
			IOR_F(Registers[REGISTER_B]);
			break;

		case 0xA3: //IOR A,C
			IOR_F(Registers[REGISTER_C]);
			break;

		case 0xB3: //IOR A,D
			IOR_F(Registers[REGISTER_D]);
			break;

		case 0xC3: //IOR A,E
			IOR_F(Registers[REGISTER_E]);
			break;

		case 0xD3: //IOR A,L
			IOR_F(Registers[REGISTER_L]);
			break;

		case 0xE3: //IOR A,H
			IOR_F(Registers[REGISTER_H]);
			break;

		case 0xF3: //IOR A,M
			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			IOR_F(Registers[REGISTER_M]);
			break;

		#pragma endregion

		/*
		OPERATOR: AND
		DESC: Register bitwise and with Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region AND
		case 0x94: //AND A,B
			AND_F(Registers[REGISTER_B]);
			break;

		case 0xA4: //AND A,C
			AND_F(Registers[REGISTER_C]);
			break;

		case 0xB4: //AND A,D
			AND_F(Registers[REGISTER_D]);
			break;

		case 0xC4: //AND A,E
			AND_F(Registers[REGISTER_E]);
			break;

		case 0xD4: //AND A,L
			AND_F(Registers[REGISTER_L]);
			break;

		case 0xE4: //AND A,H
			AND_F(Registers[REGISTER_H]);
			break;

		case 0xF4: //AND A,M
			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			AND_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: XOR
		DESC REGISTER BITWISE EXCLUSIVE OR WITH ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region XOR
		case 0x95: //XOR A,B
			XOR_F(Registers[REGISTER_B]);
			break;

		case 0xA5: //XOR A,C
			XOR_F(Registers[REGISTER_C]);
			break;

		case 0xB5: //XOR A,D
			XOR_F(Registers[REGISTER_D]);
			break;

		case 0xC5: //XOR A,E
			XOR_F(Registers[REGISTER_E]);
			break;

		case 0xD5: //XOR A,L
			XOR_F(Registers[REGISTER_L]);
			break;

		case 0xE5: //XOR A,H
			XOR_F(Registers[REGISTER_H]);
			break;

		case 0xF5: //XOR A,M
			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			XOR_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: BT
		DESC: Register Bit tested with Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BT
		case 0x96: //BT A,B
			BT_F(Registers[REGISTER_B]);
			break;

		case 0xA6: //BT A,C
			BT_F(Registers[REGISTER_C]);
			break;

		case 0xB6: //BT A,D
			BT_F(Registers[REGISTER_D]);
			break;

		case 0xC6: //BT A,E
			BT_F(Registers[REGISTER_E]);
			break;

		case 0xD6: //BT A,L
			BT_F(Registers[REGISTER_L]);
			break;

		case 0xE6: //BT A,H
			BT_F(Registers[REGISTER_H]);
			break;

		case 0xF6: //BT A,M
			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			BT_F(Registers[REGISTER_M]);
			break;
		#pragma endregion

		/*
		OPERATOR: ADI
		DESC: DATA added to accumulator with carry
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ADI
		case 0x25: //ADI #
			data = fetch();
			param1 = Registers[REGISTER_A];
			param2 = data;
			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0) {
				temp_word++;
			}
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			set_flag_v(param1, param2, temp_word);
			Registers[REGISTER_A] = temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: SBI
		DESC: DATA SUBTRACTED TO ACCUMULATOR WITH CARRY
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SBI
		case 0x26: //SBI #
			data = fetch();
			param1 = Registers[REGISTER_A];
			param2 = data;
			temp_word = (WORD)param1 - (WORD)param2;
			if ((Flags & FLAG_C) != 0) {
				temp_word--;
			}
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			set_flag_v(param1, -param2, temp_word);
			Registers[REGISTER_A] = temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: CPI
		DESC: DATA COMPARED TO ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T T T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CPI
		case 0x27: //CPI #
			data = fetch();
			param1 = Registers[REGISTER_A];
			temp_word = (WORD)param1 - (WORD)data;
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			set_flag_v(param1, -data, temp_word);
			break;
		#pragma endregion

		/*
		OPERATOR: ANI
		DESC: DATA bitwise and with accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ANI
		case 0x28: //ANI #
			data = fetch();
			temp_word = data & (WORD)Registers[REGISTER_A];

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: XRI
		DESC: DATA bitwise xor with accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region XRI
		case 0x29: //XRI #
			data = fetch();
			temp_word = data ^ (WORD)Registers[REGISTER_A];

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: TST
		DESC: Bit test memory or accummulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region TST
		case 0x43: //TST abs
			address = getAddressAbs();
			temp_word = Memory[address] & Registers[REGISTER_A];

			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;

		case 0x53://TST Abs X
			address = getAddressAbsX();
			temp_word = Memory[address] & Registers[REGISTER_A];

			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;
		#pragma endregion

		/*
		OPERATOR: TSTA
		DESC: Bit test memory or accummulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region TSTA
		case 0x63: //TSTA A
			temp_word = Registers[REGISTER_A] & Memory[REGISTER_A];

			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;
		#pragma endregion

		/*
		OPERATOR: INC
		DESC: Increment memory or accumulator;
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region INC
		case 0x44: //INC Abs
			address = getAddressAbs();

			Memory[address]++;
			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);
			break;

		case 0x54: //INC Abs X
			address = getAddressAbsX();

			Memory[address]++;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;
		#pragma endregion

		/*
		OPERATOR: INCA
		DESC: INCREMENT THE MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region INCA
		case 0x64: // INCA A
			Registers[REGISTER_A]++;
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: DEC
		DESC: DECREMENT MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region DEC
		case 0x45: //DEC abs
			address = getAddressAbs();

			Memory[address]--;
			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);
			break;

		case 0x55: //DEC abs X
			address = getAddressAbsX();
			Memory[address]--;
			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);
			break;
		#pragma endregion

		/*
		OPERATOR: DECA
		DESC: DECREMENT MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region DECA
		case 0x65: //DECA A
			Registers[REGISTER_A]--;
			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: RCR
		DESC: ROTATE RIGHT THROUGH CARRY MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RCR
		case 0x46: // RCR abs
			address = getAddressAbs();
			saved_flags = Flags;
			if ((Memory[address] & 0x01) != 0) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Memory[address] = (Memory[address] >> 1);
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x80;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		case 0x56: //RCR ABS, X
			address = getAddressAbsX();
			saved_flags = Flags;
			if ((Memory[address] & 0x01) != 0) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Memory[address] = (Memory[address] >> 1);
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x80;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;
			
		#pragma endregion

		/*
		OPERATOR: RCRA
		DESC: ROTATE RIGHT THROUGH CARRY MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RCRA
		case 0x66: //RCRA A
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x01) != 0) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1);
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
			}

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: RCL
		DESC: ROTATE LEFT THROUGH CARRY MEMORY OR ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RCL
		case 0x47: //RCL abs
			address = getAddressAbs();
			saved_flags = Flags;
			if ((Memory[address] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Memory[address] = (Memory[address] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x01;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		case 0x57: //RCL abs X
			address = getAddressAbsX();
			saved_flags = Flags;
			if ((Memory[address] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Memory[address] = (Memory[address] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x01;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		#pragma endregion

		/*
		OPERATOR: RCLA
		DESC: Rotate left through carry Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RCLA
		case 0x67: //RCLA
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
			}

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: SAL
		DESC: Arithmetic shift left Memory
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		*/
		#pragma region SAL
		case 0x48: //sal abs
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				temp_word = Memory[address] << 1;
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					Flags = Flags & (0xFF - FLAG_C);
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;

		case 0x58: //SAL abs x
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				temp_word = Memory[address] << 1;
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					Flags = Flags & (0xFF - FLAG_C);
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;
		#pragma endregion

		/*
		OPERATOR: SALA
		DESC: Arithmetic shift left Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SALA
		case 0x68: //SALA A
			if ((Registers[REGISTER_A] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: ASR
		DESC: ARITHMETIC SHIFT RIGHT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ASR
		case 0x49: //ASR Abs
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE) {
				if ((Memory[address] & 0x80) == 0x80) {
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
						temp_word = (temp_word | 0x80);
					}
					else {
						temp_word = Memory[address] >> 1;
						temp_word = (temp_word | 0x80);
					}
				}
				else {
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
					}
					else {
						temp_word = Memory[address] >> 1;
					}

				}
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					Flags = Flags & (0xFF - FLAG_C);
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;

		case 0x59: //ASR Abs X
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE) {
				if ((Memory[address] & 0x80) == 0x80) {
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
						temp_word = (temp_word | 0x80);
					}
					else {
						temp_word = Memory[address] >> 1;
						temp_word = (temp_word | 0x80);
					}
				}
				else {
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
					}
					else {
						temp_word = Memory[address] >> 1;
					}

				}
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					Flags = Flags & (0xFF - FLAG_C);
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;
		#pragma endregion

		/*
		OPERATOR: ASRA
		DESC: Arithmetic shift Right Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ASRA
		case 0x69: //ASRA A
			if ((Registers[REGISTER_A] & 0x01) == 0x01) {
				Flags = Flags | FLAG_C;
			}
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;
			if ((Flags & FLAG_N) == FLAG_N) {
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
			}

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		#pragma endregion

		/*
		OPERATOR: NOT
		DESC: NEGATE MEMORY OR ACUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region NOT
		case 0x4A: //NOT abs
			address = getAddressAbs();
			temp_word = ~(Memory[address]);
			set_flag_n(temp_word);
			set_flag_z(temp_word);

			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}

			Memory[address] = (BYTE)temp_word;
			break;

		case 0x5A: // NOT abs x
			address = getAddressAbsX();
			temp_word = ~(Memory[address]);
			set_flag_n(temp_word);
			set_flag_z(temp_word);

			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}

			Memory[address] = (BYTE)temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: NOTA
		DESC: NEGATE MEMORY OR ACUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - T
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region NOTA
		case 0x6A: //NOTA A
			temp_word = ~(Registers[REGISTER_A]);
			set_flag_n(temp_word);
			set_flag_z(temp_word);

			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
			}
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		#pragma endregion

		/*
		OPERATOR: ROL
		DESC: Rotate left without carry Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ROL
		case 0x4B: //ROL abs
			address = getAddressAbs();
			temp_word = (Memory[address] << 1);

			if ((Memory[address] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		case 0x5B: //ROL abs X
			address = getAddressAbsX();
			temp_word = (Memory[address] << 1);

			if ((Memory[address] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;
		#pragma endregion

		/*
		OPERATOR: ROLA
		DESC: Rotate left without carry Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ROLA
		case 0x6B: //ROLA A
			temp_word = (Registers[REGISTER_A] << 1);

			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Registers[REGISTER_A] = temp_word;
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;
		#pragma endregion

		/*
		OPERATOR: ROR
		DESC: Rotate right without carry Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region ROR
		case 0x4C: //ROR abs
			address = getAddressAbs();
			temp_word = (Memory[address] >> 1);

			if ((Memory[address] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		case 0x5C: //ROR abs X
			address = getAddressAbsX();
			temp_word = (Memory[address] >> 1);

			if ((Memory[address] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;
		#pragma endregion

		/*
		OPERATOR: RORA
		DESC: Rotate right without carry Memory or Accumulator
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RORA
		case 0x6C: //RORA A
			temp_word = (Registers[REGISTER_A] >> 1);

			if ((Registers[REGISTER_A] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Registers[REGISTER_A] = temp_word;
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;		
		#pragma endregion

		/*
		OPERATOR: LDX
		DESC: LOADS MEMORY INTO REGISTER X
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region LDX
		case 0x4E: //LDX #
			data = fetch();
			IndexRegister = (WORD)data;
			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x5E: //LDX abs
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE) {
				IndexRegister = Memory[address];
			}
			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x6E: // LDX abs X
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE) {
				IndexRegister = Memory[address];
			}
			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: STOX
		DESC: STORES REGISTER X INTO MEMORY
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region STOX
		case 0x50: //STOX abs
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE) {
				Memory[address] = IndexRegister;
			}

			set_flag_n(Memory[address]); // set flags
			set_flag_z(Memory[address]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x60: //STOX abs X
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE) {
				Memory[address] = IndexRegister;
			}

			set_flag_n(Memory[address]); // set flags
			set_flag_z(Memory[address]);
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: DEX
		DESC: DECREMENT REGISTER X
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region DEX
		case 0x1B: //DEX impl
			--IndexRegister;
			set_flag_z(IndexRegister);
			break;
		#pragma endregion

		/*
		OPERATOR: INX
		DESC: INCREMENT REGISTER X
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - T - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region INX
		case 0x1C: //INX impl
			++IndexRegister;
			set_flag_z(IndexRegister);
			break;
		#pragma endregion

		/*
		OPERATOR: LODS
		DESC: LOADS MEMORY INTO STACKPOINTER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region LODS
		case 0x4F: // LODS #
			data = fetch();
			StackPointer = data << 8;
			StackPointer += fetch();
			set_flag_n(Memory[data]);
			set_flag_z(Memory[data]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x5F: // LODS abs
			address = getAddressAbs();
			if (address >= 0 && address < MEMORY_SIZE) {
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x6F: // LODS Abs,X
			address = getAddressAbsX();
			if (address >= 0 && address < MEMORY_SIZE) {
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: TAS
		DESC: TRANSFER ACCUMULATOR TO STATUS REGISER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region TAS
		case 0x74: //TAS impl
			Flags = Registers[REGISTER_A];
			break;
		#pragma endregion

		/*
		OPERATOR: TSA
		DESC: TRANSFERS STATUS INTO ACCUMULATOR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region TSA
		case 0x75: //TSA implied
			Registers[REGISTER_A] = Flags;
			break;
		#pragma endregion

		/*
		Operator: PSH
		DESC: Pushes value onto stack.
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region PSH
		case 0x11: //PSH A
			push(Registers[REGISTER_A]);
			break;

		case 0x21: //PSH FL
			push(Flags);
			break;

		case 0x31: //PSH B
			push(Registers[REGISTER_B]);
			break;

		case 0x41: //PSH C
			push(Registers[REGISTER_C]);
			break;

		case 0x51: //PSH D
			push(Registers[REGISTER_D]);
			break;

		case 0x61: //PSH E
			push(Registers[REGISTER_E]);
			break;

		case 0x71: //PSH L
			push(Registers[REGISTER_L]);
			break;

		case 0x81: //PSH H
			push(Registers[REGISTER_H]);
			break;
		#pragma endregion

		/*
		OPERATOR: POP
		DESC: Removes an item from top of stack.
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region POP
		case 0x12: //POP A
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}
			break;

		case 0x22: //POP FL
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Flags = Memory[StackPointer];
			}
			break;

		case 0x32: //POP B
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_B] = Memory[StackPointer];
			}
			break;

		case 0x42: //POP C
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_C] = Memory[StackPointer];
			}
			break;

		case 0x52: //POP D
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_D] = Memory[StackPointer];
			}
			break;

		case 0x62: //POP E
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];
			}
			break;

		case 0x72: //POP L
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_L] = Memory[StackPointer];
			}
			break;

		case 0x82: //POP H
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) {
				StackPointer++;
				Registers[REGISTER_H] = Memory[StackPointer];
			}
			break;
		#pragma endregion

		/*
		OPERATOR: JMP
		DESC: LOADS MEMORY INTO PROGRAM COUNTER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region JMP
		case 0xFA: //JMP abs
			address = getAddressAbs();
			ProgramCounter = address;
			break;
		#pragma endregion

		/*
		OPERATOR: MVI
		DESC: LOADS MEMORY INTO REGISTER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - T - T - - 0
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region MVI
		case 0x2A: // MVI B, #
			data = fetch();
			Registers[REGISTER_B] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x2B: // MVI C, #
			data = fetch();
			Registers[REGISTER_C] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x2C: // MVI D, #
			data = fetch();
			Registers[REGISTER_D] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x2D: // MVI E, #
			data = fetch();
			Registers[REGISTER_E] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x2E: // MVI L, #
			data = fetch();
			Registers[REGISTER_L] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;

		case 0x2F: // MVI H, #
			data = fetch();
			Registers[REGISTER_H] = data;
			set_flag_n(data);
			set_flag_z(data);
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: JPR
		DESC: JUMP TO SUBROUTINE
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region JPR
		case 0x33: //JPR abs
			address = getAddressAbs();
			if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE)) {
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				StackPointer--;
			}
			ProgramCounter = address;
			break;
		#pragma endregion

		/*
		OPERATOR: RTN
		DESC: RETURN FROM SUBROUTINE
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RTN
		case 0x0E: //RTN impl
			StackPointer++;
			address = Memory[StackPointer];
			ProgramCounter = address;
			break;
		#pragma endregion

		/*
		OPERATOR: BRA
		DESC: BRANCH ALWAYS
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BRA
		case 0x00: //BRA rel
			LB = fetch();
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) { // need to sign extend
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
			break;
		#pragma endregion

		/*
		OPERATOR: BCC
		DESC: BRANCH ON CARRY CLEAR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BCC
		case 0x01: //BCC rel
			LB = fetch();

			if (CF == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BCS
		DESC: BRANCH ON CARRY SET
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BCS
		case 0x02: //BCS rel
			LB = fetch();
			if (CF != 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BNE
		DESC: BRANCH ON RESULT NOT ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BNE
		case 0x03: //BNE rel
			LB = fetch();
			if (ZF == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BEQ
		DESC: BRANCH ON RESULT EQUAL TO ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BEQ
		case 0x04: //BEQ rel
			LB = fetch();
			if (ZF != 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BVC
		DESC: BRANCH ON OVERFLOW CLEAR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BVC
		case 0x05: //BVC rel
			LB = fetch();
			if (VF == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BVS
		DESC: BRANCH ON OVERFLOW SET
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BVS
		case 0x06: //BVS rel
			LB = fetch();
			if (VF != 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BMI
		DESC: BRANCH ON NEGATIVE RESULT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BMI
		case 0x07: //BMI rel
			LB = fetch();
			if (NF != 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BPL
		DESC: BRANCH ON POSITIVE RESULT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BPL
		case 0x08: //BPL rel
			LB = fetch();
			if (NF == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BGE
		DESC: BRANCH ON RESULT LESS THAN OR EQUAL TO ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BGE
		case 0x09: //BGE rel
			LB = fetch();
			if ((NF ^ VF) == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BLE
		DESC: BRANCH ON RESULT GREATER THAN OR EQUAL TO ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BLE
		case 0x0A: //BLE rel
			if ((ZF | (NF ^ VF)) == 1) {
				LB = fetch();
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: BLS
		DESC: BRANCH ON RESULT SAME OR LOWER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BLS
		case 0x0B: //BLS rel
			LB = fetch();
			if ((CF | ZF) != 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion 

		/*
		OPERATOR: BHI
		DESC: BRANCH ON RESULT HIGHER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region BHI
		case 0x0C: //BHI rel
			LB = fetch();
			if ((CF | ZF) == 0) {
				offset = (WORD)LB;
				if ((offset & 0x80) != 0) { // need to sign extend
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;

				ProgramCounter = address;
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CCC
		DESC: CALL ON CARRY CLEAR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CCC
		case 0x34: // CCC abs
			address = getAddressAbs();
			if (CF == 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CCS
		DESC: CALL ON CARRY SET
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CCS
		case 0x35: // CCS abs
			address = getAddressAbs();
			if (CF != 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CNE
		DESC: CALL ON RESULT NOT ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CNE
		case 0x36: // CNE abs
			address = getAddressAbs();
			if (ZF == 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CEQ
		DESC: CALL ON RESULT ZERO
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CEQ
		case 0x37: // CEQ abs
			address = getAddressAbs();
			if (ZF != 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CVC
		DESC: CALL ON OVERFLOW CLEAR
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CVC
		case 0x38: // CVC abs
			address = getAddressAbs();
			if (VF == 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CVS
		DESC: CALL ON OVERFLOW SET
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CVS
		case 0x39: // CVS abs
			address = getAddressAbs();
			if (VF != 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CMI
		DESC: CALL ON NEGATIVE RESULT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CMI
		case 0x3A: // CMI abs
			address = getAddressAbs();
			if (NF != 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CPL
		DESC: CALL ON POSITIVE RESULT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CPL
		case 0x3B: //CPL abs
			address = getAddressAbs();
			if (NF == 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CHI
		DESC: CALL ON RESULT SAME OR LOWER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CHI
		case 0x3C: //CHI abs
			address = getAddressAbs();
			if ((CF | ZF) != 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CLE
		DESC: CALL ON RESULT HIGHER
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CLE
		case 0x3D: //CLE abs
			address = getAddressAbs();
			if ((CF | ZF) == 0) {
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}
			break;
		#pragma endregion

		/*
		OPERATOR: CLC
		DESC: CLEAR CARRY FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - 0
		RETURNS: NOTHING
		WARNINGS: NOT GAINING POINTS - MAY BE BROKEN
		*/
		#pragma region CLC
		case 0x15: //CLC impl
			Flags = Flags & (0xFF - FLAG_C);
			break;
		#pragma endregion

		/*
		OPERATOR: SEC
		DESC: SET CARRY FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - 1
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SEC
		case 0x16: //SEC impl
			Flags = Flags | FLAG_C;
			break;
		#pragma endregion

		/*
		OPERATOR: CLI
		DESC: CLEAR INTERUPT FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  0 - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NOT GAINING POINTS - MAY BE BROKEN
		*/
		#pragma region CLI
		case 0x17: //CLI impl
			Flags = Flags & (0xFF - FLAG_I);
			break;
		#pragma endregion


		/*
		OPERATOR: STI
		DESC: SET INTERUPT FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  1 - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region STI
		case 0x18: //STI impl
			Flags = Flags | FLAG_I;
			break;
		#pragma endregion

		/*
		OPERATOR: SEV
		DESC: SET OVERFLOW FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - 1 - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SEV
		case 0x19: //SEV impl
			Flags = Flags | FLAG_V;
			break;
		#pragma endregion

		/*
		OPERATOR: CLV
		DESC: CLEAR OVERFLOW FLAG
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - 0 - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region CLV
		case 0x1A: //CLV impl
			Flags = Flags & (0xFF - FLAG_V);
			break;
		#pragma endregion

		/*
		OPERATOR: NOP
		DESC: NO OPERATION
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region NOP
		case 0x1D: //NOP impl
			break;
		#pragma endregion

		/*
		OPERATOR: WAI
		DESC: Wait for interupt
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region WAI
		case 0x1E: //WAI impl
			halt = true;
			break;
		#pragma endregion

		/*
		OPERATOR: SWI
		DESC: SOFTWARE INTERUPT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  1 - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region SWI
		case 0x85: //SWI Impl
			push(Registers[REGISTER_A]);
			push(ProgramCounter);
			push(Flags);
			push(Registers[REGISTER_H]);
			push(Registers[REGISTER_L]);
			push(Registers[REGISTER_E]);
			push(Registers[REGISTER_D]);
			push(Registers[REGISTER_C]);
			push(Registers[REGISTER_B]);

			address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
			Registers[REGISTER_M] = Memory[address];
			push(Registers[REGISTER_M]);

			Flags = Flags | FLAG_I;
			break;
		#pragma endregion

		/*
		OPERATOR: RTI
		DESC: RETURN FROM SOFTWARE INTERUPT
		PARAMS: NONE
		FLAGS:		I - N V Z - - C
		SET FLAGS:  - - - - - - - -
		RETURNS: NOTHING
		WARNINGS: NONE
		*/
		#pragma region RTI
		case 0x86: //RTI impl
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				address = Registers[REGISTER_H] << 8 + Registers[REGISTER_L];
				Registers[REGISTER_M] = Memory[address];
				StackPointer++;
				Registers[REGISTER_M] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_B] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_C] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_D] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_L] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_H] = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Flags = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				ProgramCounter = Memory[StackPointer];
			}
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}
			break;
		#pragma endregion	
	}
}

/*
OPERATOR: MOVE
DESC: TRANSFER FROM ONE REGISTER TO ANOTHER
PARAMS: BYTE opcode
FLAGS:		I - N V Z - - C
SET FLAGS:  - - - - - - - -
RETURNS: NOTHING
WARNINGS: NONE
*/
#pragma region MOVE
void Group_2_Move(BYTE opcode)
{
	int destination =0;
	int source = 0;

	int destReg = 0;
	int sourceReg = 0;

	WORD address;

	destination = opcode & 0x0F;

	switch (destination)
	{
	case 0x08:
		destReg = REGISTER_A;
			break;

	case 0x09:
		destReg = REGISTER_B;
			break;

	case 0x0A:
		destReg = REGISTER_C;
			break;

	case 0x0B:
		destReg = REGISTER_D;
			break;

	case 0x0C:
		destReg = REGISTER_E;
			break;

	case 0x0D:
		destReg = REGISTER_L;
			break;

	case 0x0E:
		destReg = REGISTER_H;
			break;

	case 0x0F:
		destReg = REGISTER_M;
			break;
	}

	source = (opcode >> 4) & 0x0F;

	switch (source)
	{
	case 0x07:
		sourceReg = REGISTER_A;
		break;

	case 0x08:
		sourceReg = REGISTER_B;
		break;

	case 0x09:
		sourceReg = REGISTER_C;
		break;

	case 0x0A:
		sourceReg = REGISTER_D;
		break;

	case 0x0B:
		sourceReg = REGISTER_E;
		break;

	case 0x0C:
		sourceReg = REGISTER_L;
		break;

	case 0x0D:
		sourceReg = REGISTER_H;
		break;

	case 0x0E:
		sourceReg = REGISTER_M;
		break;
	}

	if (sourceReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Registers[REGISTER_M] = Memory[address];
	}

	Registers[destReg] = Registers[sourceReg];

	if (destReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Memory[address] = Registers[REGISTER_M];
	}
}
#pragma endregion 

void execute(BYTE opcode)
{
	if (((opcode >= 0x78) && (opcode <= 0x7F))
		|| ((opcode >= 0x88) && (opcode <= 0x8F))
		|| ((opcode >= 0x98) && (opcode <= 0x9F))
		|| ((opcode >= 0xA8) && (opcode <= 0xAF))
		|| ((opcode >= 0xB8) && (opcode <= 0xBF))
		|| ((opcode >= 0xC8) && (opcode <= 0xCF))
		|| ((opcode >= 0xD8) && (opcode <= 0xDF))
		|| ((opcode >= 0xE8) && (opcode <= 0xEF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity = 0;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	//printf("                    A  B  C  D  E  L  H  X    SP\n");

	while ((!halt) && (memory_in_range) && (sanity < 500)) {
		//printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		//printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		//printf("%02X ", Registers[REGISTER_A]);
		//printf("%02X ", Registers[REGISTER_B]);
		//printf("%02X ", Registers[REGISTER_C]);
		//printf("%02X ", Registers[REGISTER_D]);
		//printf("%02X ", Registers[REGISTER_E]);
		//printf("%02X ", Registers[REGISTER_L]);
		//printf("%02X ", Registers[REGISTER_H]);
		//printf("%04X ", IndexRegister);
		//printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)	
		{
			//printf("I=1 ");
		}
		else
		{
			//printf("I=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)	
		{
			//printf("N=1 ");
		}
		else
		{
			//printf("N=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)	
		{
			//printf("V=1 ");
		}
		else
		{
			//printf("V=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)	
		{
			//printf("Z=1 ");
		}
		else
		{
			//printf("Z=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)	
		{
			//printf("C=1 ");
		}
		else
		{
			//printf("C=0 ");
		}

		//printf("\n");  // New line
		sanity++;
	}

	//printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args,_TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv){
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", 
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4], 
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6], 
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8], 
		Memory[TEST_ADDRESS_9], 
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", 
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4], 
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6], 
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8], 
						Memory[TEST_ADDRESS_9], 
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {	
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}


