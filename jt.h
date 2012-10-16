#pragma once

#define PREFIX_LOCK 0xf0
#define PREFIX_ADDRESS_SIZE 0x67

typedef struct {
	UINT8 opcode;
	
	BOOL prefixLock;
	UINT8 prefixLockCount;
	BOOL prefixAddress;
	UINT8 prefixAddressCount;
	
	//��� ���� ����� ����� ���� �������� ������, ����� �������� ���� ���� = 0x0
	//�� ������, ����� ���� ���� ��� � ����������, ������ bool ����������
	BOOL modRM;
	BOOL SIB;
	UINT8 modRMValue;
	UINT8 SIBValue;
	
	UINT displacement;
	UINT immediate;
	//���� ������������ ����������. ���� ��� �������� ��� �������, �� ���� ����� �����
	BOOL valid;
} INSTRUCTION, *PINSTRUCTION;