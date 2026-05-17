/****************************************************************************************************/
/*																									*/
/*																									*/
/*		CheckMotSts.h : Motor status monitoring processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef CHECK_MOT_STS_H
#define CHECK_MOT_STS_H

#include "Bprm.h"
#include "BaseCmnStruct.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Definition for motor rotation detection																		*/
/*--------------------------------------------------------------------------------------------------*/
#define MOTSPD_TGMGN	12710		/* 12710 / 16777216 x 6600 ≒ 5rpm		LpxCheckMotorRun( )Reference	*/
#define MOTSPD_DBLVL	60000		/* 16777216 x 0.7/175 = 67108 -> 60000	LpxCheckMotorStop( )Reference*/

/*--------------------------------------------------------------------------------------------------*/
/*		ScanCMotor status structure definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct SEQ_MOT_STS {
	struct {								/* Motor status constant definition								*/
		LONG			VcmpSpdLevel;			/* Speed match detection width				[2^24/OvrSpd]		*/
		LONG			TgonSpdLevel;			/* Motor rotation detection level			[2^24/OvrSpd]		*/
	} conf;
	struct {								/* Motor status variable definition								*/
//		BOOL			TgOn;					/* TGON Status										*/
//		BOOL			Vcmp;					/* VCMP Status										*/
//		BOOL			MotStop;				/* Motor Stop Status								*/
		LONG			MotSpd;					/* Motor Speed										*/
		LONG			AbsMotSpd;				/* Absolute Motor Speed								*/
		LONG			LstMotSpd;				/* Last Motor Speed for ModeSwAcc Cal.				*/
		LONG			MotPos;					/* Motor Position									*/
		LONG			dMotPos;				/* Delta Motor Position								*/
		LONG			PcmdCntr;				/* Pulse Command Counter	[pulse]					*/
		LONG			dPcmdCnt;				/* Delta Pulse Command		[pulse]					*/

		LONG			UnMotSpd;				/* Un000  : Motor speed				[pulse/64ms]	*/
		LONG			UnPfbk64ms;				/* Un000  : Motor speed				[pulse/64ms]	*/
		LONG			UnPcmd64ms;				/* Un007  : Command pulse speed			[pulse/64ms]	*/
		LONG			Pfbk64SumCnt;			/* For motor speed calculation for monitor							*/
		LONG			Pcmd64Sum;				/* For command pulse speed calculation for monitor						*/
		LONG			Pfbk64Sum;				/* For motor speed calculation for monitor							*/
		LONG			MotSpd64Sum;			/* For motor speed calculation for monitor							*/

		LONG			IsqrSum;				/* For current load calculation for monitor							*/
		LONG			IsqrSumCnt;				/* For current load calculation for monitor							*/
		LONG			UnIsqrSum;				/* Un009  : Cumulative load rate		[(15000/MaxTrq)/10sec]	*/
		ULONG			UnCrntRef;				/* Un142 : Square current command value	[(15000/MAXTRQ)^2]		*/
		ULONG			UnIsqrSumMax;			/* Un145 : Maximum cumulative load rate	[(15000/MaxTrq)/10sec]	*/

		LONG			TrqRefMon;				/* Torque command monitor					[2^24/MaxTrq]	*/

		LONG			UnPcmd64Sum;			/* Command pulse speed <S1B6>			[64ms]			*/
	} var;
} SEQ_MOT_STS;



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void SMotSts_InputMotorStatus( SEQ_MOT_STS *MotSts, BASE_CTRL_OUT *BaseCtrlOut,
                               SEQ_CTRL_OUT *SeqCtrlOut, BPRMDAT *Bprm );
BOOL	LpxCheckMotorStop( LONG AbsMotSpd );
BOOL	LpxCheckMotorVcmp( SEQ_MOT_STS *MotSts, LONG SpdRef, BOOL SpdCtrlMode );
BOOL	LpxCheckMotorRun( SEQ_MOT_STS *MotSts, LONG LstTgonSts );
BOOL    LpxCheckMotorSuspendLevel( SEQ_MOT_STS *MotSts,LONG NorSuspSpd);/*<S201>*/


#endif /* CHECK_MOT_STS_H */
/***************************************** end of file **********************************************/
