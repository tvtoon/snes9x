/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _65C816_H_
#define _65C816_H_

#define Carry		1
#define Zero		2
#define IRQ			4
#define Decimal		8
#define IndexFlag	16
#define MemoryFlag	32
#define Overflow	64
#define Negative	128
#define Emulation	256

#define SetCarry()			(ICPU._Carry = 1)
#define ClearCarry()		(ICPU._Carry = 0)
#define SetZero()			(ICPU._Zero = 0)
#define ClearZero()			(ICPU._Zero = 1)
#define SetIRQ()			(Registers.P.B.l |= IRQ)
#define ClearIRQ()			(Registers.P.B.l &= ~IRQ)
#define SetDecimal()		(Registers.P.B.l |= Decimal)
#define ClearDecimal()		(Registers.P.B.l &= ~Decimal)
#define SetIndex()			(Registers.P.B.l |= IndexFlag)
#define ClearIndex()		(Registers.P.B.l &= ~IndexFlag)
#define SetMemory()			(Registers.P.B.l |= MemoryFlag)
#define ClearMemory()		(Registers.P.B.l &= ~MemoryFlag)
#define SetOverflow()		(ICPU._Overflow = 1)
#define ClearOverflow()		(ICPU._Overflow = 0)
#define SetNegative()		(ICPU._Negative = 0x80)
#define ClearNegative()		(ICPU._Negative = 0)

#define CheckCarry()		(ICPU._Carry)
#define CheckZero()			(ICPU._Zero == 0)
#define CheckIRQ()			(Registers.P.B.l & IRQ)
#define CheckDecimal()		(Registers.P.B.l & Decimal)
#define CheckIndex()		(Registers.P.B.l & IndexFlag)
#define CheckMemory()		(Registers.P.B.l & MemoryFlag)
#define CheckOverflow()		(ICPU._Overflow)
#define CheckNegative()		(ICPU._Negative & 0x80)
#define CheckEmulation()	(Registers.P.W & Emulation)

#define SetFlags(f)			(Registers.P.W |= (f))
#define ClearFlags(f)		(Registers.P.W &= ~(f))
#define CheckFlag(f)		(Registers.P.B.l & (f))

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	l, h; } B;
#else
	struct { uint8	h, l; } B;
#endif
	uint16	W;
}	pair;

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	xPCl, xPCh, xPB, z; } B;
	struct { uint16	xPC, d; } W;
#else
	struct { uint8	z, xPB, xPCh, xPCl; } B;
	struct { uint16	d, xPC; } W;
#endif
    uint32	xPBPC;
}	PC_t;

struct SRegisters
{
	uint8	DB;
	pair	P;
	pair	A;
	pair	D;
	pair	S;
	pair	X;
	pair	Y;
	PC_t	PC;
};

extern struct SRegisters	Registers;

#endif
