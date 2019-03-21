#include "sci.h"

/*SCI-B�ķ��ͽ�������*/
Uint16 Tx_Buffer_B[TXNUM]={0};   	// Send data for SCI-B
Uint16 Rx_Buffer_B[RXNUM];   	// Received data for SCI-B
/*SCI-D�ķ��ͽ�������*/
Uint16 Tx_Buffer_D[TXNUM]={0};      // Send data for SCI-D
Uint16 Rx_Buffer_D[RXNUM];      // Received data for SCI-D

/**
 * SCI IO��ʼ��
 * function��
 *          1.��ʼ��SCIB��TX RX
 *          2.��ʼ��SCID��TX RX
 */
void SciIOCfg(void)
{
	/*SCIB������GPIOΪ�����͸��ù���*/ //ע������˿����ŵĸ�����Ҫ�����ݱ��� GPIO Muxed Pins�˿ڸ������ŵĵڼ��ָ��÷�ʽ SCIBΪ2
    //RX--P19
	GPIO_SetupPinMux(19, GPIO_MUX_CPU1, 2);
	GPIO_SetupPinOptions(19, GPIO_INPUT, GPIO_PULLUP);  //����Ӧ����GPIO_PULLUP ->1  ���̴��� ԭ������ΪGPIO_PUSHPULL ->0  ��ȷӦ����GPIO_PULLUP ��������
	//TX--18
	GPIO_SetupPinMux(18, GPIO_MUX_CPU1, 2);
	GPIO_SetupPinOptions(18, GPIO_OUTPUT, GPIO_ASYNC);

	/*SCID������GPIOΪ�����͸��ù���*/ //ע������˿����ŵĸ�����Ҫ�����ݱ��� GPIO Muxed Pins�˿ڸ������ŵĵڼ��ָ��÷�ʽ SCIDΪ6
	//RX--P105
	GPIO_SetupPinMux(105, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(105, GPIO_INPUT, GPIO_PULLUP);
	//TX--104
	GPIO_SetupPinMux(104, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(104, GPIO_OUTPUT, GPIO_ASYNC);

	//��ʼ�����͵������֡ͷ��֡β
	Tx_Buffer_B[0]=0x0a;
	Tx_Buffer_B[TXNUM-1]=0x0d;

	Tx_Buffer_D[0]=0x0a;
	Tx_Buffer_D[TXNUM-1]=0x0d;
}

/*---------------------------------------SCIB----------------------------------*/

/**
 * SCI_B�Ľ����ж�
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
 * SCIB�ַ������ͺ���
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
 * SCIB���ݷ��ͺ���
 * ֻ���Ͱ�λ
 */
void scib_xmit(Uint16 data)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0){} 		// �ȴ��������
    ScibRegs.SCITXBUF.all = data;
}

/**
 * SCI_B FIFO��ʼ��
 */
void scib_fifo_init(void)
{
	ScibRegs.SCICCR.all = 0x0007;      		// һ��ֹͣλ �޻ػ�
											// ����żУ��λ ,8���ַ�λ ,
									   	   	// async �������ߣ�ģʽ, ������Э��
	ScibRegs.SCICTL1.all = 0x0003;     		// ʹ�� TX, RX, �ڲ� SCICLK��SCIʱ�ӣ�,
									   	   	// Disable RX ERR, SLEEP, TXWAKE
    //�����ʸߵ�λ
	ScibRegs.SCIHBAUD.all = 0x0000;
	ScibRegs.SCILBAUD.all = SCI_PRD;		// ���ò�����ΪSCI_PRD

	ScibRegs.SCIFFTX.all = 0xE040;		// ʹ��SCI��FIFO���ܣ�����SCI,��������

	ScibRegs.SCIFFRX.bit.RXFFIENA = 1;		// ʹ��SCI FIFO���͹���
	ScibRegs.SCIFFRX.bit.RXFFIL = RXNUM;	// �趨����λ��ΪRXNUM 0h-10h
	ScibRegs.SCIFFRX.bit.RXFFINTCLR =1;		// FIFO�����жϱ�־λ����

	ScibRegs.SCIFFCT.all = 0x00;

	ScibRegs.SCICTL1.all = 0x0023;     		// ����sci
	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;	// ����sci����fifo
}



/*----------------------------------------SCID--------------------------------------*/

/**
 * SCI_D�����ж�
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
 * SCID�ַ������ͺ���
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
 * SCID���ݷ��ͺ���
 */
void scid_xmit(Uint16 data)
{
    while (ScidRegs.SCIFFTX.bit.TXFFST != 0){}      // �ȴ��������
    ScidRegs.SCITXBUF.all = data;
}



void scid_fifo_init(void)
{
    ScidRegs.SCICCR.all = 0x0007;           // һ��ֹͣλ �޻ػ�
                                            // ����żУ��λ ,8���ַ�λ ,
                                            // async �������ߣ�ģʽ, ������Э��
    ScidRegs.SCICTL1.all = 0x0003;          // ʹ�� TX, RX, �ڲ� SCICLK��SCIʱ�ӣ�,
                                            // Disable RX ERR, SLEEP, TXWAKE
    //�����ʸߵ�λ
    ScidRegs.SCIHBAUD.all = 0x0000;
    ScidRegs.SCILBAUD.all = SCI_PRD;        // ���ò�����ΪSCI_PRD

    ScidRegs.SCIFFTX.all = 0xE040;      // ʹ��SCI��FIFO���ܣ�����SCI,��������

    ScidRegs.SCIFFRX.bit.RXFFIENA = 1;      // ʹ��SCI FIFO���͹���
    ScidRegs.SCIFFRX.bit.RXFFIL = RXNUM;    // �趨����λ��ΪRXNUM 0h-10h
    ScidRegs.SCIFFRX.bit.RXFFINTCLR =1;     // FIFO�����жϱ�־λ����

    ScidRegs.SCIFFCT.all = 0x00;

    ScidRegs.SCICTL1.all = 0x0023;          // ����sci
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;   // ����sci����fifo
}


void error(void)	//debugģʽ�´�����ó���ֹͣ�ڴ�
{
    asm("     ESTOP0"); // Test failed!! Stop!
    for (;;);
}
