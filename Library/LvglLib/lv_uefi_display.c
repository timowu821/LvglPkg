#include "LvglLibCommon.h"


typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *EfiGop;
    uint8_t                      *buffer[2];
} uefi_disp_data_t;


static void uefi_disp_delete_evt_cb(lv_event_t * e)
{
    lv_display_t * disp = lv_event_get_user_data(e);
    uefi_disp_data_t * uefi_disp_data = lv_display_get_driver_data(disp);

    free(uefi_disp_data->buffer[0]);
    free(uefi_disp_data->buffer[1]);

    lv_free(uefi_disp_data);
}


void uefi_disp_flush(lv_display_t * disp, const lv_area_t * area, lv_color32_t * color32_p)
{
  EFI_STATUS                         Status;
  UINTN                              Width, Heigth;
  uefi_disp_data_t                   *uefi_disp_data;
  UINTN                              Delta;

  uefi_disp_data = lv_display_get_driver_data(disp);

  Width = area->x2 - area->x1 + 1;
  Heigth = area->y2 - area->y1 + 1;
  Delta = uefi_disp_data->EfiGop->Mode->Info->HorizontalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  Status = uefi_disp_data->EfiGop->Blt (
                                     uefi_disp_data->EfiGop,
                                     (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)color32_p,
                                     EfiBltBufferToVideo,
                                     (UINTN)area->x1,
                                     (UINTN)area->y1,
                                     (UINTN)area->x1,
                                     (UINTN)area->y1,
                                     Width,
                                     Heigth,
                                     Delta
                                     );

  lv_display_flush_ready(disp);
}


lv_display_t * lv_uefi_disp_create(int32_t hor_res, int32_t ver_res)
{
    EFI_STATUS                    Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

    Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &GraphicsOutput);
    if (EFI_ERROR(Status)) {
        return NULL;
    }

    uefi_disp_data_t * uefi_disp_data = lv_malloc_zeroed(sizeof(uefi_disp_data_t));
    LV_ASSERT_MALLOC(uefi_disp_data);
    if(NULL == uefi_disp_data) return NULL;

    uefi_disp_data->EfiGop = GraphicsOutput;

    lv_display_t * disp = lv_display_create(hor_res, ver_res);
    if(NULL == disp) {
        lv_free(uefi_disp_data);
        return NULL;
    }
    lv_display_set_driver_data(disp, uefi_disp_data);
    lv_display_set_flush_cb(disp, (lv_display_flush_cb_t)uefi_disp_flush);
    lv_display_add_event_cb(disp, uefi_disp_delete_evt_cb, LV_EVENT_DELETE, disp);

    UINTN BufSize = hor_res * ver_res * sizeof (lv_color32_t);
    uefi_disp_data->buffer[0] = malloc (BufSize);
    uefi_disp_data->buffer[1] = malloc (BufSize);

    lv_display_set_buffers(disp, uefi_disp_data->buffer[0], uefi_disp_data->buffer[1], BufSize, LV_DISPLAY_RENDER_MODE_DIRECT);

    return disp;
}