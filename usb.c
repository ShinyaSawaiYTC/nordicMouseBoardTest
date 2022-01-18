#include "usb.h"

#include "nrf_log.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_string_desc.h"

#define APP_USBD_INTERFACE_MOUSE 0
#define CONFIG_MOUSE_BUTTON_COUNT 5

uint8_t g_my_usbd_manufacturer[APP_USBD_CONFIG_DESC_STRING_SIZE];
uint8_t g_my_usbd_serial[APP_USBD_CONFIG_DESC_STRING_SIZE];
uint8_t g_my_usbd_product[APP_USBD_CONFIG_DESC_STRING_SIZE];
uint8_t g_my_usbd_configration[APP_USBD_CONFIG_DESC_STRING_SIZE];

/*** 
 * SDKのバグでAPP_USBD_STRINGS_CONFIGURATIONを外部定義できない。
 * 
 * app_usbd_string_desc.c
 * 誤 : extern uint8_t APP_USBD_STRING_CONFIGURATION[]; 
 * 正 : extern uint8_t APP_USBD_STRINGS_CONFIGURATION[]; 
 * 
 * _STRING"S"_に修正
 ***/

app_usbd_hid_mouse_t *usb_mouse_handler;

static void hid_mouse_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                      app_usbd_hid_user_event_t event)
{
    UNUSED_PARAMETER(p_inst);
    switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
            /* No output report defined for HID mouse.*/
            ASSERT(0);
            break;
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
            // bsp_board_led_invert(LED_HID_REP);
            break;
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
            UNUSED_RETURN_VALUE(hid_mouse_clear_buffer(p_inst));
            break;
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
            UNUSED_RETURN_VALUE(hid_mouse_clear_buffer(p_inst));
            break;
        default:
            break;
    }
}

APP_USBD_HID_MOUSE_GLOBAL_DEF(m_app_hid_mouse,
                              APP_USBD_INTERFACE_MOUSE,
                              NRF_DRV_USBD_EPIN1,
                              CONFIG_MOUSE_BUTTON_COUNT,
                              hid_mouse_user_ev_handler,
                              APP_USBD_HID_SUBCLASS_BOOT
);
static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SOF://SOF:Start of frame
            break;
        case APP_USBD_EVT_DRV_SUSPEND://バス休止状態
            app_usbd_suspend_req();
            //スリープモードに入る処理
            break;
        case APP_USBD_EVT_DRV_RESUME://Suspendから復帰
            // bsp_board_led_on(LED_USB_START);
            // kbd_status(); /* Restore LED state - during SUSPEND all LEDS are turned off */
            break;
        case APP_USBD_EVT_STARTED:
            // bsp_board_led_on(LED_USB_START);
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_DEBUG("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_DEBUG("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_DEBUG("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

void usb_init(){
    strcpy(g_my_usbd_manufacturer,APP_USBD_STRING_DESC("elecom test"));
    strcpy(g_my_usbd_product,APP_USBD_STRING_DESC("nRF52 ARMA PROT"));
    // strcpy(g_my_usbd_serial,APP_USBD_STRING_DESC("serial number test"));
    strcpy(g_my_usbd_configration,APP_USBD_STRING_DESC("configuration string test"));

    APP_ERROR_CHECK(nrf_drv_clock_init());

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler,
    };
    APP_ERROR_CHECK(app_usbd_init(&usbd_config));

    app_usbd_class_inst_t const * class_inst_mouse;
    class_inst_mouse = app_usbd_hid_mouse_class_inst_get(&m_app_hid_mouse);
    APP_ERROR_CHECK(app_usbd_class_append(class_inst_mouse));//マウスクラス追加
    APP_ERROR_CHECK(app_usbd_power_events_enable());
    usb_mouse_handler = &m_app_hid_mouse;
    
}
void usb_in_loop(){
    while (app_usbd_event_queue_process());
}