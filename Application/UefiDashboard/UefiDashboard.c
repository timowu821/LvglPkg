/** @file
  This sample application bases on LvglLib.

  Copyright (c) 2025, Yang Gang. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UefiDashboardCommon.h"

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t * g;
static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;
static lv_obj_t * t3;


static void BiosInfoCreate(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    UefiBasicInfo (parent);
}


static void BootManagerCreate(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    UefiBootOptions (parent);
}

static void UefiTimeCreate(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    UefiTime (parent);
}


VOID
EFIAPI
LvglUefiDemoSetup (
  VOID
  )
{
    g = lv_group_create();
    lv_group_set_default(g);

    lv_indev_t * indev = NULL;
    for(;;) {
        indev = lv_indev_get_next(indev);
        if(!indev) {
            break;
        }

        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if(indev_type == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(indev, g);
        }
    }

    tv = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_position(tv, LV_DIR_LEFT);

    lv_obj_set_style_bg_color(tv, lv_palette_lighten(LV_PALETTE_RED, 2), 0);

    lv_obj_t * tab_buttons = lv_tabview_get_tab_bar(tv);
    lv_obj_set_style_bg_color(tab_buttons, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_buttons, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);

    t1 = lv_tabview_add_tab(tv, "BIOS Info");
    lv_obj_set_style_text_font(tv, &lv_font_montserrat_20, 0);

    t2 = lv_tabview_add_tab(tv, "Time&Date");
    lv_obj_set_style_text_font(tv, &lv_font_montserrat_20, 0);

    t3 = lv_tabview_add_tab(tv, "Boot Manager");
    lv_obj_set_style_text_font(tv, &lv_font_montserrat_20, 0);

    lv_tabview_set_tab_bar_size(tv, 200);

    BiosInfoCreate (t1);
    UefiTimeCreate (t2);
    BootManagerCreate (t3);
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI EfiImageLogo.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  if (UefiLvglInit() == EFI_SUCCESS) {
    Status = UefiLvglAppRegister (LvglUefiDemoSetup);

    if (!EFI_ERROR (Status)) {
      UefiLvglDeinit();
    }
  }

  return EFI_SUCCESS;
}
