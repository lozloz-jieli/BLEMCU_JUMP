#ifndef CONFIG_AC6323A_LOST_H
#define CONFIG_AC6323A_LOST_H



extern u16 bt_ble_is_connected(void);
extern void set_enter_lp(u8 en);
extern void app_set_soft_poweroff(void);
extern void set_adv_interval_time(u8 f_s);
extern u32 get_adv_interval_time(void);
extern u8 get_link_loss(void);
extern void set_link_loss(u8 value);
extern void immediateAlert_ctrl(u8 level);
extern void disconnect_alert_set(u8 en);
extern void set_alert_para(u8 *buffer, u8 buf_size);
extern u8 get_alert_para(u8 *buf, u8 buf_size);
extern void app_cmd_alert(void);
extern void app_cmd_bat(void);
extern void app_cmd_secret(void);
extern void ble_disconnect_deal(void);
extern void disconnect_alert_del(void);
extern void immediateAlert_del(void);
extern void pwm_led_init_delay(void);
extern void pwm_led_close(void);
extern u8 ble_get_battery_level(void);
extern void set_change_vbg_value_flag(void);

extern void buzzer_ring(void);
extern void buzzer_close(void);




extern void immediateAlert_close_buzzer_deal(void);



#endif
