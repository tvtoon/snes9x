PROJECT = snes9x
MAJVER = 1.6
MINVER = 0
LIBS = ${PROJECT}
PROGS = ${PROJECT}

include make/conf
include make/cpplib
include make/libX11.mk
include make/libXext.mk
include make/libpng.mk
include make/pthread.mk

CFLAGS = -DHAVE_LIBPNG -DHAVE_STDINT_H -DHAVE_STRINGS_H -DHAVE_SYS_IOCTL_H -DJMA_SUPPORT -DUNZIP_SUPPORT -DMITSHM -DUSE_THREADS -I. -O2 -Wall -Wextra -g -o
# -DHAVE_STDINT_H -DHAVE_STRINGS_H
# UNIX: -DMITSHM -DUSE_THREADS (sound)

# NETPLAY = -DHAVE_MKSTEMP
# CCFLAGS    = -O3 -fomit-frame-pointer -fno-exceptions -fno-rtti -pedantic -Wall -W -Wno-unused-parameter -g -O2  -DJOYSTICK_SUPPORT -DRIGHTSHIFT_IS_SAR $(DEFS)
# UNUSED = -DZLIB

DATA =
DOCS =
INFOS =
INCLUDES = apu/apu.h apu/resampler.h

INCLUDES += filter/2xsai.h filter/blit.h filter/epx.h filter/hq2x.h
INCLUDES += filter/snes_ntsc_config.h filter/snes_ntsc.h filter/snes_ntsc_impl.h

INCLUDES += jma/7z.h jma/aribitcd.h jma/ariconst.h jma/ariprice.h jma/btreecd.h
INCLUDES += jma/crc32.h jma/iiostrm.h jma/inbyte.h jma/jma.h jma/lencoder.h
INCLUDES += jma/litcoder.h jma/lzmadec.h jma/lzma.h jma/portable.h jma/rcdefs.h
INCLUDES += jma/rngcoder.h jma/s9x-jma.h jma/winout.h

INCLUDES += unzip/crypt.h unzip/ioapi.h unzip/iowin32.h unzip/mztools.h
INCLUDES += unzip/unzip.h unzip/zip.h

#cpu.h
INCLUDES += 65c816.h bsx.h c4.h cheats.h conffile.h controls.h cpuexec.h
INCLUDES += cpuops.h crosshairs.h debug.h display.h dma.h dsp.h font.h fxemu.h
INCLUDES += fxinst.h getset.h gfx.h language.h logger.h memmap.h messages.h
INCLUDES += missing.h movie.h netplay.h obc1.h pixform.h port.h ppu.h
INCLUDES += sa1.h sar.h screenshot.h sdd1emu.h sdd1.h seta.h snapshot.h
INCLUDES += snes9x.h spc7110emu.h spc7110.h srtcemu.h srtc.h stream.h tile.h

# Gone from 1.53
#apu/SNES_SPC.cpp apu/SNES_SPC_misc.cpp apu/SNES_SPC_state.cpp
#apu/SPC_DSP.cpp apu/SPC_Filter.cpp
# reader.cpp turned into stream.cpp

SRC = apu/apu.cpp apu/bapu/dsp/sdsp.cpp apu/bapu/smp/smp.cpp
SRC += apu/bapu/smp/smp_state.cpp
SRC += filter/2xsai.cpp filter/epx.cpp filter/hq2x.cpp
# Unix only, not GTK+.
SRC += filter/blit.cpp filter/snes_ntsc.c

SRC += jma/7zlzma.cpp jma/crc32.cpp jma/iiostrm.cpp jma/inbyte.cpp
SRC += jma/jma.cpp jma/lzma.cpp jma/lzmadec.cpp jma/s9x-jma.cpp jma/winout.cpp
#ifndef SYSTEM_ZIP
SRC += unzip/ioapi.c unzip/unzip.c
#NOT NEEDED
#unzip/zip.c

#DEBUG
#CFLAGS += -DDEBUGGER
#SRC += debug.cpp
#NETPLAY
#CFLAGS += -DNETPLAY_SUPPORT
#SRC += netplay.cpp server.cpp

# Is loadzip.cpp needed?
SRC += bsx.cpp c4.cpp c4emu.cpp cheats.cpp cheats2.cpp clip.cpp conffile.cpp
SRC += controls.cpp cpu.cpp cpuexec.cpp cpuops.cpp crosshairs.cpp dma.cpp
SRC += dsp.cpp dsp1.cpp dsp2.cpp dsp3.cpp dsp4.cpp fxinst.cpp fxemu.cpp
SRC += getset.cpp gfx.cpp globals.cpp loadzip.cpp logger.cpp memmap.cpp
SRC += movie.cpp msu1.cpp obc1.cpp ppu.cpp sa1.cpp sa1cpu.cpp screenshot.cpp
SRC += sdd1.cpp sdd1emu.cpp seta.cpp seta010.cpp seta011.cpp seta018.cpp
SRC += snapshot.cpp snes9x.cpp spc7110.cpp srtc.cpp stream.cpp tile.cpp
SRC += statemanager.cpp sha256.cpp bml.cpp

LIBSRC = ${SRC}
#UNIX
PROGSRC = unix/unix.cpp unix/x11.cpp

include make/exconf
include make/build

dist-clean: clean

${PROGS}: unix/unix.o unix/x11.o

include make/pack
include make/rules
include make/thedep
