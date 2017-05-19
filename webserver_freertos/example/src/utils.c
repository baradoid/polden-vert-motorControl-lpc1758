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




//volatile TMotCtrlW0 mc0;
//volatile TMotCtrlW1 mc1;
uint16_t data0, data1, data2;

int32_t getPos(uint8_t mNum)
{
	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == SET) {
		Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("rx FIFO not empty on enter getPos\r\n");
	}
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);


	uint16_t rxData[3];
	TMotCtrl mc[3];
	memset(&(mc[0]), 0, 3*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type4.wNum = 4;
	mc[2].type4.wNum = 4;
	for(int i=0; i<3; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}
	int32_t pos = 0;
	pos = (int32_t)((rxData[1]|(rxData[2]<<16)));

	//return (pos<<4);
	return pos;
}


void motorSetDiv(uint8_t mNum, uint32_t div)
{
	if(div > 0x186a)
		div = 0x186a;
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;


	mc[1].type2.wNum = 2; mc[1].type2.div = div;

	for(int i=0; i<2; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}

}
void motorSetDir(uint8_t mNum, uint8_t dir)
{
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0;	mc[0].type0.motNum = mNum;
	mc[1].type1.wNum = 1;	mc[1].type1.dir = dir;

	for(int i=0; i<2; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}
}

void motorPositionReset(uint8_t mNum)
{
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	uint16_t rxData[1];
	TMotCtrl mc[1];
	memset(&(mc[0]), 0, 1*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0;
	mc[0].type0.motNum = mNum;
	mc[0].type0.posReset = 1;
	//mc[0].type0.motNum = mNum;

	for(int i=0; i<1; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}
}

void motorDisable(uint8_t mNum)
{
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type3.wNum = 3; mc[1].type3.ena = 0;

	for(int i=0; i<2; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}
}


void motorEnable(uint8_t mNum)
{
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type3.wNum = 3; mc[1].type3.ena = 1;

	for(int i=0; i<2; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
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
