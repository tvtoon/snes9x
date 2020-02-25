/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _PPU_H_
#define _PPU_H_

#define FIRST_VISIBLE_LINE	1

#define TILE_2BIT			0
#define TILE_4BIT			1
#define TILE_8BIT			2
#define TILE_2BIT_EVEN		3
#define TILE_2BIT_ODD		4
#define TILE_4BIT_EVEN		5
#define TILE_4BIT_ODD		6

#define MAX_2BIT_TILES		4096
#define MAX_4BIT_TILES		2048
#define MAX_8BIT_TILES		1024

#define CLIP_OR				0
#define CLIP_AND			1
#define CLIP_XOR			2
#define CLIP_XNOR			3

struct ClipData
{
	uint8	Count;
	uint8	DrawMode[6];
	uint16	Left[6];
	uint16	Right[6];
};

struct InternalPPU
{
	struct ClipData Clip[2][6];
	bool8	ColorsChanged;
	bool8	OBJChanged;
	uint8	*TileCache[7];
	uint8	*TileCached[7];
	bool8	Interlace;
	bool8	InterlaceOBJ;
	bool8	PseudoHires;
	bool8	DoubleWidthPixels;
	bool8	DoubleHeightPixels;
	int		CurrentLine;
	int		PreviousLine;
	uint8	*XB;
	uint32	Red[256];
	uint32	Green[256];
	uint32	Blue[256];
	uint16	ScreenColors[256];
	uint8	MaxBrightness;
	bool8	RenderThisFrame;
	int		RenderedScreenWidth;
	int		RenderedScreenHeight;
	uint32	FrameCount;
	uint32	RenderedFramesCount;
	uint32	DisplayedRenderedFrameCount;
	uint32	TotalEmulatedFrames;
	uint32	SkippedFrames;
	uint32	FrameSkip;
};

struct SOBJ
{
	int16	HPos;
	uint16	VPos;
	uint8	HFlip;
	uint8	VFlip;
	uint16	Name;
	uint8	Priority;
	uint8	Palette;
	uint8	Size;
};

struct SPPU
{
	struct
	{
		bool8	High;
		uint8	Increment;
		uint16	Address;
		uint16	Mask1;
		uint16	FullGraphicCount;
		uint16	Shift;
	}	VMA;

	uint32	WRAM;

	struct
	{
		uint16	SCBase;
		uint16	HOffset;
		uint16	VOffset;
		uint8	BGSize;
		uint16	NameBase;
		uint16	SCSize;
	}	BG[4];

	uint8	BGMode;
	uint8	BG3Priority;

	bool8	CGFLIP;
	uint8	CGFLIPRead;
	uint8	CGADD;
	uint8	CGSavedByte;
	uint16	CGDATA[256];

	struct SOBJ OBJ[128];
	bool8	OBJThroughMain;
	bool8	OBJThroughSub;
	bool8	OBJAddition;
	uint16	OBJNameBase;
	uint16	OBJNameSelect;
	uint8	OBJSizeSelect;

	uint16	OAMAddr;
	uint16	SavedOAMAddr;
	uint8	OAMPriorityRotation;
	uint8	OAMFlip;
	uint8	OAMReadFlip;
	uint16	OAMTileAddress;
	uint16	OAMWriteRegister;
	uint8	OAMData[512 + 32];

	uint8	FirstSprite;
	uint8	LastSprite;
	uint8	RangeTimeOver;

	bool8	HTimerEnabled;
	bool8	VTimerEnabled;
	short	HTimerPosition;
	short	VTimerPosition;
	uint16	IRQHBeamPos;
	uint16	IRQVBeamPos;

	uint8	HBeamFlip;
	uint8	VBeamFlip;
	uint16	HBeamPosLatched;
	uint16	VBeamPosLatched;
	uint16	GunHLatch;
	uint16	GunVLatch;
	uint8	HVBeamCounterLatched;

	bool8	Mode7HFlip;
	bool8	Mode7VFlip;
	uint8	Mode7Repeat;
	short	MatrixA;
	short	MatrixB;
	short	MatrixC;
	short	MatrixD;
	short	CentreX;
	short	CentreY;
	short	M7HOFS;
	short	M7VOFS;

	uint8	Mosaic;
	uint8	MosaicStart;
	bool8	BGMosaic[4];

	uint8	Window1Left;
	uint8	Window1Right;
	uint8	Window2Left;
	uint8	Window2Right;
	bool8	RecomputeClipWindows;
	uint8	ClipCounts[6];
	uint8	ClipWindowOverlapLogic[6];
	uint8	ClipWindow1Enable[6];
	uint8	ClipWindow2Enable[6];
	bool8	ClipWindow1Inside[6];
	bool8	ClipWindow2Inside[6];

	bool8	ForcedBlanking;

	uint8	FixedColourRed;
	uint8	FixedColourGreen;
	uint8	FixedColourBlue;
	uint8	Brightness;
	uint16	ScreenHeight;

	bool8	Need16x8Mulitply;
	uint8	BGnxOFSbyte;
	uint8	M7byte;

	uint8	HDMA;
	uint8	HDMAEnded;

	uint8	OpenBus1;
	uint8	OpenBus2;

	uint16	VRAMReadBuffer;
};

extern uint16				SignExtend[2];
extern struct SPPU			PPU;
extern struct InternalPPU	IPPU;

void S9xResetPPU (void);
void S9xResetPPUFast (void);
void S9xSoftResetPPU (void);
void S9xSetPPU (uint8, uint16);
uint8 S9xGetPPU (uint16);
void S9xSetCPU (uint8, uint16);
uint8 S9xGetCPU (uint16);
void S9xUpdateIRQPositions (bool initial);
void S9xFixColourBrightness (void);
void S9xDoAutoJoypad (void);

/*
#include "gfx.h"
#include "memmap.h"
*/

typedef struct
{
	uint8	_5C77;
	uint8	_5C78;
	uint8	_5A22;
}	SnesModel;

extern SnesModel	*Model;
extern SnesModel	M1SNES;
extern SnesModel	M2SNES;

#define MAX_5C77_VERSION	0x01
#define MAX_5C78_VERSION	0x03
#define MAX_5A22_VERSION	0x02

void S9xUpdateScreen (void);

/**/

void FLUSH_REDRAW(void);
void REGISTER_2104 ( const uint8 Byte );
void REGISTER_2118 ( const uint8 Byte );
void REGISTER_2118_tile ( const uint8 Byte );
void REGISTER_2118_linear (const uint8 Byte);
void REGISTER_2119 ( const uint8 Byte );
void REGISTER_2119_tile ( const uint8 Byte );
void REGISTER_2119_linear ( const uint8 Byte );
void REGISTER_2122 ( const uint8 Byte );
void REGISTER_2180 ( const uint8 Byte );

#endif
