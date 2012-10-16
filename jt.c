#include <windows.h>
#include <intrin.h>
#pragma intrinsic(__rdtsc)
typedef unsigned __int64 ticks;
#define getticks __rdtsc

#include <imagehlp.h>

#pragma comment(lib, "imagehlp")

#include "jt.h"

#define PRINT_ERROR(f, file, line) \
  printf("%s: error %u at %s:%d\n", f, GetLastError(), file, line);
  
#define CURRENT_BYTE (*((PUINT8) g_va))  

#define COUNT 10000 //10 000
#define BYTE 256
#define STARTLINE 16

//global variables
PVOID g_va;
UINT8 b;
void (*jumpTable[BYTE])(INSTRUCTION*);
void (*modRM[BYTE])(INSTRUCTION*);

void getByte() {
	b = CURRENT_BYTE;
	++((PUINT8) g_va);
}

void setLockPrefix(INSTRUCTION *instr) {
	instr->prefixLock = TRUE;
	instr->prefixLockCount++;
	getByte();
	((*jumpTable[b])(instr));
}

void setAddressSizePrefix(INSTRUCTION *instr) {
	instr->prefixAddress = TRUE;
	instr->prefixAddressCount++;
	getByte();
	((*jumpTable[b])(instr));
}

void setOpcode(INSTRUCTION *instr) {
	instr->opcode = b;
	instr->modRM = TRUE;
	
	instr->valid = TRUE;
	getByte();
	((*modRM[b])(instr));
}

void setModRM(INSTRUCTION *instr) {
	instr->modRMValue = b;
}

void setModRMwithSIB(INSTRUCTION *instr) {
	instr->modRMValue = b;
	//один if все-таки нужен, в 16-битной версии инструкций байта sib не бывает
	if(!instr->prefixAddress) {
		instr->SIB = TRUE;
		getByte();
		instr->SIBValue = b;
	}
}

void setInvalidInstruction(INSTRUCTION *instr) {
	instr->valid = FALSE;
}

void getInstruction(INSTRUCTION *instr) {
	getByte();
	//printf("%0x \n", b);
	//getch();
	((*jumpTable[b])(instr));
}

void initialize(PVOID va) {
	int i;
	g_va = va;
	for(i = 0; i < BYTE; ++i) {
		jumpTable[i] = setInvalidInstruction;
		modRM[i] = setModRM;
	}
	jumpTable[0xf0] = setLockPrefix;
	jumpTable[0x67] = setAddressSizePrefix;
	jumpTable[0x00] = setOpcode;
	jumpTable[0x20] = setOpcode;
	jumpTable[0x08] = setOpcode;
	jumpTable[0xf6] = setOpcode;
	
	modRM[0x04] = setModRMwithSIB;
	modRM[0x14] = setModRMwithSIB;
	modRM[0x24] = setModRMwithSIB;
	modRM[0x34] = setModRMwithSIB;
	modRM[0x44] = setModRMwithSIB;
	modRM[0x54] = setModRMwithSIB;
	modRM[0x64] = setModRMwithSIB;
	modRM[0x74] = setModRMwithSIB;
	modRM[0x84] = setModRMwithSIB;
	modRM[0x94] = setModRMwithSIB;
	modRM[0xA4] = setModRMwithSIB;
	modRM[0xB4] = setModRMwithSIB;
	modRM[0xC4] = setModRMwithSIB;
	modRM[0xD4] = setModRMwithSIB;
	modRM[0xE4] = setModRMwithSIB;
	modRM[0xF4] = setModRMwithSIB;
	
	modRM[0x0C] = setModRMwithSIB;
	modRM[0x1C] = setModRMwithSIB;
	modRM[0x2C] = setModRMwithSIB;
	modRM[0x3C] = setModRMwithSIB;
	modRM[0x4C] = setModRMwithSIB;
	modRM[0x5C] = setModRMwithSIB;
	modRM[0x6C] = setModRMwithSIB;
	modRM[0x7C] = setModRMwithSIB;
	modRM[0x8C] = setModRMwithSIB;
	modRM[0x9C] = setModRMwithSIB;
	modRM[0xAC] = setModRMwithSIB;
	modRM[0xBC] = setModRMwithSIB;
	modRM[0xCC] = setModRMwithSIB;
	modRM[0xDC] = setModRMwithSIB;
	modRM[0xEC] = setModRMwithSIB;
	modRM[0xFC] = setModRMwithSIB;
}

void print(INSTRUCTION *instr, BOOL qwer) {
	if(instr->valid) {
		if(instr->prefixLock)
			printf("lock ");
		if(instr->prefixAddress)
			printf("adress size ");
		if(qwer) {
			switch(instr->opcode) {
				case 0x00: printf("add ");
					break;
				case 0x20: printf("and ");
					break;
				case 0x08: printf("or ");
					break;
				case 0xf6: printf("not ");
					break;
			}		
		}
		else {
			printf("%0x %0x", instr->opcode, instr->modRMValue);
		}
			printf("\n");
	}
	else
		printf("Unknown instruction. \n");
}


VOID main(INT argc, PSTR argv[])
{
	INT resInstr[COUNT];
	INSTRUCTION instr[COUNT];
	
	LOADED_IMAGE image;
	PSTR imageFilename;
	INT i;
	INT ii;
	PVOID va;
	unsigned __int64 tickCount;
	
	imageFilename = argv[1];
	
	if (!MapAndLoad(imageFilename, NULL, &image, FALSE, TRUE)) {
		PRINT_ERROR("MapAndLoad", __FILE__, __LINE__);
		return;
	}
	va = ImageRvaToVa(image.FileHeader, image.MappedAddress,
						  image.FileHeader->OptionalHeader.BaseOfCode, NULL);
	
	for(ii = 0; ii < COUNT; ++ii) {
		initialize(va);
		//прогон до нужной инструкции
		//for(i = 0; i < STARTLINE; ++i)
		//	getByte();
		_asm{
			lfence
		}
		tickCount = getticks();
		for(i = 0; i < COUNT; ++i) {
			getInstruction(&instr[i]);
			//print(&instr[i], FALSE);
		}
		
		tickCount = (getticks() - tickCount);
		resInstr[ii] = tickCount;
	}
	
	for(ii = 0; ii < COUNT; ++ii) {
		printf("%d \n\r", resInstr[ii]/10000);
	}
	
	/*
	for(i = 0; i < COUNT; ++i) {
		if(0 < resInstr[i]) {
			printf("%d : %d \n", i, resInstr[i]);
			break;
			}
	}
	*/
}