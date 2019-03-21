#ifndef __SCI_H
#define __SCI_H
#include "F28x_Project.h"
//波特率计算方式
#define CPU_FREQ       	200E6
#define LSPCLK_FREQ     CPU_FREQ/4
#define SCI_FREQ        115200
#define SCI_PRD         (LSPCLK_FREQ/(SCI_FREQ*8))-1
//发送和接收的字节数
#define RXNUM			3
#define TXNUM           6

void SciIOCfg(void);
interrupt void scibRxFifoIsr(void);
interrupt void scidRxFifoIsr(void);

void scib_msg(char * msg);
void scid_msg(char * msg);

void scib_xmit(Uint16 data);
void scid_xmit(Uint16 data);

void scib_fifo_init(void);
void scid_fifo_init(void);

void error(void);


#endif
