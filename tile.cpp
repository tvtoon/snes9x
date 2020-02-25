/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// This file includes itself multiple times.
// The other option would be to have 4 files, where A includes B, and B includes C 3 times, and C includes D 5 times.
// Look for the following marker to find where the divisions are.

// Top-level compilation.

#ifndef _NEWTILE_CPP
#define _NEWTILE_CPP

#include "snes9x.h"
#include "gfx.h"
#include "memmap.h"
#include "ppu.h"
#include "tile.h"

#define CLIP_10_BIT_SIGNED(a) (((a) & 0x2000) ? ((a) | ~0x3ff) : ((a) & 0x3ff))

#ifndef MIN
#  define MIN(a,b)  ((a) < (b)? (a) : (b))
#endif

#define NOMATH(Op, Main, Sub, SD) \
 (Main)

#define REGMATH(Op, Main, Sub, SD) \
 (COLOR_##Op((Main), ((SD) & 0x20) ? (Sub) : GFX.FixedColour))

#define MATHF1_2(Op, Main, Sub, SD) \
 (GFX.ClipColors ? (COLOR_##Op((Main), GFX.FixedColour)) : (COLOR_##Op##1_2((Main), GFX.FixedColour)))

#define MATHS1_2(Op, Main, Sub, SD) \
 (GFX.ClipColors ? REGMATH(Op, Main, Sub, SD) : (((SD) & 0x20) ? COLOR_##Op##1_2((Main), (Sub)) : COLOR_##Op((Main), GFX.FixedColour)))

static uint32 pixbit[8][16];
static uint8 hrbit_odd[256];
static uint8 hrbit_even[256];

static void SELECT_PALETTE( const uint32 Tile )
{
 if (BG.DirectColourMode) /*  if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps();*/
  GFX.RealScreenColors = DirectColourMaps[(Tile >> 10) & 7];
 else GFX.RealScreenColors = &IPPU.ScreenColors[((Tile >> BG.PaletteShift) & BG.PaletteMask) + BG.StartPalette];

 GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors;
}

void S9xInitTileRenderer (void)
{
 int i;

 for (i = 0; i < 16; i++)
 {
  uint32 b = 0;

 #ifdef LSB_FIRST
  if (i & 8)
   b |= 1;
  if (i & 4)
   b |= 1 << 8;
  if (i & 2)
   b |= 1 << 16;
  if (i & 1)
   b |= 1 << 24;
 #else
  if (i & 8)
   b |= 1 << 24;
  if (i & 4)
   b |= 1 << 16;
  if (i & 2)
   b |= 1 << 8;
  if (i & 1)
   b |= 1;
 #endif

  for (uint8 bitshift = 0; bitshift < 8; bitshift++)
   pixbit[bitshift][i] = b << bitshift;
 }

 for (i = 0; i < 256; i++)
 {
  uint8 m = 0;
  uint8 s = 0;

  if (i & 0x80)
   s |= 8;
  if (i & 0x40)
   m |= 8;
  if (i & 0x20)
   s |= 4;
  if (i & 0x10)
   m |= 4;
  if (i & 0x08)
   s |= 2;
  if (i & 0x04)
   m |= 2;
  if (i & 0x02)
   s |= 1;
  if (i & 0x01)
   m |= 1;

  hrbit_odd[i]  = m;
  hrbit_even[i] = s;
 }
}

// Here are the tile converters, selected by S9xSelectTileConverter().
// Really, except for the definition of DOBIT and the number of times it is called, they're all the same.

#define DOBIT(n, i) \
 if ((pix = *(tp + (n)))) \
 { \
  p1 |= pixbit[(i)][pix >> 4]; \
  p2 |= pixbit[(i)][pix & 0xf]; \
 }

static uint8 ConvertTile2 (uint8 *pCache, const uint32 TileAddr, const uint32)
{
 uint8 *tp      = &Memory.VRAM[TileAddr];
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 for (line = 8; line != 0; line--, tp += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4 (uint8 *pCache, const uint32 TileAddr, const uint32)
{
 uint8 *tp      = &Memory.VRAM[TileAddr];
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 for (line = 8; line != 0; line--, tp += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  DOBIT(16, 2);
  DOBIT(17, 3);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile8 (uint8 *pCache, const uint32 TileAddr, const uint32)
{
 uint8 *tp      = &Memory.VRAM[TileAddr];
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 for (line = 8; line != 0; line--, tp += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  DOBIT(16, 2);
  DOBIT(17, 3);
  DOBIT(32, 4);
  DOBIT(33, 5);
  DOBIT(48, 6);
  DOBIT(49, 7);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
 if ((pix = hrbit_odd[*(tp1 + (n))])) \
  p1 |= pixbit[(i)][pix]; \
 if ((pix = hrbit_odd[*(tp2 + (n))])) \
  p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_odd (uint8 *pCache, const uint32 TileAddr, const uint32 Tile)
{
 uint8 *tp1     = &Memory.VRAM[TileAddr], *tp2;
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 if (Tile == 0x3ff)
  tp2 = tp1 - (0x3ff << 4);
 else
  tp2 = tp1 + (1 << 4);

 for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_odd (uint8 *pCache, const uint32 TileAddr, const uint32 Tile)
{
 uint8 *tp1     = &Memory.VRAM[TileAddr], *tp2;
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 if (Tile == 0x3ff)
  tp2 = tp1 - (0x3ff << 5);
 else
  tp2 = tp1 + (1 << 5);

 for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  DOBIT(16, 2);
  DOBIT(17, 3);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
 if ((pix = hrbit_even[*(tp1 + (n))])) \
  p1 |= pixbit[(i)][pix]; \
 if ((pix = hrbit_even[*(tp2 + (n))])) \
  p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_even (uint8 *pCache, const uint32 TileAddr, const uint32 Tile)
{
 uint8 *tp1     = &Memory.VRAM[TileAddr], *tp2;
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 if (Tile == 0x3ff)
  tp2 = tp1 - (0x3ff << 4);
 else
  tp2 = tp1 + (1 << 4);

 for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_even (uint8 *pCache, const uint32 TileAddr, const uint32 Tile)
{
 uint8 *tp1     = &Memory.VRAM[TileAddr], *tp2;
 uint32   *p       = (uint32 *) pCache;
 uint32   non_zero = 0;
 uint8   line;

 if (Tile == 0x3ff)
  tp2 = tp1 - (0x3ff << 5);
 else
  tp2 = tp1 + (1 << 5);

 for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
 {
  uint32   p1 = 0;
  uint32   p2 = 0;
  uint8 pix;

  DOBIT( 0, 0);
  DOBIT( 1, 1);
  DOBIT(16, 2);
  DOBIT(17, 3);
  *p++ = p1;
  *p++ = p2;
  non_zero |= p1 | p2;
 }

 return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

// First-level include: Get all the renderers.
#include "tile.cpp"

// Functions to select which converter and renderer to use.

void S9xSelectTileRenderers (int BGMode, bool8 sub, bool8 obj)
{
 bool8 hires = 0, interlace = 0;
// const uint32 lineperta[3] = { 8, 8, 4 };
 int i = Settings.Transparency;
 unsigned char idxm7b1 = ( PPU.BGMosaic[0] && PPU.Mosaic > 1 ) & 1, idxm7b2 = ( PPU.BGMosaic[1] && PPU.Mosaic > 1 ) & 1, idxn = 0, linpert = 8;

 void (**DTfunca[5])(uint32, uint32, uint32, uint32) =
{
 Renderers_DrawTile16Normal1x1, Renderers_DrawTile16Normal2x1, Renderers_DrawTile16Interlace, Renderers_DrawTile16Hires, Renderers_DrawTile16HiresInterlace
};

 void (**DCTfunca[5])(uint32, uint32, uint32, uint32, uint32, uint32) =
{
 Renderers_DrawClippedTile16Normal1x1, Renderers_DrawClippedTile16Normal2x1, Renderers_DrawClippedTile16Interlace, Renderers_DrawClippedTile16Hires, Renderers_DrawClippedTile16HiresInterlace
};

 void (**DMPfunca[5])(uint32, uint32, uint32, uint32, uint32, uint32) =
{
 Renderers_DrawMosaicPixel16Normal1x1, Renderers_DrawMosaicPixel16Normal2x1, Renderers_DrawMosaicPixel16Interlace, Renderers_DrawMosaicPixel16Hires, Renderers_DrawMosaicPixel16HiresInterlace
};

 void (**DBfunca[5])(uint32, uint32, uint32) =
{
 Renderers_DrawBackdrop16Normal1x1, Renderers_DrawBackdrop16Normal2x1, Renderers_DrawBackdrop16Normal2x1, Renderers_DrawBackdrop16Hires, Renderers_DrawBackdrop16Hires
};
/* interlace not relevant */
 void (**DM7BG1funca[6])(uint32, uint32, int) =
{
 Renderers_DrawMode7BG1Normal1x1, Renderers_DrawMode7MosaicBG1Normal1x1,
/* hires or not */
 Renderers_DrawMode7BG1Normal2x1, Renderers_DrawMode7MosaicBG1Normal2x1, Renderers_DrawMode7BG1Hires, Renderers_DrawMode7MosaicBG1Hires
};

 void (**DM7BG2funca[6])(uint32, uint32, int) =
{
 Renderers_DrawMode7BG2Normal1x1, Renderers_DrawMode7MosaicBG2Normal1x1,
/* */
 Renderers_DrawMode7BG2Normal2x1, Renderers_DrawMode7MosaicBG2Normal2x1, Renderers_DrawMode7BG2Hires, Renderers_DrawMode7MosaicBG2Hires
};

 if ( IPPU.DoubleWidthPixels != 0 )
{
  interlace = obj ? FALSE : IPPU.Interlace;
  hires = !sub && (BGMode == 5 || BGMode == 6 || IPPU.PseudoHires);
  linpert /= interlace + 1;
  idxm7b1 += 2 + ( hires * 2 );
  idxm7b2 += 2 + ( hires * 2 );
  idxn = 1 + ( ( hires * 2 ) + interlace );
}

 GFX.LinesPerTile = linpert;
 GFX.DrawTileNomath        = DTfunca[idxn][0];
 GFX.DrawClippedTileNomath = DCTfunca[idxn][0];
 GFX.DrawMosaicPixelNomath = DMPfunca[idxn][0];
 GFX.DrawBackdropNomath    = DBfunca[idxn][0];
 GFX.DrawMode7BG1Nomath    = DM7BG1funca[idxm7b1][0];
 GFX.DrawMode7BG2Nomath    = DM7BG2funca[idxm7b2][0];

 if ( i != 0 )
{
  if ( Memory.FillRAM[0x2131] & 0x80 ) i = 4;

  if ( Memory.FillRAM[0x2131] & 0x40 )
{
   if ( Memory.FillRAM[0x2130] & 2 ) i += 2;
   else i++;
}

  if ( IPPU.MaxBrightness != 0xf)
{
   if (i == 1) i = 7;
   else if (i == 3) i = 8;
}

}

 GFX.DrawTileMath        = DTfunca[idxn][i];
 GFX.DrawClippedTileMath = DCTfunca[idxn][i];
 GFX.DrawMosaicPixelMath = DMPfunca[idxn][i];
 GFX.DrawBackdropMath    = DBfunca[idxn][i];
 GFX.DrawMode7BG1Math    = DM7BG1funca[idxm7b1][i];
 GFX.DrawMode7BG2Math    = DM7BG2funca[idxm7b2][i];
}

void S9xSelectTileConverter (int depth, bool8 hires, bool8 sub, bool8 mosaic)
{
// Valids: 2, 4, 8.
 const unsigned int tshifta[9] = { 0, 0, 4, 0, 5, 0, 0, 0, 6 };
 const unsigned int pshifta[9] = { 0, 0, 10 - 2, 0, 10 - 4, 0, 0, 0, 0 };
 const unsigned int pamaska[9] = { 0, 0, 7 << 2, 0, 7 << 4, 0, 0, 0, 0 };
 const unsigned int dcmodea[9] = { 0, 0, 0, 0, 0, 0, 0, 0, Memory.FillRAM[0x2130] & 1 };
/* First index: depth / 4. */
 const unsigned char tileac[3][3] = { { TILE_2BIT, TILE_2BIT_ODD, TILE_2BIT_EVEN }, { TILE_4BIT, TILE_4BIT_ODD, TILE_4BIT_EVEN }, { TILE_8BIT, TILE_8BIT, TILE_8BIT } };
 const unsigned char idepth = depth / 4;
 uint8 (*confunca[3][3])( uint8 *pCache, const uint32 TileAddr, const uint32 Tile ) =
{
 { ConvertTile2, ConvertTile2h_odd, ConvertTile2h_even },
 { ConvertTile4, ConvertTile4h_odd, ConvertTile4h_even },
 { ConvertTile8, ConvertTile8, ConvertTile8 }
};
 unsigned char ibuff = hires * 2, ibufn = hires;

 if ( depth > 8 || depth < 0 )
{
  depth = 0;
 return;
}
/*
 HIRES is only 0 or 1, everything else cannot be trusted.
 Normal HIRES: flip gets even, normal gets odd. This is the opposite.
*/
 if ( hires && ( sub || mosaic ) )
{
  ibuff--;
  ibufn++;
}

 BG.TileShift = tshifta[depth];
 BG.PaletteShift = pshifta[depth];
 BG.PaletteMask = pamaska[depth];
 BG.DirectColourMode = dcmodea[depth];
 BG.ConvertTile = confunca[idepth][ibufn];
 BG.ConvertTileFlip = confunca[idepth][ibuff];
 BG.Buffer = IPPU.TileCache[ tileac[idepth][ibufn] ];
 BG.Buffered = IPPU.TileCached[ tileac[idepth][ibufn] ];
 BG.BufferFlip = IPPU.TileCache[ tileac[idepth][ibuff] ];
 BG.BufferedFlip = IPPU.TileCached[ tileac[idepth][ibuff] ];
}

/*****************************************************************************/
#else
#ifndef NAME1 // First-level: Get all the renderers.
/*****************************************************************************/

// Basic routine to render an unclipped tile.
// Input parameters:
//     BPSTART = either StartLine or (StartLine * 2 + BG.InterlaceLine),
//     so interlace modes can render every other line from the tile.
//     PITCH = 1 or 2, again so interlace can count lines properly.
//     DRAW_PIXEL(N, M) is a routine to actually draw the pixel. N is the pixel in the row to draw,
//     and M is a test which if false means the pixel should be skipped.
//     Z1 is the "draw if Z1 > cur_depth".
//     Z2 is the "cur_depth = new_depth". OBJ need the two separate.
//     Pix is the pixel to draw.

#define Z1 GFX.Z1
#define Z2 GFX.Z2

#define DRAW_TILE() \
 uint8 *pCache = 0, *pbufo = 0; \
 int32 i = 0, j = 7, jini = 7, jsum = -1, l = 0, linesum = -( 8 * PITCH ); \
 uint8 *bp, Pix; \
 uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift), TileNumber = 0; \
\
 if (Tile & 0x100) TileAddr += BG.NameSelect; \
\
 TileAddr &= 0xffff; \
 TileNumber = TileAddr >> BG.TileShift; \
\
 if (Tile & H_FLIP) \
{ \
  pCache = &BG.BufferFlip[TileNumber << 6]; \
\
  if (!BG.BufferedFlip[TileNumber]) BG.BufferedFlip[TileNumber] = BG.ConvertTileFlip(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.BufferedFlip[TileNumber]; \
} \
 else \
{ \
  pCache = &BG.Buffer[TileNumber << 6]; \
\
  if (!BG.Buffered[TileNumber]) BG.Buffered[TileNumber] = BG.ConvertTile(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.Buffered[TileNumber]; \
  jini = 0; \
  jsum = 1; \
} \
\
 if ( pbufo[0] == BLANK_TILE ) return; \
\
 SELECT_PALETTE(Tile); \
 OFFSET_IN_LINE; \
\
 if ( !(Tile & V_FLIP) ) \
{ \
  bp = pCache + BPSTART; \
  linesum = -(linesum); \
} \
 else \
{ \
  bp = pCache + 56 - BPSTART; \
} \
\
 for (l = LineCount; l > 0; l--, bp += linesum, Offset += GFX.PPL) \
{ \
\
  for ( i = 0, j = jini; i < 8; i++, j += jsum ) \
{ \
   DRAW_PIXEL( i, Pix = bp[j] ); \
} \
\
}

#define NAME1 DrawTile16
#define ARGS uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a clipped tile. Inputs same as above.

#define Z1 GFX.Z1
#define Z2 GFX.Z2

#define DRAW_TILE() \
 uint8 *pCache = 0, *pbufo = 0; \
 int32 i = 0, j = 7, jini = 7, jsum = -1, l = 0, linesum = -( 8 * PITCH ); \
 uint8 *bp, Pix, w; \
 uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift), TileNumber = 0, ui = 0; \
\
 if (Tile & 0x100) TileAddr += BG.NameSelect; \
\
 TileAddr &= 0xffff; \
 TileNumber = TileAddr >> BG.TileShift; \
\
 if (Tile & H_FLIP) \
{ \
  pCache = &BG.BufferFlip[TileNumber << 6]; \
\
  if (!BG.BufferedFlip[TileNumber]) BG.BufferedFlip[TileNumber] = BG.ConvertTileFlip(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.BufferedFlip[TileNumber]; \
} \
 else \
{ \
  pCache = &BG.Buffer[TileNumber << 6]; \
\
  if (!BG.Buffered[TileNumber]) BG.Buffered[TileNumber] = BG.ConvertTile(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.Buffered[TileNumber]; \
  jini = 0; \
  jsum = 1; \
} \
\
 if ( pbufo[0] == BLANK_TILE ) return; \
\
 SELECT_PALETTE(Tile); \
 OFFSET_IN_LINE; \
\
 if ( StartPixel < 8 ) \
{ \
\
  if ( Width == 0 ) \
{ \
   Width = 1; \
} \
\
  if ( !(Tile & V_FLIP) ) \
{ \
   bp = pCache + BPSTART; \
   linesum = -(linesum); \
} \
  else \
{ \
   bp = pCache + 56 - BPSTART; \
} \
\
  w = MIN( Width, 8 - StartPixel ); \
  jini += ( StartPixel * jsum ); \
\
  for ( l = LineCount; l > 0; l--, bp += linesum, Offset += GFX.PPL) \
{ \
\
   for ( i = StartPixel, j = jini, ui = 0; ui < w; i++, j += jsum, ui++ ) \
{ \
    DRAW_PIXEL( i, Pix = bp[j] ); \
} \
\
} \
\
}

#define NAME1 DrawClippedTile16
#define ARGS uint32 Tile, uint32 Offset, uint32 StartPixel, uint32 Width, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawClippedTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a single mosaic pixel.
// DRAW_PIXEL, BPSTART, Z1, Z2 and Pix are the same as above, but PITCH is not used.

#define Z1 GFX.Z1
#define Z2 GFX.Z2

#define DRAW_TILE() \
 uint8 *pCache = 0, *pbufo = 0; \
 int32 l, w; \
 uint8 Pix; \
 uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift), TileNumber = 0; \
\
 if (Tile & 0x100) TileAddr += BG.NameSelect; \
\
 TileAddr &= 0xffff; \
 TileNumber = TileAddr >> BG.TileShift; \
\
 if (Tile & H_FLIP) \
{ \
  pCache = &BG.BufferFlip[TileNumber << 6]; \
\
  if (!BG.BufferedFlip[TileNumber]) BG.BufferedFlip[TileNumber] = BG.ConvertTileFlip(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.BufferedFlip[TileNumber]; \
  StartPixel = 7 - StartPixel; \
} \
 else \
{ \
  pCache = &BG.Buffer[TileNumber << 6]; \
\
  if (!BG.Buffered[TileNumber]) BG.Buffered[TileNumber] = BG.ConvertTile(pCache, TileAddr, Tile & 0x3ff); \
\
  pbufo = &BG.Buffered[TileNumber]; \
} \
\
 if ( pbufo[0] == BLANK_TILE ) return; \
\
 SELECT_PALETTE(Tile); \
\
 if (Tile & V_FLIP) Pix = pCache[56 - BPSTART + StartPixel]; \
 else Pix = pCache[BPSTART + StartPixel]; \
\
 if (Pix) \
{ \
  OFFSET_IN_LINE; \
\
  for (l = LineCount; l > 0; l--, Offset += GFX.PPL) \
{ \
   for (w = Width - 1; w >= 0; w--) DRAW_PIXEL(w, 1); \
} \
\
}

#define NAME1 DrawMosaicPixel16
#define ARGS uint32 Tile, uint32 Offset, uint32 StartLine, uint32 StartPixel, uint32 Width, uint32 LineCount

// Second-level include: Get the DrawMosaicPixel16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render the backdrop.
// DRAW_PIXEL is the same as above, but since we're just replicating a single pixel there's no need for PITCH or BPSTART
// (or interlace at all, really).
// The backdrop is always depth = 1, so Z1 = Z2 = 1. And backdrop is always color 0.

#define NO_INTERLACE 1
#define Z1    1
#define Z2    1
#define Pix    0

#define DRAW_TILE() \
 uint32 l, x; \
 \
 GFX.RealScreenColors = IPPU.ScreenColors; \
 GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
 \
 OFFSET_IN_LINE; \
 for (l = GFX.StartY; l <= GFX.EndY; l++, Offset += GFX.PPL) \
 { \
  for (x = Left; x < Right; x++) \
   DRAW_PIXEL(x, 1); \
 }

#define NAME1 DrawBackdrop16
#define ARGS uint32 Offset, uint32 Left, uint32 Right

// Second-level include: Get the DrawBackdrop16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Pix
#undef Z1
#undef Z2
#undef NO_INTERLACE

// Basic routine to render a chunk of a Mode 7 BG.
// Mode 7 has no interlace, so BPSTART and PITCH are unused.
// We get some new parameters, so we can use the same DRAW_TILE to do BG1 or BG2:
//     DCMODE tests if Direct Color should apply.
//     BG is the BG, so we use the right clip window.
//     MASK is 0xff or 0x7f, the 'color' portion of the pixel.
// We define Z1/Z2 to either be constant 5 or to vary depending on the 'priority' portion of the pixel.

extern struct SLineMatrixData LineMatrixData[240];

#define NO_INTERLACE 1
#define Z1    (D + 7)
#define Z2    (D + 7)
#define MASK   0xff
#define DCMODE   (Memory.FillRAM[0x2130] & 1)
#define BG    0

#define DRAW_TILE_NORMAL() \
 uint8 *VRAM1 = Memory.VRAM + 1; \
 \
 if (DCMODE) \
 { \
  GFX.RealScreenColors = DirectColourMaps[0]; \
 } \
 else \
  GFX.RealScreenColors = IPPU.ScreenColors; \
 \
 GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
 \
 int aa, cc; \
 int startx; \
 \
 uint32 Offset = GFX.StartY * GFX.PPL; \
 struct SLineMatrixData *l = &LineMatrixData[GFX.StartY]; \
 \
 OFFSET_IN_LINE; \
 for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Offset += GFX.PPL, l++) \
 { \
  int yy, starty; \
  \
  int32 HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
  int32 VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
  \
  int32 CentreX = ((int32) l->CentreX << 19) >> 19; \
  int32 CentreY = ((int32) l->CentreY << 19) >> 19; \
  \
  if (PPU.Mode7VFlip) \
   starty = 255 - (int) (Line + 1); \
  else \
   starty = Line + 1; \
  \
  yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
  \
  int BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
  int DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
  \
  if (PPU.Mode7HFlip) \
  { \
   startx = Right - 1; \
   aa = -l->MatrixA; \
   cc = -l->MatrixC; \
  } \
  else \
  { \
   startx = Left; \
   aa = l->MatrixA; \
   cc = l->MatrixC; \
  } \
  \
  int xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
  int AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
  int CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
  \
  uint8 Pix; \
  \
  if (!PPU.Mode7Repeat) \
  { \
   for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
   { \
    int X = ((AA + BB) >> 8) & 0x3ff; \
    int Y = ((CC + DD) >> 8) & 0x3ff; \
    \
    uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
    uint8 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
    \
    DRAW_PIXEL(x, Pix = (b & MASK)); \
   } \
  } \
  else \
  { \
   for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
   { \
    int X = ((AA + BB) >> 8); \
    int Y = ((CC + DD) >> 8); \
    \
    uint8 b; \
    \
    if (((X | Y) & ~0x3ff) == 0) \
    { \
     uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
     b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
    } \
    else \
    if (PPU.Mode7Repeat == 3) \
     b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
    else \
     continue; \
    \
    DRAW_PIXEL(x, Pix = (b & MASK)); \
   } \
  } \
 }

#define DRAW_TILE_MOSAIC() \
 uint8 *VRAM1 = Memory.VRAM + 1; \
 \
 if (DCMODE) \
 { \
  GFX.RealScreenColors = DirectColourMaps[0]; \
 } \
 else \
  GFX.RealScreenColors = IPPU.ScreenColors; \
 \
 GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
 \
 int aa, cc; \
 int startx, StartY = GFX.StartY; \
 \
 int  HMosaic = 1, VMosaic = 1, MosaicStart = 0; \
 int32 MLeft = Left, MRight = Right; \
 \
 if (PPU.BGMosaic[0]) \
 { \
  VMosaic = PPU.Mosaic; \
  MosaicStart = ((uint32) GFX.StartY - PPU.MosaicStart) % VMosaic; \
  StartY -= MosaicStart; \
 } \
 \
 if (PPU.BGMosaic[BG]) \
 { \
  HMosaic = PPU.Mosaic; \
  MLeft  -= MLeft  % HMosaic; \
  MRight += HMosaic - 1; \
  MRight -= MRight % HMosaic; \
 } \
 \
 uint32 Offset = StartY * GFX.PPL; \
 struct SLineMatrixData *l = &LineMatrixData[StartY]; \
 \
 OFFSET_IN_LINE; \
 for (uint32 Line = StartY; Line <= GFX.EndY; Line += VMosaic, Offset += VMosaic * GFX.PPL, l += VMosaic) \
 { \
  if (Line + VMosaic > GFX.EndY) \
   VMosaic = GFX.EndY - Line + 1; \
  \
  int yy, starty; \
  \
  int32 HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
  int32 VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
  \
  int32 CentreX = ((int32) l->CentreX << 19) >> 19; \
  int32 CentreY = ((int32) l->CentreY << 19) >> 19; \
  \
  if (PPU.Mode7VFlip) \
   starty = 255 - (int) (Line + 1); \
  else \
   starty = Line + 1; \
  \
  yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
  \
  int BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
  int DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
  \
  if (PPU.Mode7HFlip) \
  { \
   startx = MRight - 1; \
   aa = -l->MatrixA; \
   cc = -l->MatrixC; \
  } \
  else \
  { \
   startx = MLeft; \
   aa = l->MatrixA; \
   cc = l->MatrixC; \
  } \
  \
  int xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
  int AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
  int CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
  \
  uint8 Pix; \
  uint8 ctr = 1; \
  \
  if (!PPU.Mode7Repeat) \
  { \
   for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
   { \
    if (--ctr) \
     continue; \
    ctr = HMosaic; \
    \
    int X = ((AA + BB) >> 8) & 0x3ff; \
    int Y = ((CC + DD) >> 8) & 0x3ff; \
    \
    uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
    uint8 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
    \
    if ((Pix = (b & MASK))) \
    { \
     for (int32 h = MosaicStart; h < VMosaic; h++) \
     { \
      for (int32 w = x + HMosaic - 1; w >= x; w--) \
       DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
     } \
    } \
   } \
  } \
  else \
  { \
   for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
   { \
    if (--ctr) \
     continue; \
    ctr = HMosaic; \
    \
    int X = ((AA + BB) >> 8); \
    int Y = ((CC + DD) >> 8); \
    \
    uint8 b; \
    \
    if (((X | Y) & ~0x3ff) == 0) \
    { \
     uint8 *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
     b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
    } \
    else \
    if (PPU.Mode7Repeat == 3) \
     b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
    else \
     continue; \
    \
    if ((Pix = (b & MASK))) \
    { \
     for (int32 h = MosaicStart; h < VMosaic; h++) \
     { \
      for (int32 w = x + HMosaic - 1; w >= x; w--) \
       DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
     } \
    } \
   } \
  } \
  \
  MosaicStart = 0; \
 }

#define DRAW_TILE() DRAW_TILE_NORMAL()
#define NAME1  DrawMode7BG1
#define ARGS  uint32 Left, uint32 Right, int D

// Second-level include: Get the DrawMode7BG1 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE() DRAW_TILE_MOSAIC()
#define NAME1  DrawMode7MosaicBG1

// Second-level include: Get the DrawMode7MosaicBG1 renderers.

#include "tile.cpp"

#undef DRAW_TILE
#undef NAME1
#undef Z1
#undef Z2
#undef MASK
#undef DCMODE
#undef BG

#define NAME1  DrawMode7BG2
#define DRAW_TILE() DRAW_TILE_NORMAL()
#define Z1   (D + ((b & 0x80) ? 11 : 3))
#define Z2   (D + ((b & 0x80) ? 11 : 3))
#define MASK  0x7f
#define DCMODE  0
#define BG   1

// Second-level include: Get the DrawMode7BG2 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE() DRAW_TILE_MOSAIC()
#define NAME1  DrawMode7MosaicBG2

// Second-level include: Get the DrawMode7MosaicBG2 renderers.

#include "tile.cpp"

#undef MASK
#undef DCMODE
#undef BG
#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef DRAW_TILE_NORMAL
#undef DRAW_TILE_MOSAIC
#undef Z1
#undef Z2
#undef NO_INTERLACE

/*****************************************************************************/
#else
#ifndef NAME2 // Second-level: Get all the NAME1 renderers.
/*****************************************************************************/

#define BPSTART StartLine
#define PITCH 1

// The 1x1 pixel plotter, for speedhacking modes.

#define OFFSET_IN_LINE
#define DRAW_PIXEL(N, M) \
 if (Z1 > GFX.DB[Offset + N] && (M)) \
 { \
  GFX.S[Offset + N] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + N], GFX.SubZBuffer[Offset + N]); \
  GFX.DB[Offset + N] = Z2; \
 }

#define NAME2 Normal1x1

// Third-level include: Get the Normal1x1 renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

// The 2x1 pixel plotter, for normal rendering when we've used hires/interlace already this frame.

#define DRAW_PIXEL_N2x1(N, M) \
 if (Z1 > GFX.DB[Offset + 2 * N] && (M)) \
 { \
  GFX.S[Offset + 2 * N] = GFX.S[Offset + 2 * N + 1] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + 2 * N], GFX.SubZBuffer[Offset + 2 * N]); \
  GFX.DB[Offset + 2 * N] = GFX.DB[Offset + 2 * N + 1] = Z2; \
 }

#define DRAW_PIXEL(N, M) DRAW_PIXEL_N2x1(N, M)
#define NAME2    Normal2x1

// Third-level include: Get the Normal2x1 renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL
#undef OFFSET_IN_LINE

// Hires pixel plotter, this combines the main and subscreen pixels as appropriate to render hires or pseudo-hires images.
// Use it only on the main screen, subscreen should use Normal2x1 instead.
// Hires math:
//     Main pixel is mathed as normal: Main(x, y) * Sub(x, y).
//     Sub pixel is mathed somewhat weird: Basically, for Sub(x + 1, y) we apply the same operation we applied to Main(x, y)
//     (e.g. no math, add fixed, add1/2 subscreen) using Main(x, y) as the "corresponding subscreen pixel".
//     Also, color window clipping clips Sub(x + 1, y) if Main(x, y) is clipped, not Main(x + 1, y).
//     We don't know how Sub(0, y) is handled.

#define DRAW_PIXEL_H2x1(N, M) \
 if (Z1 > GFX.DB[Offset + 2 * N] && (M)) \
 { \
  GFX.S[Offset + 2 * N + 1] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + 2 * N], GFX.SubZBuffer[Offset + 2 * N]); \
  if ((OffsetInLine + 2 * N ) != (SNES_WIDTH - 1) << 1) \
   GFX.S[Offset + 2 * N + 2] = MATH((GFX.ClipColors ? 0 : GFX.SubScreen[Offset + 2 * N + 2]), GFX.RealScreenColors[Pix], GFX.SubZBuffer[Offset + 2 * N]); \
  if ((OffsetInLine + 2 * N) == 0 || (OffsetInLine + 2 * N) == GFX.RealPPL) \
   GFX.S[Offset + 2 * N] = MATH((GFX.ClipColors ? 0 : GFX.SubScreen[Offset + 2 * N]), GFX.RealScreenColors[Pix], GFX.SubZBuffer[Offset + 2 * N]); \
  GFX.DB[Offset + 2 * N] = GFX.DB[Offset + 2 * N + 1] = Z2; \
 }

#define OFFSET_IN_LINE \
 uint32 OffsetInLine = Offset % GFX.RealPPL;
#define DRAW_PIXEL(N, M) DRAW_PIXEL_H2x1(N, M)
#define NAME2    Hires

// Third-level include: Get the Hires renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL
#undef OFFSET_IN_LINE

// Interlace: Only draw every other line, so we'll redefine BPSTART and PITCH to do so.
// Otherwise, it's the same as Normal2x1/Hires2x1.

#undef BPSTART
#undef PITCH

#define BPSTART (StartLine * 2 + BG.InterlaceLine)
#define PITCH 2

#ifndef NO_INTERLACE

#define OFFSET_IN_LINE
#define DRAW_PIXEL(N, M) DRAW_PIXEL_N2x1(N, M)
#define NAME2    Interlace

// Third-level include: Get the double width Interlace renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL
#undef OFFSET_IN_LINE

#define OFFSET_IN_LINE \
 uint32 OffsetInLine = Offset % GFX.RealPPL;
#define DRAW_PIXEL(N, M) DRAW_PIXEL_H2x1(N, M)
#define NAME2    HiresInterlace

// Third-level include: Get the HiresInterlace renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL
#undef OFFSET_IN_LINE

#endif

#undef BPSTART
#undef PITCH

/*****************************************************************************/
#else // Third-level: Renderers for each math mode for NAME1 + NAME2.
/*****************************************************************************/

#define CONCAT3(A, B, C) A##B##C
#define MAKENAME(A, B, C) CONCAT3(A, B, C)

static void MAKENAME(NAME1, _, NAME2) (ARGS)
{
#define MATH(A, B, C) NOMATH(x, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Add_, NAME2) (ARGS)
{
#define MATH(A, B, C) REGMATH(ADD, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Add_Brightness_, NAME2) (ARGS)
{
#define MATH(A, B, C) REGMATH(ADD_BRIGHTNESS, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C) MATHF1_2(ADD, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C) MATHS1_2(ADD, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddS1_2_Brightness_, NAME2) (ARGS)
{
#define MATH(A, B, C) MATHS1_2(ADD_BRIGHTNESS, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Sub_, NAME2) (ARGS)
{
#define MATH(A, B, C) REGMATH(SUB, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C) MATHF1_2(SUB, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C) MATHS1_2(SUB, A, B, C)
 DRAW_TILE();
#undef MATH
}

static void (*MAKENAME(Renderers_, NAME1, NAME2)[9]) (ARGS) =
{
 MAKENAME(NAME1, _, NAME2),
 MAKENAME(NAME1, Add_, NAME2),
 MAKENAME(NAME1, AddF1_2_, NAME2),
 MAKENAME(NAME1, AddS1_2_, NAME2),
 MAKENAME(NAME1, Sub_, NAME2),
 MAKENAME(NAME1, SubF1_2_, NAME2),
 MAKENAME(NAME1, SubS1_2_, NAME2),
 MAKENAME(NAME1, Add_Brightness_, NAME2),
 MAKENAME(NAME1, AddS1_2_Brightness_, NAME2)
};

#undef MAKENAME
#undef CONCAT3

#endif
#endif
#endif
