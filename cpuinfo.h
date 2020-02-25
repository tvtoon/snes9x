/*
 Assert the directs, indirects etc.
 03 | 13 = StackRelative*.
 07 | 17 = DirectIndirect*.
 0F | 1F = Absolute* like 0D | 19 | 1D.
*/
cpumacroa[256] =
{
 0, ORA, 0  , ORA, TSB, ORA, ASL, ORA, 0  , 0  , 0  , 0  , TSB, ORA, ASL, ORA,
 0, ORA, ORA, ORA, TRB, ORA, ASL, ORA, 0  , ORA, 0  , 0  , TRB, ORA, ASL, ORA,
 0, AND, 0  , AND, BIT, AND, ROL, AND, 0  , 0  , 0  , 0  , BIT, AND, ROL, AND,
 0, AND, AND, AND, BIT, AND, ROL, AND, 0  , AND, 0  , 0  , BIT, AND, ROL, AND,
/* 0x40 */
 0, EOR, 0  , EOR, 0  , EOR, LSR, EOR, 0  , 0  , 0  , 0  , 0  , EOR, LSR, EOR,
 0, EOR, EOR, EOR, 0  , EOR, LSR, EOR, 0  , EOR, 0  , 0  , 0  , EOR, LSR, EOR,
 0, ADC, 0  , ADC, STZ, ADC, ROR, ADC, 0  , 0  , 0  , 0  , 0  , ADC, ROR, ADC,
 0, ADC, ADC, ADC, STZ, ADC, ROR, ADC, 0  , ADC, 0  , 0  , 0  , ADC, ROR, ADC,
/* 0x80 */
 0, STA, 0  , STA, STY, STA, STX, STA, 0  , 0  , 0  , 0  , STY, STA, STX, STA,
 0, STA, STA, STA, STY, STA, STX, STA, 0  , STA, 0  , 0  , STZ, STA, STZ, STA,
 0, LDA, 0  , LDA, LDY, LDA, LDX, LDA, 0  , 0  , 0  , 0  , LDY, LDA, LDX, LDA,
 0, LDA, LDA, LDA, LDY, LDA, LDX, LDA, 0  , LDA, 0  , 0  , LDY, LDA, LDX, LDA,
/* 0xC0 */
 0, CMP, 0  , CMP, CPY, CMP, DEC, CMP, 0  , 0  , 0  , 0  , CPY, CMP, DEC, CMP,
 0, CMP, CMP, CMP, 0  , CMP, DEC, CMP, 0  , CMP, 0  , 0  , 0  , CMP, DEC, CMP,
 0, SBC, 0  , SBC, CPX, SBC, INC, SBC, 0  , 0  , 0  , 0  , CPX, SBC, INC, SBC,
 0, SBC, SBC, SBC, 0  , SBC, INC, SBC, 0  , SBC, 0  , 0  , 0  , SBC, INC, SBC
};
/*
 Which ones are not only trough emulation flag or "Slow".
 I = Index, M = Memory.
*/
cpuaddra[256] =
{
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,0,M,M,0,0,I,0,0,0,I,I,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,M,M,M,0,0,I,0,0,I,I,I,0,
/* 0x40 */
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,0,M,M,0,0,I,0,0,0,I,I,0,
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,M,M,M,0,0,I,0,0,0,I,I,0,
/* 0x80 */
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,M,M,M,0,0,I,0,0,0,I,I,0,
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,M,M,M,0,0,I,0,0,0,I,0,0,
/* 0xC0 */
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,0,M,M,0,0,I,0,0,0,I,I,0,
 0,M,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,I,M,0,0,M,M,0,0,I,0,0,0,I,I,0
};

/*
 First, the indexes that are NOT available for arithmetic and logic operations.
 Second, the indexes that ARE available for CMP (C0, E0 only).
*/
NOALU = { 04, 06, 08, 0A, 0B, 0C, 0E, 10, 14, 16, 18, 1A, 1B, 1C, 1E };
SICPX = { 00, 04, 0C };
/* Now the direct ones. */
BIT = { 24, 2C, 34, 3C, 89 };
INC = { 1A, C8, E6, E8, EE, F6, FE };
DEC = { 3A, 88, C6, CA, CE, D6, DE };
TSB = { 04, 0C };
TRB = { 14, 1C };
ASL = { 06, 0A, 0E, 16, 1E };
LSR = { 46, 4A, 4E, 56, 5E };
ROL = { 26, 2A, 2E, 36, 3E };
ROR = { 66, 6A, 6E, 76, 7E };
JMPs = { 20, 22, 40, 4C, 5C, 60, 6B, 6C, 7C, 80, 82, DC, FC };
CJMPs = { 10, 30, 50, 70, 90, B0, D0, F0 };
IRQs = { 00, 02 };
CPUc = { 18, 38, 58, 78, B8, C2, D8, E2, F8, FB };
/* 42 and EA are NOP. */
SPECIAL = { 42, CB, DB, EA, EB };
/* Branch Instructions ***************************************************** */

// BCC
bOP(90E0,   Relative,     !CheckCarry(),    0, 0)
bOP(90E1,   Relative,     !CheckCarry(),    0, 1)
bOP(90Slow, RelativeSlow, !CheckCarry(),    0, CheckEmulation())

// BCS
bOP(B0E0,   Relative,      CheckCarry(),    0, 0)
bOP(B0E1,   Relative,      CheckCarry(),    0, 1)
bOP(B0Slow, RelativeSlow,  CheckCarry(),    0, CheckEmulation())

// BEQ
bOP(F0E0,   Relative,      CheckZero(),     2, 0)
bOP(F0E1,   Relative,      CheckZero(),     2, 1)
bOP(F0Slow, RelativeSlow,  CheckZero(),     2, CheckEmulation())

// BMI
bOP(30E0,   Relative,      CheckNegative(), 1, 0)
bOP(30E1,   Relative,      CheckNegative(), 1, 1)
bOP(30Slow, RelativeSlow,  CheckNegative(), 1, CheckEmulation())

// BNE
bOP(D0E0,   Relative,     !CheckZero(),     1, 0)
bOP(D0E1,   Relative,     !CheckZero(),     1, 1)
bOP(D0Slow, RelativeSlow, !CheckZero(),     1, CheckEmulation())

// BPL
bOP(10E0,   Relative,     !CheckNegative(), 1, 0)
bOP(10E1,   Relative,     !CheckNegative(), 1, 1)
bOP(10Slow, RelativeSlow, !CheckNegative(), 1, CheckEmulation())

// BRA
bOP(80E0,   Relative,     1,                X, 0)
bOP(80E1,   Relative,     1,                X, 1)
bOP(80Slow, RelativeSlow, 1,                X, CheckEmulation())

// BVC
bOP(50E0,   Relative,     !CheckOverflow(), 0, 0)
bOP(50E1,   Relative,     !CheckOverflow(), 0, 1)
bOP(50Slow, RelativeSlow, !CheckOverflow(), 0, CheckEmulation())

// BVS
bOP(70E0,   Relative,      CheckOverflow(), 0, 0)
bOP(70E1,   Relative,      CheckOverflow(), 0, 1)
bOP(70Slow, RelativeSlow,  CheckOverflow(), 0, CheckEmulation())

