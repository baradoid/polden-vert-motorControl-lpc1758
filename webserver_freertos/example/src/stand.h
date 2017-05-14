#pragma once

typedef enum {
	calcParams, speedDeviation, constSpeed
} TMotorState;

#define SYS_CLOCK 10000000
#define motorCount 10
typedef struct{
	uint32_t speedZadIPS;
	uint32_t speedCurIPS;
	uint32_t posZadI;
	uint8_t dir;
	TMotorState state;

	uint32_t speedOnStartDeviatonIPS;
	int32_t speedDeviation;
	uint32_t startDeviationTime;
	uint32_t DeviationTime;

} TMotorData;

#define mmPerRot 10
#define pulsePerRot 4000


#define maxHeightmm  1000
#define maxHeightImp  (maxHeightmm*pulsePerRot/mmPerRot)

#define maxSpeedMmPs 200
#define maxSpeedIPS (maxSpeedMmPs*pulsePerRot/mmPerRot))

#define maxAccelIPS2 80000

#define DIR_UP		1
#define DIR_DOWN 	0

#define MOT_ENA		1
#define MOT_DIS		0

#define moveDirBit  	20
#define enaBit 			21
#define posResetBit 	22
#define dirInverseBit 	23

#define MoveDiveInverse 1
