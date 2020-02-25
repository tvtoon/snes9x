/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _GFX_H_
#define _GFX_H_

#include "port.h"

struct SGFX
{
	uint16	*Screen;
	uint16	*SubScreen;
	uint8	*ZBuffer;
	uint8	*SubZBuffer;
	uint32	Pitch;
	uint32	ScreenSize;
	uint16	*S;
	uint8	*DB;
	uint16	*ZERO;
	uint32	RealPPL;			// true PPL of Screen buffer
	uint32	PPL;				// number of pixels on each of Screen buffer
	uint32	LinesPerTile;		// number of lines in 1 tile (4 or 8 due to interlace)
	uint16	*ScreenColors;		// screen colors for rendering main
	uint16	*RealScreenColors;	// screen colors, ignoring color window clipping
	uint8	Z1;					// depth for comparison
	uint8	Z2;					// depth to save
	uint32	FixedColour;
	uint8	DoInterlace;
	uint8	InterlaceFrame;
	uint32	StartY;
	uint32	EndY;
	bool8	ClipColors;
	uint8	OBJWidths[128];
	uint8	OBJVisibleTiles[128];

	struct ClipData	*Clip;

	struct
	{
		uint8	RTOFlags;
		int16	Tiles;

		struct
		{
			int8	Sprite;
			uint8	Line;
		}	OBJ[32];
	}	OBJLines[SNES_HEIGHT_EXTENDED];

	void	(*DrawBackdropMath) (uint32, uint32, uint32);
	void	(*DrawBackdropNomath) (uint32, uint32, uint32);
	void	(*DrawTileMath) (uint32, uint32, uint32, uint32);
	void	(*DrawTileNomath) (uint32, uint32, uint32, uint32);
	void	(*DrawClippedTileMath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawClippedTileNomath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMosaicPixelMath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMosaicPixelNomath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMode7BG1Math) (uint32, uint32, int);
	void	(*DrawMode7BG1Nomath) (uint32, uint32, int);
	void	(*DrawMode7BG2Math) (uint32, uint32, int);
	void	(*DrawMode7BG2Nomath) (uint32, uint32, int);

	const char	*InfoString;
	uint32	InfoStringTimeout;
	char	FrameDisplayString[256];
};

struct SBG
{
	uint8	(*ConvertTile) (uint8 *, uint32, uint32);
	uint8	(*ConvertTileFlip) (uint8 *, uint32, uint32);

	uint32	TileSizeH;
	uint32	TileSizeV;
	uint32	OffsetSizeH;
	uint32	OffsetSizeV;
	uint32	TileShift;
	uint32	TileAddress;
	uint32	NameSelect;
	uint32	SCBase;

	uint32	StartPalette;
	uint32	PaletteShift;
	uint32	PaletteMask;
	uint8	EnableMath;
	uint8	InterlaceLine;

	uint8	*Buffer;
	uint8	*BufferFlip;
	uint8	*Buffered;
	uint8	*BufferedFlip;
	bool8	DirectColourMode;
};

struct SLineData
{
	struct
	{
		uint16	VOffset;
		uint16	HOffset;
	}	BG[4];
};

struct SLineMatrixData
{
	short	MatrixA;
	short	MatrixB;
	short	MatrixC;
	short	MatrixD;
	short	CentreX;
	short	CentreY;
	short	M7HOFS;
	short	M7VOFS;
};

extern uint16		BlackColourMap[256];
extern uint16		DirectColourMaps[8][256];
extern uint8		mul_brightness[16][32];
extern uint8		brightness_cap[64];
extern struct SBG	BG;
extern struct SGFX	GFX;

#define H_FLIP		0x4000
#define V_FLIP		0x8000
#define BLANK_TILE	2

void S9xStartScreenRefresh (void);
void S9xEndScreenRefresh (void);
void S9xBuildDirectColourMaps (void);
void RenderLine (uint8);
void S9xComputeClipWindows (void);
void S9xDisplayChar (uint16 *, uint8);
void S9xGraphicsScreenResize (void);
// called automatically unless Settings.AutoDisplayMessages is false
void S9xDisplayMessages (uint16 *, int, int, int, int);

// external port interface which must be implemented or initialised for each port
bool8 S9xGraphicsInit (void);
void S9xGraphicsDeinit (void);
bool8 S9xInitUpdate (void);
bool8 S9xDeinitUpdate (int, int);
bool8 S9xContinueUpdate (int, int);
void S9xReRefresh (void);
void S9xSetPalette (void);
void S9xSyncSpeed (void);

// called instead of S9xDisplayString if set to non-NULL
extern void (*S9xCustomDisplayString) (const char *, int, int, bool, int type);

/**/
#define COLOR_ADD_BRIGHTNESS1_2 COLOR_ADD1_2

uint16 COLOR_ADD( const uint16 C1, const uint16 C2 );
uint16 COLOR_ADD1_2( const uint16 C1, const uint16 C2 );
uint16 COLOR_ADD_BRIGHTNESS( const uint16 C1, const uint16 C2 );
uint16 COLOR_SUB( const uint16 C1, const uint16 C2 );
uint16 COLOR_SUB1_2( const uint16 C1, const uint16 C2 );

#endif
