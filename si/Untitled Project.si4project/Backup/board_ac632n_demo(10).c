#include "app_config.h"

#ifdef CONFIG_BOARD_AC632N_DEMO

#include "system/includes.h"
#include "device/key_driver.h"
#include "asm/chargestore.h"
#include "asm/charge.h"
#include "asm/power/p33.h"
#include "rtc_alarm.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "usb/otg.h"
#include "norflash.h"

#define LOG_TAG_CONST       BOARD
#define LOG_TAG             "[BOARD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
extern OS_SEM SKIP_ROPE_COUNT;

void board_power_init(void);

/************************** LOW POWER config ****************************/
const struct low_power_param power_param = {
    .config         = TCFG_LOWPOWER_LOWPOWER_SEL,          //0：sniff时芯片不进入低功耗  1：sniff时芯片进入powerdown
    .btosc_hz       = TCFG_CLOCK_OSC_HZ,                   //外接晶振频率
    .delay_us       = TCFG_CLOCK_SYS_HZ / 1000000L,        //提供给低功耗模块的延时(不需要需修改)
    .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持
    .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.0V  2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.6V
    .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
    .osc_type       = TCFG_LOWPOWER_OSC_TYPE,
    .lpctmu_en      = TCFG_LP_TOUCH_KEY_ENABLE,
};

/************************** KEY MSG****************************/
/*各个按键的消息设置，如果USER_CFG中设置了USE_CONFIG_KEY_SETTING为1，则会从配置文件读取对应的配置来填充改结构体*/
/* u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = { */
// SHORT           LONG              HOLD              UP              DOUBLE           TRIPLE
/* }; */


// *INDENT-OFF*
/************************** UART config****************************/
#if TCFG_UART0_ENABLE
UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .tx_pin = TCFG_UART0_TX_PORT,                             //串口打印TX引脚选择
    .rx_pin = TCFG_UART0_RX_PORT,                             //串口打印RX引脚选择
    .baudrate = TCFG_UART0_BAUDRATE,                          //串口波特率

    .flags = UART_DEBUG,                                      //串口用来打印需要把改参数设置为UART_DEBUG
UART0_PLATFORM_DATA_END()
#endif //TCFG_UART0_ENABLE


/************************** CHARGE config****************************/
#if TCFG_CHARGE_ENABLE
CHARGE_PLATFORM_DATA_BEGIN(charge_data)
    .charge_en              = TCFG_CHARGE_ENABLE,              //内置充电使能
    .charge_poweron_en      = TCFG_CHARGE_POWERON_ENABLE,      //是否支持充电开机
    .charge_full_V          = TCFG_CHARGE_FULL_V,              //充电截止电压
    .charge_full_mA			= TCFG_CHARGE_FULL_MA,             //充电截止电流
    .charge_mA				= TCFG_CHARGE_MA,                  //充电电流
/*ldo5v拔出过滤值，过滤时间 = (filter*2 + 20)ms,ldoin<0.6V且时间大于过滤时间才认为拔出
 对于充满直接从5V掉到0V的充电仓，该值必须设置成0，对于充满由5V先掉到0V之后再升压到xV的
 充电仓，需要根据实际情况设置该值大小*/
    .ldo5v_off_filter		= 100,
    .ldo5v_on_filter        = 50,
    .ldo5v_keep_filter      = 220,
    .ldo5v_pulldown_lvl     = CHARGE_PULLDOWN_200K,            //下拉电阻档位选择
    .ldo5v_pulldown_keep    = 1,
//1、对于自动升压充电舱,若充电舱需要更大的负载才能检测到插入时，请将该变量置1,并且根据需求配置下拉电阻档位
//2、对于按键升压,并且是通过上拉电阻去提供维持电压的舱,请将该变量设置1,并且根据舱的上拉配置下拉需要的电阻挡位
//3、对于常5V的舱,可将改变量设为0,省功耗
//4、为LDOIN防止被误触发唤醒,可设置为200k下拉
    .ldo5v_pulldown_en		= 1,
CHARGE_PLATFORM_DATA_END()
#endif//TCFG_CHARGE_ENABLE

    /************************** AD KEY ****************************/
#if TCFG_ADKEY_ENABLE
const struct adkey_platform_data adkey_data = {
    .enable = TCFG_ADKEY_ENABLE,                              //AD按键使能
    .adkey_pin = TCFG_ADKEY_PORT,                             //AD按键对应引脚
    .ad_channel = TCFG_ADKEY_AD_CHANNEL,                      //AD通道值
    .extern_up_en = TCFG_ADKEY_EXTERN_UP_ENABLE,              //是否使用外接上拉电阻
    .ad_value = {                                             //根据电阻算出来的电压值
        TCFG_ADKEY_VOLTAGE0,
        TCFG_ADKEY_VOLTAGE1,
        TCFG_ADKEY_VOLTAGE2,
        TCFG_ADKEY_VOLTAGE3,
        TCFG_ADKEY_VOLTAGE4,
        TCFG_ADKEY_VOLTAGE5,
        TCFG_ADKEY_VOLTAGE6,
        TCFG_ADKEY_VOLTAGE7,
        TCFG_ADKEY_VOLTAGE8,
        TCFG_ADKEY_VOLTAGE9,
    },
    .key_value = {                                             //AD按键各个按键的键值
        TCFG_ADKEY_VALUE0,
        TCFG_ADKEY_VALUE1,
        TCFG_ADKEY_VALUE2,
        TCFG_ADKEY_VALUE3,
        TCFG_ADKEY_VALUE4,
        TCFG_ADKEY_VALUE5,
        TCFG_ADKEY_VALUE6,
        TCFG_ADKEY_VALUE7,
        TCFG_ADKEY_VALUE8,
        TCFG_ADKEY_VALUE9,
    },
};
#endif

/************************** IO KEY ****************************/
#if TCFG_IOKEY_ENABLE
const struct iokey_port iokey_list[] = {
    {
        .connect_way = TCFG_IOKEY_POWER_CONNECT_WAY,          //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_POWER_ONE_PORT,    //IO按键对应的引脚
        .key_value = 0,                                       //按键值
    },
/*
    {
        .connect_way = TCFG_IOKEY_PREV_CONNECT_WAY,
        .key_type.one_io.port = TCFG_IOKEY_PREV_ONE_PORT,
        .key_value = 1,
    },

    {
        .connect_way = TCFG_IOKEY_NEXT_CONNECT_WAY,
        .key_type.one_io.port = TCFG_IOKEY_NEXT_ONE_PORT,
        .key_value = 2,
    },*/
};
const struct iokey_platform_data iokey_data = {
    .enable = TCFG_IOKEY_ENABLE,                              //是否使能IO按键
    .num = ARRAY_SIZE(iokey_list),                            //IO按键的个数
    .port = iokey_list,                                       //IO按键参数表
};

#if MULT_KEY_ENABLE
//组合按键消息映射表
//配置注意事项:单个按键按键值需要按照顺序编号,如power:0, prev:1, next:2
//bit_value = BIT(0) | BIT(1) 指按键值为0和按键值为1的两个按键被同时按下,
//remap_value = 3指当这两个按键被同时按下后重新映射的按键值;
const struct key_remap iokey_remap_table[] = {
    {.bit_value = BIT(0) | BIT(1), .remap_value = 3},
    {.bit_value = BIT(0) | BIT(2), .remap_value = 4},
    {.bit_value = BIT(1) | BIT(2), .remap_value = 5},
};

const struct key_remap_data iokey_remap_data = {
    .remap_num = ARRAY_SIZE(iokey_remap_table),
    .table = iokey_remap_table,
};
#endif
#endif


#if TCFG_RTC_ALARM_ENABLE
const struct sys_time def_sys_time = {  //初始一下当前时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};
const struct sys_time def_alarm = {     //初始一下目标时间，即闹钟时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

extern void alarm_isr_user_cbfun(u8 index);
RTC_DEV_PLATFORM_DATA_BEGIN(rtc_data)
    .default_sys_time = &def_sys_time,
    .default_alarm = &def_alarm,
    /* .cbfun = NULL,                      //闹钟中断的回调函数,用户自行定义 */
    .cbfun = alarm_isr_user_cbfun,
RTC_DEV_PLATFORM_DATA_END()
#endif

/************************** PWM_LED ****************************/
#if TCFG_PWMLED_ENABLE
LED_PLATFORM_DATA_BEGIN(pwm_led_data)
    .io_mode = TCFG_PWMLED_IOMODE,              //推灯模式设置:支持单个IO推两个灯和两个IO推两个灯
    .io_cfg.one_io.pin = TCFG_PWMLED_PIN,       //单个IO推两个灯的IO口配置
LED_PLATFORM_DATA_END()
#endif

/************************** norflash ****************************/
NORFLASH_DEV_PLATFORM_DATA_BEGIN(norflash_fat_dev_data)
    .spi_hw_num     = TCFG_FLASH_DEV_SPI_HW_NUM,
    .spi_cs_port    = TCFG_FLASH_DEV_SPI_CS_PORT,
    .spi_read_width = 4,
#if (TCFG_FLASH_DEV_SPI_HW_NUM == 1)
    .spi_pdata      = &spi1_p_data,
#elif (TCFG_FLASH_DEV_SPI_HW_NUM == 2)
    .spi_pdata      = &spi2_p_data,
#endif
    .start_addr     = 0,
    .size           = 16*1024*1024,
NORFLASH_DEV_PLATFORM_DATA_END()



/************************** otg data****************************/
#if 0//TCFG_OTG_MODE
struct otg_dev_data otg_data = {
    .usb_dev_en = TCFG_OTG_USB_DEV_EN,
    .slave_online_cnt = TCFG_OTG_SLAVE_ONLINE_CNT,
    .slave_offline_cnt = TCFG_OTG_SLAVE_OFFLINE_CNT,
    .host_online_cnt = TCFG_OTG_HOST_ONLINE_CNT,
    .host_offline_cnt = TCFG_OTG_HOST_OFFLINE_CNT,
    .detect_mode = TCFG_OTG_MODE,
    .detect_time_interval = TCFG_OTG_DET_INTERVAL,
};
#endif

REGISTER_DEVICES(device_table) = {
#if 0//TCFG_OTG_MODE
    { "otg",     &usb_dev_ops, (void *) &otg_data},
#endif
#if TCFG_CHARGE_ENABLE
    { "charge", &charge_dev_ops, (void *)&charge_data },
#endif

};

void debug_uart_init(const struct uart_platform_data *data)
{
#if TCFG_UART0_ENABLE
    if (data) {
        uart_init(data);
    } else {
        uart_init(&uart0_data);
    }
#endif
}

void hall_init(void)
{
	/// hall 中断初始化
	 gpio_set_direction(TCFG_HALL1_IO_ONE_PORT, 1);
	 gpio_set_die(TCFG_HALL1_IO_ONE_PORT, 1);
	 gpio_set_dieh(TCFG_HALL1_IO_ONE_PORT, 1);
	 gpio_set_hd(TCFG_HALL1_IO_ONE_PORT, 0);
	 gpio_set_hd0(TCFG_HALL1_IO_ONE_PORT, 0);
	  gpio_set_pull_up(TCFG_HALL1_IO_ONE_PORT ,1);
	 gpio_set_pull_down(TCFG_HALL1_IO_ONE_PORT, 0);
	
	  gpio_set_direction(TCFG_HALL2_IO_ONE_PORT, 1);
	 gpio_set_die(TCFG_HALL2_IO_ONE_PORT, 1);
	 gpio_set_dieh(TCFG_HALL2_IO_ONE_PORT, 1);
	 gpio_set_hd(TCFG_HALL2_IO_ONE_PORT, 0);
	 gpio_set_hd0(TCFG_HALL2_IO_ONE_PORT, 0);
	  gpio_set_pull_up(TCFG_HALL2_IO_ONE_PORT ,1);
	 gpio_set_pull_down(TCFG_HALL2_IO_ONE_PORT, 0);

}

static void board_devices_init(void)
{
#if TCFG_PWMLED_ENABLE
    pwm_led_init(&pwm_led_data);
#endif

#if (TCFG_IOKEY_ENABLE || TCFG_ADKEY_ENABLE || TCFG_TOUCH_KEY_ENABLE)
    key_driver_init();
#endif

#if (!TCFG_CHARGE_ENABLE)
    CHARGE_EN(0);
#endif
    extern void led_8888_init();

    led_8888_init();

#if TCFG_RTC_ALARM_ENABLE
    alarm_init(&rtc_data);
#endif

	hall_init();


}

//maskrom 使用到的io
static void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};
    boot_soft_flag.flag0.boot_ctrl.wdt_dis = 0;
    boot_soft_flag.flag0.boot_ctrl.poweroff = 0;
    boot_soft_flag.flag0.boot_ctrl.is_port_b = JL_IOMAP->CON0 & BIT(16) ? 1 : 0;

    boot_soft_flag.flag1.misc.usbdm = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag1.misc.usbdp = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag1.misc.uart_key_port = 0;
    boot_soft_flag.flag1.misc.ldoin = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag2.pa7_pb4.pa7 = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag2.pa7_pb4.pb4 = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag3.pc3_pc5.pc3 = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag3.pc3_pc5.pc5 = SOFTFLAG_HIGH_RESISTANCE;
    mask_softflag_config(&boot_soft_flag);
}


extern void cfg_file_parse(u8 idx);
void board_init()
{
    board_power_init();
    adc_vbg_init();
    adc_init();
    cfg_file_parse(0);
    devices_init();

    board_devices_init();

    if(get_charge_online_flag()) {
        power_set_mode(PWR_LDO15);
    } else {
        power_set_mode(TCFG_LOWPOWER_POWER_SEL);
    }

    /*close FAST CHARGE */
#if TCFG_UART0_ENABLE
    if (uart0_data.rx_pin < IO_MAX_NUM) {
        gpio_set_die(uart0_data.rx_pin, 1);
    }
#endif

}

enum {
    PORTA_GROUP = 0,
    PORTB_GROUP,
    PORTC_GROUP,
};

static void port_protect(u16 *port_group, u32 port_num)
{
    if (port_num == NO_CONFIG_PORT) {
        return;
    }
    port_group[port_num / IO_GROUP_NUM] &= ~BIT(port_num % IO_GROUP_NUM);
}

void usb1_iomode(u32 enable);
/*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
static void close_gpio(void)
{
    u16 port_group[] = {
        [PORTA_GROUP] = 0x1ff,
        [PORTB_GROUP] = 0x3ff,//
        [PORTC_GROUP] = 0x3ff,//
    };

    if(P3_ANA_CON2 & BIT(3))
    {
        port_protect(port_group, IO_PORTB_02);	//protect VCM_IO
    }

    if(P3_PINR_CON & BIT(0))
    {
        u8 port_sel = P3_PORT_SEL0;
        if((port_sel >= 1) && (port_sel <= 10)){
            port_sel = IO_GROUP_NUM * 0 + port_sel - 1;
            port_protect(port_group, port_sel);				//protect 长按复位
        }else if((port_sel >= 11) && (port_sel <= 20)){
            port_sel = IO_GROUP_NUM * 1 + port_sel - 11;
            port_protect(port_group, port_sel);				//protect 长按复位
        }else if((port_sel >= 21) && (port_sel <= 25)){
            port_sel = IO_GROUP_NUM * 2 + port_sel - 21;
            port_protect(port_group, port_sel);				//protect 长按复位
        }else if(port_sel == 26){
            port_protect(port_group, IO_PORT_DP);			//protect 长按复位
        }else if(port_sel == 27){
            port_protect(port_group, IO_PORT_DM);			//protect 长按复位
        }else if(port_sel == 28){
            port_protect(port_group, IO_PORT_DP1);			//protect 长按复位
        }else if(port_sel == 29){
            port_protect(port_group, IO_PORT_DM1);			//protect 长按复位
        }
    }

#if TCFG_ADKEY_ENABLE
    port_protect(port_group,TCFG_ADKEY_PORT);
    port_protect(port_group, TCFG_HALL1_IO_ONE_PORT);
    port_protect(port_group, TCFG_HALL2_IO_ONE_PORT);//
  	port_protect(port_group, LED_TEST_PORT);
#endif /* */

    //< close gpio
    gpio_dir(GPIOA, 0, 9, port_group[PORTA_GROUP], GPIO_OR);
    gpio_set_pu(GPIOA, 0, 9, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_set_pd(GPIOA, 0, 9, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_die(GPIOA, 0, 9, ~port_group[PORTA_GROUP], GPIO_AND);
    gpio_dieh(GPIOA, 0, 9, ~port_group[PORTA_GROUP], GPIO_AND);

    gpio_dir(GPIOB, 0, 10, port_group[PORTB_GROUP], GPIO_OR);
    gpio_set_pu(GPIOB, 0, 10, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_set_pd(GPIOB, 0, 10, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_die(GPIOB, 0, 10, ~port_group[PORTB_GROUP], GPIO_AND);
    gpio_dieh(GPIOB, 0, 10, ~port_group[PORTB_GROUP], GPIO_AND);

//    < close usb io
    usb_iomode(1);
    gpio_set_pull_up(IO_PORT_DP, 0);
    gpio_set_pull_down(IO_PORT_DP, 0);
    gpio_set_direction(IO_PORT_DP, 1);
    gpio_set_die(IO_PORT_DP, 0);
    gpio_set_dieh(IO_PORT_DP, 0);
	 
    gpio_set_pull_up(IO_PORT_DM, 0);
    gpio_set_pull_down(IO_PORT_DM, 0);
    gpio_set_direction(IO_PORT_DM, 1);
    gpio_set_die(IO_PORT_DM, 0);
    gpio_set_dieh(IO_PORT_DM, 0);
   
   

    /* printf("JL_USB_IO->CON0=0x%x\r\n", JL_USB_IO->CON0); */
    /* printf("JL_USB_IO->CON1=0x%x\r\n", JL_USB_IO->CON1); */
    /* printf("JL_USB->CON0=0x%x\r\n", JL_USB->CON0); */
    /*  */
    /* printf("JL_USB1_IO->CON0=0x%x\r\n", JL_USB1_IO->CON0); */
    /* printf("JL_USB1_IO->CON1=0x%x\r\n", JL_USB1_IO->CON1); */
    /* printf("JL_USB1->CON0=0x%x\r\n", JL_USB1->CON0); */
}

/************************** PWR config ****************************/
struct port_wakeup port0 = {
    .pullup_down_enable = ENABLE,                            //配置I/O 内部上下拉是否使能
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
    .both_edge          = 0,

#if TCFG_ADKEY_ENABLE
    .iomap              = TCFG_ADKEY_PORT,                   //唤醒口选择
#else
    .iomap              = TCFG_IOKEY_POWER_ONE_PORT,         //唤醒口选择
#endif
    .filter             = PORT_FLT_2ms,
};

#if TCFG_TEST_BOX_ENABLE
struct port_wakeup port1 = {
    .pullup_down_enable = DISABLE,                            //配置I/O 内部上下拉是否使能
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
    .both_edge          = 1,
    .filter             = PORT_FLT_1ms,
    .iomap              = TCFG_CHARGESTORE_PORT,             //唤醒口选择
};
#endif

#if TCFG_CHARGE_ENABLE
struct port_wakeup charge_port = {
    .edge               = RISING_EDGE,                       //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .both_edge          = 0,
    .filter             = PORT_FLT_16ms,
    .iomap              = IO_CHGFL_DET,                      //唤醒口选择
};

struct port_wakeup vbat_port = {
    .edge               = BOTH_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .both_edge          = 1,
    .filter             = PORT_FLT_16ms,
    .iomap              = IO_VBTCH_DET,                      //唤醒口选择
};

struct port_wakeup ldoin_port = {
    .edge               = BOTH_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿\双边沿
    .both_edge          = 1,
    .filter             = PORT_FLT_16ms,
    .iomap              = IO_LDOIN_DET,                      //唤醒口选择
};
#endif
//----------------------jimmy add  ,the Hall sigals s---------------------

struct port_wakeup port2 = {
    .pullup_down_enable = ENABLE,                            //配置I/O 内部上下拉是否使能
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
    .both_edge          = 0,
    .iomap              = TCFG_HALL1_IO_ONE_PORT,         //唤醒口选择
    .filter             = PORT_FLT_2ms,
};

struct port_wakeup port3 = {
    .pullup_down_enable = ENABLE,                            //配置I/O 内部上下拉是否使能
    .edge               = FALLING_EDGE,                      //唤醒方式选择,可选：上升沿\下降沿
    .both_edge          = 0,
    .iomap              = TCFG_HALL2_IO_ONE_PORT,         //唤醒口选择
    .filter             = PORT_FLT_2ms,
};


const struct wakeup_param wk_param = {

#if TCFG_ADKEY_ENABLE || TCFG_IOKEY_ENABLE
    .port[1]    = &port0,
    .port[3]    = &port2,
    .port[4]    = &port3,

    /* .sub        = &sub_wkup, */
    /* .charge     = &charge_wkup, */
#endif

#if TCFG_TEST_BOX_ENABLE
    .port[2] = &port1,
#endif
#if TCFG_CHARGE_ENABLE
    .aport[0] = &charge_port,
    .aport[1] = &vbat_port,
    .aport[2] = &ldoin_port,
#endif

};

//-----------------------------------------------


/*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
extern void dac_power_off(void);
void board_set_soft_poweroff(void)
{
    log_info("%s",__FUNCTION__);
    mask_io_cfg();

#if TCFG_TEST_BOX_ENABLE
    power_wakeup_index_disable(2);
#endif

//	power_wakeup_index_disable(3);
//    power_wakeup_index_disable(4);
//	power_wakeup_index_disable(5);

    close_gpio();


}

#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}


void sleep_exit_callback(u32 usec)
{
    putchar('>');
    APP_IO_DEBUG_0(A, 5);
}

void sleep_enter_callback(u8  step)
{
    /* 此函数禁止添加打印 */
    if (step == 1) {
        putchar('<');
        APP_IO_DEBUG_1(A, 5);
        /*dac_power_off();*/
    } else {
        close_gpio();
    }
}

static void wl_audio_clk_on(void)
{
    JL_WL_AUD->CON0 = 1;
}

static u32 HALL_1_COUNTER=0;
static u32 HALL_2_COUNTER=0;
static u32 HALL_3_COUNTER=0;
static u32 HALL_valided_COUNTER=0;

static u16 Hall_timer_id=0;
static  u32 valid_circle =0;

u8 get_power_on_status(void)
{

#if TCFG_IOKEY_ENABLE
    struct iokey_port *power_io_list = NULL;
    power_io_list = iokey_data.port;

    if (iokey_data.enable) {
        if (gpio_read(TCFG_IOKEY_POWER_ONE_PORT) == TCFG_IOKEY_POWER_CONNECT_WAY){
            return 1;
        }
    }
#endif

#if TCFG_ADKEY_ENABLE
 //  if (adkey_data.enable) {

 //     return 1;
//    }
  // return 1;
    //printf("ad_value = %d \n", ad_data); */

       if ((adc_get_value(TCFG_ADKEY_AD_CHANNEL) <= TCFG_ADKEY_VOLTAGE0) && (TCFG_ADKEY_VOLTAGE0 < 0x3ffL)) {
           return 1;
        }

#endif

    return 0;
}

void skip_rope_date_update()
{
 #if 0
  struct sys_event e;
   u32  totalNo=0;
   //totalNo =(HALL_1_COUNTER+HALL_2_COUNTER+HALL_3_COUNTER)/3;
      totalNo = (HALL_1_COUNTER < HALL_2_COUNTER) ? HALL_1_COUNTER : HALL_2_COUNTER;
     totalNo =(totalNo < HALL_3_COUNTER) ? totalNo : HALL_3_COUNTER;
     HALL_1_COUNTER=0;
     HALL_2_COUNTER=0;
     HALL_3_COUNTER=0;



   if(Hall_timer_id)
    {
        sys_timeout_del(Hall_timer_id);
         Hall_timer_id=0;
    }
    //if(Hall_timer_id_3)
    //   sys_timeout_del(Hall_timer_id_3);
    if(totalNo)
        {
            e.type = SYS_KEY_EVENT;
            e.u.key.init = 1;
            //e.u.key.type = scan_para->key_type;//区分按键类型
            e.u.key.event = KEY_EVENT_HALL_COUNTER;
            e.u.key.value = totalNo;
            e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
            sys_event_notify(&e);
            log_info("Skipping rope Num: %d", totalNo);
        }
   #endif
    if(Hall_timer_id)
    {
        sys_timeout_del(Hall_timer_id);
         Hall_timer_id=0;
    }
  valid_circle=0;

}

u8 can_enter_lp = 1; //置1可进睡眠,置0不可进睡眠
void set_enter_lp(u8 en)
{
    g_printf("%s[%d]", __func__, en);
    can_enter_lp = en;
}
static u8 custom_idle_query(void)
{
    if(can_enter_lp){
        return 1;
    }else{
        return 0;
    }
}
REGISTER_LP_TARGET(custom_lp_target) = {
    .name = "custom_lp",
    .is_idle = custom_idle_query,
};


static void port_wakeup_callback(u8 index, u8 gpio)
{
    /* log_info("%s:%d,%d",__FUNCTION__,index,gpio); */

    switch (index) {
#if TCFG_TEST_BOX_ENABLE
        case 2:
            extern void chargestore_ldo5v_fall_deal(void);
            chargestore_ldo5v_fall_deal();
            break;
#endif
    }
    #if 0//TCFG_SKIP_ROPE_ENABLE
     switch (gpio) {
        case TCFG_HALL1_IO_ONE_PORT://hall NO1 port

              if(HALL_1_COUNTER==0&&Hall_timer_id==0)   //  start counter ,if
                {
                log_info("befoer Hall_timer_id: %d", Hall_timer_id);
                Hall_timer_id =sys_timeout_add(0, skip_rope_date_update, 1200);
                log_info("later Hall_timer_id: %d", Hall_timer_id);
                }
                 HALL_1_COUNTER ++;
                 if (Hall_timer_id)
                     {
                     sys_timer_modify(Hall_timer_id, 1200);
                     }

            log_info("HALL_1_COUNTER: %d", HALL_1_COUNTER);
            break;
        case TCFG_HALL2_IO_ONE_PORT://hall NO1 port


            HALL_2_COUNTER++;
            log_info("HALL_2_COUNTER: %d", HALL_2_COUNTER);
            break;

        case TCFG_HALL3_IO_ONE_PORT://hall NO1 port

              HALL_3_COUNTER++;
            log_info("HALL_3_COUNTER: %d", HALL_3_COUNTER);

            break;
       default :
         break;
    }

   #else
    switch (gpio) {
        case TCFG_HALL1_IO_ONE_PORT://hall NO1 port
              valid_circle |=BIT(0);


            //log_info("HALL_1_COUNTER: %d", HALL_1_COUNTER);
            break;
        case TCFG_HALL2_IO_ONE_PORT://hall NO1 port
        valid_circle |=BIT(1);


            //log_info("HALL_2_COUNTER: %d", HALL_2_COUNTER);
            break;

       default :
         break;
    }
     if(Hall_timer_id==0)
              Hall_timer_id =sys_timeout_add(0, skip_rope_date_update, 1200);
            else
                sys_timer_modify(Hall_timer_id, 1200);
    if(valid_circle==0x0003)
    //if(valid_circle==0x0003)
        {
         //os_sem_set(&SKIP_ROPE_COUNT, 1);
        //  os_sem_post(&SKIP_ROPE_COUNT);
        extern u32 CURRENT_SKIP_NUM ;
         extern void fresh_rope_date();
        //  CURRENT_SKIP_NUM++;
         // extern void Send_SKIP_ROPE_NUM_BLE(u32 num);
          // Send_SKIP_ROPE_NUM_BLE(CURRENT_SKIP_NUM);
          fresh_rope_date();
        //  log_info("HALL_valided_COUNTER: %d", CURRENT_SKIP_NUM);
          valid_circle =0;
        }

#endif
}

static void aport_wakeup_callback(u8 index, u8 gpio, u8 edge)
{
 log_info("%s:%d,%d",__FUNCTION__,index,gpio);
#if TCFG_CHARGE_ENABLE
    switch (gpio) {
        case IO_CHGFL_DET://charge port
            charge_wakeup_isr();
            break;
        case IO_VBTCH_DET://vbat port
        case IO_LDOIN_DET://ldoin port
            ldoin_wakeup_isr();
            break;
    }
#endif
}

void board_power_init(void)
{
    log_info("Power init : %s", __FILE__);

    power_init(&power_param);

    //< close short key reset
    /* power_mclr(0); */
    //< close long key reset
    /* power_pin_reset(0); */

    power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, sleep_enter_callback, sleep_exit_callback, board_set_soft_poweroff);

//    wl_audio_clk_on();

    power_keep_dacvdd_en(0);

    power_wakeup_init(&wk_param);

    aport_edge_wkup_set_callback(aport_wakeup_callback);
    port_edge_wkup_set_callback(port_wakeup_callback);

    /* #if (!TCFG_IOKEY_ENABLE && !TCFG_ADKEY_ENABLE) */
    /* charge_check_and_set_pinr(0); */
/* #endif */
}
#endif
