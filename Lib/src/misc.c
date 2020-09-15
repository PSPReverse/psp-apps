/** @file Miscellaneous functions.
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
#include <mmio.h>
#include <smn-map.h>

uint32_t pspGetPhysDieId(void)
{
  void *pvMap = NULL;
  int rc = pspSmnMap(0x5a078, &pvMap);
  if (!rc)
  {
    uint32_t uVal;
    pspMmioAccess(&uVal, (void *)pvMap, sizeof(uint32_t));
    pspSmnUnmapByPtr(pvMap);
    return uVal & 0x3;
  }

  return 0xffffffff;
}
