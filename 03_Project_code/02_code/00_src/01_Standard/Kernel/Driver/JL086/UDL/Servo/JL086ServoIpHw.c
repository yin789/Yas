/****************************************************************************
Description	: JL086 ServoIP Hardware Definition
----------------------------------------------------------------------------
Author			: Yaskawa Electric Corp.,
					Copyright (c) 2012 All Rights Reserved

Project			: Mercury

Functions		:

----------------------------------------------------------------------------
Changes			:
	Name	Date		Description
  ------------------------------------------------------------------------
	Y.Oka	2012.08.06	created

****************************************************************************/
#ifndef _JL076_USE_

#include "Basedef.h"
#include "HwDeviceIf.h"
#include "JL086ServoIpHw.h"
#include "KLib.h"
#include "EncData.h"/* <S004> */

#include "ARMPF.h"
#include "ARMPF_SCU.h"

//#include "MpConstTbl.h"

/* <S068> */
#define TABLESTART	0xC0007A00
#define TABLESIZE 	64

//<S146>extern CSHORT OVMODTBLG[16][2];
//extern CSHORT OVMODTBLO[16][2];//<S0C7>
/* <S068> */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* Low Level functions */
void	SHal_InitAsicSystem( ASIC_HW_IF_STR* );
void	SHal_InitAsicAlarm( ASIC_HW_IF_STR* );
void	SHal_InitAsicInterrupt( ASIC_HW_IF_STR* );
void	SHal_InitAsicSencIF( ASIC_HW_IF_STR* );
void	SHal_InitAsicSerialIF( ASIC_HW_IF_STR* );
LONG	SHal_InitAsicPulseIF( ASIC_HW_IF_STR *pAsicHwReg, USHORT prstsw, USHORT syssw1, USHORT comfil );
void	SHal_InitAsicAdInterface( ASIC_HW_IF_STR* pAsicHwReg );
void	SHal_InitAsicTimer( USHORT syssw2, ASIC_HW_IF_STR *pAsicHwReg );
void	SHal_InitAsicRegenerate( ASIC_HW_IF_STR *pAsicHwReg );
void	SHal_InitAsicPwmControl( ASIC_HW_IF_STR *pAsicHwReg, LONG odt_c );



/****************************************************************************************************/
/*																									*/
/*		ASICSoftware reset process(Axis common processing)									<S0E4>			*/
/*																									*/
/****************************************************************************************************/
void	SHal_AsicSwReset( ASIC_HW_IF_STR *pAsicHwReg )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Clock function initialization(JL-076for clock enable before soft reset)						*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	pAsicHwReg->AREG_CSTP		= 0x00000000;	/* ASIC Clock Stop Selection						*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Soft reset																			*/
	/*----------------------------------------------------------------------------------------------*/
#if 0	/* JL-086A UDLClock synchronization problem countermeasures		*/
	pAsicHwReg->AREG_SETREG1	= 0x00000009;	/* ASIC S/W Reset Enable							*/
	KlibWaitus( 1 );							/* Wait												*/
	pAsicHwReg->AREG_SRESET		= 0x00000000;	/* ASIC Soft Reset									*/
	KlibWaitus( 1 );							/* Wait												*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC S/W Reset Disable							*/
#else	/* UDLClock stop→On restartUDLSame effect as reset		*/
	SCU_CLK_STOP(UDL_SERVO_CLK);
	KlibWaitus(1);
	SCU_CLK_START(UDL_SERVO_CLK);
	KlibWaitus(1);
#endif
}


/****************************************************************************************************/
/*																									*/
/*		ASICInitialization processA(Parameter independent)															*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicA( ASIC_HW_IF_STR *AsicHwReg )
{
	SHal_InitAsicSystem( AsicHwReg );				/* System control function(Clock,WDT,etc)		*/
	SHal_InitAsicAlarm( AsicHwReg );				/* Initialization of alarm function							*/
	SHal_InitAsicInterrupt( AsicHwReg );			/* Interrupt function initialization							*/
	SHal_InitAsicSencIF( AsicHwReg );				/* Serial encoder IF initialization					*/
	SHal_InitAsicSerialIF( AsicHwReg );				/* Serial IF initialization							*/
	SHal_InitAsicAdInterface( AsicHwReg );			/* AD interface initialization					*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		ASICInitialization processB(Parameter dependent part)															*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_InitAsicB( ASIC_HW_IF_STR* pAsicHwReg, SVIP_INIT_PRM *pSvipInitPrm )
{
	BOOL	ret;
	ret = SVIP_NOALM;

	/*------------------------------------------------------------------------------------------*/
	/*		Serial encoder IF register initialization												*/
	/*------------------------------------------------------------------------------------------*/
	if( 0 != (pSvipInitPrm->syssw1 & 0x2000) )
	{ /* PnE0B.D == 1 Disable crosstalk */
		pAsicHwReg->AREG_SEPGSET00 &= (0xffffffff & ~BIT1);
		pAsicHwReg->AREG_SEPGSET10 &= (0xffffffff & ~BIT1);
	}

	/* PulseIFInitialize */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	ret = SHal_InitAsicPulseIF( pAsicHwReg,
								pSvipInitPrm->prstsw,
								pSvipInitPrm->syssw1,
								pSvipInitPrm->comfil );
#endif

	/* Timer function initialization */
#ifndef CSW_CSTR_DB_REVERSE
	/* Timer function initialization <2010.12.2 Delete>
		Initializing because the logic is reversed(WDThits)isDB */
	SHal_InitAsicTimer( pSvipInitPrm->syssw2, pAsicHwReg );
#endif

	/* Initialization of regenerative function(For initialization process) */
	SHal_InitAsicRegenerate( pAsicHwReg );

	/* PWMControl function initialization */
	SHal_InitAsicPwmControl( pAsicHwReg, pSvipInitPrm->odt_c.b.l );

#ifdef CSW_MICRO_DEBUG_MODE
	/* In test modeINT2Signal output setting */
	SHal_SetMicroDebugMode( pAsicHwReg );
#endif

	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		ASICInitialization processB(Parameter dependent part)	(For parameter reset)									*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_PrstAsicB( ASIC_HW_IF_STR* pAsicHwReg, SVIP_INIT_PRM *pSvipInitPrm )
{
	BOOL	ret;
	ret = SVIP_NOALM;

	/* PulseIFInitialize */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	ret = SHal_InitAsicPulseIF( pAsicHwReg,
								pSvipInitPrm->prstsw,
								pSvipInitPrm->syssw1,
								pSvipInitPrm->comfil );
#endif

#ifndef CSW_CSTR_DB_REVERSE
	/* Timer function initialization */
	SHal_InitAsicTimer( pSvipInitPrm->syssw2, pAsicHwReg );
#endif

	/* PWMControl function initialization */
	SHal_InitAsicPwmControl( pAsicHwReg, pSvipInitPrm->odt_c.b.l);

	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		System control function(Clock,WDT,etc)														*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicSystem( ASIC_HW_IF_STR *pAsicHwReg )
{

//LONG work;//<S0C7>

#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)
	/*----------------------------------------------------------------------------------------------*/
	/*		Fully closed and semi-closed position correction module settings using the feedback option are		*/
	/*		2It is necessary to set it on the axis.																*/
	/*----------------------------------------------------------------------------------------------*//* <S04D> */
ASIC_HW_IF_STR *pAsicHwReg2;

	pAsicHwReg2 = (ASIC_HW_IF_STR*)((UCHAR*)pAsicHwReg + 0x1000);
#endif/* (CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)	*/

//-	SHal_AsicSwReset() /* <S0E4> */
//-	/*----------------------------------------------------------------------------------------------*/
//-	/*		Clock function initialization(JL-076for clock enable before soft reset)						*/
//-	/*----------------------------------------------------------------------------------------------*/
//-	pAsicHwReg->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
//-	pAsicHwReg->AREG_CSTP		= 0x00000000;	/* ASIC Clock Stop Selection						*/
//-	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)
	/*----------------------------------------------------------------------------------------------*/
	/*		Fully closed and semi-closed position correction module settings using the feedback option are		*/
	/*		2It is necessary to set it on the axis.																*/
	/*----------------------------------------------------------------------------------------------*//* <S04D> */
	pAsicHwReg2->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	pAsicHwReg2->AREG_CSTP		= 0x00000000;	/* ASIC Clock Stop Selection						*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/
#endif/* (CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)	*/

//-	SHal_AsicSwReset() /* <S0E4> */
//	/*----------------------------------------------------------------------------------------------*/
//	/*		Soft reset																			*/
//	/*----------------------------------------------------------------------------------------------*/
//#if 0	/* JL-086A UDLClock synchronization problem countermeasures		*/
//	pAsicHwReg->AREG_SETREG1	= 0x00000009;	/* ASIC S/W Reset Enable							*/
//	KlibWaitus( 1 );							/* Wait												*/
//	pAsicHwReg->AREG_SRESET		= 0x00000000;	/* ASIC Soft Reset									*/
//	KlibWaitus( 1 );							/* Wait												*/
//	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC S/W Reset Disable							*/
//#else	/* UDLClock stop→On restartUDLSame effect as reset		*/
//	SCU_CLK_STOP(UDL_SERVO_CLK);
//	KlibWaitus(1);
//	SCU_CLK_START(UDL_SERVO_CLK);
//	KlibWaitus(1);
//#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		Clock function initialization																	*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	/*----------------------------------------------------------------------------------------------*/
#if 0 /* <S0004> Comment out because similar processing is directly below */
	pAsicHwReg->AREG_CSTP		= 0x04028410;	/* ASIC Clock Stop Selection						*/
												/*	Encoder ADIF, NCT, Full Closed Cntr,			*/
												/*	New Incre Decoder, OC Detect Clock Stop			*/
	
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	pAsicHwReg->AREG_CSTP		|= 0x00080000;	/*  M-II Slave I/F Clock Stop						*/
#elif (CSW_NETIF_TYPE == NETIF_M2_TYPE)
//	pAsicHwReg->AREG_CSTP		|= 0x00004000;	/*  Pulse Counter Clock Stop						*/
#else /* (CSW_NETIF_TYPE == NETIF_M3_TYPE || NETIF_CM_TYPE) */
//	pAsicHwReg->AREG_CSTP		|= 0x00084000;	/*  M-II Slave I/F, Pulse Counter Clock Stop		*/
	pAsicHwReg->AREG_CSTP		|= 0x00080000;	/*  M-II Slave I/F									*/
#endif
#endif /* <S0004> Comment out because similar processing is directly below */
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_CSTP		= (0x04028410	/* ASIC Clock Stop Selection						*/
												/*	Encoder ADIF, NCT, Full Closed Cntr,			*/
												/*	New Incre Decoder, OC Detect Clock Stop			*/
								 | (1<<2)		/* bit2: Reference A/D clock Stop 					*/
								 | (1<<14)  	/* bit14: Command Pulse Counter clock Stop			*/
								 | (1<<19)   ); /* bit19: M-II  Slave I/F Clock Stop				*/
								 
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	pAsicHwReg->AREG_CSTP		&= ~((1<<2) 	/*  Ref A/D clock enable							*/
									| (1<<14));	/*  Pulse Counter clock enable						*/
#elif (CSW_NETIF_TYPE == NETIF_M2_TYPE)
	pAsicHwReg->AREG_CSTP		&= ~((1<<19)	/*  M-II Slave I/F Clock Enable						*/
									| (1<<14));	/*  Pulse Counter clock enable	(for Latch)			*//* <S0004> */
#else /* (CSW_NETIF_TYPE == NETIF_M3_TYPE || NETIF_CM_TYPE) */
//	pAsicHwReg->AREG_CSTP		&= 0xFFFFFFFF;	/*  												*//* <S0004>Delete */
	pAsicHwReg->AREG_CSTP		&= ~(1<<14);	/*  Pulse Counter clock enable	(for Latch)			*//* <S0004> */
#endif
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)
	/*----------------------------------------------------------------------------------------------*/
	/*		Fully closed and semi-closed position correction module settings using the feedback option are		*/
	/*		2It is necessary to set it on the axis.																*/
	/*----------------------------------------------------------------------------------------------*//* <S04D> */
	pAsicHwReg2->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	pAsicHwReg2->AREG_CSTP	&= ~(1<<14);		/* clock enable	(for Position compensate )			*/
	pAsicHwReg2->AREG_CSTP	&= ~(1<<25);		/* clock enable	(for Position compensate )			*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/
#endif/* (CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)	*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Computation unit(MicroProgram)Initialization of watchdog function & pulse conversion C phase mask bit				*/
	/*----------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_WDT1SET	= 0x00000150;	/* Set C-Phase Mask(bit8), WDT Stop(bit6),			*/
	pAsicHwReg->AREG_WDT1SET	= 0x00000168;	/* Set C-Phase Mask(bit8), WDT Stop(bit6),			@@TEST*/
												/*		and Micro WDT Time Over(0x10*16us=256us)	*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Host(CPU)Initialization of watchdog function													*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_WDT2SET	= 0x00000020;	/* Set CPU WDT Time Over(64us*0x20=2048us)			*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Analog monitor function initialization																*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_PWMU		= 0x00000000;	/* Set Zero for Initial Out							*/
	pAsicHwReg->AREG_PWMV		= 0x00000000;	/* Set Zero for Initial Out							*/


#if 0
/********************************************************************************************************************************
		DEBUG
********************************************************************************************************************************/ 
	/*----------------------------------------------------------------------------------------------*/
	/*		Clock function initialization(JL-076for clock enable before soft reset)						*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	pAsicHwReg2->AREG_CSTP		= 0x00000000;	/* ASIC Clock Stop Selection						*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Soft reset																			*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000009;	/* ASIC S/W Reset Enable							*/
	KlibWaitus( 1 );							/* Wait												*/
	pAsicHwReg2->AREG_SRESET		= 0x00000000;	/* ASIC Soft Reset									*/
	KlibWaitus( 1 );							/* Wait												*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000001;	/* ASIC S/W Reset Disable							*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Clock function initialization																	*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg2->AREG_CSTP		= 0x04028410;	/* ASIC Clock Stop Selection						*/
												/*	Encoder ADIF, NCT, Full Closed Cntr,			*/
												/*	New Incre Decoder, OC Detect Clock Stop			*/
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	pAsicHwReg2->AREG_CSTP		|= 0x00080000;	/*  M-II Slave I/F Clock Stop						*/
#elif (CSW_NETIF_TYPE == NETIF_M2_TYPE)
	pAsicHwReg2->AREG_CSTP		|= 0x00004000;	/*  Pulse Counter Clock Stop						*/
#else /* (CSW_NETIF_TYPE == NETIF_M3_TYPE || NETIF_CM_TYPE) */
	pAsicHwReg2->AREG_CSTP		|= 0x00084000;	/*  M-II Slave I/F, Pulse Counter Clock Stop		*/
#endif
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg2->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/
/********************************************************************************************************************************
		DEBUG
********************************************************************************************************************************/
#endif

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Initialization of alarm function																		*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicAlarm( ASIC_HW_IF_STR *pAsicHwReg )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Alarm settings																			*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_ALMSET		= 0x00000000;	/* HWGBON:Invalid, DBONL:DBON						*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Polarity & filter setting : OV, OC, VCAP, BBON1, BBON2											*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
//	pAsicHwReg->AREG_SETREG2	= 0x000038F0;	/* OV, OC : ActiveLow, Filter valid					*/
												/*	BBON1-2 : ActiveHigh, Filter invalid			*/
												/*	Ondelay Counter Clock : 320MHz					*/
												/*	INT1L Recovery Time : 640ns						*/
//	pAsicHwReg->AREG_SETREG2	= 0x000021F0;	/* Ondelay Counter Clock : 26.7MHz					*//* @@Interim */
//	pAsicHwReg->AREG_SETREG2	= 0x000029F0;	/* Ondelay Counter Clock : 80MHz					*//* @@Interim,Setting value correction<S017> */
	pAsicHwReg->AREG_SETREG2	= 0x00002950;	/* Ondelay Counter Clock : 80MHz					*//* @@Interim,Noise responseBBON *//*<S141>*/
	pAsicHwReg->AREG_SETREG2	|= BIT9;		/* Carrer Counter Sync				<S019>				*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Alarm mask setting : OVInput, OCInput, OC2Input, OCDetection									*/
	/*----------------------------------------------------------------------------------------------*/
	//	pAsicHwReg->AREG_SETMSK	= 0x0000000F;	/* All Alarm Mask									*/
#if CSW_BOARD_TYPE == SGD7S_TYPE /* <S0A2> */
	pAsicHwReg->AREG_SETMSK		= 0x0000000D;	/* OV, OC2, OC_UVW Mask								*//* <S01B> */
#else /* CSW_BOARD_TYPE == SGD7W_TYPE *//* <S0A2> */
	pAsicHwReg->AREG_SETMSK		= 0x00000009;	/* OV, OC_UVW Mask									*//* <S0A2> */
#endif /* CSW_BOARD_TYPE == SGD7S_TYPE *//* <S0A2> */
	/*----------------------------------------------------------------------------------------------*/
	/*		Alarm clear																			*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_ALMCLR		= 0x0000FFFF;	/* Clear All Alarm									*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Interrupt function initialization																			*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicInterrupt( ASIC_HW_IF_STR *pAsicHwReg )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Interrupt enable register settings																*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_INT1SET	= 0x00000000;	/* Disable All Interrupt							*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Interrupt timer register settings																*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_INT0SET	= 0x00000000;	/* INT0 Frequency									*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Each interrupt delay time setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_ENCTXDLY0	= 0x00000000;	/* CH0 Tx Delay : 0ns								*/
	pAsicHwReg->AREG_ENCTXDLY1	= 0x00000000;	/* CH1 Tx Delay : 0ns								*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Encoder read interrupt delay time setting														*/
	/*		For initial value0and detect the encoder communication cycle(8Mbps/4Mbps)			*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_ENCRDDLY0	= 0x00000000;	/* CH0 Read Interrupt Delay : 0ns <S009><S09C>		*/
	pAsicHwReg->AREG_ENCRDDLY1	= 0x00000000;	/* CH1 Read Interrupt Delay : 0ns					*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_INTASET	= 0x00000078;	/* ScanA Interrupt Delay : 1500ns					*/
												/* (jitter & Position Compensation Delay)			*/
	pAsicHwReg->AREG_INTBSET	= 0x000000C8;	/* ScanB Interrupt Delay : 2500ns (ScanA + 1.0us)	*/
	pAsicHwReg->AREG_INTCSET	= 0x00000118;	/* ScanC Interrupt Delay : 3500ns (ScanB + 1.0us)	*/
	pAsicHwReg->AREG_INTNSET	= 0x00000168;	/* ScanN Interrupt Delay : 4500ns (ScanC + 1.0us)	*/
												/* ScanN is non-use									*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Setting each interrupt cycle																		*/
	/*----------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_INTCYCLE	= 0x00011f10;	/* ScanA = Encoder Communication Cycle		62.5us	*/
												/* ScanB = Encoder Communication Cycle * 2	125us	*/
												/* ScanC = Encoder Communication Cycle * 32	2ms		*/
												/* ScanN = Encoder Communication Cycle * 2	125us	*/
	pAsicHwReg->AREG_INTCYCLE	= 0x00000000;	/* Reset after interrupt activation					<S1D3>	*/

	/*----------------------------------------------------------------------------------------------*/
	/*		μProgram encoder processing interrupt setting													*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_INTMPESET	= 0x00000000;	/* ScanANot used because I want to process after completion			*/

	/*----------------------------------------------------------------------------------------------*/
	/*		External interrupt signal output selection															*/
	/*----------------------------------------------------------------------------------------------*/
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	pAsicHwReg->AREG_INTSET		= 0x00000005;	/* MECHATROLINK-3									*/
#else
	pAsicHwReg->AREG_INTSET		= 0x00000000;	/* MECHATROLINK-2 or Non-use						*/
#endif
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder IF initialization																*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicSencIF( ASIC_HW_IF_STR *pAsicHwReg )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Serial encoder selection																	*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_PFNCSET	&= 0xFFFFFF80;	/* Terminal function setting : SESEL0:S-PG0, SESEL1:S-PG1		*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Serial encoder IF register initialization													*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SEPGSET00	= 0x00000006;	/* CH0,1Common CrossTalk Avoidance ON, Delay Sync		*/
	pAsicHwReg->AREG_SEPGSET10	= 0x00000006;	/* CH0,1Common ManchesterCode Squelch ON				*/
#if 0/* A synchronization request is made at the start of communication. At initialization, it is disabled.	*/
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	/* Nothing Additional Establishment */
#elif (CSW_NETIF_TYPE == NETIF_M2_TYPE)
	pAsicHwReg->AREG_SEPGSET00	|= 0x00000020;	/*  Encoder CH0 Sync with M-II Network				*/
	pAsicHwReg->AREG_SEPGSET10	|= 0x00000020;	/*  Encoder CH1 Sync with M-II Network				*/
#elif (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	pAsicHwReg->AREG_SEPGSET00	|= 0x00000080;	/*  Encoder CH0 Sync with M-III Network				*/
	pAsicHwReg->AREG_SEPGSET10	|= 0x00000080;	/*  Encoder CH1 Sync with M-III Network				*/
#else /* (CSW_NETIF_TYPE == NETIF_CM_TYPE) */
	pAsicHwReg->AREG_SEPGSET00	|= 0x00000001;	/*  Encoder CH0 Sync with SYNCL Signal				*/
	pAsicHwReg->AREG_SEPGSET10	|= 0x00000001;	/*  Encoder CH1 Sync with SYNCL Signal				*/
#endif
#endif
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SEPGSET01	= 0x00000001;	/* CH0,1Common Slave Address : 01h					*/
	pAsicHwReg->AREG_SEPGSET11	= 0x00000001;	/* CH0,1Common 16Byte P0 Master Mode					*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Delay synchronization time setting																		*/
	/*----------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_SEDLLD		= 0x00000000;	/* Delay Sync Time : (n + 1) * 125 = 125[ns]
	pAsicHwReg->AREG_SEDLLD		= 0x0000001F;	/* Delay Sync Time : (0x1F + 1) * 125 = 4000[ns]	*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Serial encoder transmission data register initialization												*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SE0_TXD01	= 0x00000000;	/* CH0 Transmit Register							*/
	pAsicHwReg->AREG_SE0_TXD23	= 0x00000000;	/* CH0 Transmit Register							*/
	pAsicHwReg->AREG_SE0_TXD45	= 0x00000000;	/* CH0 Transmit Register							*/
	pAsicHwReg->AREG_SE0_TXD67	= 0x00000000;	/* CH0 Transmit Register							*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SE1_TXD01	= 0x00000000;	/* CH1 Transmit Register							*/
	pAsicHwReg->AREG_SE1_TXD23	= 0x00000000;	/* CH1 Transmit Register							*/
	pAsicHwReg->AREG_SE1_TXD45	= 0x00000000;	/* CH1 Transmit Register							*/
	pAsicHwReg->AREG_SE1_TXD67	= 0x00000000;	/* CH1 Transmit Register							*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Serial IF initialization																			*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicSerialIF( ASIC_HW_IF_STR *pAsicHwReg )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		General-purpose synchronous serial IF initialization															*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_IOSTAT	= KPX_AREG_IOSTAT_INIOUT;	/* Set Initial Output Data to High Word	*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Asynchronous serial IF initialization																*/
	/*----------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_UARTSTS	= 0x00000000;	/* UART Status Register								*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Pulse IF initialization																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC Command counter setting value table																*/
/*--------------------------------------------------------------------------------------------------*/
const	USHORT	AsicPocsetTable[7] =
{
	0x0650,		/* 0 : ~A,~B, 101=Sign+Pulse		*/
	0x0640,		/* 1 : ~A,~B, 100=CW+CCW			*/
	0x0630,		/* 2 : ~A,~B, 011=(A+B)*1			*/
	0x0610,		/* 3 : ~A,~B, 001=(A+B)*2			*/
	0x0600,		/* 4 : ~A,~B, 000=(A+B)*4			*/
	0x0050,		/* 5 :  A, B, 101=Sign+Pulse		*/
	0x0040,		/* 6 :  A, B, 100=CW+CCW			*/
};
/*--------------------------------------------------------------------------------------------------*/
LONG	SHal_InitAsicPulseIF( ASIC_HW_IF_STR *pAsicHwReg, USHORT prstsw, USHORT syssw1, USHORT comfil )
{
	LONG	i;
	USHORT	uswk;
	LONG	ret;
	ret = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Command counter setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	i = prstsw & 0x0F;									/* Command pulse form							*/
	if( i > 6 )											/* Parameter setting range check				*/
	{
		uswk = 0;
		ret = TRUE;										/* Parameter setting errors						*/
	}
	else
	{
		uswk = AsicPocsetTable[i];						/* Count mode,etc						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	if( ((prstsw >> 4) & 0x0F) < 2 )
	{
		uswk = ( uswk | 0x0800 );						/* CCPolarity setting								*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		CA,CB Filter settings																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (prstsw >> 12) & 0x0F )
	{
	case 0x00:											/* Line driver signal command input filter1	*/
		uswk = (uswk | ((syssw1 >> 4) & 0x3) );			/* (PnE0B.4-5), set with						*/
		break;
	case 0x01:											/* Command input filter for open collector signal	*/
		uswk = (uswk | ((comfil) & 0x3) );				/* (PnE0E.0-1), set with						*/
		break;
	case 0x02:											/* Line driver signal command input filter2	*/
		uswk = (uswk | ((comfil >> 8) & 0x3) );			/* (PnE0E.8-9), set with						*/
		break;
	default:
		uswk = (uswk | ((syssw1 >> 4) & 0x3) );			/* (PnE0B.4-5), set with						*/
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_POCSET = uswk;						/* Command counter setting							*/
	uswk = ( uswk | 0x0100 );							/* CLMLatch enable setting					*/
	pAsicHwReg->AREG_POCSET = uswk;						/* Command counter setting							*/
	pAsicHwReg->AREG_LODSET = LODSET_SET_LDCMD;			/* Counter load setting						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Command filter setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (prstsw >> 12) & 0x0F )
	{
	case 0x00:											/* Line driver signal command input filter1	*/
		pAsicHwReg->AREG_FCCSET = syssw1 & 0x0F00;		/* (PnE0B.8-11), set with						*/
		break;
	case 0x01:											/* Command input filter for open collector signal	*/
		pAsicHwReg->AREG_FCCSET = (comfil<<4) & 0x0F00;	/* (PnE0E.4-7), set with						*/
		break;
	case 0x02:											/* Line driver signal command input filter2	*/
		pAsicHwReg->AREG_FCCSET = (comfil>>4) & 0x0F00;	/* (PnE0E.12-15), set with					*/
		break;
	default:
		pAsicHwReg->AREG_FCCSET = syssw1 & 0x0F00;		/* (PnE0B.8-11), set with						*/
		break;
	}

#if 0 /* 2012.11.01 Y.Oka (>_<) Division output currently not supported */
// TODO: Frequency division setting should be separated into a dedicated function
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(BitNumber,Conversion cycle)											*/
/*--------------------------------------------------------------------------------------------------*/
		if( Kprm.f.FencUse == TRUE )					/* Fulque encoder frequency division output					*/
		{
			ASIC_WR( AREG_PLSET, 0x3400 );				/* ConvertBitNumber:16bit, Conversion cycle:64us			*//* <V501> */
		}
		else if( MencP.f.PlsTrn13bit == TRUE )			/* Pulse conversion13bit							*/
		{
			ASIC_WR( AREG_PLSET, 0x3100 );				/* ConvertBitNumber:13bit, Conversion cycle:64us			*/
		}
		else
		{
			ASIC_WR( AREG_PLSET, 0x3400 );				/* ConvertBitNumber:16bit, Conversion cycle:64us			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(Position,Origin,etc)												*/
/*--------------------------------------------------------------------------------------------------*/
		KPI_PCMASK_ON( );								/* Pulse conversion C phase mask(As a precaution)			*/
		ASIC_WR( AREG_DIVSET, 0x0102 );					/* Pulse conversion writing side setting:HostCPU			*//* <JL076_TEST> */
		ASIC_WR( AREG_POSET0, 0xFFFF );					/* Pulse conversion position setting						*/
		ASIC_WR( AREG_POSET0H, 0x0000 );				/* Pulse conversion position settingbit16-23				*/
		ASIC_WR( AREG_POTEST, 0xFFFF );					/* Pulse conversion counter setting					*/
		ASIC_WR( AREG_POSET1, 0x0000 );					/* Pulse conversion origin correction 1						*/
		ASIC_WR( AREG_POSET2, 0xFFFF );					/* Pulse conversion origin correction 2						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Division input filter setting																		*/
/*--------------------------------------------------------------------------------------------------*/
		ASIC_WR( AREG_FBCSET, 0x0005 );					/* Division input filter setting						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Divided pulse calculation & ASIC settings																*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencP.f.DivOut == TRUE )
		{
			KpxSetMencDividedPulse( );					/* Motor encoder frequency division pulse calculation and setting		*/
		}
		else if( FencP.f.DivOut == TRUE )
		{
			KpxSetFencDividedPulse( );					/* Fulque encoder frequency division pulse calculation & setting		*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		A/DInterface initialization																	*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicAdInterface( ASIC_HW_IF_STR *pAsicHwReg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		CommonA/DSetting(CommandA/D & Current detectionA/D)															*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_ADSET1		= 0x00001043;	/* Command AD 16Bit AD I/F							*/
												/* Current AD 10Bit Decimation Filter				*/
												/* Current AD Sync Invalid							*/

/*--------------------------------------------------------------------------------------------------*/
/*		CommandA/DInterface initialization																*/
/*--------------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_ADSET2		= 0x0000C0CE;	/* AD Clock, AD Frequency, Base Frequency			*/
	pAsicHwReg->AREG_ADSET2		= 0x0000D4CE;	/* AD Clock, AD Frequency, Base Frequency			*/
//	pAsicHwReg->AREG_ADSET3		= 0x00008280;	/* AD Start Timing, AD I/F Cycle					*/
	pAsicHwReg->AREG_ADSET3		= 0x0000A7A0;	/* AD Start Timing, AD I/F Cycle					*/
//	pAsicHwReg->AREG_ADSET4		= 0x0000DADA;	/* AD Latch Timing, AD Latch Timing					*/
	pAsicHwReg->AREG_ADSET4		= 0x0000FEFE;	/* AD Latch Timing, AD Latch Timing					*/
//	pAsicHwReg->AREG_ADSET5		= 0x000080DB;	/* AD Control H to L Edge, L to H Edge				*/
	pAsicHwReg->AREG_ADSET5		= 0x0000A5A1;	/* AD Control H to L Edge, L to H Edge				*/
//	pAsicHwReg->AREG_ADSET6		= 0x000080DB;	/* AD Control H to L Edge, L to H Edge				*/
	pAsicHwReg->AREG_ADSET6		= 0x0000A5A1;	/* AD Control H to L Edge, L to H Edge				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Current detectionA/DInterface initialization			   												*/
/*--------------------------------------------------------------------------------------------------*/
	/* Mode2 */									/* Current control cycle = 15.625us(64kHz)					*/
	pAsicHwReg->AREG_ADFCNT1	= 0x000000FB;	/* CurrentADIFBasic clock frequency = 3.2MHz				*/
	pAsicHwReg->AREG_ADFCNT2	= 0x00000038;	/* ADIF Cycle, (256 - 38h) / 3.2MHz = 62.5us		*/
	pAsicHwReg->AREG_ADFCNT3	= 0x00000000;	/* AD Latch Timing, AD Latch Timing					*/
	pAsicHwReg->AREG_ADFCNT4	= 0x00003839;	/* AD Control H to L Edge, L to H Edge				*/
	pAsicHwReg->AREG_ADFCNT5	= 0x00000000;	/* AD Control H to L Edge, L to H Edge				*/

/*--------------------------------------------------------------------------------------------------*/
/*		A/DDecimation filter initialization(Current detectionA/D & Voltage detectionA/D)									*/
/*--------------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_SDMCONF1	= 0x00001864;	/* CH1,2,3 Mode2, Current detection resolution = 12.8bit			*/
												/* CH1FixedA/DConversion completion interrupt				*/
if( CSW_DECIMATION == 0x0000 )	/* <S202> */
{
	pAsicHwReg->AREG_SDMCONF1	= 0x0000187B;	/* CH1,2 Mode3, Current detection resolution = 10.9bit				*//* <S1F2> */
}												/* CH3 Mode1, Voltage detection resolution= 13.5bit				*//* <S1F2> */
else
{
	pAsicHwReg->AREG_SDMCONF1	= 0x0000185B;	/* CH1,2,3 Mode3, Current detection resolution = 10.9bit			*//* @@ */
}												/* CH1FixedA/DConversion completion interrupt						*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SDMCONF2	= 0x00003A47;	/* Pre-trigger calculation:1/8times, Number of parallel units:4					*/
												/* CH1,2:20MHz Clock Out, CH3 External Clock In		*/
												/* Over range Detect:Mask							*/
/*--------------------------------------------------------------------------------------------------*/
if( CSW_DECIMATION == 0x0000 )	/* <S202> */
{
	pAsicHwReg->AREG_SDMCONF3	= 0x00001F06;	/* Voltage detection period = 250us(4kHz)							*//* <S1F2> */
}												/* CurrentADIFBasic clock frequency = 1.6MHz					*/
												/* ADIF Cycle, (256-06h)*(16-Fh)/16 = 250us			*//* <S1F2> */
else
{
	pAsicHwReg->AREG_SDMCONF3	= 0x00001638;	/* Voltage detection period = 125us(8kHz)							*/
}
												/* MSBWith inversion										*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SDMCONF4	= 0x00003839;	/* AD Control H to L Edge, L to H Edge				*/

/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection decimation filter initialization	OCDetection Contents not yet examined 2013.07.12 Y.Oka (>_<)				*/
/*--------------------------------------------------------------------------------------------------*/
//	pAsicHwReg->AREG_OCDCONF1	= 0x0000000F;	/* CH1,2 Mode3 Current detection resolution = 11.9bit				*/	/* <S1F7> */
//	pAsicHwReg->AREG_OCSDMCONF3	= 0x00000000;	/* @@												*/	/* <S1F7> */
//	pAsicHwReg->AREG_OCSDMCONF4	= 0x00000000;	/* @@												*/	/* <S1F7> */

/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection level setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_OCLVUP		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/
	pAsicHwReg->AREG_OCLVUM		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/
	pAsicHwReg->AREG_OCLVVP		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/
	pAsicHwReg->AREG_OCLVVM		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/
	pAsicHwReg->AREG_OCLVWP		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/
	pAsicHwReg->AREG_OCLVWM		= 0x0000FFFF;	/* @@UPhase upper limitOCDetection level							*/


	return;
}



/****************************************************************************************************/
/*																									*/
/*		Timer function initialization																			*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicTimer( USHORT syssw2, ASIC_HW_IF_STR *pAsicHwReg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		DBPulse setting:DBMethod = Thyristor															*/
/*--------------------------------------------------------------------------------------------------*/
	if( syssw2 & 0x0010 )						/* PnE0C.5:4 = x:1 : Thyristor						*/
	{											/* LowByte:Frequency,  HighByte:HighPulseWidth		*/
		pAsicHwReg->AREG_TIM1SET = 0x0000EB9C;	/* TB=(256 - 156)[us], TW=(255 - 235)[us]			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		DBPulse setting:DBMethod = Relay																*/
/*--------------------------------------------------------------------------------------------------*/
	else										/* PnE0C.5:4 = x:0 : Relay							*/
	{											/* LowByte:Frequency,  HighByte:HighPulseWidth		*/
		pAsicHwReg->AREG_TIM1SET = 0x0000009C;	/* TB=(256 - 156)[us], TW=(255 - 0)[us]				*/
	}

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Initialization of regenerative function																			*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicRegenerate( ASIC_HW_IF_STR *pAsicHwReg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration setting																					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Bit13 :	REGOFF	: 0:regenerative braking, 1:No regenerative braking											*/
/*		Bit14 :	DISCH	: 0:Normal operation,     1:Rapid discharge started											*/
/*		Bit15 : GFTON   : 0:Normal operation,     1:Ground fault test(BBEnabled at the time of occurrence)								*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG3	= 0x00000000;	/* REGOFF=0, DISCH=0, GFTON=0						*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		PWMControl function initialization																			*/
/*																									*/
/****************************************************************************************************/
void	SHal_InitAsicPwmControl( ASIC_HW_IF_STR *pAsicHwReg, LONG odt_c )
{
	ULONG	OnDelay;

/*--------------------------------------------------------------------------------------------------*/
/*		On-delay time setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;	/* ASIC Write Enable								*/
//	pAsicHwReg->AREG_SETREG2	&= (~BIT12);	/* Clock Select : 80MHz								*/
//	pAsicHwReg->AREG_SETREG2	|= (BIT11);		/* Clock Select : 80MHz								*/
/* for debug (INGRAMSo it was necessary) */
//	pAsicHwReg->AREG_SETREG2		|= BIT9;	/* Carrer Counter Sync								*/
/* for debug (INGRAMSo it was necessary) */
	/*----------------------------------------------------------------------------------------------*/
	OnDelay = (ULONG)((ASIC_CLKMHZ * odt_c) / 10);	/* 320[MHz] * Td[0.1us] / 10					*/
	pAsicHwReg->AREG_DYSETUP	= OnDelay;		/* UP1 : U-Phase,Positive-Side						*/
	pAsicHwReg->AREG_DYSETUN	= OnDelay;		/* UN1 : U-Phase,Negative-Side						*/
	pAsicHwReg->AREG_DYSETVP	= OnDelay;		/* VP1 : V-Phase,Positive-Side						*/
	pAsicHwReg->AREG_DYSETVN	= OnDelay;		/* VN1 : V-Phase,Negative-Side						*/
	pAsicHwReg->AREG_DYSETWP	= OnDelay;		/* WP1 : W-Phase,Positive-Side						*/
	pAsicHwReg->AREG_DYSETWN	= OnDelay;		/* WN1 : W-Phase,Negative-Side						*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;	/* ASIC Write Disable								*/

	return;
}

/****************************************************************************************************/
/*																									*/
/*		Encoder read interruptch0Delay time initialization													*/
/*																									*/
/****************************************************************************************************//*<S09C>*/
void SHal_InitEncRdDly_ch0( ASIC_HW_IF_STR *pAsicHwReg, USHORT PgBaudRate )
{
	LONG delay;

	/* Encoder read interruptch0Delay time setting */
	if( PgBaudRate == SENC_COMRATE_8MBPS )
	{
		delay = ENCRDDLY_8MBPS;
	}
	else
	{
		delay = ENCRDDLY_4MBPS;
	}
	/* Encoder read interruptch0Delay time setting */
	pAsicHwReg->AREG_ENCRDDLY0	= delay;	/* CH0 Read Interrupt Delay */

}

/****************************************************************************************************/
/*																									*/
/*		Encoder read interruptch1Delay time initialization													*/
/*																									*/
/****************************************************************************************************//*<S09C>*/
void SHal_InitEncRdDly_ch1( ASIC_HW_IF_STR *pAsicHwReg, USHORT PgBaudRate )
{
	LONG delay;

	/* Encoder read interruptch1Delay time setting */
	if( PgBaudRate == SENC_COMRATE_8MBPS )
	{
		delay	= ENCRDDLY_8MBPS;
	}
	else
	{
		delay	= ENCRDDLY_4MBPS;
	}
	pAsicHwReg->AREG_ENCRDDLY1	= delay;	/* CH1 Read Interrupt Delay */
}

/****************************************************************************************************/
/*																									*/
/*		Encoder read interrupt delay time initialization														*/
/*																									*/
/****************************************************************************************************//*<S09C>*/
//void SHal_InitEncRdDly( ASIC_HW_IF_STR *pAsicHwReg, USHORT PgBaudRate ) /* <S0E4> */
void SHal_InitEncRdDly( ASIC_HW_IF_STR *pAsicHwReg, USHORT PgBaudRate, UINT ch ) /* <S0E4> */
{
//	if(pAsicHwReg->AREG_ENCRDDLY0 == 0)	/* SGD7S, andSGD7W 0Axis only						*//* <S0E4> */
	if(ch == 0)	/* SGD7S, andSGD7W 0Axis only *//* <S0E4> */
	{
		/* Encoder read interruptch0Delay time setting */
		SHal_InitEncRdDly_ch0( pAsicHwReg, PgBaudRate );
	}
	else								/* SGD7W 1Axis only										*/
	{
		/* Encoder read interruptch1Delay time setting */
		SHal_InitEncRdDly_ch1( pAsicHwReg, PgBaudRate );
	}
}
/****************************************************************************************************/
/*																									*/
/*		Encoder position correction function initialization																*/
/*																									*/
/****************************************************************************************************//* <S004> */
void SHal_InitEncPosComp( ASIC_HW_IF_STR *pAsicHwReg, SVIP_INIT_PCOMP *SvipInitPcomp )/*<S09C>Variable name change*/
{
	UCHAR BitNo;
	UCHAR Dpoint;
	LONG PosSftR;
	LONG PosSftX;
	LONG PosSign;
	LONG PosLeng;

	BitNo  = SvipInitPcomp->bit_dp.b.l;
	Dpoint = SvipInitPcomp->bit_dp.b.h;

	if( SvipInitPcomp->AxisMotType  == MOTTYPE_ROTARY)
	{	/* For rotary type */
		PosLeng = 0;
		PosSftR = 24 - BitNo;
		PosSftX = 32 - BitNo - (80 - Dpoint);	/* JL-086(Mistakes in specifications)	*//* <S210>:From common to rotating type only */
	}
	else
	{	/* In the case of linear */
		PosLeng = HPOS_TO_XPOS;
		PosSftR = 24 - PosLeng;
		PosSftX = LMX_MPOS_SFT;					/* <S210> */
	}
	PosSign = (SvipInitPcomp->RevAsmMotor == TRUE) ? 1 : 0;

	/* Encoder correction settings and alarms:EncFlag(0390h)	*/
	pAsicHwReg->AREG_ENCFLAG = ( ENCINT << TRGSEL )						/* Startup method setting 			*/
							 + ( SvipInitPcomp->AxisMotType<<MOT_AMPTYPE )/* Rotation type/Linear type		*/
							 + ( POSDATSEL_11 << POSDAT_SEL );			/* Position correction enable setting	*/

	/* Shift amount setting:PGDCAL(0394h)					*/
	pAsicHwReg->AREG_PGDCAL = ( PosSftX << POS_SFTX )					/*Position data shift amount1		*/
							+ ( PosSftR << POS_SFTR )					/*Position data shift amount2		*/
							+ ( PosLeng << POS_LENG )					/*Position data length			*/
							+ ( PosSign << POS_SIGN );					/*Position data code			*/

	/* 1Rotation encoder pulse number setting:PulseNo(0398h)	*/
	pAsicHwReg->AREG_PULSENO = SvipInitPcomp->PulseNo;

	/* Acceleration error level:AccErrLvl(03A0h)			*/
	pAsicHwReg->AREG_ACCERRLVL = SvipInitPcomp->AccErrLvl;

	/* Cumulative error level:SPGFAIL(03A8h)	 			*/
	pAsicHwReg->AREG_SPGFAIL = SvipInitPcomp->spgfail.b.l;

	/* Number of clocks during latch cycle:LchClkNum(03ACh) 		*/
	pAsicHwReg->AREG_LCHCLKNUM = ENCLCHCLKNUM;

	/* Number of clocks in calculation cycle:SaclkNum(03B0h) 		*/
	pAsicHwReg->AREG_SACLKNUM = SAENCCLKNUM;

	/* Reset setting:RSTSET(03CCh)	 				*/
	pAsicHwReg->AREG_RSTSET = ( 1 << ALMCLR0 )					/* ComAlarmClear					*/
							+ ( 1 << ALMCLR1 )					/* AccAlarmClear					*/
							+ ( 1 << ACENCLR )					/* Acceleration check enable clear	*/
							+ ( 1 << SRECLR )					/* Clear reception error flag			*/
							+ ( 1 << SRCCLR );					/* Clear reception completion flag				*/
}

/****************************************************************************************************/
/*																									*/
/*		Encoder position correction function(Acceleration check level)										*/
/*																									*/
/* Feedback option related information isCh1use.													*/
/* Ch1, control whether low-speed OL can be detected in1Axis number(+0x1000)area.				*/
/****************************************************************************************************//* <S004> */
void SHal_InitEncPosCompAccLvl( ASIC_HW_IF_STR *pAsicHwReg )
{
	/* Initial value0so set a provisional value before connecting to the encoder.	*/
	/* Because acceleration errors are detected							*/

	/* Acceleration error level:AccErrLvl(03A0h)			*/
	pAsicHwReg->AREG_ACCERRLVL = 0x007FFFFF;

}
/****************************************************************************************************/
/*																									*/
/*		Initialize timer latch function																	*/
/*																									*/
/****************************************************************************************************//* <S004> */
BOOL SHalDrv_InitLatchSignal(ASIC_HW_IF_STR *AsicHwReg, SHAL_LATCH_STS_ADR* LtStsAdr,
												USHORT PrmInselA, BOOL* ExtLtEnable)
{
LONG	i;
BOOL	rc;
UCHAR	prmSet[3];
UCHAR	prmChek[3];

	/* Local variable setting	*/
	prmSet[0] = (PrmInselA >>  4) & 0x0F;
	prmSet[1] = (PrmInselA >>  8) & 0x0F;
	prmSet[2] = (PrmInselA >> 12) & 0x0F;
	rc = TRUE;

	/* External latch input signal selection	*/
	for( i=0; i < 3; i++ )
	{
		switch( prmSet[i] )
		{
		case KPI_EXT1_LOWLEVEL:		/* <S1D6> SI4, LowLevel			*/
					LtStsAdr[i].STimShift = 13;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT5;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT1;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI4;
					break;
		case KPI_EXT2_LOWLEVEL:		/* <S1D6> SI5, LowLevel			*/
					LtStsAdr[i].STimShift = 14;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT6;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT3;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI5;
					break;
		case KPI_EXT3_LOWLEVEL:		/* <S1D6> SI6, LowLevel			*/
					LtStsAdr[i].STimShift = 15;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT7;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT5;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI6;
					break;
		case KPI_EXT1_HIGHLEVEL:	/* <S1D6> SI4, HighLevel		*/
					LtStsAdr[i].STimShift = 9;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT1;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT0;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI4;
					break;
		case KPI_EXT2_HIGHLEVEL:	/* <S1D6> SI5, HighLevel		*/
					LtStsAdr[i].STimShift = 10;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT2;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT2;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI5;
					break;
		case KPI_EXT3_HIGHLEVEL:	/* <S1D6> SI6, HighLevel		*/
					LtStsAdr[i].STimShift = 11;
					LtStsAdr[i].STimLtAdr = &AsicHwReg->AREG_STIMLT3;
					LtStsAdr[i].SFbLtAdr = &AsicHwReg->AREG_SFBLT4;
					ExtLtEnable[i] = TRUE;
					prmChek[i]=HW_SI6;
					break;
		default:	/* Not Select				*/
					LtStsAdr[i].STimShift = 0;
					LtStsAdr[i].STimLtAdr = 0;
					LtStsAdr[i].SFbLtAdr = 0;
					ExtLtEnable[i] = FALSE;
					prmChek[i]=HW_SI_NO_DEFINE;
					break;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Latch selection consistency check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( prmChek[0] == prmChek[1] )
	{
		if( prmChek[0] != 0 ){	rc = FALSE;}
	}
	else if( prmChek[0] == prmChek[2] )
	{
		if( prmChek[0] != 0 ){	rc = FALSE;}
	}
	else if( prmChek[1] == prmChek[2] )
	{
//		if( prmChek[0] != 0 ){	rc = FALSE;}
		if( prmChek[1] != 0 ){	rc = FALSE;}	/* <S226> */
	}
/*--------------------------------------------------------------------------------------------------*/
	return rc;

}

/****************************************************************************************************/
/*																									*/
/*		Microprogram load processing																*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_LoadAsicMicroProgram( ASICS *SvAsicRegs, CULONG *src_adr, LONG u_size)
{
	LONG				i;
#ifndef _VC_DEBUG_ /* 2012.08.24 Y.Oka Make Mercury Base Software */
	VULONG				dummy;
#endif /* _VC_DEBUG_ */ /* 2012.08.24 Y.Oka Make Mercury Base Software */
	CULONG				*PrgSrc;							/* MicroProgram Src  Address			*/
	LONG				PrgSize;							/* MicroProgram Word Size				*/
	ASIC_HW_IF_STR		*pAsicHwReg;

	pAsicHwReg = SvAsicRegs->AsicHwReg;
/*--------------------------------------------------------------------------------------------------*/
/*		μProgram selection																			*/
/*--------------------------------------------------------------------------------------------------*/
	PrgSrc  = src_adr;										/* Set Src  Address						*/
	PrgSize = u_size;										/* Set Word Size						*/
	if( (PrgSize % 2) != 0 )								/* Size check						*/
	{
		return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		μProgram write permission																	*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1 = 0x00000005;					/* ASIC Write Enable					*/
	pAsicHwReg->MREG_CTSTR = MPCMD_PRGSTOP;					/* μStop program execution					*/
	SvAsicRegs->MicroRunningSts = FALSE;					/* μStop program execution(CPUManagement)		*/
/*--------------------------------------------------------------------------------------------------*/
/*		μProgram transfer processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->MREG_UPLDA	= 0;
	for( i=0; i<PrgSize; i++ )
	{

#ifndef _VC_DEBUG_ /* 2012.08.24 Y.Oka Make Mercury Base Software */

		/* 2010.05.21 Y.Oka WaitFor */
		dummy = pAsicHwReg->AREG_TSTT1CNT;

		/* Load pointer check */
		if( ( pAsicHwReg->MREG_UPLDA ) != ((i%2)<<14)+(i/2) )
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}

		pAsicHwReg->MREG_UPLD	= PrgSrc[i];		/* μWrite program					*/

#else /* _VC_DEBUG_ */ /* 2012.08.24 Y.Oka Make Mercury Base Software */

		/* μWrite program */
		*(ULONG*)(&SvAsicRegs->pDummyMicroSram[i*2]) = *(ULONG*)(&PrgSrc[i]);

#endif /* _VC_DEBUG_ */ /* 2012.08.24 Y.Oka Make Mercury Base Software */

	}
/*--------------------------------------------------------------------------------------------------*/
/*		μProgram verification processing																*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->MREG_UPLDA	= 0;
	for( i=0; i<PrgSize; i++ )
	{

#ifndef _VC_DEBUG_ /* 2012.08.24 Y.Oka Make Mercury Base Software */

		/* 2010.05.21 Y.Oka WaitFor */
		dummy = pAsicHwReg->AREG_TSTT1CNT;

		/* Load pointer check */
		if( ( pAsicHwReg->MREG_UPLDA ) != ((i%2)<<14)+(i/2) )
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}

		/* μProgram read & data check */
		if( ( pAsicHwReg->MREG_UPLD ) != PrgSrc[i] )
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}

#else /* _VC_DEBUG_ */ /* 2012.08.24 Y.Oka Make Mercury Base Software */

		/* μProgram read & data check */
		if( SvAsicRegs->pDummyMicroSram[i] != PrgSrc[i] )
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}

#endif /* _VC_DEBUG_ */ /* 2012.08.24 Y.Oka Make Mercury Base Software */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		ASIC write prohibition																			*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;				/* ASIC Write Disable					*/

	return FALSE;
}



/****************************************************************************************************/
/*																									*/
/*		Reset microprogram register														*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_ResetAsicMicroRegister( ASICS *SvAsicRegs )
{
	LONG				i;									/* Loop Index							*/
	VULONG				vulwk;								/* ULONG Work							*/
	VULONG				Dummy;								/* Dummy for Read						*/
	VULONG				*WkReg;								/* MicroProgram Work Register Address	*/
	ASIC_HW_IF_STR		*pAsicHwReg;

	WkReg = (VULONG*)(SvAsicRegs->AsicMicroReg);			/* MicroProgram Work Register Address	*/
	pAsicHwReg = SvAsicRegs->AsicHwReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Stop microprogram execution(As a precaution)														*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;				/* ASIC Write Enable					*/
	pAsicHwReg->MREG_CTSTR		= MPCMD_PRGSTOP;			/* μStop program execution					*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;				/* ASIC Write Disable					*/
	SvAsicRegs->MicroRunningSts = FALSE;					/* μStop program execution(CPUManagement)		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Check and clear work register															*/
/*--------------------------------------------------------------------------------------------------*/
	for( i=0; i<(ASIC_MREGSIZE/4); i++ )					/* Write to WkReg[i]					*/
	{
		WkReg[i]	= (ULONG)(i);							/* Write 0,1,2,3,4,5,...				*/
	}
	/*----------------------------------------------------------------------------------------------*/
	for( i=0; i<(ASIC_MREGSIZE/4); i++ )					/* Check WkReg[i]						*/
	{
		vulwk = WkReg[i];									/* Read WkReg[i]						*/
		if( vulwk != (i) )									/* Check Read Data						*/
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	for( i=0; i<(ASIC_MREGSIZE/4); i++ )					/* Check and Clear WkReg[i]				*/
	{
		WkReg[i]	= 0x5A5A5A5A;							/* Write 0x5A5A5A5A to WkReg[i]			*/
		Dummy = pAsicHwReg->AREG_PWMU;						/* Dummy Read							*/
		vulwk = WkReg[i];									/* Read and Check						*/
		if( vulwk != 0x5A5A5A5A )							/* Check Read Data						*/
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}
	/*----------------------------------------------------------------------------------------------*/
		WkReg[i]	= 0xA5A5A5A5;							/* Write 0xA5A5A5A5 to WkReg[i]			*/
		Dummy = pAsicHwReg->AREG_PWMU;						/* Dummy Read							*/
		vulwk = WkReg[i];									/* Read and Check						*/
		if( vulwk != 0xA5A5A5A5 )							/* Check Read Data						*/
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}
	/*----------------------------------------------------------------------------------------------*/
		WkReg[i]	= 0x00000000;							/* Clear WkReg[i]						*/
		Dummy = pAsicHwReg->AREG_PWMU;						/* Dummy Read							*/
		vulwk = WkReg[i];									/* Read and Check						*/
		if( vulwk != 0x00000000 )							/* Check Read Data						*/
		{
			return TRUE;	/* A.BF2 : System alarm 2(MicroAbnormal) */
		}
	}
	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Load processing of microprogram table data					<NeCO>_130919				*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_LoadAsicMicroTable( USHORT *asic_adr, USHORT *src_adr, ULONG array_size)
{
	LONG				i;									/* Loop Index							*/
	
	for( i=0; i<array_size; i++ )							/* Write to AsicReg[i]					*/
	{
		asic_adr[i] = src_adr[i];
	}
	return FALSE;
}

// <S146>
#define JL086_MCAREA_S_TOP		(0xFFE70000)
#define	JL086UprgSz_SGD7_S		*(CULONG*)(JL086_MCAREA_S_TOP)
#define	YecAsicPrg_SGD7_S_Addr	(JL086_MCAREA_S_TOP+4)
#if (CSW_BOARD_TYPE == SGD7S_TYPE)
#define	JL086UprgSz_SGD7		JL086UprgSz_SGD7_S
#define	YecAsicPrg_SGD7_Addr	YecAsicPrg_SGD7_S_Addr
#elif	(CSW_BOARD_TYPE == SGD7W_TYPE)
#define JL086_MCAREA_W_TOP		(0xFFE75000)
#define	JL086UprgSz_SGD7		*(CULONG*)(JL086_MCAREA_W_TOP)
#define	YecAsicPrg_SGD7_Addr	(JL086_MCAREA_W_TOP+4)
#endif
#define	YecAsicPrg_SGD7			(CULONG*)(YecAsicPrg_SGD7_Addr)
#define	OVMODTBLG				(CUSHORT*)((CULONG*)YecAsicPrg_SGD7_S_Addr + JL086UprgSz_SGD7_S)
/****************************************************************************************************/
/*																									*/
/*		Initialization of ASIC microprogram															*/
/*																									*/
/****************************************************************************************************/
BOOL	SHal_InitMicroProgram( ASICS *SvAsicRegs )
{
	LONG rst_err, load_err, table_err;

	/* Microprogram load processing */
	load_err = SHal_LoadAsicMicroProgram( SvAsicRegs, YecAsicPrg_SGD7, (JL086UprgSz_SGD7) );

	/* Clear microprogram register */
	rst_err = SHal_ResetAsicMicroRegister( SvAsicRegs );

	/* Load processing of microprogram table data */ /* <NeCO>_130919 */
	table_err = SHal_LoadAsicMicroTable( (USHORT *)TABLESTART, (USHORT *)OVMODTBLG, TABLESIZE );

	
	return (load_err | rst_err | table_err);
}


/****************************************************************************************************/
/*																									*/
/*		ASIC Microprogram start processing															*/
/*																									*/
/****************************************************************************************************/
void	SHal_StartMicroProgram( ASICS *SvAsicRegs )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Start ASIC Current-A/D, Reference-A/D, Micro-Program										*/
/*--------------------------------------------------------------------------------------------------*/
	/* Current-A/D Cycle Synchronize */
	SvAsicRegs->AsicHwReg->AREG_ADFSYNC = 0x0000;
	/* Reference-A/D Cycle Synchronize */
	SvAsicRegs->AsicHwReg->AREG_ADSYNC = 0x0000;
/*----------------------------------------------------------------------------------------------*/
	/* Clear Alarm */
	SvAsicRegs->AsicHwReg->AREG_ALMCLR = 0x802F;
	/* ASIC Write Enable */
	SvAsicRegs->AsicHwReg->AREG_SETREG1 = 0x0005L;
	/* Start Micro-Program */
	// ★(MregCtstr = MPCMD_PRGSTART) MregCtstrDon't forget to update!
	SvAsicRegs->AsicHwReg->MREG_CTSTR = MPCMD_PRGSTART;
	SvAsicRegs->MicroRunningSts = TRUE;
	/* Set ASIC Start Register */
	SvAsicRegs->AsicHwReg->AREG_SETREG1 = 0x0001;
}



/****************************************************************************************************/
/*																									*/
/*		ASIC Microprogram stop processing															*/
/*																									*/
/****************************************************************************************************/
void	SHal_StopMicroProgram( ASICS *SvAsicRegs )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Stop ASIC Micro-Program																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* ASIC Write Enable */
	SvAsicRegs->AsicHwReg->AREG_SETREG1 = 0x0005L;
	/* Start Micro-Program */
	// ★(MregCtstr = MPCMD_PRGSTART) MregCtstrDon't forget to update!
	SvAsicRegs->AsicHwReg->MREG_CTSTR = MPCMD_PRGSTOP;
	SvAsicRegs->MicroRunningSts = FALSE;
}



#ifdef CSW_MICRO_DEBUG_MODE
/****************************************************************************************************/
/*																									*/
/*	System clock (feedback timer) difference acquisition function ( 1Clock : 37.5[ns] )											*/
/*																									*/
/****************************************************************************************************/
void SHal_SetMicroDebugMode( ASIC_HW_IF_STR *pAsicHwReg )
{
	/* Output INT2 Interrupt Signal from TP5 */
	pAsicHwReg->MREG_DBOS = 0x00000900;
}
#endif



/****************************************************************************************************/
/*																									*/
/*	System clock→[ns]Convert μFor measuring program processing time											*/
/*																									*/
/****************************************************************************************************/
ULONG SHal_ClockToTime( USHORT clock_diff )
{
	float time_in_ns;
	time_in_ns = (float)clock_diff * (float)(37.5);
	return (ULONG)time_in_ns; /* [ns] */
}


/****************************************************************************************************/
/*																									*/
/*	System clock (feedback timer) acquisition function 					)											*/
/*																									*/
/****************************************************************************************************/
USHORT SHal_GetSysClock( ASICS *SvAsicRegs )
{
	/* ASIC(16bit : 1Clock : 37.5[ns]) */
	return SvAsicRegs->AsicHwReg->AREG_TCNT;
}

/****************************************************************************************************/
/*																									*/
/*	System clock (feedback timer) difference acquisition function ( 1Clock : 37.5[ns] )											*/
/*																									*/
/****************************************************************************************************/
USHORT SHal_Get_dClock( USHORT last_clk , USHORT clk )
{
	USHORT d_clk;

	if( clk > last_clk )
	{
		d_clk = clk - last_clk;
	}
	else
	{/* Overflow Countermeasures*/
		d_clk = ( 0xFFFF - last_clk ) + clk;
	}
	/* Clock this time(clk)and the previous clock(last_clk) */
	return d_clk;
}



#endif /* _JL076_USE_ */
/***************************************** end of file **********************************************/
