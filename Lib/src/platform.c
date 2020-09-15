/** @file Platform specific initialization routines.
 */

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
 * Copyright (C) 2020 Robert Buhren <robert.buhren@sect.tu-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <smn-map.h>
#include <x86-map.h>
#include <platform.h>

static void pspSOCInit(void)
{
    pspX86MmioWriteU32(0xfffe000a3048, 0x0020ff00);
    pspX86MmioWriteU32(0xfffe000a30d0, 0x08fdff86);
    pspX86MmioWriteU8(0xfed81e77, 0x27);
    pspX86MmioWriteU32(0xfec20040, 0x0);
    pspX86MmioWriteU32(0xfffe000a3044, 0xc0);
    pspX86MmioWriteU32(0xfffe000a3048, 0x20ff07);
    pspX86MmioWriteU32(0xfffe000a3064, 0x1640);
    pspX86MmioWriteU32(0xfffe000a3000, 0xffffff00);
    pspX86MmioWriteU32(0xfffe000a30a0, 0xfec10002);
    pspX86MmioWriteU32(0xfed80300,     0xe3020b11);
    pspX86MmioWriteU8(0xfffdfc000072, 0x6);
    pspX86MmioWriteU8(0xfffdfc000072, 0x7);
    pspSmnWrU32(0x2dc58d0, 0x0c7c17cf);
    pspX86MmioWriteU32(0xfffe000a3044, 0xc0);
    pspX86MmioWriteU32(0xfffe000a3048, 0x20ff07);
    pspX86MmioWriteU32(0xfffe000a3064, 0x1640);
}

static void pspSuperIoInit_PrimeX370(void)
{
    pspX86MmioWriteU8(0xfffdfc00002e, 0x87);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x24);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x00);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x10);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x87);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x23);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x40);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x40);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x61);
    pspX86MmioWriteU8(0xfffdfc00002f, 0xf8);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x60);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x03);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x30);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x02);
}

static void pspSuperIoInit_H11DSU_iN(void)
{
    pspX86MmioWriteU8(0xfffdfc00002e, 0xa5);
    pspX86MmioWriteU8(0xfffdfc00002e, 0xa5);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x7);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x2);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x61);
    pspX86MmioWriteU8(0xfffdfc00002f, 0xf8);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x60);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x3);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x30);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x1);
    pspX86MmioWriteU8(0xfffdfc00002e, 0xaa);
}

void pspPlatformInit(void)
{
    pspSOCInit();
#if TARGET == TARGET_PRIME_X370
    pspSuperIoInit_PrimeX370();
#elif TARGET == TARGET_H11DSU_IN
    pspSuperIoInit_H11DSU_iN();
#else
#error "Invalid target platform"
#endif

}
