#pragma once
#include <stdbool.h>
typedef enum {
	idle, errorState, seekKonc, calcParams, speedDeviation, constSpeed, constSpeedTimeCtrl,
	period, goDown
} TMotorState;

#define SYS_CLOCK 50000000
#define MOTOR_COUNT 10
typedef struct{
	uint32_t speedMaxIPS; //max
	uint32_t speedZadIPS;
	uint32_t speedCurIPS;
	int32_t posZadI;
	uint8_t dir;
	TMotorState state;

	uint32_t speedOnStartDeviatonIPS;
	int32_t speedDeviation;
	uint32_t startDeviationTime;
	uint32_t DeviationTime;

	uint8_t bReverse;

	uint32_t startCmdProcessTime;
	uint32_t cmdEndProcessTime;
	bool bFirstEnter;


} TMotorData;

typedef struct{
	int32_t posImp;
	uint32_t time;
} TPosCmd;

#define mmPerRot 10
#define pulsePerRot 4000
#define pulsePerMm (pulsePerRot/mmPerRot) //important ()!!


//#define maxHeightmm  1000
//#define maxHeightImp  (maxHeightmm*pulsePerRot/mmPerRot)

#define maxSpeedMmPs 200
#define maxSpeedIPS (maxSpeedMmPs*pulsePerRot/mmPerRot))

#define maxAccelIPS2 80000

#define DIR_STOP	2
#define DIR_UP		1
#define DIR_DOWN 	0

#define MOT_ENA		1
#define MOT_DIS		0

#define moveDirBit  	20
#define enaBit 			21
#define posResetBit 	22
#define dirInverseBit 	23

#define MoveDiveInverse 1
