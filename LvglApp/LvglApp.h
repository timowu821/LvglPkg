

#ifndef __LVGL_DXE_H__
#define __LVGL_DXE_H__

#include "lvgl/lvgl.h"

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiApplicationEntryPoint.h>

VOID
EFIAPI
TestDemo (
  VOID
  );

#endif
