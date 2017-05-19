#include "utils.h"
#include "stand.h"
#include "board.h"


//uint8_t RxBuf[RXBUFSIZE];// the receiver buffer.
//uint32_t RxHead = 0; // the circular buffer index
//uint32_t RxTail = 0;

//uint8_t isDataAvailable()
//{
//	return (RxTail != RxHead);
//}

//unsigned char _getchar()
//{
// alt_u8 temp;
//// while (RxTail == RxHead){
////	 nr_delay(1);
//// }
// temp = RxBuf[RxTail];
// if (++RxTail > (RXBUFSIZE -1))
// {
//	 RxTail = 0;
// }
// return(temp);
//}




volatile TMotCtrlW0 mc0;
volatile TMotCtrlW1 mc1;
uint16_t data0, data1, data2;

int32_t getPos(uint8_t mNum)
{
	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == SET) {
		Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("rx FIFO not empty on enter getPos\r\n");
	}
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	int32_t pos = 0;
	mc0.wNum = 0;
	mc0.motNum = mNum;

	//DEBUGOUT(" mc0.data %x\r\n", mc0.data);
	while(Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_TNF) != SET){
		DEBUGOUT("wait for TX \r\n");
	}
	Chip_SSP_SendFrame(LPC_SSP0, mc0.data);

	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
	DEBUGOUT("ddddd %x \r\n", Chip_SSP_ReceiveFrame(LPC_SSP0));

	mc1.wNum = 1;
	while(Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_TNF) != SET){
		DEBUGOUT("wait for TX \r\n");
	}
	Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET) ;
	DEBUGOUT("ddddd %x \r\n", Chip_SSP_ReceiveFrame(LPC_SSP0));


	mc1.wNum = 2;
	while(Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_TNF) != SET){
		DEBUGOUT("wait for TX \r\n");
	}
	Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) ==SET){
		DEBUGOUT("ddddd %x \r\n", Chip_SSP_ReceiveFrame(LPC_SSP0));
	}

	//data0 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	//data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	//data2 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	//Chip_SSP_SendFrame(LPC_SSP0, mc0.data);
	//DEBUGOUT("ddddd %x %x %x\r\n", data0, data1, data2);
	//data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	//Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
	//data2 = Chip_SSP_ReceiveFrame(LPC_SSP0);


	pos = (int32_t)*((int16_t*)&data0);

	//DEBUGOUT(" datam %x %x %x\r\n", data0 , data1, data2);
	//if(data0&0x8000){
		//DEBUGOUT(" datam %x %d %d\r\n", data0 , data0, pos);
//}

	//return (pos<<4);
	return pos;
}




void setDiv(uint8_t mNum, uint8_t e, uint8_t dir, uint32_t div)
{
	if(div > 0x186a)
		div = 0x186a;
	if(mNum == 0){
		//TMotCtrlW0 mc0;
		//TMotCtrlW1 mc1;
		//uint32_t d = (MoveDiveInverse<<dirInverseBit)|(e<<21)|(dir<<20)|div;
		//wr(mNum, d)

		mc0.wNum = 0;
		mc0.motNum = 0;
		mc0.dir = dir;


		mc1.ena = e;
		mc1.div = div;
		mc1.wNum = 1;

		Chip_SSP_SendFrame(LPC_SSP0, mc0.data);
		data0 = Chip_SSP_ReceiveFrame(LPC_SSP0);

		Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
		data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	}

}
void motorSetDir(uint8_t mNum, uint8_t dir)
{
	uint32_t d = (dir<<20);
//	switch(mNum){
//	case 0:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, d);
//		break;
//	case 1:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_1_BASE, d);
//		break;
//	case 2:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_2_BASE, d);
//		break;
//	case 3:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, d);
//		break;
//	case 4:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_4_BASE, d);
//		break;
//	case 5:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_5_BASE, d);
//		break;
//	case 6:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_6_BASE, d);
//		break;
//	case 7:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_7_BASE, d);
//		break;
//	case 8:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_8_BASE, d);
//		break;
//	case 9:
//		IOWR_ALTERA_AVALON_PIO_DATA(PIO_9_BASE, d);
//		break;
//	}
}

void motorPositionReset(uint8_t mNum)
{
	if(mNum == 0){
		//TMotCtrlW0 mc0;
		mc0.wNum = 0;
		mc0.motNum = 1;
		mc0.posReset = 1;
		Chip_SSP_SendFrame(LPC_SSP0, mc0.data);
		//mc0.posReset = 0;
		//Chip_SSP_SendFrame(LPC_SSP0, mc0.data);
		//data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	}
}

void motorDisable(uint8_t mNum)
{
	if(mNum == 0){
		//TMotCtrlW1 mc1;
		mc1.ena = 0;
		mc1.wNum = 1;

		Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
		data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	}
}


void motorEnable(uint8_t mNum)
{
	if(mNum == 0){
		//TMotCtrlW1 mc1;
		mc1.ena = 1;
		mc1.wNum = 1;

		Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
		data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
	}
}

bool getKoncState(uint8_t mNum)
{
	bool ret = false;
	switch(mNum){
	case 0:
		ret = Chip_GPIO_GetPinState(LPC_GPIO, 2, 0);
		break;
	case 1:
		ret = Chip_GPIO_GetPinState(LPC_GPIO, 2, 1);
		break;
	}
	ret = Chip_GPIO_GetPinState(LPC_GPIO, 2, 0);
	ret = !ret;
	return ret;
}

int32_t convertImpToMm(int32_t posImp)
{
	int32_t posInMm = (int)(posImp/(float)pulsePerMm);
	return posInMm;
}
