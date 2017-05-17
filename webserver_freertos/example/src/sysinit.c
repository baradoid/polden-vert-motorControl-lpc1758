/*
 * @brief Common SystemInit function for LPC17xx/40xx chips
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

 #include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void Chip_SetupOscClockingCustom(void)
{
	/* Disconnect the Main PLL if it is connected already */
	if (Chip_Clock_IsMainPLLConnected()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
	}

	/* Disable the PLL if it is enabled */
	if (Chip_Clock_IsMainPLLEnabled()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);
	}


	Chip_Clock_SetCrystalRangeHi();
	/* Enable the crystal */
	if (!Chip_Clock_IsCrystalEnabled())
		Chip_Clock_EnableCrystal();
	while(!Chip_Clock_IsCrystalEnabled()) {}

	/* Set PLL0 Source to Crystal Oscillator */
	Chip_Clock_SetCPUClockDiv(0);
	Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);

	/* FCCO = ((15+1) * 2 * 12MHz) / (0+1) = 384MHz */
	/* FCCO = ((8+1) * 2 * 16MHz) / (0+1) = 288MHz */
	/* FCCO = ((11+1) * 2 * 16MHz) / (0+1) = 384MHz */
	Chip_Clock_SetupPLL(SYSCTL_MAIN_PLL, 8, 0);

	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);

	/* 384MHz / (3+1) = 96MHz */
	/* 288MHz / (2+1) = 96MHz */
	Chip_Clock_SetCPUClockDiv(2);
	while (!Chip_Clock_IsMainPLLLocked()) {} /* Wait for the PLL to Lock */

	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
}

void Chip_SetupIrcClockingCustom(void)
{
	/* Disconnect the Main PLL if it is connected already */
	if (Chip_Clock_IsMainPLLConnected()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
	}

	/* Disable the PLL if it is enabled */
	if (Chip_Clock_IsMainPLLEnabled()) {
		Chip_Clock_DisablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);
	}

	Chip_Clock_SetCPUClockDiv(0);
	Chip_Clock_SetMainPLLSource(SYSCTL_PLLCLKSRC_IRC);

	/* FCCO = ((44+1) * 2 * 4MHz) / (0+1) = 360MHz */
	/* FCCO = ((36+1) * 2 * 4MHz) / (0+1) = 296MHz */
	Chip_Clock_SetupPLL(SYSCTL_MAIN_PLL, 36, 0);

	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE);

	/* 384MHz / (3+1) = 96MHz */
	/* 288MHz / (2+1) = 96MHz */
	/* 296MHz / (3+1) = 96MHz */
	Chip_Clock_SetCPUClockDiv(3);
	while (!Chip_Clock_IsMainPLLLocked()) {} /* Wait for the PLL to Lock */

	Chip_Clock_EnablePLL(SYSCTL_MAIN_PLL, SYSCTL_PLL_CONNECT);
}

//#define NO_BOARD_LIB
/* Set up and initialize hardware prior to call to main */
void SystemInit(void)
{
	unsigned int *pSCB_VTOR = (unsigned int *) 0xE000ED08;

#if defined(__IAR_SYSTEMS_ICC__)
	extern void *__vector_table;

	*pSCB_VTOR = (unsigned int) &__vector_table;
#elif defined(__CODE_RED)
	extern void *g_pfnVectors;

	*pSCB_VTOR = (unsigned int) &g_pfnVectors;
#elif defined(__ARMCC_VERSION)
	extern void *__Vectors;

	*pSCB_VTOR = (unsigned int) &__Vectors;
#endif

#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
	fpuInit();
#endif

//#if defined(NO_BOARD_LIB)
//	/* Chip specific SystemInit */
//	Chip_SystemInit();
//#else
//	/* Setup system clocking and muxing */
//	Board_SystemInit();
//#endif
	//Chip_SetupOscClockingCustom();

	/* Setup FLASH access to 4 clocks (100MHz clock) */
	//Chip_SYSCTL_SetFLASHAccess(FLASHTIM_100MHZ_CPU);
	Chip_SetupIrcClockingCustom();

}

