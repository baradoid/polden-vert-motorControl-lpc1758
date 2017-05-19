#include <stdint.h>
#include <string.h>
#include "stand.h"
#include "ring_buffer.h"
#include "utils.h"


#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include <string.h>
#include "board.h"
#include "lpc_phy.h"
#include "arch\lpc17xx_40xx_emac.h"
#include "arch\lpc_arch.h"
#include "arch\sys_arch.h"
#include "lpc_phy.h"/* For the PHY monitor support */


#include "ring_buffer.h"

typedef struct{
	int32_t posImp;
	uint32_t time;
} TPosCmd;

/* Transmit and receive ring buffers */
#define POS_CMD_RB_SIZE 512	/* Receive */
RINGBUFF_T posCmdRB[MOTOR_COUNT];
uint8_t posCmdBuff[MOTOR_COUNT*POS_CMD_RB_SIZE*sizeof(TPosCmd)];

extern RINGBUFF_T uartRxRb;

void fillCustom1()
{
	TPosCmd pc;
	pc.time = 500;
	for(int i=0; i<MOTOR_COUNT; i++){
		pc.posImp = 1000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
		pc.posImp = 5000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
		pc.posImp = 10000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
		pc.posImp = 15000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
		pc.time = 20000;
		/*pc.pos = 15000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 25000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 35000;	RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 50000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 65000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 80000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 95000; RingBuffer_Insert(&posCmdRB, &pc);
		pc.pos = 110000; RingBuffer_Insert(&posCmdRB, &pc);*/
		pc.posImp = 496000; RingBuffer_Insert(&(posCmdRB[i]), &pc);
	//	//pc.pos = 640000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 600000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 580000; RingBuffer_Insert(&posCmdRB, &pc);

	}
}


#define posMMtoPosImp(a) (50+(a*600));
#define addCmdToRb(a, b) pc.posImp = posMMtoPosImp(a); RingBuffer_Insert(&(posCmdRB[b]), &pc)

void fillCustom2()
{
	TPosCmd pc;
	pc.time = 100;
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


TMotorData mst[MOTOR_COUNT];
void vUartctrl(void *pvParameters)
{
//	mc0.wNum = 0;
//	mc0.motNum = 0;
//	mc0.dir = 1;
//
//	mc1.div = 0x186a;
//	mc1.ena = 1;
//	mc1.wNum = 1;

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




	//TPosCmd posCmdRB[100];
	//int32_t posCmdRbHead = 0;
	//int32_t posCmdRbTail = 0;
	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	for(int i=0; i<MOTOR_COUNT; i++){
		int bufInd = i*POS_CMD_RB_SIZE*sizeof(TPosCmd);
		RingBuffer_Init(&(posCmdRB[i]), &(posCmdBuff[bufInd]), sizeof(TPosCmd), POS_CMD_RB_SIZE);

	}



	char msg[200];

	strcpy(msg, "hello\r\n");

	#define recvBufLen 100
	uint8_t inputStrInd=0;
	char inputStr[recvBufLen];
	//Chip_UART_SendBlocking(LPC_UART0, msg, sizeof(msg));

	uint32_t lastTickVal = 0;
	//uint32_t lastMtcVal = 0;
	//int divVal = 0x4e2;
	//int divInc = -5;
	int ledNum = 0;
	uint32_t itCnt=0;

	for(int i=0; i<MOTOR_COUNT; i++){
		mst[i].speedZadIPS = 0;
		mst[i].speedCurIPS = 0;
		mst[i].posZadI = 0;
		mst[i].bReverse = 0;
		motorPositionReset(i);
		if(getKoncState(i) == true){
			mst[i].state = seekKonc;//idle;


		}
		else{
			mst[i].state = seekKonc;
			DEBUGOUT("%d seek for Konc \r\n", i);

		}


	}


	TPosCmd pc;
	TMotorData *pMd;
//	while(getKoncState(0) == false){;
//		DEBUGOUT("%d seeking for Konc \r\n", 0);
//		uint32_t div = SYS_CLOCK/4000;
//		setDiv(0, MOT_ENA, DIR_DOWN, div);
//	}

	int32_t deltaPos;
	uint32_t idlePauseStart = 0;
	bool bKoncState[MOTOR_COUNT];
	bool bKs;
	int32_t pos;
	uint32_t div;
	TPosCmd posCmd;

	pc.time = 4000;

	for(int i=0; i<MOTOR_COUNT; i++){
		addCmdToRb(20, i);
	}
	for(int i=0; ;i++){
		//sprintf(msg, "data %d", i);
		//strcpy(msg, "hello\r\n");
		//Chip_UART_SendBlocking(LPC_UART0, msg, sizeof(msg));

		for(int mi=0; mi<MOTOR_COUNT; mi++){
			pMd = &(mst[mi]);
			bKs = getKoncState(mi);
			if(bKoncState[mi] != bKs){
				bKoncState[mi] = bKs;
				DEBUGOUT("koncState[%d]-> %d\r\n", mi, bKs);
			}
			switch(pMd->state){
			case idle:
				if(idlePauseStart == 0){
					idlePauseStart = xTaskGetTickCount();
				}
				else if((xTaskGetTickCount() -idlePauseStart)> 1000){
					idlePauseStart = 0;

					if(RingBuffer_Pop(&(posCmdRB[mi]), &posCmd)){
						DEBUGOUT("%d idle new cmd p%d t%d->constSpeedTimeCtrl\r\n", mi, posCmd.posImp, posCmd.time);
						pMd->state = constSpeedTimeCtrl;
						pMd->posZadI = posCmd.posImp;
						int32_t deltaPos = pMd->posZadI - getPos(mi);
						pMd->speedZadIPS = (abs(deltaPos)*1000)/posCmd.time;
						if(deltaPos == 0)
							pMd->dir = DIR_STOP;
						else
							pMd->dir = deltaPos>0? DIR_UP : DIR_DOWN;

						pMd->startCmdProcessTime = xTaskGetTickCount();
						pMd->cmdEndProcessTime = pMd->startCmdProcessTime + posCmd.time;

						//DEBUGOUT("move to %d delta %d speed IPS %d\r\n", mst[mi].posZadI, deltaPos, mst[mi].speedZadIPS);
					}
//					else{
//						fillCustom2();
//					}
				}
				break;

			case seekKonc:
				//bKs = getKoncState(mi);

				if(bKs == false){
					pMd->dir = DIR_DOWN;
					div = SYS_CLOCK/12000;
					setDiv(mi, MOT_ENA, pMd->dir, div);
					pMd->state = seekKonc;
				}
				else{
					setDiv(mi, MOT_DIS, pMd->dir, div);
					motorPositionReset(mi);
					pMd->state = idle;
					DEBUGOUT("%d konc  found -> idle\r\n", mi);
					//fillCustom1();
				}

				break;


				case constSpeedTimeCtrl:
					pos = getPos(mi);

					deltaPos = mst[mi].posZadI - pos;
					//DEBUGOUT("move to %d pos %d delta %d speed IPS %d\r\n", mst[mi].posZadI, pos, deltaPos, mst[mi].speedZadIPS);
//					bool borderReached = false;
//					if(pMd->dir==DIR_UP)
//						borderReached = (pos>=pMd->posZadI);
//					else if(pMd->dir==DIR_DOWN)
//						borderReached = (pos<=pMd->posZadI)

					bool upBorderReached = ((pMd->dir==DIR_UP) && (pos>=pMd->posZadI));
					bool downBorderReached = ((pMd->dir==DIR_DOWN) && (pos<=pMd->posZadI));
					bool bTimeReached = (xTaskGetTickCount() > pMd->cmdEndProcessTime);
					if(upBorderReached){
						DEBUGOUT("%d upBorder!\r\n", mi);
					}
					if(downBorderReached){
						DEBUGOUT("%d downBorder!\r\n", mi);
					}
					if(bTimeReached){
						DEBUGOUT("%d bTimeReached and dir=%d ur:%d dr:%d cmdrb:%d\r\n", mi, pMd->dir, upBorderReached, downBorderReached, RingBuffer_GetCount(&(posCmdRB[mi])));

					}
					if(bTimeReached && (pMd->dir != DIR_STOP) &&(upBorderReached == false) && (downBorderReached == false)){
						DEBUGOUT("%d moving error. going to errorState\r\n", mi);
						pMd->state = errorState;
						break;
					}



					if( ((pMd->dir != DIR_STOP)&&(upBorderReached || downBorderReached)) ||
						((pMd->dir == DIR_STOP)&&bTimeReached)
					){
						if(RingBuffer_Pop(&(posCmdRB[mi]), &posCmd)){
							DEBUGOUT("%d constSpeedTimeCtrl new cmd p%d t%d\r\n", mi, posCmd.posImp, posCmd.time);
							pMd->state = constSpeedTimeCtrl;
							pMd->posZadI = posCmd.posImp;
							int32_t deltaPos = pMd->posZadI - getPos(mi);
							pMd->speedZadIPS = (abs(deltaPos)*1000)/posCmd.time;
							if(deltaPos == 0)
								pMd->dir = DIR_STOP;
							else
								pMd->dir = deltaPos>0? DIR_UP : DIR_DOWN;

							pMd->startCmdProcessTime = xTaskGetTickCount();
							pMd->cmdEndProcessTime = pMd->startCmdProcessTime + posCmd.time;


							//DEBUGOUT("move to %d delta %d speed IPS %d\r\n", mst[mi].posZadI, deltaPos, mst[mi].speedZadIPS);
						}
						else{
							DEBUGOUT("%d constSpeedTimeCtrl no cmd -> idle\r\n",  mi);
							pMd->state = idle;
							//pMd->state = seekKonc;

							div = SYS_CLOCK/1000;
							setDiv(mi, MOT_DIS, pMd->dir, div);
						}
					}

					else{
						//DEBUGOUT("constSpeedTimeCtrl p:%d pz:%d pd:%d\r\n", pos, pMd->posZadI, pMd->dir);
						pMd->state = constSpeedTimeCtrl;
						if(pMd->dir == DIR_STOP){
							setDiv(mi, MOT_DIS, pMd->dir, div);
						}
						else{
							div = SYS_CLOCK/pMd->speedZadIPS;
							setDiv(mi, MOT_ENA, pMd->dir, div);
						}

					}
					break;
			}

		}
		if(xTaskGetTickCount() - lastTickVal > 1000){
			lastTickVal = xTaskGetTickCount() ;
//			char s;
//			if(getPos(0) >=0) s = '+';
//			else s = '-';
			DEBUGOUT("it %x %x  ", xTaskGetTickCount(), itCnt);

			for(int i=0; i<MOTOR_COUNT; i++){
				int32_t pos = getPos(i);
				DEBUGOUT("%x(%d) ", pos, convertImpToMm(pos));
			}
			DEBUGOUT("cmdrb:");
			for(int i=0; i<MOTOR_COUNT; i++){
				DEBUGOUT("%d ", RingBuffer_GetCount(&(posCmdRB[i])));
			}
			DEBUGOUT("\r\n");

			itCnt = 0;

			//IOWR_ALTERA_AVALON_PIO_DATA(PIO_LEDS_BASE, 1<<ledNum);
			if(ledNum++ >7)
				ledNum = 0;

		}
//		if(alt_nticks() - lastMtcVal > 50){
//			lastMtcVal = alt_nticks() ;
//
//
//			//IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, ((1<<21)|(divVal)));
//			IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, ((1<<21)|(divVal)));
//			divVal+=divInc;
//			if(divVal <= 0xB2){
//				divInc = 5;
//			}
//			if(divVal >= 0x4e2){
//				divInc = -5;
//			}
//
//		}
		itCnt++;
		//DEBUGOUT("0x%x 0x%x\r\n" , data0, data1);


		//while(Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR){
		char prompt = 0;
		while(RingBuffer_Pop(&uartRxRb, &prompt)){

			//prompt = DEBUGIN();
			inputStr[inputStrInd++] = prompt;
			if ((prompt == '\n') || (inputStrInd == recvBufLen )){
				bool bMotNumInited = false;
				bool bPosInited = false;
				bool bTimeInited = false;
				bool bVelocityInited = false;

				uint32_t time;
				inputStr[inputStrInd] = 0;
				//DEBUGOUT("string recvd %s", inputStr);
				inputStrInd = 0;
				uint8_t motInd = -1;
				int32_t pos = 0;		//procents*10
				int32_t posImp = 0;
				uint16_t velocity = 0; //mm per sec
				if(inputStr[0] == 'S'){
					motInd = atoi(&(inputStr[1]));

					if(motInd > (MOTOR_COUNT-1)){
					//if(!((motNum>=0) && (motNum<10))){
						motInd = -1;
						bMotNumInited = false;

					//}
					}
					else{
						bMotNumInited = true;
					}
					//DEBUGOUT("mn %x \r\n", motInd);
				}
				char *p = strchr(inputStr, 'p');
				if(p != NULL){
					p++;
					pos = atoi(p);
					if((pos>=0)&& (pos<=1000) && bMotNumInited){
						if(pos != mst[motInd].posZadI){

							//mst[motNum].posZadI=((pos*maxHeightImp)/1000);
							posImp=((pos*maxHeightImp)/1000);
							//DEBUGOUT("-- SET vcur %d pos %x pz %x vmax %d \r\n", mst[motNum].speedCurIPS, pos, mst[motNum].posZadI, mst[motNum].speedMaxIPS );
							bPosInited = true;
						}
					}
				}
				p = strchr(inputStr, 'v');
				if(p != NULL){
					p++;
					velocity = atoi(p);
					if((velocity>=100)&& (velocity<=4000) && bMotNumInited){
						mst[motInd].speedMaxIPS = ((velocity*pulsePerRot*10)/100)/mmPerRot;
						//DEBUGOUT("vel %x %x div=%x\r\n", velocity, mst[motNum].speedMaxIPS,  SYS_CLOCK/mst[motNum].speedMaxIPS);
						bVelocityInited = true;
					}
				}
				p = strchr(inputStr, 't');
				if(p != NULL){
					p++;
					time = atoi(p);
					if((time>0)&& (time<4000) && bMotNumInited){
						mst[motInd].speedMaxIPS = ((velocity*pulsePerRot*10)/100)/mmPerRot;
						//DEBUGOUT("vel %x %x div=%x\r\n", velocity, mst[motNum].speedMaxIPS,  SYS_CLOCK/mst[motNum].speedMaxIPS);
						bTimeInited = true;
					}
				}
				//DEBUGOUT("\r\n", inputStr);
				if(bPosInited && bMotNumInited){
					TPosCmd pc;
					if(bTimeInited == true)
						pc.time = time;
					else
						pc.time = 100;
					pc.posImp = posImp;
					RingBuffer_Insert(&(posCmdRB[motInd]), &pc);
					//DEBUGOUT("putCmd in RB p%d p%d t%d\r\n", pos, pc.posImp, pc.time);
					//DEBUGOUT("pC %d %d pos:%d t:%d\r\n", RingBuffer_GetCount(&posCmdRB), RingBuffer_GetCount(&uartRxRb),
					//									pos, time);
				}
				else{

				}
			}
		}

		//printf("lala\r\n");
		//vTaskDelay(configTICK_RATE_HZ/100 );
	}
}


//static void vCPLDctrl(void *pvParameters)
//{
//
//	Board_SSP_Init(LPC_SSP0);
//	Chip_SSP_Init(LPC_SSP0);
//
//	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
//	ssp_format.bits = SSP_BITS_16;
//	ssp_format.clockMode = SSP_CLOCK_MODE0;
//	Chip_SSP_SetFormat(LPC_SSP0, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
//
//	Chip_SSP_Enable(LPC_SSP0);
//
//	Chip_SSP_SetMaster(LPC_SSP0, 1);
//
//	TMotCtrlW0 mc0;
//	TMotCtrlW1 mc1;
//	mc0.wNum = 0;
//	mc0.motNum = 0;
//	mc0.dir = 1;
//
//	mc1.div = 0x186a;
//	mc1.ena = 1;
//	mc1.wNum = 1;
//
//	typedef enum { speedUp, speedConst, speedDown} TMotState;
//
//
//	TMotState ms = speedConst;
//
//	while(1){
//		//mc0.dir = !mc0.dir;
//		Chip_SSP_SendFrame(LPC_SSP0, mc0.data);
//		data0 = Chip_SSP_ReceiveFrame(LPC_SSP0);
//		//data0 = Chip_SSP_ReceiveFrame(LPC_SSP0);
//		switch(ms){
//		case speedUp:
//			mc1.div -= 10;
//			if(mc1.div <= 0x64){
//				ms = speedDown;
//				mc1.div = 0x64;
//			}
//			else
//
//			break;
//		case speedConst:
//			mc1.div = 0x186a;
//			break;
//		case speedDown:
//			mc1.div += 10;
//			if(mc1.div >= 0x186a){
//				ms = speedUp;
//				mc1.div = 0x186a;
//			}
//			break;
//		}
//		Chip_SSP_SendFrame(LPC_SSP0, mc1.data);
//		data1 = Chip_SSP_ReceiveFrame(LPC_SSP0);
//
//	}
//
//}


//				case idle:
//					motorDisable(mi);
//					pMd->speedCurIPS = 0;
//
//					break;
//				case calcParams:
//					deltaPos = pMd->posZadI - getPos(mi);
//
//					if(abs(deltaPos) > 2000){
//						if((pMd->speedCurIPS != pMd->speedZadIPS)){
//							if( ((pMd->dir == DIR_UP) && (deltaPos<0)) || ((pMd->dir == DIR_DOWN) && (deltaPos>0)) ){
//								pMd->speedZadIPS = 0;
//								pMd->bReverse = 1;
//							}
//							else{
//								pMd->bReverse = 0;
//							}
//							//else{
//							//	pMd->speedZadIPS = pMd->speedMaxIPS;
//							//}
//						}
//						else if((pMd->speedCurIPS == 0)){
//							pMd->speedZadIPS = pMd->speedMaxIPS;
//							if(deltaPos > 0){
//								pMd->dir = DIR_UP;
//								DEBUGOUT("state idle -> speedConst DIR_UP\r\n");
//							}
//							else{
//								pMd->dir = DIR_DOWN;
//								DEBUGOUT("state idle -> speedConst DIR_DOWN\r\n");
//							}
//						}
//						if(pMd->speedCurIPS != pMd->speedZadIPS){
//							pMd->speedDeviation = ( pMd->speedZadIPS - pMd->speedCurIPS);
//							pMd->state = speedDeviation;
//
//							pMd->startDeviationTime =  xTaskGetTickCount();
//							pMd->DeviationTime = abs(pMd->speedDeviation*1000/maxAccelIPS2);
//							pMd->speedOnStartDeviatonIPS = pMd->speedCurIPS;
//							DEBUGOUT("vCur %d vZad %d vDev %d tDevms %d dp %d\r\n", pMd->speedCurIPS, pMd->speedZadIPS, pMd->speedDeviation, pMd->DeviationTime, deltaPos);
//						}
//						else{
//							pMd->state = constSpeed;
//							DEBUGOUT("vCur %d vZad %d dp %d equals!\r\n", pMd->speedCurIPS, pMd->speedZadIPS,  deltaPos);
//						}
//					}
//					else{
//						//motorDisable(mi);
//
//					}
//
//
//					break;

//				case speedDeviation:
//					l = (uint32_t)((pMd->speedCurIPS/(float)(2*maxAccelIPS2))*pMd->speedCurIPS);
//					uint32_t d;
//					if(pMd->dir == DIR_UP)
//						d= abs(pMd->posZadI - getPos(mi));
//					else
//						d= abs(getPos(mi)-pMd->posZadI);
//					if((l >= d)&& (pMd->speedDeviation>0)){
//						DEBUGOUT("l >pz-cp  l=%d, d=%d\r\n", l, d);
//						pMd->state = calcParams;
//						DEBUGOUT("state speedDeviation -> calcParams \r\n");
//						pMd->speedZadIPS = 0;
//						break;
//
//					}
//					devProc = (xTaskGetTickCount()-pMd->startDeviationTime)/(float)pMd->DeviationTime;
//
//
//					pMd->speedCurIPS =pMd->speedOnStartDeviatonIPS + pMd->speedDeviation*devProc;
//
//					if(pMd->speedCurIPS > 1600){
//						div = SYS_CLOCK/pMd->speedCurIPS;
//					}
//					else{
//						div = SYS_CLOCK/1600;
//					}
//					setDiv(mi, MOT_ENA, pMd->dir, div);
//
//					DEBUGOUT("devProc %f cs %d l=%d, d=%d\r\n", devProc, pMd->speedCurIPS, l, d);
//
//					if(devProc>= 1.){
//						pMd->state = constSpeed;
//					}
//
//					break;

//				case constSpeed:
//
//					if(pMd->speedCurIPS == 0){
//						motorDisable(mi);
//						if(pMd->bReverse){
//							pMd->state = calcParams;
//							pMd->bReverse = 0;
//						}
//					}
//					else{
//						l = (uint32_t)((pMd->speedCurIPS/(float)(2*maxAccelIPS2))*pMd->speedCurIPS);
//						uint32_t d;
//						if(pMd->dir == DIR_UP)
//							d= abs(pMd->posZadI - getPos(mi));
//						else
//							d= abs(getPos(mi)-pMd->posZadI);
//						if(l >= d){
//							DEBUGOUT("l >pz-cp  l=%d, d=%d\r\n", l, d);
//							pMd->state = calcParams;
//							DEBUGOUT("state constSpeed -> calcParams \r\n");
//							pMd->speedZadIPS = 0;
//							break;
//
//						}
//
//					}
//
//					//uint32_t div = SYS_CLOCK/pMd->speedZadIPS;
//
//					//setDiv(mi, MOT_ENA, pMd->dir, div);
//					break;
