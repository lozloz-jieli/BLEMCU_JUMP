#ifndef _IIC_HW_H_
#define _IIC_HW_H_

#include "system/generic/typedef.h"

#define IIC_HW_NUM                  1
#define IIC_PORT_GROUP_NUM          4

#define iic_enable(reg)             (reg->CON0 |= BIT(0))
#define iic_disable(reg)            (reg->CON0 &= ~BIT(0))
#define iic_kick_start(reg)         (reg->CON0 |= BIT(1))

#define iic_isel_direct(reg)        (reg->CON0 &= ~BIT(3))
#define iic_isel_filter(reg)        (reg->CON0 |= BIT(3))

#define iic_dir_out(reg)            (reg->CON0 &= ~BIT(4))
#define iic_dir_in(reg)             (reg->CON0 |= BIT(4))

#define iic_int_enable(reg)         (reg->CON0 |= BIT(8))
#define iic_int_disable(reg)        (reg->CON0 &= ~BIT(8))

#define iic_is_pnding(reg)          ((reg->CON0 & BIT(31)))
#define iic_pnding_clr(reg)         ((reg->CON0 |= BIT(7)))

#define iic_auto_ack(reg) 			(reg->CON0 &= ~BIT(11)) //该bit默认设0
#define iic_force_nack(reg) 		(reg->CON0 |= BIT(11))

#define iic_restart(reg) 			(reg->CON0 |= BIT(13)) //没有stop, 发start

#define iic_baud_reg(reg)           (reg->BAUD)
#define iic_buf_reg(reg)            (reg->BUF)

//主机模式相关配置
#define iic_host_mode(reg)          				(reg->CON0 &= ~BIT(2))
#define iic_host_send_stop(reg)						(reg->CON0 |= BIT(5))
#define iic_host_is_stop_pending(reg)				((reg->CON0 & BIT(29)))
#define iic_host_send_is_ack(reg)   				((reg->CON0 & BIT(30)))
#define iic_host_receive_single_byte(reg) 			(reg->CON0 &= ~BIT(9))
#define iic_host_receive_continue_byte(reg) 		(reg->CON0 |= BIT(9))
#define iic_host_receive_continue_byte_stop(reg) 	(reg->CON0 |= BIT(10))
#define iic_host_nack_auto_stop(reg) 				(reg->CON0 |= BIT(14))
#define iic_host_read_kick_start(reg) 				(reg->CON0 |= BIT(16))

//从机模式相关配置
#define iic_slave_mode(reg)         			(reg->CON0 |= BIT(2))
#define iic_slave_dma_enable(reg)         		(reg->CON0 |= BIT(15))
#define iic_slave_dma_disable(reg)         		(reg->CON0 &= ~BIT(15))
#define iic_slave_dma_is_enable(reg)         	(reg->CON0 & BIT(15))
#define iic_slave_dma_buf(reg)         			(reg->CON1)
#define iic_slave_dma_big_endian(reg) 			(reg->CON0 |= BIT(17))
#define iic_slave_dma_little_endian(reg) 		(reg->CON0 &= ~BIT(17))
#define iic_slave_dma_buf_depth(reg)         	(reg->CON2)
#define iic_slave_dma_get_buf_len(reg)         	(((reg->CON2) & 0xFFFF) * sizeof(u32))
#define iic_slave_scl_pull_down_enble(reg) 		(reg->CON0 |= BIT(12))
#define iic_slave_scl_pull_down_release(reg) 	(reg->CON0 |= BIT(6))
#define iic_slave_is_required_send(reg) 		(reg->CON0 & BIT(28))
#define iic_slave_send_is_end(reg) 				(reg->CON0 & BIT(27))



typedef const int hw_iic_dev;

struct hw_iic_config {
    u8 port;   //example: 'A', 'B', 'C', 'D'
    u32 baudrate;
    u8 hdrive;
    u8 io_filter;
    u8 io_pu;
};

extern const struct hw_iic_config hw_iic_cfg[];

int hw_iic_init(hw_iic_dev iic);
void hw_iic_uninit(hw_iic_dev iic);
void hw_iic_suspend(hw_iic_dev iic);
void hw_iic_resume(hw_iic_dev iic);
void hw_iic_start(hw_iic_dev iic);
void hw_iic_stop(hw_iic_dev iic);
u8 hw_iic_tx_byte(hw_iic_dev iic, u8 byte, u8 start, u8 end, u8 restart);
u8 hw_iic_rx_byte_pre_send_dev_addr(hw_iic_dev iic, u8 addr, u8 start, u8 end, u8 restart);
u8 hw_iic_rx_byte(hw_iic_dev iic, u8 ack);
int hw_iic_read_buf(hw_iic_dev iic, void *buf, int len);
int hw_iic_write_buf(hw_iic_dev iic, const void *buf, int len);
int hw_iic_set_baud(hw_iic_dev iic, u32 baud);


#endif
