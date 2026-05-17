/****************************************************************************
Description	: Serial Encoder Interface
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes	      :
	Name   Date          Description
	------------------------------------------------------------------------
	Y.Oka  2010.02.23    created
	I.M.   2011.02.08    reviewed

****************************************************************************/
#ifndef SERIAL_ENC_IF_H
#define SERIAL_ENC_IF_H


#include "EncData.h"
#include "JL086ServoIpHw.h"
#include "PrmAccessIf.h"
#include "UnitPrmStruct.h"
#include "AlarmManager.h"		/* <S018> */
#include "SvOptCardIf.h"		/* <S04D> */

#define ECHO_DATA_NUM	7
#define	ENCODER_WAIT_TIMEOUT	2000	/* 2000[ms](2[s]) *//* <S223> */

/****************************************************************************************************/
/*		Encoder Error Definition																	*/
/****************************************************************************************************/
typedef enum SENC_ERROR_ {
	SENCERR_SUCCESS		= 0x0000,		/* Return value: Communication normal											*/
	SENCERR_BUSY		= 0x0001,		/* Return value: Processing											*/
	/*----------------------------------------------------------------------------------------------*/
	SENCERR_PGCOM		= 0x0002,		/* Return value:A.C90 : Encoder communication error						*/
	SENCERR_PGECHO		= 0x0004,		/* Return value:A.CB0 : Encoder echo back error				*/
	SENCERR_PRMDATA		= 0x0008,		/* Return value:A.CA0 : Encoder parameter error					*/
	SENCERR_PRMSUM		= 0x0010,		/* Return value:A.820 : Encoder sum check alarm			*/
	SENCERR_MTLMT		= 0x0020,		/* Return value:A.CC0 : Multiturn limit value mismatch				*/
	SENCERR_SCALE		= 0x0040,		/* Return value:A.080 : Scale pitch setting error					*/
	SENCERR_PHASEANGLE	= 0x0080,		/* Return value:A.C22 : Phase information mismatch							*/
	SENCERR_POLESENSOR	= 0x0100,		/* Return value:A.C21 : Pole sensor error							*/
	SENCERR_BRCHNGFAIL	= 0x0200,		/* Return value:A.C93 : Encoder communication system error				*/
	SENCERR_PGCLR		= 0x0400,		/* Return value:A.C80 : Encoder clear error						*//* <S1BE> */
	SENCERR_NOTSUPPORT	= 0x0800,		/* Return value:A.051 : Product not supported alarm					*//* <S1BE> */
	/*----------------------------------------------------------------------------------------------*/
} SENC_ERROR;



 /* <S003> */
/****************************************************************************************************/
/*		Baud Rate Exchange Selection																*/
/****************************************************************************************************/
typedef enum SENC_BRCHNG_SUP_ {
	SENCBR_AUTO_SEL		= 0x0000,		/* Encoder communication speed switching selection							*/
	SENCBR_8MBPS_SEL	= 0x0001,		/* Encoder communication speed8MBpsSelection								*/
	SENCBR_4MBPS_SEL	= 0x0002,		/* Encoder communication speed4MBpsSelection								*/
} SENC_BRCHNG_SUP;
 /* <S003> */
 


/****************************************************************************************************/
/*		Baud Rate Exchange Definition																*/
/****************************************************************************************************/
typedef enum SENC_BRCHNG_SET_ {
	SENCBR_SUPPORT		= 0x0001,		/* Encoder communication speed switching support							*/
	SENCBR_4MBPS_SUP	= 0x0002,		/* Encoder communication speed4MBpsCorrespondence								*/
	SENCBR_8MBPS_SUP	= 0x0004,		/* Encoder communication speed8MBpsCorrespondence								*/
} SENC_BRCHNG_SET;


/****************************************************************************************************/
/*		Expand area Use Definition										<S1A6>						*/
/****************************************************************************************************/
typedef enum SENC_EXAREA_SET_ {
	SENCBR_EXAREA1		= 0x0001,		/* Encoder extension area1Using									*/
} SENC_EXAREA_SET;



/****************************************************************************************************/
/*		DivSet Error Definition																		*/
/****************************************************************************************************/
typedef enum DIVSET_ERROR_ {
	DIVSET_SUCCESS = 0,	/* Return value: Frequency division setting normal						*/
	DIVSET_PRMERR_R,	/* Return value: Parameter setting error(Rotation):A.040	*/
	DIVSET_PRMERR_L,	/* Return value: Parameter setting error(Linear):A.040	*/
	DIVSET_DIVERR,		/* Return value: Divide pulse setting error:A.041			*/
} DIVSET_ERROR;

/* API's for Serial Encoder Operations */
/* for scan service */
BOOL LmxSencScanServiceB( MENCV *MencV, BOOL *LatchClrReq );
BOOL RmxSencScanServiceB( MENCV *MencV, BOOL *LatchClrReq );
LONG FencReadPosition(MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete,	/* <S04D> *//* <S0E1>:AlmManageraddition */
						void *BaseLoops );/*<S1B7>*/
/* for initializing Motor encoder *//* <S04D>:FbkOpt,SencFbOptUseaddition */
SENC_ERROR	SencInitMotorEncoder( MENCV *MencV, MENC_INIT_PRM *MencInitPrm,	ASIC_HW_IF_STR *AsicHwReg,
										OPFBK_HNDL *FbkOpt, USHORT EncBaudrate, BOOL SencFbOptUse );
										
SENC_ERROR	SencCalcBaseParameters( MENCV *MencV, MENC_INIT_PRM *MencInitPrm );
SENC_ERROR SencInitMencVariables(MENCV *MencV, MENCPRM *MencP, MENC_REF_PRM *MencInPrm);
void	SencMakeAbsPosition(MENCV *MencV, USHORT limmlt, DBYTEX bit_dp, BOOL RvsDir);
DIVSET_ERROR SencInitDivPulseIF( ASIC_HW_IF_STR* AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms );
BOOL	SencSetEncoderType( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2, BOOL SencFbOptUse ); /* <S16A> */
/*--------------------------------------------------------------------------------------------------*/
/* Serial encoder PAO phase serial output												<S02D>		*/
/*--------------------------------------------------------------------------------------------------*/
void	SencOutputPaoSerial( MENCV *MencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
							 BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag );
/*--------------------------------------------------------------------------------------------------*/
/* Initial increment pulse output processing															<S02D>		*/
/*--------------------------------------------------------------------------------------------------*/
void	SencOutIniIncPulse( MENCV  *MencV, ASIC_HW_IF_STR  *pAsicHwReg );				/* Rotation		*/
void	LmxOutSencIniIncPulse( MENCV  *MencV, ASIC_HW_IF_STR  *pAsicHwReg, UCHAR Sftx );/* Linear	*//* <S209> */
/*--------------------------------------------------------------------------------------------------*/


/* for initializing Fully-closed encoder */
SENC_ERROR FencInitEncoder( MENCV *MencV, MENC_INIT_PRM *MencInitPrm,	ASIC_HW_IF_STR *AsicHwReg,
										OPFBK_HNDL *FbkOpt, USHORT EncBaudrate, BOOL SencFbOptUse, BOOL rvsDir);/*<S02D>*//* <S04D> */
void FencBasePrmCalc(MENCV *FencV, MENCPRM *MencP, USHORT b_prm2, ULONG fcplsml, BOOL rvsDir);/*<S02D>*/


SENC_ERROR FencInitVariables(MENCV *FencV, ASIC_HW_IF_STR *AsicHwReg,
                             MENCPRM *MencP, USHORT scnvorgoffset, BOOL OrgOffsetEnable,
                             LONG KConvLoadSpd, LONG sfgfail_b_h);
UCHAR FencGetFencAlarm(MENCV *FencV);
/*--------------------------------------------------------------------------------------------------*/
/* Fulque encoder: Divide pulse IF initialization												<S02D>		*/
/*--------------------------------------------------------------------------------------------------*/
DIVSET_ERROR FencSencInitDivPulseIF( ASIC_HW_IF_STR* AsicHwReg, MENCV *FencV, UNI_PRM_STR *UniPrms );
/*--------------------------------------------------------------------------------------------------*/
/* Fulque encoder: Initial incremental pulse output processing											<S02D>		*/
/*--------------------------------------------------------------------------------------------------*/
void  FencSencOutIniIncPulse( MENCV  *FencV, ASIC_HW_IF_STR  *AsicHwReg );
/*--------------------------------------------------------------------------------------------------*/
/* Fulque encoder: Serial encoder PAO phase serial output								<S02D>		*/
/*--------------------------------------------------------------------------------------------------*/
void  FencSencOutputPaoSerial( MENCV *MencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
							   BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag );
/*--------------------------------------------------------------------------------------------------*/

/* common functions(SerialEnc.c) */
void SencInitChannel( MENCV *MencV, UINT ch, ASIC_HW_IF_STR *encAsicHwReg, ASIC_HW_IF_STR *posCmpAsicHwReg );/* <S04D> */
SENC_ERROR SencConnect( MENCV *MencV, ASIC_HW_IF_STR *AsicHwReg, USHORT EncBaudrate,  
										OPFBK_HNDL *FbkOpt, BOOL SencFbOptUse );/* <S04D>:FbkOpt,SencFbOptUseaddition */
SENC_ERROR SencExchngBaudrate( MENCV *MencV, MENC_INIT_PRM *MencInitPrm,
										ASIC_HW_IF_STR *AsicHwReg, UCHAR BaudRate, BOOL *EncExchgSupported);/*<S09C>*/
SENC_ERROR SencReadParamAll( MENCV *MencV, UCHAR *PrmBuf );
SENC_ERROR SencSendRstOrgCmd(MENCV *MencV);
SENC_ERROR SencReadInitialP1Data(MENCV *MencV);
void SencChangeP0Mode( MENCV *MencV );
void SencInitChangeP0Mode( MENCV *MencV );			/* <S0F8> */
void SencChangeP1Mode( MENCV *MencV );
void SencSetIntCycleP0Mode( MENCV *MencV );			/* <S1D3> */
void SencSetIntCycleP1Mode( MENCV *MencV );
BOOL SencReadParam8byte( SENCX *Enc, USHORT Addr, UCHAR *PrmBuf );
BOOL SencBankChange( SENCX *Enc, USHORT BankNum );/* <S0D7> */
void SencSetIDINFO( USHORT addr, UCHAR *buffer, IDINFO *idinfo );
void SencFillNullIDINFO( IDINFO *idinfo );

SENC_ERROR SencChkConnect( SENCX *Enc );/* <S04D> */

SENC_ERROR SencResetMultiturn(MENCV *MencV, ULONG limmlt, ULONG bit_dp_l_h);
SENC_ERROR SencChangeMultiturnLimit(MENCV *MencV, USHORT mtlmt);
SENC_ERROR SencResetScaleZeroPosition( MENCV *MencV );
SENC_ERROR SencWriteMotorPhaseOffset( MENCV *MencV, USHORT PhaseOffset );
SENC_ERROR SencReadMotorParameter(MENCV *MencV, USHORT address, UINT num, USHORT *buffer);
SENC_ERROR SencWriteMotorParameter(MENCV *MencV, USHORT address, UINT num, USHORT *buffer);
#if 0 /*<S02D>*/
PRM_RSLT	RmxSencAdjustMencDivPulse( PRMDATA *Prm, MENCV *MencV );
#else/*<S02D>*/
PRM_RSLT	RmxSencAdjustMencDivPulse( PRMDATA *Prm, MENCV *MencV, PRM_ACCCMD Cmd, LONG Data );
#endif/*<S02D>*/

/* 2012.08.24 Y.Oka Make Mercury Base Software */
void	SencReadPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
											ALMMAN_HANDLE *AlmManager, BOOL BeComplete, void *BaseLoops );/* <S018> *//*<S0C5>*//*<S1B7>*/
/* 2012.08.24 Y.Oka Make Mercury Base Software */
LONG	TimeBasesMotSpdCalc( VFBTIMBASE *Vfbtimbase, MENCV *MencV, LONG dPos, LONG dPosSpd,
																	BOOL VcompSts, BOOL EnableFlag ); /* <S067> */
/* <S1B7> >>>>> */
/*--------------------------------------------------------------------------------------------------*/
/* Motorless test																		<S00X>		*/
/*--------------------------------------------------------------------------------------------------*/
/* --- Semi-closed encoder --- */
/* SencScanService.c */
/* Motorless: Encoder variable initialization processing	*/
void LpxSencMotLessInitVariables( MENCV *MencV, ASIC_HW_IF_STR *AsicHwReg );
/* Motor-less: Reading rotational position data	*/
void RmxReadMotorLessPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
									ALMMAN_HANDLE *AlmManager, BOOL BeComplete,	void *BaseLoops );
/* Motor-less: Reading linear position data	*/
void LmxReadMotorLessPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
									ALMMAN_HANDLE *AlmManager, BOOL BeComplete,	void *BaseLoops );
/* Motorless: Encoder parameter setting processing	*/
void LpxSetSencInfoMotorLessTestMode( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2 );

/* --- Full-closed encoder --- */
/* SencScanService.c */
/* Motorless: Encoder variable initialization processing	*/
void LpxFencMotLessInitVariables( MENCV *FencV, ASIC_HW_IF_STR *AsicHwReg );
/* Motorless: Encoder parameter setting processing	*/
void LpxSetFencInfoMotorLessTestMode( MENCV *FencV, MENCPRM *MencP );
/* Motor-less: Reading position data	*/
LONG LpxReadMotorLessFencPosition( MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete, void *BaseLoops );
/* <<<<< <S1B7> */


#endif /* SERIAL_ENC_IF */
/***************************************** end of file **********************************************/
