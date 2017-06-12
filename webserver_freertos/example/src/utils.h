#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "stand.h"

#define RXBUFSIZE 128

extern uint8_t RxBuf[RXBUFSIZE];// the receiver buffer.
uint8_t _getchar();
extern uint32_t RxHead; // the circular buffer index
extern uint32_t RxTail;
uint8_t isDataAvailable();


int32_t getPos(uint8_t mNum);
int32_t convertImpToMm(int32_t posImp);
//void motorSetDiv(uint8_t mNum, uint8_t e, uint8_t dir, uint32_t div);
void motorSetDiv(uint8_t mNum, uint32_t div);

void motorDisable(uint8_t mNum);
void motorEnable(uint8_t mNum);
void motorPositionReset(uint8_t mNum);

bool getKoncState(uint8_t mNum);

void calcMoveParams(TMotorData *pMd, int32_t pos, TMoveCmd *posCmd);

//typedef union{
//	struct {
//		unsigned motNum : 4;
//		unsigned dir : 1;
//		unsigned posReset : 1;
//		unsigned res : 8;
//		unsigned wNum : 2;
//	};
//
//	uint16_t data;
//}TMotCtrlW0;
//
//typedef union{
//	struct {
//		unsigned div : 13;
//		unsigned ena : 1;
//		unsigned wNum : 2;
//	};
//	uint16_t data;
//}TMotCtrlW1;

typedef union{
	struct {
		unsigned motNum : 4;
		unsigned posReset : 1;
		unsigned res : 8;
		unsigned wNum : 3;
	} type0;

	struct{
		unsigned dir : 1;
		unsigned res : 12;
		unsigned wNum : 3;
	} type1;

	struct{
		unsigned div : 13;
		unsigned wNum : 3;
	}type2;

	struct{
		unsigned ena : 1;
		unsigned res : 12;
		unsigned wNum : 3;
	}type3;

	struct{
		unsigned res : 13;
		unsigned wNum : 3;
	}type4;
	uint16_t data;
}TMotCtrl;


int32_t impToMm(int32_t posImp);
int32_t mmToImp(int32_t mm);

void parseStr(char *inputStr);

//#define addCmdToRb(a, b) mc.posImp = mmToImp(a); RingBuffer_Insert(&(posCmdRB[b]), &mc)
#define addCmdToRb(a, b)

