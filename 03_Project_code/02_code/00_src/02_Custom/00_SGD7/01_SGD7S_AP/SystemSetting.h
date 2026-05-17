/****************************************************************************
Description	: Definitions for each product functionality
----------------------------------------------------------------------------
Author			: Yaskawa Electric Corp.,
					Copyright (c) 2012 All Rights Reserved

Project			: Mercury

----------------------------------------------------------------------------
Changes :
	Name	Date		Description
  ------------------------------------------------------------------------
	Y.Oka	2012.08.06	created

******************************************************************************/
#ifndef _SYSTEM_SETTING_H
#define _SYSTEM_SETTING_H

#include "SpecCommon.h"

#ifdef _TEST_BOARD_
#define _JL076_USE_
#endif

/*****************************************************************************/
/*		Product information													 */
/*****************************************************************************/
#define MAX_AXIS_NUMBER		1			/* Maximum number of connected axes */
#define MAX_SLOT_NUMBER		1			/* Maximum slot(ASIC)Number */

#define	CSW_NETIF_TYPE		NETIF_ANLGPLS_TYPE
#define CSW_BOARD_TYPE		SGD7S_TYPE										  /* <S0A2> */


/****************************************************************************/
/*		Model dependentH/WDefinition														*/
/****************************************************************************/
#define SVFSEL_EEPROM_USE 	(1)			/* 1: EEPROMUsing 0: Unused			*/
#define SVFSEL_VIRSIF_USE	(1)			/* 1: Panel use 0: Unused			*/
#define SVFSEL_PANELOP_USE	(1)			/* 1: Panel use 0: Unused			*/
#define EEPROM_SIZE			0x1000		/* EEPROM Memory Size				*//* <S10A> */


/*****************************************************************************/
/*		Compiling for debuggingSWDefinition											 */
/*****************************************************************************/
#if !defined(CSW_CSTR_MBB_TYPE) && defined(CSW_BSP_CSTR_IFM3)
#define CSW_CSTR_DB_REVERSE		TRUE
#endif


#define CSW_CSTR_IF_PROTO_USE		FALSE
#define CSW_CSTR_AUTO_ADDR_CFG		FALSE	/* MDINIT=ON */
#define CSW_DEBUG_PRINT_EBABLE		FALSE
#define CSW_DX_CONVERTER_USE		FALSE
#define CSW_DIF_USE					FALSE
#ifdef _VC_DEBUG_
//#define CSW_NO_SERVO_ASICS
#define CSW_NO_M3_ASICS
#else
//#define CSW_NO_SERVO_ASICS
#endif

//#ifdef _IF_BOARD_REVC_	/* <S063> */
//#define	_USB_FUNC_USE_		/* revCFrom boardUSBFunction enabled(Use compile switch as it cannot be determined by software) */
//#endif	//#ifdef _IF_BOARD_REVC_	/* <S063> */



#define	CSW_SCANA_ENABLE				/* CPU ScanAProcessing enabled */


/****************************************************************************/
/*		Servo function selection switch(CSW_SVCF_****)								*/
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*		Setting whether to use electronic gear(Processing time countermeasures)								*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_EGEAR_USE			TRUE
/*--------------------------------------------------------------------------*/
/*		Model following control use/nonuse setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_MFC_USE			TRUE
/*--------------------------------------------------------------------------*/
/*		AMold vibration damping control use/nonuse setting											*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_ATYPE_RES_VIB_USE	TRUE
/*--------------------------------------------------------------------------*/
/*		Vibration detection use/non-use setting											*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_VIB_DETECT_USE		TRUE
/*--------------------------------------------------------------------------*/
/*		Vibration suppression filter use/non-use setting									*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_RESVIB_FIL_USE		TRUE
/*--------------------------------------------------------------------------*/
/*		Field-weakening control use/non-use setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_WEAKENFIELD_USE	TRUE
/*--------------------------------------------------------------------------*/
/*		Use/non-use setting of micro voltage compensation gain function							*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_MPKVVCOMP			TRUE
/*--------------------------------------------------------------------------*/
/*		Disturbance observer use/non-use setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_DISTURBOBS			TRUE
/*--------------------------------------------------------------------------*/
/*		Phase compensated speed observer use/nonuse setting								*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_EHVOBS_USE			TRUE
/*--------------------------------------------------------------------------*/
/*		Position command moving average use/nonuse setting									*/
/*--------------------------------------------------------------------------*/
#define CSW_SERVO_PCMD_FIL_USE		TRUE									  /* <S065> */
/*--------------------------------------------------------------------------*/
/*		Position command moving average(For command update cycle interpolation)Use/non-use setting				*/
/*--------------------------------------------------------------------------*/
#define CSW_CMDRENEW_PCMD_FIL_USE	FALSE									  /* <S065> */
/*--------------------------------------------------------------------------*/
/*		Position commandHigh Filter Use/non-use setting				                        */
/*--------------------------------------------------------------------------*/
#define CSW_SERVO_PCMD_HIGH_FIL_USE   TRUE                                    /* <S0C3> */
/*--------------------------------------------------------------------------*/
/*		Full-closed loop control use/non-use setting									*/
/*--------------------------------------------------------------------------*/
#define CSW_FULC_NO_SUPPORT			0	/* Fulke control disabled 					*/
#define CSW_FULC_BASIC_SUPPORT		1	/* Fulke control enabled					*/
#define CSW_FULC_SB_SUPPORT			2	/* Fulke control enabled(ScanBPosition loop)	*/
//#define CSW_FULC_SA_SUPPORT		3	/* Fulke control enabled(ScanAPosition loop)	*/
#define CSW_SVCF_FULC_MODE			CSW_FULC_BASIC_SUPPORT
/*--------------------------------------------------------------------------*/
/*		Backlash use/non-use setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_BACKLASHCOMP		TRUE		/* <S18B> */
/*--------------------------------------------------------------------------*/
/*		FBPosition latch use/nonuse settings										*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_FBLATCH			FALSE
/*--------------------------------------------------------------------------*/
/*		Model definition (presence or absence of low-speed OL function depending on the model)							*/
/*--------------------------------------------------------------------------*/
#define CSW_SVCF_CHKAMPLOL 			1										/* <S154> */
/*--------------------------------------------------------------------------*/
/*		Lifetime prediction use/nonuse setting											*/
/*--------------------------------------------------------------------------*/
//#define CSW_SVCF_LIFEPRED_USE 		TRUE									/* <S154> *//* <S186> */
#define CSW_SVCF_LIFEPRED_USE 		FALSE									/* <S154> *//* <S186> */
/*--------------------------------------------------------------------------*/
/*		Power consumption monitor use/non-use setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_POWER_MONITOR_USE		TRUE									/* <S020> */
/*--------------------------------------------------------------------------*/
/*		Trace network trigger use/nonuse setting							*/
/*--------------------------------------------------------------------------*/
//#define CSW_NET_TRG_USE			FALSE									/* <S021> */	/* <S0C9> Delete */
#define CSW_NET_TRG_USE				TRUE									/* <S0C9> Delete */
/*--------------------------------------------------------------------------*/
/*		TemperatureICOHUse/non-use setting										*/
/*--------------------------------------------------------------------------*/
#define CSW_DETECT_OH_FROM_TEMPIC 	TRUE									  /* <S0ED> */
/*--------------------------------------------------------------------------*/
/*		Σ-7/Σ-M Option													*/
/*--------------------------------------------------------------------------*/
#define   PRODUCT_SERIES_SEVEN    FALSE                            /* <S09F> */
#if(PRODUCT_SERIES_SEVEN==FALSE)                                            /* <S09F> */
/*		HWBBControl execution selection													 */
/*---------------------------------------------------------------------------*/
#define CSW_HW_HWBB_SUPPORT			FALSE                                   /* <S09F> */
/*		ADControl execution selection													 */
/*---------------------------------------------------------------------------*/
#define		SVFSEL_ADC_USE 			(0)				/* 1: ADC?? 0: Main??		<S09F>	*/
/*--------------------------------------------------------------------------*/
/*		Option card compatibility definition											*/
/*--------------------------------------------------------------------------*/
/* CSW_OPT_CARD_OPTION_SUPPORT 0x****									*/
/* 								   │││└	Safety response status			*/
/* 								   ││└ Command option compatible status			*/
/* 								   │└ Feedback option support status	*/
/* 								   └ Reservation									*/
/*--------------------------------------------------------------------------*//* <S04D> */
/*	Safety response definition													*/
#define CSW_SAFETY_NO_SUPPORT	0x0000			/* Safety disabled 		*/
#define CSW_OPT_CARD_SAFETY		CSW_SAFETY_NO_SUPPORT

/*	Definition for command options													*/
#define CSW_CMDOPTION_NO_SUPPORT	0x0000		/* Command option disabled 		*/
#define CSW_OPT_CARD_CMDOPTION		CSW_CMDOPTION_NO_SUPPORT

/*	Feedback option support definitions										*/
#define CSW_FBOPTION_NO_SUPPORT	0x0000		/* Feedback option disabled */
#define CSW_FBOPTION_SUPPORT	0x0100		/* Feedback option enabled */
#define CSW_OPT_CARD_FBOPTION	CSW_FBOPTION_NO_SUPPORT	  /* <S09F> */

/*	Option card compatibility definition												*/
#define CSW_OPT_CARD_NO_SUPPORT	0x0000		/* Option card disabled			*/
#define CSW_OPT_CARD			(CSW_OPT_CARD_SAFETY |\
								  CSW_OPT_CARD_CMDOPTION |\
								   CSW_OPT_CARD_FBOPTION )
									   
#define	SVFSEL_ONLY_OH_ADC_USE 		(0)		/* 1: ADCUsing 0: Unused			*/
/*	Decimation filter	<S202>											*/
#define CSW_DECIMATION	0x0001		/* CH1,2,3 Mode3 */

#else
#define CSW_HW_HWBB_SUPPORT			FALSE
/*		ADControl execution selection													 */
/*---------------------------------------------------------------------------*/
#define		SVFSEL_ADC_USE 			(1)				/* 1: ADC?? 0: Main??		 */
/*--------------------------------------------------------------------------*/
/*		Option card compatibility definition											*/
/*--------------------------------------------------------------------------*/
/* CSW_OPT_CARD_OPTION_SUPPORT 0x****									*/
/* 								   │││└	Safety response status			*/
/* 								   ││└ Command option compatible status			*/
/* 								   │└ Feedback option support status	*/
/* 								   └ Reservation									*/
/*--------------------------------------------------------------------------*//* <S04D> */
/*	Safety response definition													*/
#define CSW_SAFETY_NO_SUPPORT	0x0000			/* Safety disabled 		*/
#define CSW_OPT_CARD_SAFETY		CSW_SAFETY_NO_SUPPORT

/*	Definition for command options													*/
#define CSW_CMDOPTION_NO_SUPPORT	0x0000		/* Command option disabled 		*/
#define CSW_OPT_CARD_CMDOPTION		CSW_CMDOPTION_NO_SUPPORT

/*	Feedback option support definitions										*/
#define CSW_FBOPTION_NO_SUPPORT	0x0000		/* Feedback option disabled */
#define CSW_FBOPTION_SUPPORT	0x0100		/* Feedback option enabled */
#define CSW_OPT_CARD_FBOPTION	CSW_FBOPTION_SUPPORT

/*	Option card compatibility definition												*/
#define CSW_OPT_CARD_NO_SUPPORT	0x0000		/* Option card disabled			*/
#define CSW_OPT_CARD			(CSW_OPT_CARD_SAFETY |\
								  CSW_OPT_CARD_CMDOPTION |\
								   CSW_OPT_CARD_FBOPTION )
									   
#define	SVFSEL_ONLY_OH_ADC_USE 		(0)		/* 1: ADCUsing 0: Unused			*/
/*	Decimation filter	<S202>											*/
#define CSW_DECIMATION	0x0000		/* CH1,2 Mode3 CH3 Mode1	*/
#endif
/*--------------------------------------------------------------------------*/
/*		Board temperature detection function definition												*/
/*--------------------------------------------------------------------------*/
#define CSW_DETECT_TEMPERATURE		TRUE									/* <S03A> */
/*--------------------------------------------------------------------------*/
/*		Division pulse output definition													*/
/*--------------------------------------------------------------------------*/
#define	SVFSEL_DIVOUT_USE			TRUE /* Divided output(TRUE:Using;FALSE:Not used)*//*<S02D>*/

/****************************************************************************/
/*		Scan cycle, execution timing definition									*/
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*		Cycle time definition													*/
/*--------------------------------------------------------------------------*/
#define NS_CHANGE_MS	1000000					/* [ms] ⇒ [ns]Conversion coefficient		*/
#define US_CHANGE_MS	1000					/* [us] ⇒ [ms]Conversion coefficient		*/
#define NS_CHANGE_US	1000					/* [ns] ⇒ [us]Conversion coefficient		*/

/*--------------------------------------------------------------------------*/
/*		JL-077μScan cycle												*/
/*--------------------------------------------------------------------------*/
#define KPI_MACYCLENS	15625							/* ScanI Cycle[ns]	*/
#define KPI_MBCYCLENS	62500							/* MprgB Cycle[ns]	*/
#define KPI_SACYCLENS	62500							/* ScanA Cycle[ns]	*/
#define KPI_SACYCLEUS	((KPI_SACYCLENS + 500)/1000)	/* ScanA Cycle[us]	*/
#define KPI_MBCYCLEUS	((KPI_MBCYCLENS + 500)/1000)	/* MprgB Cycle[us]	*/
#define KPI_SACOUNTMS	(1000000/KPI_SACYCLENS)			/* ScanA Cycle Count per 1ms */

/*--------------------------------------------------------------------------*/
/*		ScanBPeriod(ScanBSince the cycle is determined by the parameter, a default value is defined)	*/
/*--------------------------------------------------------------------------*/
#define KPI_SBCYCLE_BASE_NS		125000		/* ScanB Base Cycle Time [ns]	*/
#define KPI_SBCYCLEUS			(KPI_SBCYCLE_BASE_NS / 1000 )	  /* [us]	*//* <S039> */
#define KPI_SBCYCLE_TIME		2			/* ScanB Cycle Time [125us]		*/
#define KPI_SBCOUNTMS			(1000000/KPI_SBCYCLE_BASE_NS)	/* ScanB Cycle Count per 1 ms */
#define	KPI_SCANB_MS( ms )		( (ms)*KPI_SBCOUNTMS )

/*--------------------------------------------------------------------------*/
/*		ScanCPeriod														 	*/
/*--------------------------------------------------------------------------*/
#define KPI_SCCYCLEMS			2
#define KPI_SCCYCLEUS			( KPI_SCCYCLEMS * 1000  )
#define KPI_SCANC_MS( ms )		( (ms)/KPI_SCCYCLEMS )

/*--------------------------------------------------------------------------------------------------*/
/*		ScanNPeriod														 							*/
/*--------------------------------------------------------------------------------------------------*/
//#define KPI_SNCYCLENS			( 250000 )						/* Interim: ScanN Cycle[ns] 			*/
#define KPI_SNCYCLENS			( KPI_SBCYCLE_BASE_NS )			/* Interim: ScanN Cycle[ns] 			*/
#define KPI_SNCYCLEUS			( KPI_SNCYCLENS/1000 )			/* Interim: ScanN Cycle[us] 			*/
#define KPI_SCANN_MS( ms )		( (ms*1000)/KPI_SNCYCLEUS )		/* Interim: ScanN Cycle Count per 1ms	*/

/*--------------------------------------------------------------------------------------------------*/
/*		MECHATROLINKProcessing timing definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define SCAN_N_DEFCYCLE			1				/* Default communicationTASKPeriod[ScanBCount] */
#define CFG_TIO_BASE_TIME		500				/* Servo control cycle[0.25us]	*/
//#define CFG_POSERR_DELAY		KPI_MBCYCLENS	/* TODO:Review!!! Correction time for position deviation monitor[ns] */
#define CFG_POSERR_DELAY		KPI_SBCYCLE_BASE_NS	/* Correction time for position deviation monitor[ns] *//* <S0C0> */


/*****************************************************************************/
/*		Power related,HWControl functionSWDefinition											 */
/*****************************************************************************/
/*---------------------------------------------------------------------------*/
/*		DMATransfer enable switch													 */
/*---------------------------------------------------------------------------*/
#define CSW_MAICRO_DMA_TRANSFER	FALSE	/* Enable DMA transfer for JL077-uP and CPU */
#define CSW_M3_DMA_TRANSFER		FALSE	/* Enable DMA transfer for JL100 and CPU */

/*---------------------------------------------------------------------------*/
/*		PN Voltage Detect by Decimation Filter(PnE0C.bit13, PnE7C)	 */
/*---------------------------------------------------------------------------*/
/* PnE0C.bit13=1 & ThisSWtoggle(Parameters laterOnly,) */
#define AD_DETECT_DECIMATION		TRUE

#define CSW_HW_POWER_MANAGE			TRUE	/* Power management process execution selection */
#define CSW_HW_REGENERATION_CTRL	TRUE	/* Regenerative processing selection */
#define CSW_HW_CHKRUSH				TRUE	/* Inrush resistance overload detection selection */
#define CSW_HW_DC_DETECT			TRUE	/* DCPower supply voltage detection selection */
#define CSW_HW_INTERNAL_FAN			TRUE	/* Presence of built-in fan */
#define CSW_HW_DCBUS				FALSE	/* DCInput mode selection */


/*---------------------------------------------------------------------------*/
/*		DBControl execution selection														 */
/*---------------------------------------------------------------------------*/
#define CSW_HW_DB_CTRL				TRUE
/*---------------------------------------------------------------------------*/
/*		HWBBControl execution selection													 */
/*---------------------------------------------------------------------------*/
//#define CSW_HW_HWBB_SUPPORT			FALSE                          /* <S09F> */

/*---------------------------------------------------------------------------*/
/*		Multi-axis simultaneous interlock function selection										 */
/*---------------------------------------------------------------------------*/
#ifdef CSW_BSP_SGDZ_IFBS80
#define CSW_SVON_INTERLOCK
#define SVON_LOCK_GRP1				0x00000006		// Axis 1 and 2 on CB2 board
#define SVON_LOCK_GRP2				0x000000f8		// Axis 3 and 8 on CC board
#define SVON_LOCK_GRP1_MAX			1
#define SVON_LOCK_GRP2_MAX			3

#define OH_ALM_SLOT_GRP1			0x00000006		// Axis 1 and 2 on CB2 board
#define OH_ALM_SLOT_GRP2			0x000000f8		// Axis 3 and 8 on CC board

#endif	//CSW_BSP_SGDZ_IFBS80

/*****************************************************************************/
/*		Others																 */
/*****************************************************************************/
/*---------------------------------------------------------------------------*/
/*		Amp and Mot Type Definition											 */
/*---------------------------------------------------------------------------*/
#define MOTTYPE_ROTARY				0x00		/* Rotary type motor */
#define MOTTYPE_LINEAR				0x01		/* Linear motor */

/*---------------------------------------------------------------------------*/
/*		Un Monitor Average time												 */
/*---------------------------------------------------------------------------*/
#define SVCDEF_UNSPD_SCMEANNUM		((64 + KPI_SCCYCLEMS - 1) / KPI_SCCYCLEMS)
#define SVCDEF_UNSPD_SCMEANTMMS		( KPI_SCCYCLEMS * SVCDEF_UNSPD_SCMEANNUM )
/*---------------------------------------------------------------------------*/
/*		Motor Selection Definition											 */
/*---------------------------------------------------------------------------*//* <S04D> */
#define	MOTSEL_YASKAWA		0x00				/* Yaskawa motor(Standard)			 */
#define	MOTSEL_NONYASKAWA	0xF0				/* Other company's motor				 */
/*---------------------------------------------------------------------------*/
/*		Encoder Selection Definition										 */
/*---------------------------------------------------------------------------*//* <S04D> */
#define	ENCSEL_SERIAL		0x00				/* Serial encoder		 */
#define	ENCSEL_PULSE		0x10				/* AB PAL encoder		 */
#define	ENCSEL_ANALOG 		0x20				/* Analog encoder		 */
#define	ENCSEL_ENCLESS		0xFF				/* Encoderless			 */


/*---------------------------------------------------------------------------*/
/*		Normalize Multiplier Factor											 */
/*---------------------------------------------------------------------------*/
#define NORM_TRQ_TO_IQ			3840000		/* TrqRef(2^24) --> IqRef(15000) */
#define NORM_IQ_TO_TRQ			73300775	/* IqRef(15000) --> TrqRef(2^24) */

/* <S056>	Start */
/*---------------------------------------------------------------------------*/
/*		Information Register Definition										 */
/*---------------------------------------------------------------------------*/
#define SERVO_ID			0x070		/* Servo modelID : SFD7S_AP = 0x0070 */
/* <S173> Start */
/* Input/Output signal Enabled/Comment out to review invalid information */
#if 0
#define INPUT_SIGNAL_INFO	0x0000007f	/* Input signal Enabled/Invalid information */
										/* SI0?SI6:Enabled */
										/* SI7?SI31:Disabled */
#define OUTPUT_SIGNAL_INFO	0x0000000E	/* Output signal Enabled/Invalid information */
										/* SO1?SO3:Enabled */
										/* SO0,SO4?SO31:Disabled */
#endif

/* Input/Output signal Enabled/Review invalid information */
#define INPUT_SIGNAL_INFO	0x000000ff	/* Input signal Enabled/Invalid information */
										/* SI0~SI6,SI7(SENSignal):Enabled */
										/* SI8~SI31:Disabled */
#define OUTPUT_SIGNAL_INFO	0x0000007f	/* Output signal Enabled/Invalid information */
										/* ALM,SO1~SO6:Enabled */
										/* SO7~SO31:Disabled */
/* <S173> End */
/* <S056>	End */

#define ALM_WARNING_AUTO_CLEAR	TRUE	/* <S184> Selection of automatic warning clearing */

extern LONG NumberOfAxes;		// Number of control axes

/****************************************************************************************************/
/*																									*/
/*		Panel board definition																				*/
/*																									*/
/****************************************************************************************************/
//#define		SVFSEL_PANELOP	(0)						/* No panel										*/
//#define		CCSW_PANELID	PANELID_NONE			/* No panel										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Panel button switch reading macro															*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPX_READ_PNLBTN( x )		{ ;}
/*--------------------------------------------------------------------------------------------------*/
/*		Station address rotarySWRead macro															*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPX_READ_PNLRTSW( x )		{ ;}
/*--------------------------------------------------------------------------------------------------*/
/*		Communication setting dipSWRead macro																*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPX_READ_PNLDPSW( x )		{ ;}
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Servo Function Using Definition 															*/
/*																									*/
/****************************************************************************************************/
#include "SVFSEL.cfg"


#endif /* _SYSTEM_SETTING_H */

