#include "sci.h"

/*SCI-B的发送接收数组*/
Uint16 Tx_Buffer_B[TXNUM]={0};   	// Send data for SCI-B
Uint16 Rx_Buffer_B[RXNUM];   	// Received data for SCI-B
/*SCI-D的发送接收数组*/
Uint16 Tx_Buffer_D[TXNUM]={0};      // Send data for SCI-D
Uint16 Rx_Buffer_D[RXNUM];      // Received data for SCI-D

/**
 * SCI IO初始化
 * function：
 *          1.初始化SCIB的TX RX
 *          2.初始化SCID的TX RX
 */
void SciIOCfg(void)
{
	/*SCIB：配置GPIO为上拉和复用功能*/ //注意这里端口引脚的复用需要找数据表中 GPIO Muxed Pins端口复用引脚的第几种复用方式 SCIB为2
    //RX--P19
	GPIO_SetupPinMux(19, GPIO_MUX_CPU1, 2);
	GPIO_SetupPinOptions(19, GPIO_INPUT, GPIO_PULLUP);  //这里应该是GPIO_PULLUP ->1  例程错误 原本例程为GPIO_PUSHPULL ->0  正确应该是GPIO_PULLUP 启用上拉
	//TX--18
	GPIO_SetupPinMux(18, GPIO_MUX_CPU1, 2);
	GPIO_SetupPinOptions(18, GPIO_OUTPUT, GPIO_ASYNC);

	/*SCID：配置GPIO为上拉和复用功能*/ //注意这里端口引脚的复用需要找数据表中 GPIO Muxed Pins端口复用引脚的第几种复用方式 SCID为6
	//RX--P105
	GPIO_SetupPinMux(105, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(105, GPIO_INPUT, GPIO_PULLUP);
	//TX--104
	GPIO_SetupPinMux(104, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(104, GPIO_OUTPUT, GPIO_ASYNC);

	//初始化发送的数组的帧头和帧尾
	Tx_Buffer_B[0]=0x0a;
	Tx_Buffer_B[TXNUM-1]=0x0d;

	Tx_Buffer_D[0]=0x0a;
	Tx_Buffer_D[TXNUM-1]=0x0d;
}

/*---------------------------------------SCIB----------------------------------*/

/**
 * SCI_B的接收中断
 */
interrupt void scibRxFifoIsr(void)
{
	Uint16 i;
	for(i=0;i<RXNUM;i++)
	{
		Rx_Buffer_B[i] = ScibRegs.SCIRXBUF.bit.SAR;
	}
	ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1;   			// Clear Interrupt flag
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;        // Issue PIE ack
}


/**
 * SCIB字符串发送函数
 */
void scib_msg(char * msg)
{
    Uint16 i;
    i = 0;
    while(msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

/**
 * SCIB数据发送函数
 * 只发低八位
 */
void scib_xmit(Uint16 data)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0){} 		// 等待发送完毕
    ScibRegs.SCITXBUF.all = data;
}

/**
 * SCI_B FIFO初始化
 */
void scib_fifo_init(void)
{
	ScibRegs.SCICCR.all = 0x0007;      		// 一个停止位 无回环
											// 无奇偶校验位 ,8个字符位 ,
									   	   	// async （空闲线）模式, 空闲线协议
	ScibRegs.SCICTL1.all = 0x0003;     		// 使能 TX, RX, 内部 SCICLK（SCI时钟）,
									   	   	// Disable RX ERR, SLEEP, TXWAKE
    //波特率高低位
	ScibRegs.SCIHBAUD.all = 0x0000;
	ScibRegs.SCILBAUD.all = SCI_PRD;		// 配置波特率为SCI_PRD

	ScibRegs.SCIFFTX.all = 0xE040;		// 使能SCI的FIFO功能，重置SCI,发送重置

	ScibRegs.SCIFFRX.bit.RXFFIENA = 1;		// 使能SCI FIFO发送功能
	ScibRegs.SCIFFRX.bit.RXFFIL = RXNUM;	// 设定接受位数为RXNUM 0h-10h
	ScibRegs.SCIFFRX.bit.RXFFINTCLR =1;		// FIFO计数中断标志位清零

	ScibRegs.SCIFFCT.all = 0x00;

	ScibRegs.SCICTL1.all = 0x0023;     		// 重启sci
	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;	// 重启sci接收fifo
}



/*----------------------------------------SCID--------------------------------------*/

/**
 * SCI_D接收中断
 */
interrupt void scidRxFifoIsr(void)
{
    Uint16 i;
    for(i=0;i<RXNUM;i++)
    {
        Rx_Buffer_D[i] = ScidRegs.SCIRXBUF.bit.SAR;
    }
    ScidRegs.SCIFFRX.bit.RXFFINTCLR = 1;            // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP8;        // Issue PIE ack
}


/**
 * SCID字符串发送函数
 */
void scid_msg(char * msg)
{
    Uint16 i;
    i = 0;
    while(msg[i] != '\0')
    {
        scid_xmit(msg[i]);
        i++;
    }
}

/**
 * SCID数据发送函数
 */
void scid_xmit(Uint16 data)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0){}      // 等待发送完毕
    ScidRegs.SCITXBUF.all = data;
}



void scid_fifo_init(void)
{
    ScidRegs.SCICCR.all = 0x0007;           // 一个停止位 无回环
                                            // 无奇偶校验位 ,8个字符位 ,
                                            // async （空闲线）模式, 空闲线协议
    ScidRegs.SCICTL1.all = 0x0003;          // 使能 TX, RX, 内部 SCICLK（SCI时钟）,
                                            // Disable RX ERR, SLEEP, TXWAKE
    //波特率高低位
    ScidRegs.SCIHBAUD.all = 0x0000;
    ScidRegs.SCILBAUD.all = SCI_PRD;        // 配置波特率为SCI_PRD

    ScidRegs.SCIFFTX.all = 0xE040;      // 使能SCI的FIFO功能，重置SCI,发送重置

    ScidRegs.SCIFFRX.bit.RXFFIENA = 1;      // 使能SCI FIFO发送功能
    ScidRegs.SCIFFRX.bit.RXFFIL = RXNUM;    // 设定接受位数为RXNUM 0h-10h
    ScidRegs.SCIFFRX.bit.RXFFINTCLR =1;     // FIFO计数中断标志位清零

    ScidRegs.SCIFFCT.all = 0x00;

    ScidRegs.SCICTL1.all = 0x0023;          // 重启sci
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;   // 重启sci接收fifo
}


void error(void)	//debug模式下错误会让程序停止在此
{
    asm("     ESTOP0"); // Test failed!! Stop!
    for (;;);
}
