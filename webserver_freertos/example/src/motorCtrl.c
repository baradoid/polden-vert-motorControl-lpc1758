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
#include "utils.h"


/* Transmit and receive ring buffers */
#define POS_CMD_RB_SIZE 256	/* Receive */
volatile RINGBUFF_T posCmdRB[MOTOR_COUNT];
uint8_t posCmdBuff[MOTOR_COUNT*POS_CMD_RB_SIZE*sizeof(TMoveCmd)];

extern RINGBUFF_T uartRxRb;



TMotorData mst[MOTOR_COUNT];
int32_t periodUpBorderMm = 500;
void motorCtrlPoll();
void vUartctrl(void *pvParameters)
{
	//bigTest();
	for(int i=0; i<MOTOR_COUNT; i++){
		int bufInd = i*POS_CMD_RB_SIZE*sizeof(TMoveCmd);
		RingBuffer_Init(&(posCmdRB[i]), &(posCmdBuff[bufInd]), sizeof(TMoveCmd), POS_CMD_RB_SIZE);
	}

	#define recvBufLen 100
	uint8_t inputStrInd=0;
	char inputStr[recvBufLen];

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
//		if(getKoncState(i) == true){
//			mst[i].state = period;//seekKonc;//idle;
//		}
//		else{
//			mst[i].state = seekKonc;
//			DEBUGOUT("%d seek for Konc \r\n", i);
//		}
		mst[i].dir = DIR_UP;
		mst[i].state =period;// idle;////seekKonc;/
		mst[i].cmdEndProcessTime = xTaskGetTickCount()+5000+2000*i;
		//mst[i].state =goDown;// idle;////seekKonc;/
		mst[i].bFirstEnter = true;
		//mst[i].state = seekKonc;
		//mst[i].state =goDown;
		mst[i].state = idle;
	}



	TMotorData *pMd;
//	while(getKoncState(0) == false){;
//		DEBUGOUT("%d seeking for Konc \r\n", 0);
//		uint32_t div = SYS_CLOCK/4000;
//		setDiv(0, MOT_ENA, DIR_DOWN, div);
//	}

	int32_t deltaPos;
	//uint32_t idlePauseStart = 0;
	bool bKoncState[MOTOR_COUNT];
	bool bKs;
	int32_t pos;
	uint32_t div;
	TMoveCmd posCmd;
	TMoveCmd mc;
	//pc.time = 4000;

	for(int i=0; i<MOTOR_COUNT; i++){
		addCmdToRb(0, i);
	}

	for(int i=0; i<MOTOR_COUNT; i++){
		addCmdToRb(2, i);
		addCmdToRb(4, i);
		addCmdToRb(6, i);
		addCmdToRb(8, i);
		addCmdToRb(10, i);
		addCmdToRb(12, i);
		addCmdToRb(14, i);
		addCmdToRb(16, i);
		addCmdToRb(18, i);
		addCmdToRb(20, i);


	}
	uint32_t gpio2Val = Chip_GPIO_GetPortValue(LPC_GPIO, 2);
	bool bootButLaststate = Chip_GPIO_GetPinState(LPC_GPIO, 2, 10);


	for(int i=0; ;i++){
		//sprintf(msg, "data %d", i);
		//strcpy(msg, "hello\r\n");
		//Chip_UART_SendBlocking(LPC_UART0, msg, sizeof(msg));

		if(gpio2Val != Chip_GPIO_GetPortValue(LPC_GPIO, 2)){
			gpio2Val = Chip_GPIO_GetPortValue(LPC_GPIO, 2);
			//DEBUGOUT("%x\r\n", Chip_GPIO_GetPortValue(LPC_GPIO, 2));
		}
		//DEBUGOUT("main \r\n");
//		if(Chip_GPIO_GetPinState(LPC_GPIO, 2, 10) != bootButLaststate){
//			bootButLaststate = Chip_GPIO_GetPinState(LPC_GPIO, 2, 10);
//			for(int mi=0; mi<MOTOR_COUNT; mi++){
//				DEBUGOUT("but 2[10] det -> go down state\r\n");
//					//mst[mi].state = goDown;
//			}
//		}


		motorCtrlPoll();


		//DEBUGOUT("iter1\r\n");
		if(xTaskGetTickCount() - lastTickVal > 1000){
			lastTickVal = xTaskGetTickCount() ;
//			char s;
//			if(getPos(0) >=0) s = '+';
//			else s = '-';
			uint32_t t = xTaskGetTickCount();
			uint16_t sTot = t/1000;
			uint16_t m = sTot/60;
			uint16_t sCur = (t-(m*60*1000))/1000;

			DEBUGOUT("%02d%02d %x  ", m, sCur, itCnt);

			for(int i=0; i<MOTOR_COUNT; i++){
				int32_t pos = getPos(i);
				DEBUGOUT("%d ", impToMm(pos));
			}
			DEBUGOUT("rb:");
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

		//DEBUGOUT("iter2\r\n");
		//while(Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR){
		char prompt = 0;
		while(RingBuffer_Pop(&uartRxRb, &prompt)){

			//prompt = DEBUGIN();
			inputStr[inputStrInd++] = prompt;
			if ((prompt == '\n') || (inputStrInd == recvBufLen )){
				inputStr[inputStrInd] = 0;
				//DEBUGOUT("string recvd %s", inputStr);
				parseStr(&(inputStr[0]));
				inputStrInd = 0;
				break;
			}
		}

		//printf("lala\r\n");
		//vTaskDelay(configTICK_RATE_HZ/100 );
		//DEBUGOUT("iter3\r\n");
	}
	DEBUGOUT("exit\r\n");
}

int32_t lastPos[MOTOR_COUNT] = {0,0,0,0,0,0,0,0,0,0};
void parsePosCmd(int motInd, char *p)
{
	int32_t pos = atoi(p); //mm*10
	if((pos>=0)&& (pos<1000)){
//						if(pos != mst[motInd].posZadI){
//
//							//mst[motNum].posZadI=((pos*maxHeightImp)/1000);
//							//posImp=((pos*maxHeightImp)/1000);

//							//DEBUGOUT("-- SET vcur %d pos %x pz %x vmax %d \r\n", mst[motNum].speedCurIPS, pos, mst[motNum].posZadI, mst[motNum].speedMaxIPS );
//						}
		TMoveCmd mc;

		int32_t posImp = mmToImp(pos);
		int32_t deltaPos = posImp - lastPos[motInd];
		lastPos[motInd] = posImp;
		uint32_t speedZadIPS = (abs(deltaPos)*1000)/ mcContrPeriodms;

		if(deltaPos == 0){
			mc.dir = DIR_STOP;
			mc.div = 0x186a;
		}
		else{
			mc.dir = deltaPos>0? DIR_UP : DIR_DOWN;
			mc.div = SYS_CLOCK/speedZadIPS;
		}

		if(RingBuffer_IsFull(&(posCmdRB[motInd])) == 0){
			RingBuffer_Insert(&(posCmdRB[motInd]), &mc);
			//DEBUGOUT("ok\r\n");
			if(mc.dir != DIR_STOP){
				DEBUGOUT("%d->p %d v %d d %x:%x\r\n", motInd, pos, speedZadIPS, mc.dir, mc.div);
			}
		}
		else{
			DEBUGOUT("ff\r\n");
		}

	}
	//DEBUGOUT("mn %x p%d \r\n", motInd, pos);

}

void parseStr(char *inputStr)
{
	int inputStrLen = strlen(inputStr);
	if(inputStrLen == 42){
		bool gridStrOk = true;
		for(int i=0; i<10; i++){
			if(inputStr[i*4] != 'p'){
				gridStrOk = false;
			}
		}
		if(gridStrOk){
			//DEBUGOUT("grid str OK!\r\n");
			for(int i=0; i<MOTOR_COUNT; i++){
				parsePosCmd(i, &(inputStr[i*4+1]));
			}

		}
		else{
			DEBUGOUT("grid str FAIL!\r\n");
			return;
		}
	}
	else if(inputStrLen == 4){
		if(strcmp(inputStr, "Sd\r\n") == 0){
			DEBUGOUT("go to godownState\r\n");
			for(int mi=0; mi<MOTOR_COUNT; mi++){
				mst[mi].state = seekKonc;
			}
			return;
		}
	}
	return;

	bool bMotNumInited = false;
	bool bPosInited = false;
	bool bTimeInited = false;
	bool bVelocityInited = false;
	uint32_t time;
	TMoveCmd mc;

	uint8_t motInd = -1;
	int32_t pos = 0;		//procents*10
	int32_t posImp = 0;
	uint16_t velocity = 0; //mm per sec
	if(inputStr[0] == 'S'){
		if(inputStr[1] == 'd'){
			DEBUGOUT("go to godownState\r\n");
			for(int mi=0; mi<MOTOR_COUNT; mi++){

				mst[mi].state = goDown;
			}
			return;
		}
		else if(inputStr[1] == 'i'){
			DEBUGOUT("go to idle state\r\n");
			for(int mi=0; mi<MOTOR_COUNT; mi++){
				mst[mi].state = idle;
				motorDisable(mi);
			}
			return;
		}
		else if(inputStr[1] == 'p'){

			pos = atoi(&(inputStr[2]) );
			if((pos > 100) && (pos <1000)){
				periodUpBorderMm = pos;
			}
			else{

			}

			DEBUGOUT("go to period state with up border %d\r\n", periodUpBorderMm);

			for(int i=0; i<MOTOR_COUNT; i++){
				mst[i].dir = DIR_UP;
				mst[i].state =period;// idle;////seekKonc;/
				mst[i].cmdEndProcessTime = xTaskGetTickCount()+2000*i;
				//mst[i].state =goDown;// idle;////seekKonc;/
				mst[i].bFirstEnter = true;
			}
			return;
		}
		else if(inputStr[1] == 'r'){
			DEBUGOUT("reset pos state\r\n");
			for(int mi=0; mi<MOTOR_COUNT; mi++){
				motorPositionReset(mi);
			}
			return;
		}
		else if(inputStr[1] == 'n'){
			int32_t ind = atoi(&(inputStr[2]) );
			if((ind >=0) && (ind <10)){
				DEBUGOUT("set one state %d\r\n", ind);

				for(int i=0; i<MOTOR_COUNT; i++){
					if(i == ind){
						mst[i].state = goTop;
					}
					else{
						mst[i].state = goBottom;
					}
					mst[i].bFirstEnter = true;
				}
			}
			else{
				DEBUGOUT("set one state param error\r\n");
			}
			return;
		}
		else if(inputStr[1] == 'u'){
			DEBUGOUT("move up\r\n");
			for(int i=0; i<MOTOR_COUNT; i++){
				addCmdToRb(2, i);
				addCmdToRb(4, i);
				addCmdToRb(6, i);
				addCmdToRb(8, i);
				addCmdToRb(10, i);
				addCmdToRb(12, i);
				addCmdToRb(14, i);
				addCmdToRb(16, i);
				addCmdToRb(18, i);
				addCmdToRb(20, i);
			}
			return;
		}
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
//						if(pos != mst[motInd].posZadI){
//
//							//mst[motNum].posZadI=((pos*maxHeightImp)/1000);
//							//posImp=((pos*maxHeightImp)/1000);
				posImp = mmToImp(pos);
//							//DEBUGOUT("-- SET vcur %d pos %x pz %x vmax %d \r\n", mst[motNum].speedCurIPS, pos, mst[motNum].posZadI, mst[motNum].speedMaxIPS );
				bPosInited = true;
//						}
		}
		//DEBUGOUT("mn %x p%d \r\n", motInd, pos);
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
		TMoveCmd mc;
//					if(bTimeInited == true)
//						pc.time = time;
//					else
//						pc.time = 100;
		//mc.posImp = posImp; !!!
		if(RingBuffer_IsFull(&(posCmdRB[motInd])) == 0){
			RingBuffer_Insert(&(posCmdRB[motInd]), &mc);
			//DEBUGOUT("ok\r\n");
			//DEBUGOUT("%d->p %d  pI \r\n", motInd, pos, pc.posImp);
		}
		else{
			DEBUGOUT("ff\r\n");
		}
		//DEBUGOUT("putCmd in RB p%d p%d t%d\r\n", pos, pc.posImp, pc.time);
		//DEBUGOUT("pC %d %d pos:%d t:%d\r\n", RingBuffer_GetCount(&posCmdRB), RingBuffer_GetCount(&uartRxRb),
		//									pos, time);
	}
	else{

	}
}


void motorCtrlPoll()
{
	static uint32_t lastTickVal = 0;
	uint32_t curTickVal = xTaskGetTickCount();

	TMoveCmd mc;

	if((curTickVal - lastTickVal) >= mcContrPeriodms){
		lastTickVal = curTickVal;
		for(int mi=0; mi<MOTOR_COUNT; mi++){
			if(mst[mi].state == seekKonc){
				//setDiv(mi, MOT_ENA, pMd->dir, div);
				motorSetDiv(mi, SYS_CLOCK/16000);
				motorSetDir(mi, DIR_DOWN);
				motorEnable(mi);
			}
			else{
				if(RingBuffer_Pop(&(posCmdRB[mi]), &mc)){
					if(mc.dir == DIR_STOP){
						//DEBUGOUT("%d cmd STOP rb:%d\r\n", mi, RingBuffer_GetCount(&(posCmdRB[mi])));
						motorSetDiv(mi, 0x186a);
						motorDisable(mi);
					}
					else{
						DEBUGOUT("%d cmd div:%x rb:%d\r\n", mi, mc.div, RingBuffer_GetCount(&(posCmdRB[mi])));
						motorSetDir(mi, mc.dir);
						motorSetDiv(mi, mc.div);
						motorEnable(mi);
					}
				}
				else{
					//DEBUGOUT("%d constSpeedTimeCtrl no cmd -> idle\r\n",  mi);
					//div = SYS_CLOCK/1000;
					//setDiv(mi, MOT_DIS, pMd->dir, div);
					motorDisable(mi);
					motorSetDir(mi, DIR_STOP);
					motorSetDiv(mi, 0x186a);
					//DEBUGOUT("%d thats all\r\n",  mi);
				}
				//DEBUGOUT("%d thats all 2\r\n",  mi);
			}
		}
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
