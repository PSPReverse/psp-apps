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

#include <err.h>
#include <x86mem.h>
#include <svc.h>


int psp_x86_memory_copy_from_host_fallback(X86PADDR PhysX86AddrSrc, void *pvDst, size_t cbCopy)
{
    int rc = INF_SUCCESS;
    size_t cbLeft = cbCopy;
    uint8_t *pbDst = (uint8_t *)pvDst;
    PSPX86MEMCOPYREQ Req;

    while (cbLeft >= 4)
    {
        Req.PhysX86AddrSrc = PhysX86AddrSrc;
        Req.pvDst          = pbDst;
        Req.cbCopy         = 4;
        Req.enmMemType     = PSP_X86_MEM_TYPE_UNKNOWN_4;

        rc = svc_x86_host_memory_copy_to_psp(&Req);
        if (rc != PSPSTATUS_SUCCESS)
            break;

        pbDst          += 4;
        cbLeft         -= 4;
        PhysX86AddrSrc += 4;
    }

    if (   rc == PSPSTATUS_SUCCESS
        && cbLeft)
    {
        while (cbLeft)
        {
            Req.PhysX86AddrSrc = PhysX86AddrSrc;
            Req.pvDst          = pbDst;
            Req.cbCopy         = 1;
            Req.enmMemType     = PSP_X86_MEM_TYPE_UNKNOWN_4;

            rc = svc_x86_host_memory_copy_to_psp(&Req);
            if (rc != PSPSTATUS_SUCCESS)
                break;

            pbDst          += 1;
            cbLeft         -= 1;
            PhysX86AddrSrc += 1;
        }
    }

    return rc;
}


int psp_x86_mmio_read(X86PADDR PhysX86AddrSrc, void *pvDst, size_t cbRead)
{
    PSPX86MEMCOPYREQ Req;
    Req.PhysX86AddrSrc = PhysX86AddrSrc;
    Req.pvDst          = pvDst;
    Req.cbCopy         = cbRead;
    Req.enmMemType     = PSP_X86_MEM_TYPE_UNKNOWN_6;

    return svc_x86_host_memory_copy_to_psp(&Req);
}


int psp_x86_mmio_write(X86PADDR PhysX86AddrDst, const void *pvSrc, size_t cbWrite)
{
    PSPX86MEMCOPYREQ Req;
    Req.PhysX86AddrSrc = PhysX86AddrDst;
    Req.pvDst          = (void *)pvSrc;
    Req.cbCopy         = cbWrite;
    Req.enmMemType     = PSP_X86_MEM_TYPE_UNKNOWN_6;

    return svc_x86_host_memory_copy_from_psp(&Req);
}

