#include "app_config.h"
#include "asm/charge.h"
#include "asm/pwm_led.h"
#include "system/event.h"
#include "system/app_core.h"
#include "system/includes.h"
#include "app_action.h"
#include "asm/wdt.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "7PIN_led8888.h"

#define LOG_TAG_CONST       APP_CHARGE
#define LOG_TAG             "[APP_CHARGE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_CHARGE_ENABLE
// disconnet BT if it be connected,close hall power, disanble keyevent ,close display and open ui for charge
static u16 charge_ui_timer_id ;
extern void ble_module_enable(u8 en);

void update_charge_ui()
{
    static var=1;
    DISPLAY_STATE_DATA_UPDATE(BATT_REPORT, var);
    if(++var >4)
        var=1;
}
#define JIMMYADD 1

void ui_charger_start()
{
    // close display (all date to be set 0)
   // DISPLAY_NUM_DATA_UPDATE(0, 0xff);
    // disconnet BT if it be connected
#if TCFG_USER_BLE_ENABLE
    ble_module_enable(0);
#endif

#if(TCFG_USER_EDR_ENABLE && TRANS_DATA_EN)
    bt_wait_phone_connect_control_ext(0, 0);
    transport_spp_disconnect();
#endif
    //close hall power
    //gpio_direction_output(TCFG_HALL_POWER_PORT,0) ;
    //open ui for charge
   extern  void time_down_over_deal();
   time_down_over_deal();
   sys_key_event_disable();
    
	DISPLAY_NUM_DATA_UPDATE(0, 0xff);
	DISPLAY_STATE_DATA_UPDATE(BATT_REPORT, 01);
    charge_ui_timer_id= sys_hi_timer_add(NULL, update_charge_ui, 400); // 每秒更新一次

	
    log_info("%s\n", __func__);
}

void charge_start_deal(void)
{
    log_info("%s\n", __func__);

    power_set_mode(PWR_LDO15);
    // disconnet BT if it be connected,close hall power, disanble keyevent ,close display and open ui for charge
#if JIMMYADD
    led_8888_init();

    ui_charger_start();
#endif
}

void ldo5v_keep_deal(void)
{
    log_info("%s\n", __func__);
}

void charge_full_deal(void)
{
    log_info("%s\n", __func__);
    charge_close();
#if JIMMYADD
    DISPLAY_STATE_DATA_UPDATE(BATT_REPORT, 03);
    if(charge_ui_timer_id)
        sys_hi_timeout_del(charge_ui_timer_id);
    charge_ui_timer_id=0;
#endif
}

void charge_close_deal(void)
{
    log_info("%s\n", __FUNCTION__);
	#if JIMMYADD
   // DISPLAY_STATE_DATA_UPDATE(BATT_REPORT, 03);
    if(charge_ui_timer_id)
        sys_hi_timeout_del(charge_ui_timer_id);
    charge_ui_timer_id=0;
	DISPLAY_NUM_DATA_UPDATE(0, 0xff);
#endif
	
}
extern void app_switch(const char *name, int action);
extern void app_set_soft_poweroff(void);

void charge_ldo5v_in_deal(void)
{
    log_info("%s\n", __FUNCTION__);
   
	 struct application *app;
     charge_start();
	/* app = get_current_app();
    if (app) {
        r_printf("in %s app",app->name);
		if(!strcmp(app->name, "idle")){
          #if TCFG_USER_BLE_ENABLE
            ble_module_enable(0);
          #endif
		  os_time_dly(10);
		  cpu_reset();
		}
		
    }*/
	
	
}

void charge_ldo5v_off_deal(void)
{
    log_info("%s\n", __FUNCTION__);
    charge_close();
    power_set_mode(TCFG_LOWPOWER_POWER_SEL);
	sys_key_event_enable();
#if TCFG_SYS_LVD_EN
    vbat_check_init();
#endif
   struct application *app;
    app = get_current_app();
    if (app) {
        r_printf("in %s app",app->name);
		if(!strcmp(app->name, "idle")){
            power_set_soft_poweroff();
		}
    }
}


int app_charge_event_handler(struct device_event *dev)
{
    switch (dev->event) {
    case CHARGE_EVENT_CHARGE_START:
        charge_start_deal();
        break;
    case CHARGE_EVENT_CHARGE_CLOSE:
        charge_close_deal();
        break;
    case CHARGE_EVENT_CHARGE_FULL:
        charge_full_deal();
        break;
    case CHARGE_EVENT_LDO5V_KEEP:
        ldo5v_keep_deal();
        break;
    case CHARGE_EVENT_LDO5V_IN:
        charge_ldo5v_in_deal();
        break;
    case CHARGE_EVENT_LDO5V_OFF:
        charge_ldo5v_off_deal();
        break;
    default:
        break;
    }
    return 0;
}

#endif
