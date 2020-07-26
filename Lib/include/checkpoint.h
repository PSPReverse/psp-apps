/** @file
 * PSP internal interfaces - Checkpoint API.
 */

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
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
#ifndef INCLUDED_Lib_include_checkpoint_h
#define INCLUDED_Lib_include_checkpoint_h

#if defined(IN_PSP)
# include <common/types.h>
#else
# error "Invalid environment"
#endif

/** PSP checkpoint. */
typedef union PSPCHCKPT
{
    /** Array view. */
    uint32_t                        au32Regs[17];
    /** Field view. */
    struct
    {
        uint32_t                    u32R0;
        uint32_t                    u32R1;
        uint32_t                    u32R2;
        uint32_t                    u32R3;
        uint32_t                    u32R4;
        uint32_t                    u32R5;
        uint32_t                    u32R6;
        uint32_t                    u32R7;
        uint32_t                    u32R8;
        uint32_t                    u32R9;
        uint32_t                    u32R10;
        uint32_t                    u32R11;
        uint32_t                    u32R12;
        uint32_t                    u32SP;
        uint32_t                    u32LR;
        uint32_t                    u32PC;
        uint32_t                    u32Cpsr;
    } Regs;
} PSPCHCKPT;
/** Pointer to a checkpoint. */
typedef PSPCHCKPT *PPSPCHCKPT;
/** Pointer to a const checkpoint. */
typedef const PSPCHCKPT *PCPSPCHCKPT;


/**
 * Sets an execution checkpoint.
 *
 * @returns true if the checkpoint was set, false if we restarted from the previously set checkpoint.
 * @param   pChkPt                  Where to store the checkpoint state.
 */
bool PSPCheckPointSet(PPSPCHCKPT pChkPt);

#endif /* !INCLUDED_Lib_include_checkpoint_h */

