/** @file
 * X86 host memory related APIs.
 */

/*
 * Copyright (C) 2019 Alexander Eichner <aeichner@aeichner.de>
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
#ifndef ___x86mem_h
#define ___x86mem_h

#include <types.h>

/**
 * Fallback method to copy memory from the x86 host using svc 0x26 to copy 4 bytes at a time.
 */
int psp_x86_memory_copy_from_host_fallback(X86PADDR PhysX86AddrSrc, void *pvDst, size_t cbCopy);

#endif /* ___x86mem_h */
