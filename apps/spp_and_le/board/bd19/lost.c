// PA9  key
// PA1  PWM1
// PB5  BLED

#include "app_config.h"
#include "system/includes.h"
#include "asm/includes.h"
#include "device/key_driver.h"
#include "asm/power/p33.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "btstack/bluetooth.h"
#include "lost.h"
#include "app_power_manage.h"

#define LOG_TAG_CONST       LOST
#define LOG_TAG             "[LOST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define FAST_ADV_INTERVAL_MIN          (800)   //500ms            蓝牙连接是由这两个宏决定的，广播间隔短，连接块
#define SLOW_ADV_INTERVAL_MIN          (2400)  //1.5s

u32 adv_interval_time = FAST_ADV_INTERVAL_MIN; // 0.625ms单位
static int adv_fast_timer = 0;
static int adv_slow_timer = 0;
static int auto_off_timer = 0;
#define FAST_ADV_TIMEOUT  90 * 1000  // 180 * 1000           //180s
#define SLOW_ADV_TIMEOUT  90 * 1000  // 60 * 60 * 1000       //1小时
#define AUTO_OFF_TIMEOUT  7* 24 *3600 * 1000  // 7 * 24 * 3600 * 1000 //7天

#define ALERT_FRE           300
static int immediate_alert_timer = 0;
static int alert_timer_reach = 0;
static int aler_click_over = 0;

static u8 alert_st = 0;
#define REASON_DISCONNECT   1 // 1为断开报警
#define REASON_IMMEDIATE    2 // 2为主动报警
static u8 alert_reason = 0;

u8 ble_get_battery_level(void)
{
    /* set_change_vbg_value_flag(); */
    static u16 vbat = 310;
    u16 temp = 0;
    u16 level = 0;
    temp = get_vbat_level();
    if((temp > 220)&&(temp < 340)){
        vbat = temp;
    }
    if(vbat > 300){
        level = 100;
    }else if(vbat > 290){
        level = 75;
    }else if(vbat > 270){
        level = 50;
    }else if(vbat > 260){
        level = 25;
    }else if(vbat > 220){
        level = 0;
    }
    log_info("%s[%d %d %d]", __func__, vbat, temp, level);
    return level;
}

void set_adv_interval_time(u8 f_s)
{
    log_info("%s[%d]", __func__, f_s);
    if(f_s){
        adv_interval_time = FAST_ADV_INTERVAL_MIN;
    }else{
        adv_interval_time = SLOW_ADV_INTERVAL_MIN;
    }
}
u32 get_adv_interval_time(void)
{
    log_info("%s[%d]", __func__, adv_interval_time);
    return adv_interval_time;
}
static void ble_auto_off_timer(void *priv)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    app_set_soft_poweroff();
    auto_off_timer = 0;
}
static void ble_adv_slow_timer(void *priv)
{
    log_info("%s[%d]", __func__, AUTO_OFF_TIMEOUT);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    adv_slow_timer = 0;
    if(auto_off_timer == 0){
        auto_off_timer = sys_timeout_add(NULL, ble_auto_off_timer, AUTO_OFF_TIMEOUT);
    }
}
static void ble_adv_fast_timer(void *priv)
{
    log_info("%s", __func__);
    extern void ble_adv_interval_set(u8 mode);
    ble_adv_interval_set(1);
    adv_fast_timer = 0;
}
void ble_adv_interval_set(u8 mode)
{
    r_printf("%s[%d]", __func__, mode);
    static u8 mode_is = 0xff;
    struct ble_server_operation_t *ble_opt;
    ble_get_server_operation_table(&ble_opt);
    if(mode_is == mode){
        return;
    }
    if(bt_ble_is_connected()){
        return;
    }
    mode_is = mode;
    if(mode == 1){
        adv_interval_time = SLOW_ADV_INTERVAL_MIN;
        pwm_led_mode_set(PWM_LED_USER_SLOW_ADV);
        if(adv_slow_timer == 0){
            log_info("%s[%d]", __func__, SLOW_ADV_TIMEOUT);
            adv_slow_timer = sys_timeout_add(NULL, ble_adv_slow_timer, SLOW_ADV_TIMEOUT);
        }
    }else{
        adv_interval_time = FAST_ADV_INTERVAL_MIN;
        /* pwm_led_mode_set(PWM_LED1_ON);// */
//        if(adv_fast_timer == 0){
//            log_info("%s[%d]", __func__, FAST_ADV_TIMEOUT);
//            adv_fast_timer = sys_timeout_add(NULL, ble_adv_fast_timer, FAST_ADV_TIMEOUT);
//        }
    }
    ble_opt->adv_enable(NULL, 0);
    ble_opt->adv_enable(NULL, 1);
}
void ble_fast_adv_to_slow(void)
{
    log_info("%s[%d]", __func__, adv_interval_time);
    if(adv_interval_time == FAST_ADV_INTERVAL_MIN){
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        if(adv_fast_timer == 0){
            log_info("%s[%d]", __func__, FAST_ADV_TIMEOUT);
            adv_fast_timer = sys_timeout_add(NULL, ble_adv_fast_timer, FAST_ADV_TIMEOUT);
        }
    }
}

void adv_timer_del(void)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    if(adv_fast_timer){
        sys_timeout_del(adv_fast_timer);
        adv_fast_timer = 0;
    }
    if(adv_slow_timer){
        sys_timeout_del(adv_slow_timer);
        adv_slow_timer = 0;
    }
    if(auto_off_timer){
        sys_timeout_del(auto_off_timer);
        auto_off_timer = 0;
    }
    disconnect_alert_del();
}

#define BZR_PWM_TIMER    JL_TIMER3
#define BZR_PWM_FRE      2750
#define BZR_PWM_IO       TCFG_BUZZER_PIN///IO_PORTA_01
#define BZR_DUTY         20
static u32 bzr_duty = BZR_DUTY;
static void buzzer_ctrl(u8 en, u32 duty)
{
    log_info("%s[%d  %d]", __func__, en, duty);
    static u8 init = 0;
    duty = duty * 100;
    if(en){
        if(!init){
            timer_pwm_init(BZR_PWM_TIMER, BZR_PWM_IO, BZR_PWM_FRE, duty);
            init = 1;
            set_enter_lp(0);
        }
        set_timer_pwm_duty(BZR_PWM_TIMER, duty);
    }else{
        timer_pwm_delete(BZR_PWM_TIMER, BZR_PWM_IO, BZR_PWM_FRE, duty);
        init = 0;
        set_enter_lp(1);
    }
}

void buzzer_ring(void)
{
    log_info("%s", __func__);
	buzzer_ctrl(1, bzr_duty);
    set_enter_lp(0);	
}


void buzzer_close(void)
{
    log_info("%s", __func__);
	buzzer_ctrl(0, 0);
    set_enter_lp(1);	
}

////APP  协议部分
static u8 linkloss = 60; ///延时报警值, 100ms单位
void set_link_loss(u8 value)
{
    log_info("%s[%d -> %d]", __func__, linkloss, value);
    if(value == 0){
        linkloss = 60;
    }
    linkloss = value;
}
u8 get_link_loss(void)
{
    log_info("%s[%d]", __func__, linkloss);
    return linkloss;
}

void immediateAlert_del(void)
{
    log_info("%s", __func__);
    alert_st = 0;
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    if(immediate_alert_timer){
        sys_timer_del(immediate_alert_timer);
        immediate_alert_timer = 0;
    }
}
static void immediateAlert_deal(void)
{
    log_info("%s[%d]", __func__, alert_st);
    if(alert_st){
        alert_st = 0;
        pwm_led_mode_set(PWM_LED_ALL_OFF);
        buzzer_ctrl(0, 0);
    }else{
        alert_st = 1;
        pwm_led_mode_set(PWM_LED1_ON);
        buzzer_ctrl(1, bzr_duty);
    }
}

void immediateAlert_close_buzzer_deal(void)
{
    buzzer_ctrl(0, 0);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    sys_timer_del(immediate_alert_timer);
    immediate_alert_timer = 0;
}

void immediateAlert_ctrl(u8 level)
{
    log_info("%s[%d]", __func__, level);
	static u8 alert_status = 0;
    if(level == 0){
        alert_reason = 0;
        alert_st = 0;
        buzzer_ctrl(0, 0);
        pwm_led_mode_set(PWM_LED_ALL_OFF);
		alert_status = 0;
        if(immediate_alert_timer){
            sys_timer_del(immediate_alert_timer);
            immediate_alert_timer = 0;
        }
    }else if((level == 1)||(level == 2)){
        alert_reason = REASON_IMMEDIATE;
        bzr_duty = BZR_DUTY;
        buzzer_ctrl(1, bzr_duty);
        alert_st = 1;
		alert_status = 1;
        if(immediate_alert_timer == 0){
            immediate_alert_timer = sys_timer_add(NULL,immediateAlert_deal,ALERT_FRE);
			alert_timer_reach = sys_timeout_add(NULL,immediateAlert_close_buzzer_deal,180*1000);
        }
    }else{
        alert_reason = REASON_IMMEDIATE;
        bzr_duty = BZR_DUTY/2;
        buzzer_ctrl(1, bzr_duty);
        alert_st = 1;
		alert_status = 1;
        if(immediate_alert_timer == 0){
            immediate_alert_timer = sys_timer_add(NULL,immediateAlert_deal,ALERT_FRE);
        }
    }
	aler_click_over = alert_status;
}

static u8 alert_en = 1;
void disconnect_alert_set(u8 en)
{
    log_info("%s[%d]", __func__, en);
    alert_en = en;
}

static u8 alert_para[3] = {0};
void set_alert_para(u8 *buf, u8 buf_size)
{
    log_info("%s", __func__);
    log_info_hexdump(buf, buf_size);
    memcpy(alert_para, buf, sizeof(alert_para));
}
u8 get_alert_para(u8 *buf, u8 buf_size)
{
    log_info("%s", __func__);
    log_info_hexdump(alert_para, sizeof(alert_para));
    if(buf){
        memcpy(buf, alert_para, buf_size);
    }
    return sizeof(alert_para);
}

static int disconn_alert_timer = 0;
static u32 alert_cnt = 0;
void disconnect_alert_del(void)
{
    log_info("%s", __func__);
    alert_st = 0;
    alert_reason = 0;
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    alert_cnt = 0;
    if(disconn_alert_timer){
        sys_timer_del(disconn_alert_timer);
        disconn_alert_timer = 0;
    }
}
static void disconnect_alert_deal(void)
{
    log_info("%s[%d %d]", __func__, 180000/ALERT_FRE, alert_cnt);
    if(alert_st){
        alert_st = 0;
        pwm_led_mode_set(PWM_LED_ALL_OFF);
        buzzer_ctrl(0, 0);
    }else{
        alert_st = 1;
        pwm_led_mode_set(PWM_LED1_ON);
        buzzer_ctrl(1, BZR_DUTY);
    }
    if(alert_cnt++ > 180000/ALERT_FRE){
        disconnect_alert_del();
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
    }
}
void ble_disconnect_deal(void)
{
    log_info("%s[%d]", __func__, alert_en);
    if(!alert_en) return;
    alert_reason = REASON_DISCONNECT;
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    alert_st = 1;
    alert_cnt = 0;
    if(disconn_alert_timer == 0){
        disconn_alert_timer = sys_timer_add(NULL, disconnect_alert_deal, ALERT_FRE);
    }
}

// UI功能
extern void clr_wdt(void);

void check_power_on_key(void)
{
    log_info("%s", __func__);
    u32 delay_10ms_cnt = 0;
    while (1) {
        clr_wdt();
        os_time_dly(1);
        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+[%d]", delay_10ms_cnt);
            if(delay_10ms_cnt == 110){
                pwm_led_mode_set(PWM_LED1_ON);
                buzzer_ctrl(1, BZR_DUTY);
            }else if(delay_10ms_cnt == 120){
                buzzer_ctrl(0, 0);
            }else if(delay_10ms_cnt == 140){
                buzzer_ctrl(1, BZR_DUTY);
            }else if(delay_10ms_cnt == 150){
                buzzer_ctrl(0, 0);
            }
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 150) {

                return;
            }
        } else {
            log_info("-");
            buzzer_ctrl(0, 0);
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
            return;
        }
    }
}

void check_power_on_key_A(void)
{
    log_info("%s", __func__);
    u32 delay_10ms_cnt = 0;
    gpio_direction_input(TCFG_IOKEY_POWER_ONE_PORT);
    gpio_set_pull_down(TCFG_IOKEY_POWER_ONE_PORT, 0);
    gpio_set_pull_up(TCFG_IOKEY_POWER_ONE_PORT, 1);
    gpio_set_die(TCFG_IOKEY_POWER_ONE_PORT, 1);
    while (1) {
        clr_wdt();
        os_time_dly(1);
        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+[%d]", delay_10ms_cnt);
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 100) { //150
                break;
            }
        } else {
            log_info("-");
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
            return;
        }
    }
    pwm_led_init_delay();
    clr_wdt();
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    os_time_dly(10);
    buzzer_ctrl(0, 0);
    os_time_dly(10);
    buzzer_ctrl(1, BZR_DUTY);
    os_time_dly(10);
    buzzer_ctrl(0, 0);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    clr_wdt();
}

static int goto_power_off = 0;
void lost_enter_soft_poweroff(void)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    pwm_led_close();
    sys_timeout_add(NULL, app_set_soft_poweroff, 1000);
}
void lost_enter_soft_poweroff_close_buzzer(void)
{
    log_info("%s", __func__);
    buzzer_ctrl(1, BZR_DUTY);
    pwm_led_mode_set(PWM_LED1_ON);
    sys_timeout_add(NULL, lost_enter_soft_poweroff, 1000);
}
static void check_power_off_key(u8 event_type, u8 key_value)
{
    log_info("%s[%d]", __func__, goto_power_off);
    if(goto_power_off == -1){
        return;
    }
    if((event_type == KEY_EVENT_HOLD)&&(key_value == 0)){
        goto_power_off++;
        P33_CON_SET(P3_PINR_CON, 0, 1, 0);//关闭长按复位
    }
    if((event_type == KEY_EVENT_UP)&&(key_value == 0)){
        if(goto_power_off != -1)
            goto_power_off = 0;
    }
    if(goto_power_off > 10){
        goto_power_off = -1;
        pwm_led_mode_set(PWM_LED_USER_POWER_OFF);
        sys_timeout_add(NULL, lost_enter_soft_poweroff_close_buzzer, 1200);
    }
}
static int bat_timer = 0;
static void bat_deal(void)
{
    log_info("%s", __func__);
    app_cmd_bat();
    bat_timer = 0;
}
static int key_click_timer = 0;
static u8 last_display = 0;
static void key_led_close(void)
{
    log_info("%s", __func__);
    /* pwm_led_mode_set(PWM_LED_USER_FAST_ADV); */
    /* pwm_led_mode_set(PWM_LED_ALL_OFF); */
    pwm_led_mode_set(last_display);
    buzzer_ctrl(0, 0);
    key_click_timer = 0;
}
static void key_click_deal(void)
{
    log_info("%s", __func__);

	#if 0
    if(alert_reason == REASON_IMMEDIATE){ //主动报警,按键不能终止
        return;
    }
	#endif
	
    if(disconn_alert_timer){
        disconnect_alert_del();
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        ble_adv_interval_set(0);
        return;
    }else if(aler_click_over){
		log_info("----------------------here are the click stop----------------------------");
		immediateAlert_ctrl(0);
        //pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        ble_adv_interval_set(0);
	}
    ble_adv_interval_set(0);
    last_display = pwm_led_display_mode_get();
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    app_cmd_alert();
	app_cmd_secret();
    if(key_click_timer == 0){
        key_click_timer = sys_timeout_add(NULL, key_led_close, 50);
    }
    if(bat_timer == 0){
        bat_timer = sys_timeout_add(NULL, bat_deal, 200);
    }
}
static void key_double_click_deal(void)
{
    log_info("%s", __func__);
    app_cmd_alert();
    app_cmd_alert();
}

void app_key_deal(u8 event_type, u8 key_value)
{
    log_info("%s[%d %d]", __func__, event_type, key_value);
    check_power_off_key(event_type, key_value);
    if(goto_power_off == -1){
        return;
    }
    ble_get_battery_level();
    if(event_type == KEY_EVENT_CLICK){
        log_info("KEY_EVENT_CLICK");
        key_click_deal();
    }
    if(event_type == KEY_EVENT_DOUBLE_CLICK){
        log_info("KEY_EVENT_DOUBLE_CLICK");
        /* key_double_click_deal(); */
    }
    if(event_type == KEY_EVENT_TRIPLE_CLICK){
        log_info("KEY_EVENT_TRIPLE_CLICK");
        immediateAlert_close_buzzer_deal();
    }
}


/*****************************************灯光指示**********************************************/
/*
	PB7 : yellow
	PA0 : red
	PA2 : blue

*/


