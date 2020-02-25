/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

uint8 S9xGetByte( const uint32 Address );
uint16 S9xGetWord ( const uint32 Address, const enum s9xwrap_t w = WRAP_NONE);
void S9xSetByte ( const uint8 Byte, const uint32 Address );
void S9xSetWord ( const uint16 Word, const uint32 Address, const enum s9xwrap_t w = WRAP_NONE, const enum s9xwriteorder_t o = WRITE_01);
void S9xSetPCBase ( const uint32 Address );
uint8 * S9xGetBasePointer ( const uint32 Address );
uint8 * S9xGetMemPointer ( const uint32 Address );
