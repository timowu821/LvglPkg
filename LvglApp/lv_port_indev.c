/**
 * @file lv_port_indev.c
 *
 */


/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"

#include "lvgl/src/indev/lv_indev_private.h"

/*********************
 *      DEFINES
 *********************/
BOOLEAN  mEscExit = FALSE;

extern const lv_img_dsc_t mouse_cursor_icon;

typedef struct {
  EFI_SIMPLE_POINTER_PROTOCOL    *SimplePointer;
  EFI_ABSOLUTE_POINTER_PROTOCOL  *AbsPointer;
  UINTN                          LastCursorX;
  UINTN                          LastCursorY;
  UINT32                         ActiveButtons;
  BOOLEAN                        LeftButton;
  BOOLEAN                        RightButton;
  EFI_EVENT                      MouseEvent;
} LVGL_UEFI_MOUSE;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);

static void keypad_read(lv_indev_t * indev, lv_indev_data_t * data);


/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_mouse;
lv_indev_t * indev_keypad;

LVGL_UEFI_MOUSE *mLvglUefiMouse = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void keypad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  ReadKey;

  Status =  gST->ConIn->ReadKeyStroke (gST->ConIn, &ReadKey);
  if (!EFI_ERROR (Status)) {
    switch (ReadKey.UnicodeChar) {
      case CHAR_CARRIAGE_RETURN:
        data->key = LV_KEY_ENTER;
        break;

      case CHAR_BACKSPACE:
        data->key = LV_KEY_BACKSPACE;
        break;

      case CHAR_NULL:
        switch (ReadKey.ScanCode) {
        case SCAN_UP:
          data->key = LV_KEY_UP;
          break;
        
        case SCAN_DOWN:
          data->key = LV_KEY_DOWN;
          break;

        case SCAN_RIGHT:
          data->key = LV_KEY_RIGHT;
          break;

        case SCAN_LEFT:
          data->key = LV_KEY_LEFT;
          break;

        case SCAN_ESC:
          data->key = LV_KEY_ESC;
          mEscExit = TRUE;
          break;

        case SCAN_DELETE:
          data->key = LV_KEY_DEL;
          break;

        case SCAN_PAGE_DOWN:
          data->key = LV_KEY_NEXT;
          break;

        case SCAN_PAGE_UP:
          data->key = LV_KEY_PREV;
          break;

        case SCAN_HOME:
          data->key = LV_KEY_HOME;
          break;

        case SCAN_END:
          data->key = LV_KEY_END;
          break;

        default:
          break;
        }
        break;

      case CHAR_LINEFEED:
      case CHAR_TAB:
        break;

      default:
        data->key = ReadKey.UnicodeChar;
        break;
    }

    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

}


lv_indev_t * lv_uefi_keyboard_create(void)
{
    lv_indev_t * indev = lv_indev_create();
    LV_ASSERT_MALLOC(indev);
    if(indev == NULL) {
        return NULL;
    }

    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keypad_read);

    return indev;
}


EFI_STATUS
EFIAPI
GetXY (
  IN  EFI_EVENT    Event,
  IN  VOID         *Context
  )
{
  EFI_STATUS                     Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL  *AbsPointer = NULL;
  EFI_ABSOLUTE_POINTER_STATE     AbsState;

  if (mLvglUefiMouse->AbsPointer != NULL && mLvglUefiMouse->MouseEvent != NULL) {
    AbsPointer = mLvglUefiMouse->AbsPointer;
    Status = gBS->CheckEvent (AbsPointer->WaitForInput);
    if (EFI_ERROR (Status)) {
      return EFI_NOT_READY;
    }
    Status = AbsPointer->GetState (AbsPointer, &AbsState);
    if (!EFI_ERROR (Status)) {
      if (AbsState.CurrentX != mLvglUefiMouse->LastCursorX && AbsState.CurrentY != mLvglUefiMouse->LastCursorY) {
        mLvglUefiMouse->LastCursorX = AbsState.CurrentX;
        mLvglUefiMouse->LastCursorY = AbsState.CurrentY;
      }
      mLvglUefiMouse->LeftButton = AbsState.ActiveButtons & BIT0;

      return EFI_SUCCESS;
    } else {
      return EFI_NOT_READY;
    }
  }

  return EFI_NOT_READY;
}


EFI_STATUS
EFIAPI
EfiMouseInit (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL  *AbsPointer = NULL;

  Status = gBS->LocateProtocol(&gEfiAbsolutePointerProtocolGuid, NULL, (VOID **) &AbsPointer);

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  DebugPrint (DEBUG_INFO, "EfiMouseInit()\n");

  if (AbsPointer != NULL) {
    AbsPointer->Reset(AbsPointer, TRUE);
  }

  mLvglUefiMouse->AbsPointer = AbsPointer;
  mLvglUefiMouse->MouseEvent = NULL;
  mLvglUefiMouse->LastCursorX = 0;
  mLvglUefiMouse->LastCursorY = 0;
  mLvglUefiMouse->ActiveButtons = 0;
  mLvglUefiMouse->LeftButton = FALSE;

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  (EFI_EVENT_NOTIFY) GetXY,
                  NULL,
                  &mLvglUefiMouse->MouseEvent
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->SetTimer(mLvglUefiMouse->MouseEvent, TimerPeriodic, 100 * 1000);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}


static void mouse_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{

  lv_display_t *disp = lv_indev_get_display(indev_drv);

  int32_t hor_res = lv_display_get_horizontal_resolution(disp);
  int32_t ver_res = lv_display_get_vertical_resolution(disp);

  data->point.x = (mLvglUefiMouse->LastCursorX * hor_res) / (0xFFFF + 1);
  data->point.y = (mLvglUefiMouse->LastCursorY * ver_res) / (0xFFFF + 1);
  if (mLvglUefiMouse->LeftButton) {
    data->state = LV_INDEV_STATE_PRESSED;
    mLvglUefiMouse->LeftButton = FALSE;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

}


void lv_indev_set_cusor_start(lv_indev_t * indev)
{
    if(indev == NULL) return;

    lv_display_t *disp = lv_indev_get_display(indev);

    int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    int32_t ver_res = lv_display_get_vertical_resolution(disp);

    indev->pointer.act_point.x = hor_res / 2;
    indev->pointer.act_point.y = ver_res / 2;
    mLvglUefiMouse->LastCursorX = hor_res / 2;
    mLvglUefiMouse->LastCursorY = ver_res / 2;
    mLvglUefiMouse->ActiveButtons = 0;
    mLvglUefiMouse->LeftButton = FALSE;
}

lv_indev_t * lv_uefi_mouse_create(lv_display_t * disp)
{
    lv_indev_t * indev = lv_indev_create();
    LV_ASSERT_MALLOC(indev);
    if(indev == NULL) {
      return NULL;
    }

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, mouse_read);
    lv_indev_set_display (indev, disp);

    LV_IMG_DECLARE(mouse_cursor_icon);
    lv_obj_t * mouse_cursor = lv_image_create(lv_screen_active());
    lv_image_set_src(mouse_cursor, &mouse_cursor_icon);
    // lv_indev_set_cusor_start(indev);
    lv_indev_set_cursor(indev, mouse_cursor);

    return indev;
}



void lv_port_indev_init(lv_display_t * disp)
{

    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your mouse if you have*/
    if (EfiMouseInit() == EFI_SUCCESS) {
      DebugPrint (DEBUG_INFO, "Create Mouse\n");
      lv_uefi_mouse_create(disp);
    }


    /*------------------
     * Keypad
     * -----------------*/
    lv_uefi_keyboard_create();

}

void lv_port_indev_close()
{

    if (mLvglUefiMouse->MouseEvent != NULL) {
      gBS->CloseEvent (mLvglUefiMouse->MouseEvent);
    }
}

