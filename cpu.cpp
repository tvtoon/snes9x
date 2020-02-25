/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "port.h"
#include "snes9x.h"
#include "memmap.h"
#include "bsx.h"
#include "c4.h"
#include "cpu.h"
#include "cpuexec.h"
#include "dma.h"
#include "dsp.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "getset.h"
#include "msu1.h"
#include "obc1.h"
#include "ppu.h"
#include "sa1.h"
#include "sdd1.h"
#include "spc7110.h"
#include "srtc.h"
#include "snapshot.h"
#include "cheats.h"
#include "logger.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

static void S9xSoftResetCPU (void)
{
	CPU.Cycles = 182; // Or 188. This is the cycle count just after the jump to the Reset Vector.
	CPU.PrevCycles = CPU.Cycles;
	CPU.V_Counter = 0;
	CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	CPU.PCBase = NULL;
	CPU.NMIPending = FALSE;
	CPU.IRQLine = FALSE;
	CPU.IRQTransition = FALSE;
	CPU.IRQExternal = FALSE;
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
	CPU.FastROMSpeed = SLOW_ONE_CYCLE;
	CPU.InDMA = FALSE;
	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.HDMARanInDMA = 0;
	CPU.CurrentDMAorHDMAChannel = -1;
	CPU.WhichEvent = HC_RENDER_EVENT;
	CPU.NextEvent  = Timings.RenderPos;
	CPU.WaitingForInterrupt = FALSE;
	CPU.AutoSaveTimer = 0;
	CPU.SRAMModified = FALSE;

	Registers.PC.xPBPC = 0;
	Registers.PC.B.xPB = 0;
	Registers.PC.W.xPC = S9xGetWord(0xfffc);
	OpenBus = Registers.PC.B.xPCh;
	Registers.D.W = 0;
	Registers.DB = 0;
	Registers.S.B.h = 1;
	Registers.S.B.l -= 3;
	Registers.X.B.h = 0;
	Registers.Y.B.h = 0;

	ICPU.ShiftedPB = 0;
	ICPU.ShiftedDB = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);

	Timings.InterlaceField = FALSE;
	Timings.H_Max = Timings.H_Max_Master;
	Timings.V_Max = Timings.V_Max_Master;
	Timings.NMITriggerPos = 0xffff;
	Timings.NextIRQTimer = 0x0fffffff;
	Timings.IRQFlagChanging = IRQ_NONE;

	if (Model->_5A22 == 2)
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2;
	else
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v1;

	S9xSetPCBase(Registers.PC.xPBPC);

	ICPU.S9xOpcodes = S9xOpcodesE1;
	ICPU.S9xOpLengths = S9xOpLengthsM1X1;

	S9xUnpackStatus();
}

static void S9xResetCPU (void)
{
	S9xSoftResetCPU();
	Registers.S.B.l = 0xff;
	Registers.P.W = 0;
	Registers.A.W = 0;
	Registers.X.W = 0;
	Registers.Y.W = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);
}

void S9xReset (void)
{
	S9xResetSaveTimer(FALSE);
	S9xResetLogger();

	memset(Memory.RAM, 0x55, 0x20000);
	memset(Memory.VRAM, 0x00, 0x10000);
	memset(Memory.FillRAM, 0, 0x8000);

	S9xResetBSX();
	S9xResetCPU();
	S9xResetPPU();
	S9xResetDMA();
	S9xResetAPU();
    S9xResetMSU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();
	if (Settings.MSU1)
		S9xMSU1Init();

	S9xInitCheatData();
}

void S9xSoftReset (void)
{
	S9xResetSaveTimer(FALSE);

	memset(Memory.FillRAM, 0, 0x8000);

	if (Settings.BS)
		S9xResetBSX();

	S9xSoftResetCPU();
	S9xSoftResetPPU();
	S9xResetDMA();
	S9xSoftResetAPU();
    S9xResetMSU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();
	if (Settings.MSU1)
		S9xMSU1Init();

	S9xInitCheatData();
}

/**/

void S9xUnpackStatus (void)
{
	ICPU._Zero = (Registers.P.B.l & Zero) == 0;
	ICPU._Negative = (Registers.P.B.l & Negative);
	ICPU._Carry = (Registers.P.B.l & Carry);
	ICPU._Overflow = (Registers.P.B.l & Overflow) >> 6;
}

void S9xPackStatus (void)
{
	Registers.P.B.l &= ~(Zero | Negative | Carry | Overflow);
	Registers.P.B.l |= ICPU._Carry | ((ICPU._Zero == 0) << 1) | (ICPU._Negative & 0x80) | (ICPU._Overflow << 6);
}

void S9xFixCycles (void)
{
	if (CheckEmulation())
	{
		ICPU.S9xOpcodes = S9xOpcodesE1;
		ICPU.S9xOpLengths = S9xOpLengthsM1X1;
	}
	else
	if (CheckMemory())
	{
		if (CheckIndex())
		{
			ICPU.S9xOpcodes = S9xOpcodesM1X1;
			ICPU.S9xOpLengths = S9xOpLengthsM1X1;
		}
		else
		{
			ICPU.S9xOpcodes = S9xOpcodesM1X0;
			ICPU.S9xOpLengths = S9xOpLengthsM1X0;
		}
	}
	else
	{
		if (CheckIndex())
		{
			ICPU.S9xOpcodes = S9xOpcodesM0X1;
			ICPU.S9xOpLengths = S9xOpLengthsM0X1;
		}
		else
		{
			ICPU.S9xOpcodes = S9xOpcodesM0X0;
			ICPU.S9xOpLengths = S9xOpLengthsM0X0;
		}
	}
}
