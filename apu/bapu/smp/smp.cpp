#ifdef DEBUGGER
#include "../../../snes9x.h"
#include "../../../debug.h"
char tmp[1024];
#endif

#include "../snes/snes.hpp"

#define SMP_CPP
namespace SNES {

#ifdef DEBUGGER
#include "debugger/disassembler.cpp"
#endif

SMP smp;

uint8 SMP::op_adc(uint8 x, uint8 y) {
  int r = x + y + regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = ~(x ^ y) & (x ^ r) & 0x80;
  regs.p.h = (x ^ y ^ r) & 0x10;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r > 0xff;
  return r;
}

uint16 SMP::op_addw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 0;
  r  = op_adc(x, y);
  r |= op_adc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMP::op_and(uint8 x, uint8 y) {
  x &= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_cmp(uint8 x, uint8 y) {
  int r = x - y;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint16 SMP::op_cmpw(uint16 x, uint16 y) {
  int r = x - y;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint8 SMP::op_eor(uint8 x, uint8 y) {
  x ^= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_or(uint8 x, uint8 y) {
  x |= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_sbc(uint8 x, uint8 y) {
  int r = x - y - !regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = (x ^ y) & (x ^ r) & 0x80;
  regs.p.h = !((x ^ y ^ r) & 0x10);
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return r;
}

uint16 SMP::op_subw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 1;
  r  = op_sbc(x, y);
  r |= op_sbc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMP::op_inc(uint8 x) {
  x++;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_dec(uint8 x) {
  x--;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_asl(uint8 x) {
  regs.p.c = x & 0x80;
  x <<= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_lsr(uint8 x) {
  regs.p.c = x & 0x01;
  x >>= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_rol(uint8 x) {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = x & 0x80;
  x = (x << 1) | carry;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMP::op_ror(uint8 x) {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = x & 0x01;
  x = carry | (x >> 1);
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

void SMP::tick() {
  timer0.tick();
  timer1.tick();
  timer2.tick();

  clock++;
  dsp.clock++;
}

void SMP::tick(unsigned clocks) {
  timer0.tick(clocks);
  timer1.tick(clocks);
  timer2.tick(clocks);

  clock += clocks;
  dsp.clock += clocks;
}

void SMP::op_io() {
  tick();
}

void SMP::op_io(unsigned clocks) {
  tick(clocks);
}

uint8 SMP::op_read(uint16 addr) {
  tick();
  if((addr & 0xfff0) == 0x00f0) return mmio_read(addr);
  if(addr >= 0xffc0 && status.iplrom_enable) return iplrom[addr & 0x3f];
  return apuram[addr];
}

void SMP::op_write(uint16 addr, uint8 data) {
  tick();
  if((addr & 0xfff0) == 0x00f0) mmio_write(addr, data);
  apuram[addr] = data;  //all writes go to RAM, even MMIO writes
}

uint8 SMP::op_readstack()
{
  tick();
  return apuram[0x0100 | ++regs.sp];
}

void SMP::op_writestack(uint8 data)
{
  tick();
  apuram[0x0100 | regs.sp--] = data;
}

void SMP::op_step() {
  #define op_readpc() op_read(regs.pc++)
  #define op_readdp(addr) op_read((regs.p.p << 8) + ((addr) & 0xff))
  #define op_writedp(addr, data) op_write((regs.p.p << 8) + ((addr) & 0xff), data)
  #define op_readaddr(addr) op_read(addr)
  #define op_writeaddr(addr, data) op_write(addr, data)

  if(opcode_cycle == 0)
  {
#ifdef DEBUGGER
    if (Settings.TraceSMP)
    {
      disassemble_opcode(tmp, regs.pc);
      S9xTraceMessage (tmp);
    }
#endif
    opcode_number = op_readpc();
  }

  switch(opcode_number) {

case 0x00: {
  op_io();
  break;
}

case 0xef: {
  op_io(2);
  regs.pc--;
  break;
}

case 0xff: {
  op_io(2);
  regs.pc--;
  break;
}

case 0x9f: {
  op_io(4);
  regs.B.a = (regs.B.a >> 4) | (regs.B.a << 4);
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0xdf: {
  op_io(2);
  if(regs.p.c || (regs.B.a) > 0x99) {
    regs.B.a += 0x60;
    regs.p.c = 1;
  }
  if(regs.p.h || (regs.B.a & 15) > 0x09) {
    regs.B.a += 0x06;
  }
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0xbe: {
  op_io(2);
  if(!regs.p.c || (regs.B.a) > 0x99) {
    regs.B.a -= 0x60;
    regs.p.c = 0;
  }
  if(!regs.p.h || (regs.B.a & 15) > 0x09) {
    regs.B.a -= 0x06;
  }
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0x60: {
  op_io();
  regs.p.c = 0;
  break;
}

case 0x20: {
  op_io();
  regs.p.p = 0;
  break;
}

case 0x80: {
  op_io();
  regs.p.c = 1;
  break;
}

case 0x40: {
  op_io();
  regs.p.p = 1;
  break;
}

case 0xe0: {
  op_io();
  regs.p.v = 0;
  regs.p.h = 0;
  break;
}

case 0xed: {
  op_io(2);
  regs.p.c = !regs.p.c;
  break;
}

case 0xa0: {
  op_io(2);
  regs.p.i = 1;
  break;
}

case 0xc0: {
  op_io(2);
  regs.p.i = 0;
  break;
}

case 0x02: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x01;
  op_writedp(dp, rd);
  break;
}

case 0x12: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x01;
  op_writedp(dp, rd);
  break;
}

case 0x22: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x02;
  op_writedp(dp, rd);
  break;
}

case 0x32: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x02;
  op_writedp(dp, rd);
  break;
}

case 0x42: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x04;
  op_writedp(dp, rd);
  break;
}

case 0x52: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x04;
  op_writedp(dp, rd);
  break;
}

case 0x62: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x08;
  op_writedp(dp, rd);
  break;
}

case 0x72: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x08;
  op_writedp(dp, rd);
  break;
}

case 0x82: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x10;
  op_writedp(dp, rd);
  break;
}

case 0x92: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x10;
  op_writedp(dp, rd);
  break;
}

case 0xa2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x20;
  op_writedp(dp, rd);
  break;
}

case 0xb2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x20;
  op_writedp(dp, rd);
  break;
}

case 0xc2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x40;
  op_writedp(dp, rd);
  break;
}

case 0xd2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x40;
  op_writedp(dp, rd);
  break;
}

case 0xe2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd |=  0x80;
  op_writedp(dp, rd);
  break;
}

case 0xf2: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd &= ~0x80;
  op_writedp(dp, rd);
  break;
}

case 0x2d: {
  op_io(2);
  op_writestack(regs.B.a);
  break;
}

case 0x4d: {
  op_io(2);
  op_writestack(regs.x);
  break;
}

case 0x6d: {
  op_io(2);
  op_writestack(regs.B.y);
  break;
}

case 0x0d: {
  op_io(2);
  op_writestack(regs.p);
  break;
}

case 0xae: {
  op_io(2);
  regs.B.a = op_readstack();
  break;
}

case 0xce: {
  op_io(2);
  regs.x = op_readstack();
  break;
}

case 0xee: {
  op_io(2);
  regs.B.y = op_readstack();
  break;
}

case 0x8e: {
  op_io(2);
  regs.p = op_readstack();
  break;
}

case 0xcf: {
  op_io(8);
  ya = regs.B.y * regs.B.a;
  regs.B.a = ya;
  regs.B.y = ya >> 8;
  //result is set based on y (high-byte) only
  regs.p.n = !!(regs.B.y & 0x80);
  regs.p.z = (regs.B.y == 0);
  break;
}

case 0x9e: {
  op_io(11);
  ya = regs.ya;
  //overflow set if quotient >= 256
  regs.p.v = !!(regs.B.y >= regs.x);
  regs.p.h = !!((regs.B.y & 15) >= (regs.x & 15));
  if(regs.B.y < (regs.x << 1)) {
    //if quotient is <= 511 (will fit into 9-bit result)
    regs.B.a = ya / regs.x;
    regs.B.y = ya % regs.x;
  } else {
    //otherwise, the quotient won't fit into regs.p.v + regs.B.a
    //this emulates the odd behavior of the S-SMP in this case
    regs.B.a = 255    - (ya - (regs.x << 9)) / (256 - regs.x);
    regs.B.y = regs.x + (ya - (regs.x << 9)) % (256 - regs.x);
  }
  //result is set based on a (quotient) only
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

/* MOV */
case 0x7d: {
  op_io();
  regs.B.a = regs.x;
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0xdd: {
  op_io();
  regs.B.a = regs.B.y;
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0x5d: {
  op_io();
  regs.x = regs.B.a;
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xfd: {
  op_io();
  regs.B.y = regs.B.a;
  regs.p.n = !!(regs.B.y & 0x80);
  regs.p.z = (regs.B.y == 0);
  break;
}

case 0x9d: {
  op_io();
  regs.x = regs.sp;
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xbd: {
  op_io();
  regs.sp = regs.x;
  break;
}

case 0xe8: {
  regs.B.a = op_readpc();
  regs.p.n = !!(regs.B.a & 0x80);
  regs.p.z = (regs.B.a == 0);
  break;
}

case 0xcd: {
  regs.x = op_readpc();
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0x8d: {
  regs.B.y = op_readpc();
  regs.p.n = !!(regs.B.y & 0x80);
  regs.p.z = (regs.B.y == 0);
  break;
}

case 0xe6: {
  switch(++opcode_cycle) {
  case 1:
    op_io();
    break;
  case 2:
    regs.B.a = op_readdp(regs.x);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xbf: {
  switch(++opcode_cycle) {
  case 1:
    op_io();
    break;
  case 2:
    regs.B.a = op_readdp(regs.x++);
    op_io();
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe4: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.B.a = op_readdp(sp);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf8: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.x = op_readdp(sp);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xeb: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.B.y = op_readdp(sp);
    regs.p.n = !!(regs.B.y & 0x80);
    regs.p.z = (regs.B.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf4: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    op_io();
    break;
  case 2:
    regs.B.a = op_readdp(sp + regs.x);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf9: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    op_io();
    break;
  case 2:
    regs.x = op_readdp(sp + regs.B.y);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfb: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    op_io();
    break;
  case 2:
    regs.B.y = op_readdp(sp + regs.x);
    regs.p.n = !!(regs.B.y & 0x80);
    regs.p.z = (regs.B.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe5: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    regs.B.a = op_readaddr(sp);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe9: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    sp |= op_readpc() << 8;
    break;
  case 2:
    regs.x = op_readaddr(sp);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xec: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    sp |= op_readpc() << 8;
    break;
  case 2:
    regs.B.y = op_readaddr(sp);
    regs.p.n = !!(regs.B.y & 0x80);
    regs.p.z = (regs.B.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf5: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    sp |= op_readpc() << 8;
    op_io();
    break;
  case 2:
    regs.B.a = op_readaddr(sp + regs.x);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf6: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    sp |= op_readpc() << 8;
    op_io();
    break;
  case 2:
    regs.B.a = op_readaddr(sp + regs.B.y);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe7: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc() + regs.x;
    op_io();
    break;
  case 2:
    sp  = op_readdp(dp);
    break;
  case 3:
    sp |= op_readdp(dp + 1) << 8;
    break;
  case 4:
    regs.B.a = op_readaddr(sp);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf7: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    op_io();
    break;
  case 2:
    sp  = op_readdp(dp);
    break;
  case 3:
    sp |= op_readdp(dp + 1) << 8;
    break;
  case 4:
    regs.B.a = op_readaddr(sp + regs.B.y);
    regs.p.n = !!(regs.B.a & 0x80);
    regs.p.z = (regs.B.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfa: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    rd = op_readdp(sp);
    break;
  case 3:
    dp = op_readpc();
    break;
  case 4:
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8f: {
  switch(++opcode_cycle) {
  case 1:
    rd = op_readpc();
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc6: {
  switch(++opcode_cycle) {
  case 1:
    op_io();
    break;
  case 2:
    op_readdp(regs.x);
    break;
  case 3:
    op_writedp(regs.x, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xaf: {
  switch(++opcode_cycle) {
  case 1:
    op_io(2);
    break;
  case 2:
    op_writedp(regs.x++, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc4: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd8: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcb: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.B.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd4: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    op_io();
    dp += regs.x;
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd9: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    op_io();
    dp += regs.B.y;
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xdb: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    op_io();
    dp += regs.x;
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.B.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc5: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc9: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcc: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.B.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd5: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    dp |= op_readpc() << 8;
    op_io();
    dp += regs.x;
    break;
  case 2:
    op_readaddr(dp);
    break;
  case 3:
    op_writeaddr(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd6: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    dp |= op_readpc() << 8;
    op_io();
    dp += regs.B.y;
    break;
  case 2:
    op_readaddr(dp);
    break;
  case 3:
    op_writeaddr(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc7: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    op_io();
    sp += regs.x;
    break;
  case 2:
    dp  = op_readdp(sp);
    break;
  case 3:
    dp |= op_readdp(sp + 1) << 8;
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd7: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    dp  = op_readdp(sp);
    break;
  case 3:
    dp |= op_readdp(sp + 1) << 8;
    op_io();
    dp += regs.B.y;
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, regs.B.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xba: {
  switch(++opcode_cycle) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.B.a = op_readdp(sp);
    op_io();
    break;
  case 3:
    regs.B.y = op_readdp(sp + 1);
    regs.p.n = !!(regs.ya & 0x8000);
    regs.p.z = (regs.ya == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xda: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp,     regs.B.a);
    break;
  case 4:
    op_writedp(dp + 1, regs.B.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xaa: {
  switch(++opcode_cycle) {
  case 1:
    sp  = op_readpc();
    sp |= op_readpc() << 8;
    break;
  case 2:
    bit = sp >> 13;
    sp &= 0x1fff;
    rd = op_readaddr(sp);
    regs.p.c = !!(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xca: {
  switch(++opcode_cycle) {
  case 1:
    dp  = op_readpc();
    dp |= op_readpc() << 8;
    break;
  case 2:
    bit = dp >> 13;
    dp &= 0x1fff;
    rd = op_readaddr(dp);
    if(regs.p.c)rd |=  (1 << bit);
    else        rd &= ~(1 << bit);
    op_io();
    break;
  case 3:
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

/* PC */
case 0x2f: {
  rd = op_readpc();
  if(0){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xf0: {
  rd = op_readpc();
  if(!regs.p.z){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xd0: {
  rd = op_readpc();
  if(regs.p.z){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xb0: {
  rd = op_readpc();
  if(!regs.p.c){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x90: {
  rd = op_readpc();
  if(regs.p.c){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x70: {
  rd = op_readpc();
  if(!regs.p.v){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x50: {
  rd = op_readpc();
  if(regs.p.v){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x30: {
  rd = op_readpc();
  if(!regs.p.n){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x10: {
  rd = op_readpc();
  if(regs.p.n){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x03: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x01) != 0x01){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x13: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x01) == 0x01){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x23: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x02) != 0x02){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x33: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x02) == 0x02){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x43: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x04) != 0x04){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x53: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x04) == 0x04){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x63: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x08) != 0x08){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x73: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x08) == 0x08){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x83: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x10) != 0x10){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x93: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x10) == 0x10){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xa3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x20) != 0x20){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xb3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x20) == 0x20){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xc3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x40) != 0x40){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xd3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x40) == 0x40){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xe3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x80) != 0x80){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xf3: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((sp & 0x80) == 0x80){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x2e: {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if(regs.B.a == sp){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xde: {
  dp = op_readpc();
  op_io();
  sp = op_readdp(dp + regs.x);
  rd = op_readpc();
  op_io();
  if(regs.B.a == sp){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x6e: {
  dp = op_readpc();
  wr = op_readdp(dp);
  op_writedp(dp, --wr);
  rd = op_readpc();
  if(wr == 0x00){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0xfe: {
  rd = op_readpc();
  op_io();
  regs.B.y--;
  op_io();
  if(regs.B.y == 0x00){ break; }
  op_io(2);
  regs.pc += (int8)rd;
  break;
}

case 0x5f: {
  rd  = op_readpc();
  rd |= op_readpc() << 8;
  regs.pc = rd;
  break;
}

case 0x1f: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  dp += regs.x;
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  regs.pc = rd;
  break;
}

case 0x3f: {
  rd  = op_readpc();
  rd |= op_readpc() << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x4f: {
  rd = op_readpc();
  op_io(2);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = 0xff00 | rd;
  break;
}

case 0x01: {
  dp = 0xffde - (0 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x11: {
  dp = 0xffde - (1 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x21: {
  dp = 0xffde - (2 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x31: {
  dp = 0xffde - (3 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x41: {
  dp = 0xffde - (4 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x51: {
  dp = 0xffde - (5 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x61: {
  dp = 0xffde - (6 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x71: {
  dp = 0xffde - (7 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x81: {
  dp = 0xffde - (8 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x91: {
  dp = 0xffde - (9 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xa1: {
  dp = 0xffde - (10 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xb1: {
  dp = 0xffde - (11 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xc1: {
  dp = 0xffde - (12 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xd1: {
  dp = 0xffde - (13 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xe1: {
  dp = 0xffde - (14 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0xf1: {
  dp = 0xffde - (15 << 1);
  rd  = op_readaddr(dp);
  rd |= op_readaddr(dp + 1) << 8;
  op_io(3);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  regs.pc = rd;
  break;
}

case 0x0f: {
  rd  = op_readaddr(0xffde);
  rd |= op_readaddr(0xffdf) << 8;
  op_io(2);
  op_writestack(regs.pc >> 8);
  op_writestack(regs.pc);
  op_writestack(regs.p);
  regs.pc = rd;
  regs.p.b = 1;
  regs.p.i = 0;
  break;
}

case 0x6f: {
  rd  = op_readstack();
  rd |= op_readstack() << 8;
  op_io(2);
  regs.pc = rd;
  break;
}

case 0x7f: {
  regs.p = op_readstack();
  rd  = op_readstack();
  rd |= op_readstack() << 8;
  op_io(2);
  regs.pc = rd;
  break;
}

/* READ */
case 0x88: {
  rd = op_readpc();
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x28: {
  rd = op_readpc();
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x68: {
  rd = op_readpc();
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0xc8: {
  rd = op_readpc();
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0xad: {
  rd = op_readpc();
  regs.B.y = op_cmp(regs.B.y, rd);
  break;
}

case 0x48: {
  rd = op_readpc();
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x08: {
  rd = op_readpc();
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xa8: {
  rd = op_readpc();
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x86: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x26: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x66: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x46: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x06: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xa6: {
  op_io();
  rd = op_readdp(regs.x);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x84: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x24: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x64: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x3e: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0x7e: {
  switch(++opcode_cycle) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.B.y = op_cmp(regs.B.y, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x44: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x04: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xa4: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x94: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x34: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x74: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x54: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x14: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xb4: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x85: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x25: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x65: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x1e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0x5e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.y = op_cmp(regs.B.y, rd);
  break;
}

case 0x45: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x05: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xa5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x95: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x96: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x35: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x36: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x75: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x76: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x55: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x56: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x15: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0x16: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xb5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0xb6: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.B.y);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x87: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x27: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x67: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x47: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x07: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xa7: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x97: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_adc(regs.B.a, rd);
  break;
}

case 0x37: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_and(regs.B.a, rd);
  break;
}

case 0x77: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_cmp(regs.B.a, rd);
  break;
}

case 0x57: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_eor(regs.B.a, rd);
  break;
}

case 0x17: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_or(regs.B.a, rd);
  break;
}

case 0xb7: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.B.y);
  regs.B.a = op_sbc(regs.B.a, rd);
  break;
}

case 0x99: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x39: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_and(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x79: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x59: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x19: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_or(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0xb9: {
  op_io();
  rd = op_readdp(regs.B.y);
  wr = op_readdp(regs.x);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x89: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x29: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_and(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x69: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x49: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x09: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_or(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0xa9: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x98: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x38: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_and(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x78: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x58: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x18: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_or(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0xb8: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x7a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  op_io();
  rd |= op_readdp(dp + 1) << 8;
  regs.ya = op_addw(regs.ya, rd);
  break;
}

case 0x9a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  op_io();
  rd |= op_readdp(dp + 1) << 8;
  regs.ya = op_subw(regs.ya, rd);
  break;
}

case 0x5a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  rd |= op_readdp(dp + 1) << 8;
  op_cmpw(regs.ya, rd);
  break;
}

case 0x4a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  regs.p.c = regs.p.c & !!(rd & (1 << bit));
  break;
}

case 0x6a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  regs.p.c = regs.p.c & !(rd & (1 << bit));
  break;
}

case 0x8a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c ^ !!(rd & (1 << bit));
  break;
}

case 0xea: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  rd ^= (1 << bit);
  op_writeaddr(dp, rd);
  break;
}

case 0x0a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c | !!(rd & (1 << bit));
  break;
}

case 0x2a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c | !(rd & (1 << bit));
  break;
}

/* RMW */
case 0xbc: {
  op_io();
  regs.B.a = op_inc(regs.B.a);
  break;
}

case 0x3d: {
  op_io();
  regs.x = op_inc(regs.x);
  break;
}

case 0xfc: {
  op_io();
  regs.B.y = op_inc(regs.B.y);
  break;
}

case 0x9c: {
  op_io();
  regs.B.a = op_dec(regs.B.a);
  break;
}

case 0x1d: {
  op_io();
  regs.x = op_dec(regs.x);
  break;
}

case 0xdc: {
  op_io();
  regs.B.y = op_dec(regs.B.y);
  break;
}

case 0x1c: {
  op_io();
  regs.B.a = op_asl(regs.B.a);
  break;
}

case 0x5c: {
  op_io();
  regs.B.a = op_lsr(regs.B.a);
  break;
}

case 0x3c: {
  op_io();
  regs.B.a = op_rol(regs.B.a);
  break;
}

case 0x7c: {
  op_io();
  regs.B.a = op_ror(regs.B.a);
    break;
}

case 0xab: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_inc(rd);
  op_writedp(dp, rd);
  break;
}

case 0x8b: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_dec(rd);
  op_writedp(dp, rd);
  break;
}

case 0x0b: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_asl(rd);
  op_writedp(dp, rd);
  break;
}

case 0x4b: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_lsr(rd);
  op_writedp(dp, rd);
  break;
}

case 0x2b: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_rol(rd);
  op_writedp(dp, rd);
  break;
}

case 0x6b: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = op_ror(rd);
  op_writedp(dp, rd);
  break;
}

case 0xbb: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_inc(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0x9b: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_dec(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0x1b: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_asl(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0x5b: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_lsr(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0x3b: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_rol(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0x7b: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = op_ror(rd);
  op_writedp(dp + regs.x, rd);
  break;
}

case 0xac: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_inc(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x8c: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_dec(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x0c: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_asl(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x4c: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_lsr(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x2c: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_rol(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x6c: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  rd = op_ror(rd);
  op_writeaddr(dp, rd);
  break;
}

case 0x0e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.p.n = !!((regs.B.a - rd) & 0x80);
  regs.p.z = ((regs.B.a - rd) == 0);
  op_readaddr(dp);
  op_writeaddr(dp, rd | regs.B.a);
  break;
}

case 0x4e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.p.n = !!((regs.B.a - rd) & 0x80);
  regs.p.z = ((regs.B.a - rd) == 0);
  op_readaddr(dp);
  op_writeaddr(dp, rd &~ regs.B.a);
  break;
}

case 0x3a: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd++;
  op_writedp(dp++, rd);
  rd += op_readdp(dp) << 8;
  op_writedp(dp, rd >> 8);
  regs.p.n = !!(rd & 0x8000);
  regs.p.z = (rd == 0);
  break;
}

case 0x1a: {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd--;
  op_writedp(dp++, rd);
  rd += op_readdp(dp) << 8;
  op_writedp(dp, rd >> 8);
  regs.p.n = !!(rd & 0x8000);
  regs.p.z = (rd == 0);
  break;
}

  }
}

#ifdef SMP_CPP

//this is the IPLROM for the S-SMP coprocessor.
//the S-SMP does not allow writing to the IPLROM.
//all writes are instead mapped to the extended
//RAM region, accessible when $f1.d7 is clear.

const uint8 SMP::iplrom[64] = {
/*ffc0*/  0xcd, 0xef,        //mov   x,#$ef
/*ffc2*/  0xbd,              //mov   sp,x
/*ffc3*/  0xe8, 0x00,        //mov   a,#$00
/*ffc5*/  0xc6,              //mov   (x),a
/*ffc6*/  0x1d,              //dec   x
/*ffc7*/  0xd0, 0xfc,        //bne   $ffc5
/*ffc9*/  0x8f, 0xaa, 0xf4,  //mov   $f4,#$aa
/*ffcc*/  0x8f, 0xbb, 0xf5,  //mov   $f5,#$bb
/*ffcf*/  0x78, 0xcc, 0xf4,  //cmp   $f4,#$cc
/*ffd2*/  0xd0, 0xfb,        //bne   $ffcf
/*ffd4*/  0x2f, 0x19,        //bra   $ffef
/*ffd6*/  0xeb, 0xf4,        //mov   y,$f4
/*ffd8*/  0xd0, 0xfc,        //bne   $ffd6
/*ffda*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffdc*/  0xd0, 0x0b,        //bne   $ffe9
/*ffde*/  0xe4, 0xf5,        //mov   a,$f5
/*ffe0*/  0xcb, 0xf4,        //mov   $f4,y
/*ffe2*/  0xd7, 0x00,        //mov   ($00)+y,a
/*ffe4*/  0xfc,              //inc   y
/*ffe5*/  0xd0, 0xf3,        //bne   $ffda
/*ffe7*/  0xab, 0x01,        //inc   $01
/*ffe9*/  0x10, 0xef,        //bpl   $ffda
/*ffeb*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffed*/  0x10, 0xeb,        //bpl   $ffda
/*ffef*/  0xba, 0xf6,        //movw  ya,$f6
/*fff1*/  0xda, 0x00,        //movw  $00,ya
/*fff3*/  0xba, 0xf4,        //movw  ya,$f4
/*fff5*/  0xc4, 0xf4,        //mov   $f4,a
/*fff7*/  0xdd,              //mov   a,y
/*fff8*/  0x5d,              //mov   x,a
/*fff9*/  0xd0, 0xdb,        //bne   $ffd6
/*fffb*/  0x1f, 0x00, 0x00,  //jmp   ($0000+x)
/*fffe*/  0xc0, 0xff         //reset vector location ($ffc0)
};

#endif

unsigned SMP::port_read(unsigned addr) {
  return apuram[0xf4 + (addr & 3)];
}

void SMP::port_write(unsigned addr, unsigned data) {
  apuram[0xf4 + (addr & 3)] = data;
}

unsigned SMP::mmio_read(unsigned addr) {
  switch(addr) {

  case 0xf2:
    return status.dsp_addr;

  case 0xf3:
    return dsp.read(status.dsp_addr & 0x7f);

  case 0xf4:
  case 0xf5:
  case 0xf6:
  case 0xf7:
    return cpu.port_read(addr);

  case 0xf8:
    return status.ram00f8;

  case 0xf9:
    return status.ram00f9;

  case 0xfd: {
    unsigned result = timer0.stage3_ticks & 15;
    timer0.stage3_ticks = 0;
    return result;
  }

  case 0xfe: {
    unsigned result = timer1.stage3_ticks & 15;
    timer1.stage3_ticks = 0;
    return result;
  }

  case 0xff: {
    unsigned result = timer2.stage3_ticks & 15;
    timer2.stage3_ticks = 0;
    return result;
  }

  }

  return 0x00;
}

void SMP::mmio_write(unsigned addr, unsigned data) {
  switch(addr) {

  case 0xf1:
    status.iplrom_enable = data & 0x80;

    if(data & 0x30) {
      if(data & 0x20) {
        cpu.port_write(3, 0x00);
        cpu.port_write(2, 0x00);
      }
      if(data & 0x10) {
        cpu.port_write(1, 0x00);
        cpu.port_write(0, 0x00);
      }
    }

    if(timer2.enable == false && (data & 0x04)) {
      timer2.stage2_ticks = 0;
      timer2.stage3_ticks = 0;
    }
    timer2.enable = data & 0x04;

    if(timer1.enable == false && (data & 0x02)) {
      timer1.stage2_ticks = 0;
      timer1.stage3_ticks = 0;
    }
    timer1.enable = data & 0x02;

    if(timer0.enable == false && (data & 0x01)) {
      timer0.stage2_ticks = 0;
      timer0.stage3_ticks = 0;
    }
    timer0.enable = data & 0x01;

    break;

  case 0xf2:
    status.dsp_addr = data;
    break;

  case 0xf3:
    if(status.dsp_addr & 0x80) break;
    dsp.write(status.dsp_addr, data);
    break;

  case 0xf4:
  case 0xf5:
  case 0xf6:
  case 0xf7:
    port_write(addr, data);
    break;

  case 0xf8:
    status.ram00f8 = data;
    break;

  case 0xf9:
    status.ram00f9 = data;
    break;

  case 0xfa:
    timer0.target = data;
    break;

  case 0xfb:
    timer1.target = data;
    break;

  case 0xfc:
    timer2.target = data;
    break;
  }
}


template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick() {
  if(++stage1_ticks < cycle_frequency) return;

  stage1_ticks = 0;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}

template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick(unsigned clocks) {
  stage1_ticks += clocks;
  if(stage1_ticks < cycle_frequency) return;

  stage1_ticks -= cycle_frequency;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}

void SMP::enter() {
  while(clock < 0) op_step();
}

void SMP::power() {
  Processor::clock = 0;

  timer0.target = 0;
  timer1.target = 0;
  timer2.target = 0;

  reset();
}

void SMP::reset() {
  for(unsigned n = 0x0000; n <= 0xffff; n++) apuram[n] = 0x00;

  opcode_number = 0;
  opcode_cycle = 0;

  regs.pc = 0xffc0;
  regs.sp = 0xef;
  regs.B.a = 0x00;
  regs.x = 0x00;
  regs.B.y = 0x00;
  regs.p = 0x02;

  //$00f1
  status.iplrom_enable = true;

  //$00f2
  status.dsp_addr = 0x00;

  //$00f8,$00f9
  status.ram00f8 = 0x00;
  status.ram00f9 = 0x00;

  //timers
  timer0.enable = timer1.enable = timer2.enable = false;
  timer0.stage1_ticks = timer1.stage1_ticks = timer2.stage1_ticks = 0;
  timer0.stage2_ticks = timer1.stage2_ticks = timer2.stage2_ticks = 0;
  timer0.stage3_ticks = timer1.stage3_ticks = timer2.stage3_ticks = 0;
}

SMP::SMP() {
  apuram = new uint8[64 * 1024];
}

SMP::~SMP() {
	delete[] apuram;
}

}
