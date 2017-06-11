#include "utils.h"
#include "stand.h"
#include "board.h"
#include <string.h>

void sspExch(TMotCtrl *mc, uint16_t *rxData, uint8_t cnt)
{
	while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == SET) {
		Chip_SSP_ReceiveFrame(LPC_SSP0);
		DEBUGOUT("rx FIFO not empty on enter getPos\r\n");
	}
	/* Clear status */
	Chip_SSP_ClearIntPending(LPC_SSP0, SSP_INT_CLEAR_BITMASK);

	for(int i=0; i<cnt; i++){
		Chip_SSP_SendFrame(LPC_SSP0, mc[i].data);
		while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) !=SET);
		rxData[i] = Chip_SSP_ReceiveFrame(LPC_SSP0);
		//DEBUGOUT("%04x -> %04x \r\n", mc[i].data , rxData[i]);
	}

}

int32_t getPos(uint8_t mNum)
{
	uint16_t rxData[3];
	TMotCtrl mc[3];
	int32_t posS=0, posSL=0;
	uint32_t pos = 0;

	memset(&(mc[0]), 0, 3*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type4.wNum = 4;
	mc[2].type4.wNum = 4;


	sspExch(&(mc[0]), &(rxData[0]), 3);
	pos = (int32_t)((rxData[1]|(rxData[2]<<16)));

	if(rxData[2]&0x8)
		pos |= 0xfff00000;

	posS = *((int32_t*)&pos);

	while(1){
		posSL = posS;
		sspExch(&(mc[0]), &(rxData[0]), 3);
		pos = (int32_t)((rxData[1]|(rxData[2]<<16)));

		if(rxData[2]&0x8)
			pos |= 0xfff00000;

		posS = *((int32_t*)&pos);
		if(abs(posS - posSL) < 3000)
			break;


		DEBUGOUT("pos get repeat\r\n");
	}
	//return (pos<<4);
	return posS;
}


void motorSetDiv(uint8_t mNum, uint32_t div)
{
	if(div > 0x186a)
		div = 0x186a;

	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type2.wNum = 2; mc[1].type2.div = div;

	sspExch(&(mc[0]), &(rxData[0]), 2);
}
void motorSetDir(uint8_t mNum, uint8_t dir)
{
	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0;	mc[0].type0.motNum = mNum;
	mc[1].type1.wNum = 1;	mc[1].type1.dir = dir;

	sspExch(&(mc[0]), &(rxData[0]), 2);
}

void motorPositionReset(uint8_t mNum)
{
	uint16_t rxData[1];
	TMotCtrl mc[1];
	memset(&(mc[0]), 0, 1*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0;
	mc[0].type0.motNum = mNum;
	mc[0].type0.posReset = 1;
	//mc[0].type0.motNum = mNum;

	sspExch(&(mc[0]), &(rxData[0]), 1);
}

void motorDisable(uint8_t mNum)
{
	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].type3.wNum = 3; mc[1].type3.ena = 0;

	sspExch(&(mc[0]), &(rxData[0]), 2);
}


void motorEnable(uint8_t mNum)
{
	uint16_t rxData[2];
	TMotCtrl mc[2];
	memset(&(mc[0]), 0, 2*sizeof(TMotCtrl));
	mc[0].data =0; mc[0].type0.wNum = 0; mc[0].type0.motNum = mNum;
	mc[1].data =0; mc[1].type3.wNum = 3; mc[1].type3.ena = 1;

	sspExch(&(mc[0]), &(rxData[0]), 2);
}

bool getKoncState(uint8_t mNum)
{
	bool ret = false;
	switch(mNum){
		case 0: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 1); break;
		//case 1:
			//ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 2);
			//break;
		//case 3: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 2); break;
		case 5: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 7); break;
		case 6: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 4); break;
		case 7: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 6); break;
		case 8: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 9); break;
		case 9: ret = !Chip_GPIO_GetPinState(LPC_GPIO, 2, 8); break;
		default:
			ret = true;
	}
	//ret = Chip_GPIO_GetPinState(LPC_GPIO, 2, 1);

	//ret = true;
	return ret;
}

int32_t impToMm(int32_t posImp)
{
	return posImp/pulsePerMm;
}
int32_t mmToImp(int32_t mm)
{
	return mm*pulsePerMm;
}
//#define filterNum 10
//int32_t posArr[filterNum] = {0, 0, 0, 0 , 0, 0, 0, 0, 0 , 0};
void calcMoveParams(TMotorData *pMd, int32_t pos, TPosCmd *pPosCmd)
{
	//for(int i=1; i<filterNum;i++){
	//	posArr[i] = posArr[i-1];
	//}
	//posArr[0] = pos;

	//int32_t avPos=0;
	//for(int i=0; i<filterNum;i++){
	//	avPos += posArr[i];
	//}
	//avPos/=filterNum;
	//pos /= 10;
	//pos *=10;

	pMd->state = constSpeedTimeCtrl;
	pMd->posZadI = pPosCmd->posImp;
	int32_t deltaPos = pMd->posZadI -  pos; //avPos
	//pMd->speedZadIPS = (abs(deltaPos)*1000)/ pPosCmd->time;
	pMd->speedZadIPS = (abs(deltaPos)*1000)/ mcContrPeriodms;
	if(deltaPos == 0)
		pMd->dir = DIR_STOP;
	else
		pMd->dir = deltaPos>0? DIR_UP : DIR_DOWN;

	pMd->startCmdProcessTime = xTaskGetTickCount();
	//pMd->cmdEndProcessTime = pMd->startCmdProcessTime +  pPosCmd->time +  pPosCmd->time/10; //10%
	pMd->cmdEndProcessTime = pMd->startCmdProcessTime +  mcContrPeriodms; //10%
	//DEBUGOUT("move to %d(%d) f %d(%d) d %d IPS %d\r\n", pMd->posZadI, impToMm(pMd->posZadI), pos, impToMm(pos), deltaPos, pMd->speedZadIPS);

}

void bigTest()
{
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorDisable(0);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorPositionReset(0);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorSetDiv(0, 0x186a);
	motorEnable(0);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorPositionReset(0);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorSetDiv(0, 0x4e2);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	//motorEnable(0);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));

	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorEnable(1);
	DEBUGOUT("pos: %d %d\r\n", getPos(0), getPos(1));
	motorSetDiv(0, 0x4d8);
	uint16_t data0, rcv;
	SSP_STATUS_T stat;
	for(int i=0; ;){
//		//data0 = 0xabab;
//		if(Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_TFE) == SET){
//			while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == SET) {
//				rcv = Chip_SSP_ReceiveFrame(LPC_SSP0);
//				DEBUGOUT("freeFifo 				rcv %x\r\n", rcv);
//			}
//
//			Chip_SSP_SendFrame(LPC_SSP0, i);
//			DEBUGOUT("snd %x\r\n", i++);
//			Chip_SSP_SendFrame(LPC_SSP0, i);
//			DEBUGOUT("snd %x\r\n", i++);
//
//			while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == SET) {
//				rcv = Chip_SSP_ReceiveFrame(LPC_SSP0);
//				DEBUGOUT(" 				rcv %x\r\n", rcv);
//			}
//		}


		DEBUGOUT("%d %d\r\n", getPos(0), getPos(1));
		//Chip_SSP_SendFrame(LPC_SSP0, data0);
		//rcv = Chip_SSP_ReceiveFrame(LPC_SSP0);
		//DEBUGOUT("%x\r\n", rcv);
	}
}
extern RINGBUFF_T posCmdRB[MOTOR_COUNT];
#define addCmdToRb(a, b) pc.posImp = mmToImp(a); RingBuffer_Insert(&(posCmdRB[b]), &pc)

void fillCustom2()
{
	TPosCmd pc;
	//pc.time = 100;
	for(int i=0; i<MOTOR_COUNT; i++){
	//	addCmdToRb(1);
	//	addCmdToRb(2);
	//	addCmdToRb(3);
	//	addCmdToRb(4);
	//	addCmdToRb(5);
	//	addCmdToRb(7);
	//	addCmdToRb(11);
	//	addCmdToRb(15);
		addCmdToRb(20, i);
		addCmdToRb(21, i);
		addCmdToRb(25, i);
		addCmdToRb(30, i);
		addCmdToRb(38, i);
		addCmdToRb(47, i);
		addCmdToRb(58, i);
		addCmdToRb(71, i);
		addCmdToRb(84, i);
		addCmdToRb(99, i);
		addCmdToRb(116, i);
		addCmdToRb(132, i);
		addCmdToRb(150, i);
		addCmdToRb(169, i);
		addCmdToRb(188, i);
		addCmdToRb(207, i);
		addCmdToRb(227, i);
		addCmdToRb(246, i);
		addCmdToRb(266, i);
		addCmdToRb(285, i);
		addCmdToRb(304, i);
		addCmdToRb(322, i);
		addCmdToRb(339, i);
		addCmdToRb(356, i);
		addCmdToRb(372, i);
		addCmdToRb(387, i);
		addCmdToRb(400, i);
		addCmdToRb(413, i);
		addCmdToRb(423, i);
		addCmdToRb(432, i);
		addCmdToRb(440, i);
		addCmdToRb(445, i);
		addCmdToRb(448, i);
		addCmdToRb(450, i);
		addCmdToRb(451, i);
		addCmdToRb(451, i);
		addCmdToRb(449, i);
		addCmdToRb(449, i);
		addCmdToRb(449, i);
		addCmdToRb(450, i);
		addCmdToRb(451, i);
		addCmdToRb(450, i);
		addCmdToRb(448, i);
		addCmdToRb(444, i);
		addCmdToRb(438, i);
		addCmdToRb(431, i);
		addCmdToRb(421, i);
		addCmdToRb(411, i);
		addCmdToRb(398, i);
		addCmdToRb(384, i);
		addCmdToRb(370, i);
		addCmdToRb(354, i);
		addCmdToRb(337, i);
		addCmdToRb(319, i);
		addCmdToRb(301, i);
		addCmdToRb(282, i);
		addCmdToRb(263, i);
		addCmdToRb(244, i);
		addCmdToRb(224, i);
		addCmdToRb(205, i);
		addCmdToRb(185, i);
		addCmdToRb(166, i);
		addCmdToRb(147, i);
		addCmdToRb(129, i);
		addCmdToRb(112, i);
		addCmdToRb(96, i);
		addCmdToRb(82, i);
		addCmdToRb(68, i);
		addCmdToRb(56, i);
		addCmdToRb(45, i);
		addCmdToRb(36, i);
		addCmdToRb(29, i);
		addCmdToRb(24, i);
		addCmdToRb(21, i);
		addCmdToRb(20, i);
	//	addCmdToRb(15);
	//	addCmdToRb(11);
	//	addCmdToRb(7);
	//	addCmdToRb(5);
	//	addCmdToRb(4);
	//	addCmdToRb(3);
	//	addCmdToRb(2);
	//	addCmdToRb(1);
	}
}

//void fillCustom1()
//{
//	TPosCmd pc;
//	pc.time = 500;
//	for(int i=0; i<MOTOR_COUNT; i++){
//		pc.posImp = 1000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
//		pc.posImp = 5000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
//		pc.posImp = 10000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
//		pc.posImp = 15000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
//		pc.time = 20000;
//		/*pc.pos = 15000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 25000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 35000;	RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 50000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 65000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 80000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 95000; RingBuffer_Insert(&posCmdRB, &pc);
//		pc.pos = 110000; RingBuffer_Insert(&posCmdRB, &pc);*/
//		pc.posImp = 496000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
//	//	//pc.pos = 640000; RingBuffer_Insert(&posCmdRB, &pc);
//	//	//pc.pos = 600000; RingBuffer_Insert(&posCmdRB, &pc);
//	//	//pc.pos = 580000; RingBuffer_Insert(&posCmdRB, &pc);
//
//	}
//}



