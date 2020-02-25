/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

void S9xOpcode_NMI (void);
void S9xOpcode_IRQ (void);
/*
#ifndef SA1_OPCODES
#define CHECK_FOR_IRQ() {} // if (CPU.IRQLine) S9xOpcode_IRQ(); }
#else
#define CHECK_FOR_IRQ()	{}
#endif
#endif
*/
