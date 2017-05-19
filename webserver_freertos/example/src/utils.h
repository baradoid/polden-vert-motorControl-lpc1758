#pragma once
#include <stdint.h>
#include <stdbool.h>

#define RXBUFSIZE 128

extern uint8_t RxBuf[RXBUFSIZE];// the receiver buffer.
uint8_t _getchar();
extern uint32_t RxHead; // the circular buffer index
extern uint32_t RxTail;
uint8_t isDataAvailable();

void setDiv(uint8_t mNum, uint8_t e, uint8_t dir, uint32_t div);
int32_t getPos(uint8_t mNum);
int32_t convertImpToMm(int32_t posImp);

void motorDisable(uint8_t mNum);
void motorEnable(uint8_t mNum);
void motorPositionReset(uint8_t mNum);

bool getKoncState(uint8_t mNum);

typedef union{
	struct {
		unsigned motNum : 4;
		unsigned dir : 1;
		unsigned posReset : 1;
		unsigned res : 8;
		unsigned wNum : 2;
	};

	uint16_t data;
}TMotCtrlW0;

typedef union{
	struct {
		unsigned div : 13;
		unsigned ena : 1;
		unsigned wNum : 2;
	};
	uint16_t data;
}TMotCtrlW1;

