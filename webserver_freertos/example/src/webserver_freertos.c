/*
 * @brief LWIP FreeRTOS HTTP Webserver example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

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
#include "stand.h"
#include "utils.h"
#include "ring_buffer.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* NETIF data */
static struct netif lpc_netif;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

extern void http_server_netconn_init(void);

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	//Board_Init();

	/* LED0 is used for the link status, on = PHY cable detected */
	/* Initial LED state is off to show an unconnected cable state */
	Board_LED_Set(0, false);
}

/* Callback for TCPIP thread to indicate TCPIP init is done */
static void tcpip_init_done_signal(void *arg)
{
	/* Tell main thread TCP/IP init is done */
	*(s32_t *) arg = 1;
}

/* LWIP kickoff and PHY link monitor thread */
static void vSetupIFTask(void *pvParameters) {
	ip_addr_t ipaddr, netmask, gw;
	volatile s32_t tcpipdone = 0;
	uint32_t physts;
	static int prt_ip = 0;
	
	DEBUGSTR("LWIP HTTP Web Server FreeRTOS Demo...\r\n");
	
	/* Wait until the TCP/IP thread is finished before
	   continuing or wierd things may happen */
	DEBUGSTR("Waiting for TCPIP thread to initialize...\r\n");
	tcpip_init(tcpip_init_done_signal, (void *) &tcpipdone);
	while (!tcpipdone) {
		msDelay(1);
	}

	DEBUGSTR("Starting LWIP HTTP server...\r\n");

	/* Static IP assignment */
#if LWIP_DHCP
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
	IP4_ADDR(&gw, 192, 168, 1, 1);
	IP4_ADDR(&ipaddr, 192, 168, 1, 10);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
#endif

	DEBUGSTR("1\r\n");
	/* Add netif interface for lpc17xx_8x */
	memset(&lpc_netif, 0, sizeof(lpc_netif));
	if (!netif_add(&lpc_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
				   tcpip_input)) {
		DEBUGSTR("Net interface failed to initialize\r\n");
		while(1);			   
	}
	DEBUGSTR("2\r\n");
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

	/* Enable MAC interrupts only after LWIP is ready */
	NVIC_SetPriority(ETHERNET_IRQn, config_ETHERNET_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(ETHERNET_IRQn);

	DEBUGSTR("3\r\n");
#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif
	
	/* Initialize and start application */
	http_server_netconn_init();
	
	/* This loop monitors the PHY link and will handle cable events
	   via the PHY driver. */
	while (1) {
		/* Call the PHY status update state machine once in a while
		   to keep the link status up-to-date */
		physts = lpcPHYStsPoll();

		/* Only check for connection state when the PHY status has changed */
		if (physts & PHY_LINK_CHANGED) {
			if (physts & PHY_LINK_CONNECTED) {
				Board_LED_Set(0, true);
				prt_ip = 0;

				/* Set interface speed and duplex */
				if (physts & PHY_LINK_SPEED100) {
					Chip_ENET_Set100Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
				}
				else {
					Chip_ENET_Set10Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 10000000);
				}
				if (physts & PHY_LINK_FULLDUPLX) {
					Chip_ENET_SetFullDuplex(LPC_ETHERNET);
				}
				else {
					Chip_ENET_SetHalfDuplex(LPC_ETHERNET);
				}

				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_up,
										  (void *) &lpc_netif, 1);
			}
			else {
				Board_LED_Set(0, false);
				tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_down,
										  (void *) &lpc_netif, 1);
			}

			/* Delay for link detection (250mS) */
			vTaskDelay(configTICK_RATE_HZ / 4);
		}

		/* Print IP address info */
		if (!prt_ip) {
			if (lpc_netif.ip_addr.addr) {
				static char tmp_buff[16];
				DEBUGOUT("IP_ADDR    : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.ip_addr, tmp_buff, 16));
				DEBUGOUT("NET_MASK   : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.netmask, tmp_buff, 16));
				DEBUGOUT("GATEWAY_IP : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.gw, tmp_buff, 16));
				prt_ip = 1;
			}
		}
	}
}

void initSSP();

static SSP_ConfigFormat ssp_format;

//typedef struct {
//	unsigned motNum : 4;
//	unsigned div : 10;
//	unsigned dir : 1;
//	unsigned ena : 1;
//} TMotCtrl;




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

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, uartRxRb;

/* Transmit and receive ring buffer sizes */
#define UART_RRB_SIZE 8192	/* Receive */
/* Transmit and receive buffers */
static uint8_t uartRxBuff[UART_RRB_SIZE];



typedef struct{
	int32_t posImp;
	uint32_t time;
} TPosCmd;

/* Transmit and receive ring buffers */
#define POS_CMD_RB_SIZE 1024	/* Receive */
RINGBUFF_T posCmdRB;
uint8_t posCmdBuff[POS_CMD_RB_SIZE*sizeof(TPosCmd)];

void fillCustom1()
{
	TPosCmd pc;
	pc.time = 500;
	pc.posImp = 1000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.posImp = 5000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.posImp = 10000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.posImp = 15000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.time = 20000;
	/*pc.pos = 15000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 25000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 35000;	RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 50000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 65000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 80000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 95000; RingBuffer_Insert(&posCmdRB, &pc);
	pc.pos = 110000; RingBuffer_Insert(&posCmdRB, &pc);*/
	pc.posImp = 496000; RingBuffer_Insert(&posCmdRB, &pc);
//	//pc.pos = 640000; RingBuffer_Insert(&posCmdRB, &pc);
//	//pc.pos = 600000; RingBuffer_Insert(&posCmdRB, &pc);
//	//pc.pos = 580000; RingBuffer_Insert(&posCmdRB, &pc);
}


#define posMMtoPosImp(a) (50+(a*600));
#define addCmdToRb(a) pc.posImp = posMMtoPosImp(a); RingBuffer_Insert(&posCmdRB, &pc)

void fillCustom2()
{
	TPosCmd pc;
	pc.time = 100;
//	addCmdToRb(1);
//	addCmdToRb(2);
//	addCmdToRb(3);
//	addCmdToRb(4);
//	addCmdToRb(5);
//	addCmdToRb(7);
//	addCmdToRb(11);
//	addCmdToRb(15);
	addCmdToRb(20);
	addCmdToRb(21);
	addCmdToRb(25);
	addCmdToRb(30);
	addCmdToRb(38);
	addCmdToRb(47);
	addCmdToRb(58);
	addCmdToRb(71);
	addCmdToRb(84);
	addCmdToRb(99);
	addCmdToRb(116);
	addCmdToRb(132);
	addCmdToRb(150);
	addCmdToRb(169);
	addCmdToRb(188);
	addCmdToRb(207);
	addCmdToRb(227);
	addCmdToRb(246);
	addCmdToRb(266);
	addCmdToRb(285);
	addCmdToRb(304);
	addCmdToRb(322);
	addCmdToRb(339);
	addCmdToRb(356);
	addCmdToRb(372);
	addCmdToRb(387);
	addCmdToRb(400);
	addCmdToRb(413);
	addCmdToRb(423);
	addCmdToRb(432);
	addCmdToRb(440);
	addCmdToRb(445);
	addCmdToRb(448);
	addCmdToRb(450);
	addCmdToRb(451);
	addCmdToRb(451);
	addCmdToRb(449);
	addCmdToRb(449);
	addCmdToRb(449);
	addCmdToRb(450);
	addCmdToRb(451);
	addCmdToRb(450);
	addCmdToRb(448);
	addCmdToRb(444);
	addCmdToRb(438);
	addCmdToRb(431);
	addCmdToRb(421);
	addCmdToRb(411);
	addCmdToRb(398);
	addCmdToRb(384);
	addCmdToRb(370);
	addCmdToRb(354);
	addCmdToRb(337);
	addCmdToRb(319);
	addCmdToRb(301);
	addCmdToRb(282);
	addCmdToRb(263);
	addCmdToRb(244);
	addCmdToRb(224);
	addCmdToRb(205);
	addCmdToRb(185);
	addCmdToRb(166);
	addCmdToRb(147);
	addCmdToRb(129);
	addCmdToRb(112);
	addCmdToRb(96);
	addCmdToRb(82);
	addCmdToRb(68);
	addCmdToRb(56);
	addCmdToRb(45);
	addCmdToRb(36);
	addCmdToRb(29);
	addCmdToRb(24);
	addCmdToRb(21);
	addCmdToRb(20);
//	addCmdToRb(15);
//	addCmdToRb(11);
//	addCmdToRb(7);
//	addCmdToRb(5);
//	addCmdToRb(4);
//	addCmdToRb(3);
//	addCmdToRb(2);
//	addCmdToRb(1);
}

void uartInit();
TMotorData mst[motorCount];
static void vUartctrl(void *pvParameters)
{

	Board_SSP_Init(LPC_SSP0);
	Chip_SSP_Init(LPC_SSP0);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_BITS_16;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
	Chip_SSP_SetFormat(LPC_SSP0, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);

	Chip_SSP_Enable(LPC_SSP0);

	Chip_SSP_SetMaster(LPC_SSP0, 1);

//	mc0.wNum = 0;
//	mc0.motNum = 0;
//	mc0.dir = 1;
//
//	mc1.div = 0x186a;
//	mc1.ena = 1;
//	mc1.wNum = 1;



	//TPosCmd posCmdRB[100];
	//int32_t posCmdRbHead = 0;
	//int32_t posCmdRbTail = 0;
	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&posCmdRB, posCmdBuff, sizeof(TPosCmd), POS_CMD_RB_SIZE);


	char msg[200];

	strcpy(msg, "hello\r\n");

	#define recvBufLen 100
	uint8_t inputStrInd=0;
	char inputStr[recvBufLen];
	//Chip_UART_SendBlocking(LPC_UART0, msg, sizeof(msg));

	uint32_t lastTickVal = 0;
	uint32_t lastMtcVal = 0;
	int divVal = 0x4e2;
	int divInc = -5;
	int ledNum = 0;
	uint32_t itCnt=0;

	for(int i=0; i<motorCount; i++){
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

	//	pc.time = 500;
	//	pc.pos = 1000; RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.pos = 5000; RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.pos = 10000; RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.time = 1000;
	//	pc.pos = 20000; RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.pos = 30000; RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.pos = 80000;	RingBuffer_Insert(&posCmdRB, &pc);
	//	pc.pos = 160000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 240000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 320000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 400000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 480000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 560000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 640000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 600000; RingBuffer_Insert(&posCmdRB, &pc);
	//	//pc.pos = 580000; RingBuffer_Insert(&posCmdRB, &pc);


	TMotorData *pMd;
	int32_t deltaPos;
	float devProc;
	uint32_t l;


//	while(getKoncState(0) == false){;
//		DEBUGOUT("%d seeking for Konc \r\n", 0);
//		uint32_t div = SYS_CLOCK/4000;
//		setDiv(0, MOT_ENA, DIR_DOWN, div);
//	}

	uint32_t idlePauseStart = 0;
	bool bKoncState[motorCount];
	bool bKs;
	int32_t pos;
	uint32_t div;
	TPosCmd posCmd;

	pc.time = 4000;
	addCmdToRb(20);
	for(int i=0; ;i++){
		//sprintf(msg, "data %d", i);
		//strcpy(msg, "hello\r\n");
		//Chip_UART_SendBlocking(LPC_UART0, msg, sizeof(msg));

		for(int mi=0; mi<motorCount; mi++){
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

					if(RingBuffer_Pop(&posCmdRB, &posCmd)){
						DEBUGOUT("idle new cmd p%d t%d->constSpeedTimeCtrl\r\n", posCmd.posImp, posCmd.time);
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
					DEBUGOUT("konc  found -> idle\r\n");

					//fillCustom1();



				}

				break;
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
						DEBUGOUT("upBorder!\r\n");
					}
					if(downBorderReached){
						DEBUGOUT("downBorder!\r\n");
					}
					if(bTimeReached){
						DEBUGOUT("bTimeReached and dir=%d ur:%d dr:%d cmdrb:%d\r\n", pMd->dir, upBorderReached, downBorderReached, RingBuffer_GetCount(&posCmdRB));

					}
					if(bTimeReached && (pMd->dir != DIR_STOP) &&(upBorderReached == false) && (downBorderReached == false)){
						DEBUGOUT("going o errorState\r\n");
						pMd->state = errorState;
						break;
					}



					if( ((pMd->dir != DIR_STOP)&&(upBorderReached || downBorderReached)) ||
						((pMd->dir == DIR_STOP)&&bTimeReached)
					){
						if(RingBuffer_Pop(&posCmdRB, &posCmd)){
							DEBUGOUT("constSpeedTimeCtrl new cmd p%d t%d\r\n", posCmd.posImp, posCmd.time);
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
							DEBUGOUT("constSpeedTimeCtrl no cmd -> idle\r\n");
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
			DEBUGOUT("it %x %x %d %d  cmdrb:%d\r\n", xTaskGetTickCount(), itCnt,
													getPos(0), (int)((getPos(0)*((float)mmPerRot/pulsePerRot))),
													RingBuffer_GetCount(&posCmdRB));
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
				bool motNumInited = false;
				bool bPosInited = false;
				bool bTimeInited = false;
				bool bVelocityInited = false;

				uint32_t time;
				inputStr[inputStrInd] = 0;
				//DEBUGOUT("string recvd %s", inputStr);
				inputStrInd = 0;
				uint8_t motNum = -1;
				int32_t pos = 0;		//procents*10
				int32_t posImp = 0;
				uint16_t velocity = 0; //mm per sec
				if(inputStr[0] == 'S'){
					motNum = atoi(&(inputStr[1]));
					if(motNum != 0){
					//if(!((motNum>=0) && (motNum<10))){
						motNum = -1;
						motNumInited = false;

					//}
					}
					else{
						motNumInited = true;
					}
					//DEBUGOUT("mn %x \r\n", motNum);
				}
				char *p = strchr(inputStr, 'p');
				if(p != NULL){
					p++;
					pos = atoi(p);
					if((pos>=0)&& (pos<=1000) && (motNumInited)){
						if(pos != mst[motNum].posZadI){

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
					if((velocity>=100)&& (velocity<=4000) && (motNumInited)){
						mst[motNum].speedMaxIPS = ((velocity*pulsePerRot*10)/100)/mmPerRot;
						//DEBUGOUT("vel %x %x div=%x\r\n", velocity, mst[motNum].speedMaxIPS,  SYS_CLOCK/mst[motNum].speedMaxIPS);
						bVelocityInited = true;
					}
				}
				p = strchr(inputStr, 't');
				if(p != NULL){
					p++;
					time = atoi(p);
					if((time>0)&& (time<4000) && (motNum!=-1)){
						mst[motNum].speedMaxIPS = ((velocity*pulsePerRot*10)/100)/mmPerRot;
						//DEBUGOUT("vel %x %x div=%x\r\n", velocity, mst[motNum].speedMaxIPS,  SYS_CLOCK/mst[motNum].speedMaxIPS);
						bTimeInited = true;
					}
				}
				//DEBUGOUT("\r\n", inputStr);
				if(bTimeInited && bPosInited && motNumInited){
					TPosCmd pc;
					pc.time = time;
					pc.posImp = posImp;
					RingBuffer_Insert(&posCmdRB, &pc);
					//DEBUGOUT("putCmd in RB p%d p%d t%d\r\n", pos, pc.posImp, pc.time);
					//DEBUGOUT("pC %d %d pos:%d t:%d\r\n", RingBuffer_GetCount(&posCmdRB), RingBuffer_GetCount(&uartRxRb),
					//									pos, time);

				}
			}
		}

		//printf("lala\r\n");
		//vTaskDelay(configTICK_RATE_HZ/100 );
	}

}

/**
 * @brief	MilliSecond delay function based on FreeRTOS
 * @param	ms	: Number of milliSeconds to delay
 * @return	Nothing
 * Needed for some functions, do not use prior to FreeRTOS running
 */
void msDelay(uint32_t ms)
{
	vTaskDelay((configTICK_RATE_HZ * ms) / 1000);
}


void SystemReInit (void);


int main(void)
{

	//SystemReInit();
	prvSetupHardware();
	uartInit();
	printf("sysclk %d\r\n", Chip_Clock_GetSystemClockRate());

	/* Add another thread for initializing physical interface. This
	   is delayed from the main LWIP initialization. */
//	xTaskCreate(vSetupIFTask, (signed char *) "SetupIFx",
//				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
//				(xTaskHandle *) NULL);
//	xTaskCreate(vCPLDctrl, (signed char *) "vCPLDctrl",
//				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
//				(xTaskHandle *) NULL);

	xTaskCreate(vUartctrl, (signed char *) "vUartctrl",
				configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}



void uartInit()
{

	Chip_UART_Init(LPC_UART0);
	Chip_UART_SetBaud(LPC_UART0, 115200);
	Chip_UART_ConfigData(LPC_UART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);

	/* Enable UART Transmit */
	Chip_UART_TXEnable(LPC_UART0);

	Chip_IOCON_PinMux(LPC_IOCON, 0, 2, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 3, IOCON_MODE_INACT, IOCON_FUNC1);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&uartRxRb, uartRxBuff, 1, UART_RRB_SIZE);

	/* Reset and enable FIFOs, FIFO trigger level 3 (14 chars) */
	Chip_UART_SetupFIFOS(LPC_UART0, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
							UART_FCR_TX_RS | UART_FCR_TRG_LEV3));
	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_UART0, (UART_IER_RBRINT | UART_IER_RLSINT));


	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);

//	/* Setup UART for 115.2K8N1 */
//	Chip_UART_Init(LPC_UART0);
//	Chip_UART_SetBaud(LPC_UART0, 9600);
//	//Chip_UART_ConfigData(LPC_UART0, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
//	//Chip_UART_SetupFIFOS(LPC_UART0, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
//	Chip_UART_TXEnable(LPC_UART0);


		/* Reset and enable FIFOs, FIFO trigger level 3 (14 chars) */
//	Chip_UART_SetupFIFOS(LPC_UART0, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
//								UART_FCR_TX_RS | UART_FCR_TRG_LEV3));

}

#define SCS_Val               0x00000030
void SystemReInit (void)
{

	LPC_SYSCTL->SCS       = SCS_Val;
  if (SCS_Val & (1 << 5)) {             /* If Main Oscillator is enabled      */
    while ((LPC_SYSCTL->SCS & (1<<6)) == 0);/* Wait for Oscillator to be ready    */
  }

  LPC_SYSCTL->CLKSRCSEL = 0x00000001;    /* Select Clock Source for sysclk/PLL0*/

//#if (PLL0_SETUP)
  LPC_SYSCTL->PLL[0].PLLCFG   = 6 | (0<<6);
  LPC_SYSCTL->PLL[0].PLLCON   = 0x01;             /* PLL0 Enable                        */
  LPC_SYSCTL->PLL[0].PLLFEED  = 0xAA;
  LPC_SYSCTL->PLL[0].PLLFEED  = 0x55;
  while (!(LPC_SYSCTL->PLL[0].PLLSTAT & (1<<10)));/* Wait for PLOCK0                    */
//#endif

#if (PLL1_SETUP)
  LPC_SC->PLL1CFG   = PLL1CFG_Val;
  LPC_SC->PLL1CON   = 0x01;             /* PLL1 Enable                        */
  LPC_SC->PLL1FEED  = 0xAA;
  LPC_SC->PLL1FEED  = 0x55;
  while (!(LPC_SC->PLL1STAT & (1<<10)));/* Wait for PLOCK1                    */
#endif

  LPC_SYSCTL->CCLKSEL   = 1 | (1<<8);      /* Setup Clock Divider                */
  //LPC_SC->USBCLKSEL = USBCLKSEL_Val;    /* Setup USB Clock Divider            */
  //LPC_SC->EMCCLKSEL = EMCCLKSEL_Val;    /* EMC Clock Selection                */
  LPC_SYSCTL->PCLKSEL[0]  = 1;      /* Peripheral Clock Selection         */
  //LPC_SC->PCONP     = PCONP_Val;        /* Power Control for Peripherals      */
 // LPC_SC->CLKOUTCFG = CLKOUTCFG_Val;    /* Clock Output Configuration         */

#if (FLASH_SETUP == 1)                  /* Flash Accelerator Setup            */
  //LPC_SC->FLASHCFG  = FLASHCFG_Val | 0x03A;
#endif
   //LPC_SC->FLASHCFG = 0x5000;
}

void initSSP()
{
	Chip_IOCON_PinMux(LPC_IOCON, 0, 15, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 16, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 17, IOCON_MODE_INACT, IOCON_FUNC2);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 18, IOCON_MODE_INACT, IOCON_FUNC2);
}



void UART0_IRQHandler(void)
{
	/* Want to handle any errors? Do it here. */

	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART_IRQRBHandler(LPC_UART0, &uartRxRb, NULL);
}

/**
 * @}
 */
