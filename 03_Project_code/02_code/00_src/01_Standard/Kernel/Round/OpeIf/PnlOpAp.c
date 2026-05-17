/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PnlOpAp.c : Panel Operator Interface Module (Analog & Pulse)								*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : 												*/
/*																									*/
/*			1)Monitor function : Specified monitor variable(UnXXX)												*/
/*			2)Parameter : Specified parameters(PnXXX),Editing,Change									*/
/*			3)Auxiliary function	 : Specification auxiliary function(FnXXX)and display execution results								*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2002.08.20  M.Suga																*/
/*		Rev.1.10 : 2008.04.24  T.Taniguchi															*/
/*		Rev.1.20 : 2013.08.02  K.Ozaki			##Mercury##											*/
/*		Rev.1.30 : 2013.09.26  K.Ozaki		<S00C>													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"		    /*#include "Round.h"*/
#include "LcdOp.h"			/* LcdOpIf Common Header */
#include "Monitor.h"		/* Un Monitor */
#include "Parameter.h"		/* Parameter */
#include "Function.h"		/* Fn Funstion */
#include "PrmAccessIf.h"	/* Parameter Edit */


/****************************************************************************************************/
/*																									*/
/*		Panel Operator Macro Definition																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* Panel Operator LED Codes																			*/
/*																									*/
/* (digit location)        (7seg location)															*/
/*    bit:0																							*/
/*       --                 _   _   _   _   _ 														*/
/*    5 |  | 1             |_| |_| |_| |_| |_|														*/
/*       -- 6              |_| |_| |_| |_| |_|														*/
/*    4 |  | 2      led[?]: 4   3   2   1   0 														*/
/*       -- * 7																						*/
/*       3  logic			0:ON / 1:OFF															*/
/*--------------------------------------------------------------------------------------------------*/
#define	LEDCODE_B0		0x01
#define	LEDCODE_B1		0x02
#define	LEDCODE_B2		0x04
#define	LEDCODE_B3		0x08
#define	LEDCODE_B4		0x10
#define	LEDCODE_B5		0x20
#define	LEDCODE_B6		0x40
#define	LEDCODE_B7		0x80
/*--------------------------------------------------------------------------------------------------*/
#define	LEDCODE_ZERO	0x3F
#define	LEDCODE_1		0x06
#define	LEDCODE_2		0x5B
#define	LEDCODE_3		0x4F
#define	LEDCODE_4		0x66
#define	LEDCODE_5		0x6D
#define	LEDCODE_6		0x7D
#define	LEDCODE_7		0x27
#define	LEDCODE_8		0x7F
#define	LEDCODE_9		0x6F
#define	LEDCODE_A		0x77
#define	LEDCODE_B		0x7C
#define	LEDCODE_C		0x39
#define	LEDCODE_D		0x5E
#define	LEDCODE_E		0x79
#define	LEDCODE_F		0x71
#define	LEDCODE_G		0x3D
#define	LEDCODE_H		0x76
#define	LEDCODE_sH		0x74
#define	LEDCODE_J		0x0E
#define	LEDCODE_L		0x38
#define	LEDCODE_N		0x54
#define	LEDCODE_sO		0x5C
#define	LEDCODE_P		0x73
#define	LEDCODE_Q		0x67
#define	LEDCODE_R		0x50
#define	LEDCODE_S		0x6D		/* <S1F5> */
#define	LEDCODE_T		0x78
#define	LEDCODE_U		0x3E
#define	LEDCODE_sU		0x1C
#define	LEDCODE_Y		0x6E
/*--------------------------------------------------------------------------------------------------*/
#define	LEDCODE_MINUS	0x40
#define	LEDCODE_POINT	0x80
#define	LEDCODE_BLANK	0x00
/*--------------------------------------------------------------------------------------------------*/
/* Panel Operator Buttons (Already Inverted)														*/
/*--------------------------------------------------------------------------------------------------*/
#define PNLKEY_MODE		0x1
#define PNLKEY_UP		0x2
#define PNLKEY_DOWN		0x4
#define PNLKEY_SHUP		0x8
#define PNLKEY_RST		(PNLKEY_UP | PNLKEY_DOWN)
#define PNLKEY_SWOFF	0x0
/*--------------------------------------------------------------------------------------------------*/
/* Panel Operator Display Modes																		*/
/*--------------------------------------------------------------------------------------------------*/
#define PNLMODE_STATUS		0
#define PNLMODE_FNNUM		1
#define PNLMODE_FNEXE		2
#define PNLMODE_PNNUM		3
#define PNLMODE_PNDATA		4
#define PNLMODE_UNNUM		5
#define PNLMODE_UNDATA		6
/*--------------------------------------------------------------------------------------------------*/
#define	PNLLED_BLINK		0xFF		/* PanelLEDLights off mask(For blinking)								*/
#define	PNLLED_NOBLINK		0x00		/* PanelLEDLighting mask(For blinking)								*/
/*--------------------------------------------------------------------------------------------------*/
#define	PNLMON_NOSET		0xFFF		/* Monitor default at power-on								*/
/*--------------------------------------------------------------------------------------------------*/

/* <S0CB> Start */
/*--------------------------------------------------------------------------------------------------*/
/* Push button/7LED-LEDFor confirmation mode 7LEDDisplay definition												*/
/*--------------------------------------------------------------------------------------------------*/
#define	LED_CHK_CNT_LMIT	15U
#define	PATTERN0			LEDCODE_BLANK																			/* AllLEDLights off		*/
#define	PATTERN1			LEDCODE_B0																				/* Bit0 Lighting		*/
#define	PATTERN2			LEDCODE_B0|LEDCODE_B1																	/* Bit0 - Bit1 Lighting	*/
#define	PATTERN3			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2														/* Bit0 - Bit2 Lighting	*/
#define	PATTERN4			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2|LEDCODE_B3												/* Bit0 - Bit3 Lighting	*/
#define	PATTERN5			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2|LEDCODE_B3|LEDCODE_B4									/* Bit0 - Bit4 Lighting	*/
#define	PATTERN6			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2|LEDCODE_B3|LEDCODE_B4|LEDCODE_B5						/* Bit0 - Bit5 Lighting	*/
#define	PATTERN7			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2|LEDCODE_B3|LEDCODE_B4|LEDCODE_B5|LEDCODE_B6			/* Bit0 - Bit6 Lighting	*/
#define	PATTERN8			LEDCODE_B0|LEDCODE_B1|LEDCODE_B2|LEDCODE_B3|LEDCODE_B4|LEDCODE_B5|LEDCODE_B6|LEDCODE_B7	/* Bit0 - Bit7 Lighting	*/

const UCHAR LedLightPatternTbl[] =
{	/*	0,			1,			2,			3,			*/
		PATTERN1,	PATTERN2,	PATTERN3,	PATTERN4,
	/*	4,			5,			6,			7			*/
		PATTERN5,	PATTERN6,	PATTERN7,	PATTERN8,

	/*	8,			9,			10,			11,			*/
		PATTERN7,	PATTERN6,	PATTERN5,	PATTERN4,
	/*	12,			13,			14,			15			*/
		PATTERN3,	PATTERN2,	PATTERN1,	PATTERN0
};
/* <S0CB> End */

/****************************************************************************************************/
/*																									*/
/*		Panel Operator Constant Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LED Code Table																				*/
/*--------------------------------------------------------------------------------------------------*/
const	UCHAR	ledcodetbl[] =
{
	LEDCODE_ZERO, LEDCODE_1, LEDCODE_2, LEDCODE_3, LEDCODE_4, LEDCODE_5, LEDCODE_6, LEDCODE_7,
	LEDCODE_8,	  LEDCODE_9, LEDCODE_A, LEDCODE_B, LEDCODE_C, LEDCODE_D, LEDCODE_E, LEDCODE_F
};
/*--------------------------------------------------------------------------------------------------*/
/*		LED Convert Table																			*/
/*--------------------------------------------------------------------------------------------------*/
const	UCHAR	ledconvtbl[] =
{
/* '-', */		LEDCODE_MINUS,
/* '.', */		LEDCODE_POINT,
/* '/', */		LEDCODE_BLANK,
/* '0', */		LEDCODE_ZERO ,
/* '1', */		LEDCODE_1    ,
/* '2', */		LEDCODE_2    ,
/* '3', */		LEDCODE_3    ,
/* '4', */		LEDCODE_4    ,
/* '5', */		LEDCODE_5    ,
/* '6', */		LEDCODE_6    ,
/* '7', */		LEDCODE_7    ,
/* '8', */		LEDCODE_8    ,
/* '9', */		LEDCODE_9    ,
/* ':', */		LEDCODE_BLANK,
/* ';', */		LEDCODE_BLANK,
/* '<', */		LEDCODE_BLANK,
/* '=', */		LEDCODE_BLANK,
/* '>', */		LEDCODE_BLANK,
/* '?', */		LEDCODE_BLANK,
/* '@', */		LEDCODE_BLANK,
/* 'A', */		LEDCODE_A    ,
/* 'B', */		LEDCODE_B    ,
/* 'C', */		LEDCODE_C    ,
/* 'D', */		LEDCODE_D    ,
/* 'E', */		LEDCODE_E    ,
/* 'F', */		LEDCODE_F    ,
/* 'G', */		LEDCODE_G    ,
/* 'H', */		LEDCODE_H    ,
/* 'I', */		LEDCODE_1	 ,
/* 'J', */		LEDCODE_J    ,
/* 'K', */		LEDCODE_BLANK,
/* 'L', */		LEDCODE_L    ,
/* 'M', */		LEDCODE_BLANK,
/* 'N', */		LEDCODE_N    ,
/* 'O', */		LEDCODE_ZERO ,
/* 'P', */		LEDCODE_P    ,
/* 'Q', */		LEDCODE_Q    ,
/* 'R', */		LEDCODE_R    ,
/* 'S', */		LEDCODE_5    ,
/* 'T', */		LEDCODE_T    ,
/* 'U', */		LEDCODE_U    ,
/* 'V', */		LEDCODE_BLANK,
/* 'W', */		LEDCODE_BLANK,
/* 'X', */		LEDCODE_BLANK,
/* 'Y', */		LEDCODE_Y    ,
/* 'Z', */		LEDCODE_BLANK
};
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Panel Operator Variable Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Panel Operator Structure Definition															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{
/*--------------------------------------------------------------------------------------------------*/
	USHORT	LastAlarm;		/* Previous alarm value(Used to jump to status display mode when an alarm occurs)	*/
	USHORT	BtnLast;		/* Panel button previous value													*/
	ULONG	KeyTime;		/* For panel button long press detection												*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	DispMode;		/* Display mode															*/
//	LEDDSP	DspData;		/* Display data															*/
	ULONG	BlnkTime;		/* For panel display blinking cycle control												*/
	USHORT	CurPos;			/* Panel Parameter current digit/Monitor current display section							*/
	USHORT	CurPosMax;		/* Panel Parameter current digit/Monitor current display section range						*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	CurFunIndx;		/* Panel auxiliary function current position index									*/
	USHORT	CurPrmIndx;		/* Panel parameter current position index									*/
	USHORT	PrmWrState;		/* Panel parameter writing status										*/
	USHORT	CurMonIndx;		/* Panel monitor current position index									*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	ChkTime;		/* APShape Push button&7SegLedConfirm Elapsed time								*/	/* <S0CB> */
} PNL_STRUC;
/*--------------------------------------------------------------------------------------------------*/
/*		Local Variable Definition																	*/
/*--------------------------------------------------------------------------------------------------*/
PNL_STRUC	Pnl;			/* Panel Operator Variable												*/
/*--------------------------------------------------------------------------------------------------*/



#if SVFSEL_PANELOP_USE == 1											/* Not Use Panel */
/****************************************************************************************************/
/*																									*/
/*		Panel Operator Function List																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Panel operator IF function																	*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxPnlOpMain( HMSGIF Hmsg );		/* Panel Operator Main Program							*/
void	RpxPnlOpInit( void );				/* Panel Operator Initialize							*/
void	RpxPnlOpMonInit( HMSGIF Hmsg );		/* Panel Monitor Initialize								*/
void	RpxPnlOpFinish( HMSGIF Hmsg );		/* Panel Operator Finish								*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedDispMonDecWord(				/* LEDOperator Monitor display(10Hex word with sign/None)		*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			);			/* Signed presence/absence											*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedDispMonDecLong(				/* LEDOperator Monitor display(10Advanced long/None)		*/
		ULONG	ldata,						/* ldata:Value to display										*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	dsp_pos,					/* Current display position											*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			);			/* Sign presence/absence												*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetStatusBits(				/* LEDOperator Status bit display information set processing		*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		HMSGIF	Hmsg );						/* TODO: Erase after										*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetValDec(					/* LEDOperator Display value(10Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit,						/* Number of digits processed												*/
		USHORT	sup_flg,					/* 0Display suppression flag										*/
		USHORT	decimals		);			/* Number of decimal places										*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetValHex(					/* LEDOperator Display value(16Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit			);			/* Number of digits processed												*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetStr(						/* LED Operator String Set								*/
		UCHAR	*str_ptr,					/* Character string address to display								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	length			);			/* Number of display characters											*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetBlnk(						/* LED Operator Blink Set								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		UCHAR	blnkptn			);			/* blinking pattern(1bit=1Character minutes)							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Local Function Prototype Definition															*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlKeyTranslate( HMSGIF Hmsg );		/* Panel operator Key conversion processing						*/
void	PnlStatusDispMode( HMSGIF Hmsg );	/* Panel operator State display processing						*/
void	PnlFunNoMode( HMSGIF Hmsg );		/* Panel operator FnNumber editing mode					*/
void	PnlFunExeMode( HMSGIF Hmsg );		/* Panel operator FnExecution mode						*/
void	PnlPrmNoMode( HMSGIF Hmsg );		/* Panel operator PnNumber editing mode					*/
void	PnlPrmDataMode( HMSGIF Hmsg );		/* Panel operator PnData display/Editing mode				*/
void	PnlMonNoMode( HMSGIF Hmsg );		/* Panel operator UnNumber editing mode					*/
void	PnlMonDataMode( HMSGIF Hmsg );		/* Panel operator UnData display mode					*/
void	PnlLedBlink( HMSGIF Hmsg );			/* Panel operator LEDBlink process						*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispPrmDecWordUnsigned(		/* Panel operator LEDDisplay(Parameter unsigned10Hex word)	*/
		USHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals,					/* Number of decimal places										*/
		LEDDSP	*PnlDsp );					/*														*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispPrmDecWordSigned(			/* Panel operator LEDDisplay(Parameter signed10Hex word)	*/
		SHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals, 					/* Number of decimal places										*/
		SHORT	llimit,						/* Lower limit value												*/
		SHORT	ulimit,						/* Upper limit												*/
		LEDDSP	*PnlDsp );					/*														*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispPrmHexNblWord(			/* Panel operator LEDDisplay(Parameter16Hex word)		*/
		USHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	base,						/* Bottom of display												*/
		LEDDSP	*PnlDsp );					/* @@													*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispPrmDecLong(				/* Panel operator LEDDisplay(10Advanced long)					*/
		ULONG	ldata,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign,						/* Sign presence/absence												*/
		LEDDSP	*PnlDsp );					/* @@													*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispPrmHexLong(				/* Panel operator LEDDisplay(16Advanced long)					*/
		ULONG	ldata,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		LEDDSP	*PnlDsp );					/* @@													*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispMonHexWord(				/* Panel operator LEDDisplay(Monitor16Hex word)			*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*DspData );					/* @@ Panel Display Data								*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedDispMonBit(					/* Panel operator LEDDisplay(Monitor bit byte)		*/
		UCHAR	data,						/* Value to display											*/
		LEDDSP	*leddsp			);			/*														*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedSetKnlBuffer(					/* Panel LED Set to Kernel Variables					*/
		LEDDSP	*leddsp,					/*														*/
		UCHAR	*dist,						/*														*/
		LONG	size			);			/*														*/
/*--------------------------------------------------------------------------------------------------*/
void	PnlLedSetBlnkSw(					/* Panel LED Set to Same Data							*/
		LEDDSP	*leddsp,					/*														*/
		UCHAR	data,						/*														*/
		LONG	size,						/*														*/
		LONG	flag			);			/*														*/
/*--------------------------------------------------------------------------------------------------*/
void	PbAndLedChkTest( HMSGIF Hmsg );		/* Panel operator Push button & 7SegLed Confirmation test	*//* <S0CB> */
/*--------------------------------------------------------------------------------------------------*/




/****************************************************************************************************/
/*																									*/
/*		Panel Operator Main Program																	*/
/*																									*/
/****************************************************************************************************/
void	RpxPnlOpMain( HMSGIF Hmsg )
{
#define MSGIF_SEND( resbuf, reslen )   ( Hmsg->Sif->StxRes( Hmsg->Sif, (resbuf), (reslen) ) )
USHORT	CurAlarm;
//LONG	rc_tmp;
AXIS_HANDLE	*pAxRsc = Hmsg->pAxRsc;		/* TODO: Delete it later 										*/
LEDDSP *PnlDsp = (LEDDSP *)Hmsg->ResBuf;

/* <S0CB> Start */
	/*--------------------------------------------------------------------------------------------------*/
	/*		Push button & 7SegLed Confirmation test															*/
	/*--------------------------------------------------------------------------------------------------*/
	if( pAxRsc->FnCmnCtrl->PbAndLedChkTestFlag )
	{
		/* Push button & 7SegLed Confirmation test Start */
		PbAndLedChkTest( Hmsg );
		PnlLedSetKnlBuffer( PnlDsp, PnlDsp->LedCode, PNL_N_LED );
		MSGIF_SEND( Hmsg->ResBuf, sizeof(PNLRSPMSG) );
		return;
	}
/* <S0CB> End */

/*--------------------------------------------------------------------------------------------------*/
/*		When serial communication is not connected																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( Hmsg->ComMode == SCM_NOCOM )
		{
			CurAlarm = KpiGetAlarmInfo();
		/*------------------------------------------------------------------------------------------*/
		/*	When an alarm occurs, go to status display mode														*/
		/*------------------------------------------------------------------------------------------*/
			if( (CurAlarm != 0) && ((CurAlarm & 0xF00) != 0x900) && (Pnl.LastAlarm == 0) )
			{
				Pnl.CurPos = 0;
				if(Pnl.DispMode == PNLMODE_FNEXE)
				{
					CPNLFUN->Finish( CPNLFUN, Hmsg ); /* RpxFunFinishPnl(Pnl.CurFunIndx, &PopMsg); */
				}
				if(Pnl.DispMode == PNLMODE_PNDATA)
				{
					/* TODO: EEPROM */
					//rc_tmp = RpxPrmUpdateEepromIdx(Pnl.CurPrmIndx);
					//while(rc_tmp == PRM_EEPROM_WRITE_WAIT || rc_tmp == PRM_EEPROM_WRITE_FAIL)
					//{
					//	rc_tmp = RpxPrmUpdateEepromIdx(Pnl.CurPrmIndx);
					//}
				}
				Pnl.DispMode = PNLMODE_STATUS;
			}
			Pnl.LastAlarm = CurAlarm;
		/*------------------------------------------------------------------------------------------*/
		/*																							*/
		/*------------------------------------------------------------------------------------------*/
			PnlKeyTranslate( Hmsg );
			switch( Pnl.DispMode )
			{
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_STATUS:	/* State display	*/
		/*------------------------------------------------------------------------------------------*/
				if(OpKeyMk.mode)
				{
					Pnl.DispMode = PNLMODE_FNNUM;
					Pnl.CurPos = 0;
				}
				else
				{
					PnlStatusDispMode( Hmsg );
					if(OpKeyMk.rst)
					{
						KriResetSystemAlarm();
//TODO:						NdiClearAlarm();
					}
				}
				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_FNNUM:		/* FnNumber	*/
		/*------------------------------------------------------------------------------------------*/
				if(OpKeyMk.mode)
				{
					Pnl.DispMode = PNLMODE_PNNUM;
					Pnl.CurPos = 0;
				}
				else if(OpKeyMk.data)
				{
					if( CPNLFUN->Begin( 
							CPNLFUN, 
							Pnl.CurFunIndx, 
							(UINT *)&OpKeyMk,
							(UINT *)&OpKeyBrk,
							(UINT *)&OpKeyLvl,
							(UINT *)&OpBlnkKickFlg,
							(UCHAR *)&(Hmsg->ResBuf[PNLRSP_LEDCODE_IDX]),
							Hmsg ) != OK  )				
					{														/* FnWhen execution is not possible	*/
						OpBlnkKickFlg.no_op = 1;
					}
					else
					{
						Pnl.DispMode = PNLMODE_FNEXE;
						Pnl.CurPos = 0;
					}
				}
				else
				{
					PnlFunNoMode( Hmsg );
				}
				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_FNEXE:		/* FnExecution	*/
		/*------------------------------------------------------------------------------------------*/
				if ( CPNLFUN->Exec( CPNLFUN, Hmsg ) )
				{
					Pnl.CurPos = 0;
					Pnl.DispMode = PNLMODE_FNNUM;
				}

/* <S14F> */
//				KpiRstLongTimer( &(Hmsg->ComChkTime) );				/* Reset ComChkTimer		*/	/* <S142> */

				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_PNNUM:		/* PnNumber	*/
		/*------------------------------------------------------------------------------------------*/
				if(OpKeyMk.mode)
				{
					Pnl.DispMode = PNLMODE_UNNUM;
					Pnl.CurPos = 0;
				}
				else if(OpKeyMk.data)
				{
					if( CHK_WRITEINHIBIT() && (Hmsg->AccessLvl != ACCLVL_SYSTEM) )
					{
						OpBlnkKickFlg.no_op = 1;
					}
					else
					{
						Pnl.DispMode = PNLMODE_PNDATA;
						Pnl.PrmWrState = 0;
						Pnl.CurPos = 0;
					}
				}
				else
				{
					PnlPrmNoMode( Hmsg );
				}
				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_PNDATA:	/* PnData	*/
		/*------------------------------------------------------------------------------------------*/
				PnlPrmDataMode( Hmsg );
				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_UNNUM:		/* UnNumber	*/
		/*------------------------------------------------------------------------------------------*/
				if(OpKeyMk.mode)
				{
					Pnl.DispMode = PNLMODE_STATUS;
				}
				else if(OpKeyMk.data)
				{
					Pnl.DispMode = PNLMODE_UNDATA;
					Pnl.CurPos = 0;
				}
				else {	PnlMonNoMode( Hmsg ); }
				break;
		/*------------------------------------------------------------------------------------------*/
			case PNLMODE_UNDATA:	/* UnData	*/
		/*------------------------------------------------------------------------------------------*/
				PnlMonDataMode( Hmsg );
				break;
		/*------------------------------------------------------------------------------------------*/
			default:
		/*------------------------------------------------------------------------------------------*/
				break;
			}
			PnlLedBlink( Hmsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		When serial communication is connected																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			PnlStatusDispMode( Hmsg );
			PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK,
							 PNL_N_LED, ((KpiGetLongTimer(&Pnl.BlnkTime)/100) & 0x02) );
		}
/*--------------------------------------------------------------------------------------------------*/
		PnlLedSetKnlBuffer( PnlDsp, PnlDsp->LedCode, PNL_N_LED );
		MSGIF_SEND( Hmsg->ResBuf, sizeof(PNLRSPMSG) );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator Key conversion processing																*/
/*																									*/
/****************************************************************************************************/
void	PnlKeyTranslate( HMSGIF Hmsg )
{
PNLCMDMSG *PnlCmd = (PNLCMDMSG *)Hmsg->CmdBuf;
UINT	PnlBtn = PnlCmd->PnlBtn;								/* Get Panel Button */
/*--------------------------------------------------------------------------------------------------*/
/*		Initializing variables																					*/
/*--------------------------------------------------------------------------------------------------*/
		*((UINT *)&OpKeyMk)  = 	*((UINT *)&OpKeyBrk) = *((UINT *)&OpKeyLvl) = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Ignore key input while blinking																*/
/*--------------------------------------------------------------------------------------------------*/
		if( *((UINT *)&OpBlnkFlg) != 0 )
		{
			Pnl.BtnLast = PnlBtn;
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Key input level processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( PnlBtn & PNLKEY_MODE	){ OpKeyLvl.mode = TRUE;}
		if( PnlBtn & PNLKEY_UP		){ OpKeyLvl.up   = TRUE;}
		if( PnlBtn & PNLKEY_DOWN	){ OpKeyLvl.down = TRUE;}
		if( PnlBtn & PNLKEY_SHUP	){ OpKeyLvl.shup = TRUE;}
		if( (PnlBtn & PNLKEY_RST) == PNLKEY_RST ){ OpKeyLvl.rst = TRUE;}
/*--------------------------------------------------------------------------------------------------*/
/*		Set only when there is a change in key input(=Chattering prevention)										*/
/*--------------------------------------------------------------------------------------------------*/
		if( PnlBtn != Pnl.BtnLast )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Key inputMAKE																		*/
		/*------------------------------------------------------------------------------------------*/
			switch( PnlBtn )
			{
			case PNLKEY_MODE:	OpKeyMk.mode = TRUE; break;
			case PNLKEY_UP:		OpKeyMk.up   = TRUE; KpiRstLongTimer(&Pnl.KeyTime); break;
			case PNLKEY_DOWN:	OpKeyMk.down = TRUE; KpiRstLongTimer(&Pnl.KeyTime); break;
			case PNLKEY_SHUP:	OpKeyMk.shup = TRUE; KpiRstLongTimer(&Pnl.KeyTime); break;
			case PNLKEY_RST:	OpKeyMk.rst  = TRUE; break;
			default: break;
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Key inputBREAK																		*/
		/*------------------------------------------------------------------------------------------*/
			switch(Pnl.BtnLast)
			{
			case PNLKEY_MODE:	OpKeyBrk.mode = TRUE; break;
			case PNLKEY_UP:		OpKeyBrk.up   = TRUE; break;
			case PNLKEY_DOWN:	OpKeyBrk.down = TRUE; break;
			case PNLKEY_SHUP:	OpKeyBrk.shup = TRUE; break;
			case PNLKEY_RST:	OpKeyBrk.rst  = TRUE; break;
			default: break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		When there is no change in key input: Repeat, Hold processing											*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			switch(PnlBtn)
			{
			case PNLKEY_UP:
				if(KpiGetLongTimer(&Pnl.KeyTime) >= 400)
				{	/* 400msdo nothing less than	*/
					if(!((KpiGetLongTimer(&Pnl.KeyTime)/100) & 0x01))
					{	/* Elapsed time 4xx,6xx,8xx...	*/
						OpKeyMk.up = TRUE;
					}
				}
				break;
			case PNLKEY_DOWN:
				if(KpiGetLongTimer(&Pnl.KeyTime) >= 400)
				{	/* 400msdo nothing less than	*/
					if(!((KpiGetLongTimer(&Pnl.KeyTime)/100) & 0x01))
					{	/* Elapsed time 4xx,6xx,8xx...	*/
						OpKeyMk.down = TRUE;
					}
				}
				break;
			case PNLKEY_SHUP:
				if(KpiGetLongTimer(&Pnl.KeyTime) >= 1000)
				{	/* 1s [DATA]Equivalent to key input	*/
					OpKeyMk.data = TRUE;
					KpiRstLongTimer(&Pnl.KeyTime);
				}
				break;
			default:
				break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
	Pnl.BtnLast = PnlBtn;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator State display processing																*/
/*																									*/
/****************************************************************************************************/
void	PnlStatusDispMode( HMSGIF Hmsg )
{
USHORT	CurAlarm;
AXIS_HANDLE	*pAxRsc = Hmsg->pAxRsc;		/* TODO: Delete it later 										*/
LEDDSP *PnlDsp = (LEDDSP *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		State display																					*/
/*--------------------------------------------------------------------------------------------------*/
		PnlDsp->LedCode[4] = LEDCODE_B7;
		PnlDsp->LedCode[3] = LEDCODE_BLANK;
		RpxLedSetStatusBits( PnlDsp, Hmsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm occurrence																				*/
/*--------------------------------------------------------------------------------------------------*/
		CurAlarm = KpiGetAlarmInfo();
		if( CHK_ALARM() /* Alm.f.AlmFlag */ )
		{
			/* Alarm code display	*/
			RpxLedSetValHex(CurAlarm, PnlDsp, 2, 3);
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Warning is occurring																			*/
/*--------------------------------------------------------------------------------------------------*/
		else if( CurAlarm != 0 )
		{
			/* Warning code display	*/
			RpxLedSetValHex(CurAlarm, PnlDsp, 2, 3);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Normal display(No alarms and warnings)															*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( CHK_SVON() )
			{
				if( CHK_PDET() )
				{
					PnlDsp->LedCode[3] = LEDCODE_P;	/* " P-dt"	*/
					PnlDsp->LedCode[2] = LEDCODE_B6;
					PnlDsp->LedCode[1] = LEDCODE_D;
					PnlDsp->LedCode[0] = LEDCODE_T;
				}
				else
				{
					PnlDsp->LedCode[2] = LEDCODE_R;	/* "  run"	*/
					PnlDsp->LedCode[1] = LEDCODE_sU;
					PnlDsp->LedCode[0] = LEDCODE_N;
				}
			}
			else
			{
				PnlDsp->LedCode[2] = LEDCODE_BLANK;	/* "   bb"	*/
				PnlDsp->LedCode[1] = LEDCODE_B;
				PnlDsp->LedCode[0] = LEDCODE_B;
			}
		/*------------------------------------------------------------------------------------------*/
			if( (!CHK_JOGMODE()) && (!CHK_ZRETMODE()) )
			{
				if( CHK_POT() && CHK_NOT() )	/* "Pot" AND "not" */
				{
					if(!((KpiGetLongTimer(&Pnl.BlnkTime)/100) & 0x04))	/* Period800ms	*/
					{
						PnlDsp->LedCode[2] = LEDCODE_P;
						PnlDsp->LedCode[1] = LEDCODE_sO;
						PnlDsp->LedCode[0] = LEDCODE_T;
					}
					else
					{
						PnlDsp->LedCode[2] = LEDCODE_N;
						PnlDsp->LedCode[1] = LEDCODE_sO;
						PnlDsp->LedCode[0] = LEDCODE_T;
					}
				}
				else if( CHK_POT() /* CinV.f.Sv.Pot */)				/* "Pot" */
				{
					PnlDsp->LedCode[2] = LEDCODE_P;
					PnlDsp->LedCode[1] = LEDCODE_sO;
					PnlDsp->LedCode[0] = LEDCODE_T;
				}
				else if( CHK_NOT() /* CinV.f.Sv.Not */)				/* "Not" */
				{
					PnlDsp->LedCode[2] = LEDCODE_N;
					PnlDsp->LedCode[1] = LEDCODE_sO;
					PnlDsp->LedCode[0] = LEDCODE_T;
				}
			}
		/* <S1F5> Start */
		/*------------------------------------------------------------------------------------------*/
			if ( CHK_FSTP() )										/* "FSTP" */
			{
				PnlDsp->LedCode[2] = LEDCODE_F;
				PnlDsp->LedCode[1] = LEDCODE_S;
				PnlDsp->LedCode[0] = LEDCODE_T;
			}
		/* <S1F5> End */
		/*------------------------------------------------------------------------------------------*/
			if( CHK_HBB() )											/* "Hbb" */
			{
				PnlDsp->LedCode[3] = LEDCODE_BLANK; /* Note: During magnetic pole detectionOTteethALMTherefore, it is unnecessary*/
				PnlDsp->LedCode[2] = LEDCODE_H;
				PnlDsp->LedCode[1] = LEDCODE_B;
				PnlDsp->LedCode[0] = LEDCODE_B;
			}
/* <S1B7> >>>>> */
		/*------------------------------------------------------------------------------------------*/
			if( CHK_MOTOR_LESS_TEST() )								/* "tSt" */
			{
				if(!((KpiGetLongTimer(&Pnl.BlnkTime)/100) & 0x08))	/* Period1600ms	*/
				{
					PnlDsp->LedCode[2] = LEDCODE_T;
					PnlDsp->LedCode[1] = LEDCODE_5;
					PnlDsp->LedCode[0] = LEDCODE_T;
				}
			}
/* <<<<< <S1B7> */
		/*------------------------------------------------------------------------------------------*/
			PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator FnNumber editing mode															*/
/*																									*/
/****************************************************************************************************/
void	PnlFunNoMode( HMSGIF Hmsg )
{
USHORT	i;
UINT	SrchNo;
CFUNTBL	*pTbl = GET_FUNTBL( CPNLFUN );									/* Get Function Table */
UINT	FnNo = CPNLFUN->GetNumber( CPNLFUN, Pnl.CurFunIndx, Hmsg );		/* Get Function Number */
LEDDSP	*PnlDsp = (LEDDSP *)(Hmsg->ResBuf);								/* 					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Key input processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpKeyMk.shup )
		{
			Pnl.CurPos = ((Pnl.CurPos+1) == 3)? 0: Pnl.CurPos+1;
		}
		else if( OpKeyMk.up )
		{
			SrchNo = FnNo + (0x01 << (Pnl.CurPos*4));
			if(SrchNo >= 0x1000){ SrchNo = 0xFFF; }
			Pnl.CurFunIndx = CPNLFUN->SearchB ( CPNLFUN, SrchNo, Pnl.CurFunIndx, Hmsg ); 
		}
		else if( OpKeyMk.down )
		{
			if( FnNo < (0x01 << (Pnl.CurPos*4)) )
			{
				SrchNo = 0x000;
			}
			else
			{
				SrchNo = FnNo - (0x01 << (Pnl.CurPos*4));
			}
			Pnl.CurFunIndx = CPNLFUN->SearchB ( CPNLFUN, SrchNo, Pnl.CurFunIndx, Hmsg ); 
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		PnlDsp->LedCode[4] = LEDCODE_F;
		PnlDsp->LedCode[3] = LEDCODE_N;
		RpxLedSetValHex( CPNLFUN->GetNumber( CPNLFUN, Pnl.CurFunIndx, Hmsg ), PnlDsp, 2, 3);
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < PNL_N_LED; i++ )
		{
			PnlDsp->LedBlnkSw[i] = (i == Pnl.CurPos)?	0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator FnExecution mode																*/
/*																									*/
/****************************************************************************************************/
#if 0
void	PnlFunExeMode( HMSGIF Hmsg )
{
	RpxFunExePnl( OpKeyMk, OpKeyBrk, OpKeyLvl, Pnl.CurFunIndx,
							&(Pnl.DspData), &OpBlnkKickFlg, &PopMsg ); 
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Panel operator PnNumber editing mode															*/
/*																									*/
/****************************************************************************************************/
void	PnlPrmNoMode( HMSGIF Hmsg )
{
UINT	i;
UINT	SrchNo;
CPRMDEF	*prmdef_tmp;
CPRMTBL *pTbl = GET_PNPRMTBL( CPARAMX );
UINT	PnNo = CPARAMX->GetNumber( CPARAMX, Pnl.CurPrmIndx, Hmsg );
LEDDSP *PnlDsp = (LEDDSP *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Key input processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpKeyMk.shup )
		{
			Pnl.CurPos = ((Pnl.CurPos+1) == 3)? 0: Pnl.CurPos+1;
		}
		else if( OpKeyMk.up )
		{
			SrchNo = PnNo + (0x01 << (Pnl.CurPos*4));
			if(SrchNo >= 0x1000) {	SrchNo = 0xFFF;  }
			Pnl.CurPrmIndx = CPARAMX->SearchB( (COBJECT *)CPARAMX, SrchNo, Pnl.CurPrmIndx, Hmsg ); 
		}
		else if( OpKeyMk.down )
		{
			if(PnNo < (0x01 << (Pnl.CurPos*4)))
			{
				SrchNo = 0x000;
			}
			else
			{
				SrchNo = PnNo - (0x01 << (Pnl.CurPos*4));
			}
			Pnl.CurPrmIndx = CPARAMX->SearchB( (COBJECT *)CPARAMX, SrchNo, Pnl.CurPrmIndx, Hmsg); 
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Current digit maximum value																				*/
/*--------------------------------------------------------------------------------------------------*/
		prmdef_tmp = GET_PNPRMDEF( pTbl, Pnl.CurPrmIndx );						/* Parameter definition	*/
		if( CPARAMX->isLong( CPARAMX, Pnl.CurPrmIndx, Hmsg ) )					/* Long parameter	*/
		{
			Pnl.CurPosMax = (GET_PNPRMBASE( prmdef_tmp) == DSPDEF_BASEDEC)? 10: 8;
		}
		else																	/* Word parameter	*/
		{
			if( GET_PNPRMBASE( prmdef_tmp ) == DSPDEF_BASEDEC )
			{
				if( CHK_PNPRM_SIGNED( prmdef_tmp )  &&
					( (-10000 < GET_PNPRM_LOWERLMT( prmdef_tmp ) ) && 
					  ( GET_PNPRM_UPPERLMT( prmdef_tmp ) < 10000) )
				)
				{
					Pnl.CurPosMax = 4;									/* Sign included5digits are sufficient	*/
				}
				else
				{
					Pnl.CurPosMax = 5;
				}
			}
			else
			{
				Pnl.CurPosMax = 4;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		LED display processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		PnlDsp->LedCode[4] = LEDCODE_P;
		PnlDsp->LedCode[3] = LEDCODE_N;
		RpxLedSetValHex( CPARAMX->GetNumber( CPARAMX, Pnl.CurPrmIndx, Hmsg ), 
						PnlDsp, 2, 3);
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < sizeof(PNL_N_LED); i++ )
		{
			PnlDsp->LedBlnkSw[i] = (i == Pnl.CurPos)?	0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator PnData display/Editing mode													*/
/*																									*/
/****************************************************************************************************/
void	PnlPrmDataMode( HMSGIF Hmsg )
{
ULONG	ldata_tmp;
LONG	rc;
CPRMTBL *pTbl = GET_PNPRMTBL( CPARAMX );								/* Get Parameter Table */
CPRMDEF	*prmdef_tmp = GET_PNPRMDEF( pTbl, Pnl.CurPrmIndx );				/* Get Parameter Definition	*/
LEDDSP 	*PnlDsp = (LEDDSP *)(Hmsg->ResBuf);								/* @@ 				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Panel operator PnData display/Editing process														*/
/*--------------------------------------------------------------------------------------------------*/
		switch( Pnl.PrmWrState )
		{
	  /*--------------------------------------------------------------------------------------------*/
		case OPWM_None:	/* Parameter value display/edit processing	*/
	  /*--------------------------------------------------------------------------------------------*/
			CPARAMX->Read( (COBJECT *)CPARAMX,   /* <S073> */
						Pnl.CurPrmIndx, 
						&ldata_tmp, 
						PCHK_OPE_READ, 
						Hmsg );
			if( OpKeyMk.data | OpKeyMk.mode )							/* EEPROMWriting process		*/
			{
				rc = PrmWriteValueToEeprom( Hmsg->pAxRsc->RegManager->hPnReg,						  /* <S098> */
											PnPrmTbl[Pnl.CurPrmIndx].Attribute,
											0 );
				switch( rc )
				{
				case PRM_RSLT_SUCCESS:									/* Normal	*/
					OpBlnkKickFlg.PrmEewrite = 1;						/* Blink start	*/
					break;
				/* Current statusQueueis not available, so comment out the following Y.Oka */
//				case PRM_EEPROM_WRITE_FAIL:								/* untilQueueis not vacant	*/
//					Pnl.PrmWrState = OPWM_UpdtWait;
//					break;
				case PRM_RSLT_RAMONLY:			/* RAMonly parameter								*/
				case PRM_RSLT_NO_DIFF:			/* No need to write(EEPROMThe value is the same as the written value)	*/
					Pnl.CurPos = 0;
					Pnl.DispMode = PNLMODE_PNNUM;						/* Go to parameter number mode	*/
					return;
				default:
					break;
				}

			}
			else if( OpKeyMk.shup )
			{
				Pnl.CurPos = ((Pnl.CurPos+1) == Pnl.CurPosMax)?
					0: Pnl.CurPos+1;
			}
			else if( OpKeyMk.up | OpKeyMk.down )
			{
				/*
				 *
				 * 	RpxPrmRamDigitInc() + RpxPrmWriteRamIdx() = DigitInc()
				 * 	RpxPrmRamDigitDec() + RpxPrmWriteRamIdx() = DigitDec()
				 *
				 */
				if( OpKeyMk.up )
				{
					rc = CPARAMX->DigitInc( CPARAMX, Pnl.CurPrmIndx, Pnl.CurPos, Hmsg );
				}
				else
				{
					rc = CPARAMX->DigitDec( CPARAMX, Pnl.CurPrmIndx, Pnl.CurPos, Hmsg );
				}
				/*----------------------------------------------------------------------------------*/
				if( rc == PERR_CALC )
				{
					OpBlnkKickFlg.error = 1;
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Display processing																				*/
		/*------------------------------------------------------------------------------------------*/
			if( CPARAMX->isLong( CPARAMX, Pnl.CurPrmIndx, Hmsg) )			/* Long parameter	*/
			{
				if(GET_PNPRMBASE( prmdef_tmp ) == DSPDEF_BASEDEC)		/* Comment out because the base detects10Base	*/
				{
					PnlLedDispPrmDecLong(ldata_tmp, Pnl.CurPos, 
						/*ET_PNPRM_DECIMALS( prmdef_tmp ), */
						GET_PNPRM_DECIMALS( CPARAMX, prmdef_tmp, Hmsg ), 	/* <S14F> */
						CHK_PNPRM_SIGNED( prmdef_tmp ),
						PnlDsp );
				}
				else													/* Comment out because the base detects10Non-hexadecimal	*/
				{
					PnlLedDispPrmHexLong(ldata_tmp, Pnl.CurPos, PnlDsp );
				}
				
			}
			else														/* Word parameter	*/
			{
				if( GET_PNPRMBASE( prmdef_tmp ) == DSPDEF_BASEDEC)		/* Comment out because the base detects10Base	*/
				{
					if( CHK_PNPRM_SIGNED( prmdef_tmp ) )
					{
						PnlLedDispPrmDecWordSigned((SHORT)ldata_tmp, Pnl.CurPos, 
							/*GET_PNPRM_DECIMALS( prmdef_tmp ),*/
	 						GET_PNPRM_DECIMALS( CPARAMX, prmdef_tmp, Hmsg ), 	/* <S14F> */
							GET_PNPRM_LOWERLMT( prmdef_tmp ), 
							GET_PNPRM_UPPERLMT( prmdef_tmp ),
							PnlDsp );
					}
					else
					{
						PnlLedDispPrmDecWordUnsigned((USHORT)ldata_tmp, Pnl.CurPos, 
							/*GET_PNPRM_DECIMALS( prmdef_tmp ),*/
	 						GET_PNPRM_DECIMALS( CPARAMX, prmdef_tmp, Hmsg ), 	/* <S14F> */
							PnlDsp );
					}
				}
				else													/* Comment out because the base detects10Non-hexadecimal	*/
				{
					PnlLedDispPrmHexNblWord((USHORT)ldata_tmp, Pnl.CurPos, 
						GET_PNPRMBASE( prmdef_tmp ),
						PnlDsp );
				}
			}
			break;
	  /*--------------------------------------------------------------------------------------------*/
		case OPWM_UpdtWait:											/* EEPROMWrite(Request registration)Processing	*/
	  /*--------------------------------------------------------------------------------------------*/
			switch( PRM_EEPROM_RAM_NO_DIFF /* TODO: RpxPrmUpdateEepromIdx(Pnl.CurPrmIndx) */)
			{
			case OK:												/* Normal	*/
				OpBlnkKickFlg.PrmEewrite = 1;						/* Blink start	*/
				Pnl.PrmWrState = OPWM_None;
				break;
			case PRM_EEPROM_RAM_NO_DIFF:	/* No need to write(EEPROMThe value is the same as the written value)	*/
				Pnl.PrmWrState = OPWM_None;
				Pnl.CurPos = 0;
				Pnl.DispMode = PNLMODE_PNNUM;						/* Go to parameter number mode	*/
				return;
//unreachable				break;
			default:
				break;
			}
			break;
	  /*--------------------------------------------------------------------------------------------*/
		default:
	  /*--------------------------------------------------------------------------------------------*/
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator UnNumber editing mode															*/
/*																									*/
/****************************************************************************************************/
void	PnlMonNoMode( HMSGIF Hmsg )
{
UINT	i; 
UINT	SrchNo;
CMONDEF	*mondef_tmp;
CMONTBL *pTbl = GET_UNMONTBL( CUNMONX );
UINT	UnNo = CUNMONX->GetNumber( CUNMONX, Pnl.CurMonIndx, Hmsg );
LEDDSP *PnlDsp = (LEDDSP *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Key input processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if(OpKeyMk.shup)
		{
			Pnl.CurPos = ((Pnl.CurPos+1) == 3)? 0: Pnl.CurPos+1;
		}
		else if(OpKeyMk.up)
		{
			SrchNo = UnNo + (0x01 << (Pnl.CurPos*4));
			Pnl.CurMonIndx = CUNMONX->SearchB( (COBJECT *)CUNMONX, SrchNo, Pnl.CurMonIndx, Hmsg );
		}
		else if(OpKeyMk.down)
		{
			if(UnNo < (0x01 << (Pnl.CurPos*4)))
			{
				SrchNo = 0x000;
			}
			else
			{
				SrchNo = UnNo - (0x01 << (Pnl.CurPos*4));
			}
			Pnl.CurMonIndx = CUNMONX->SearchB( (COBJECT *)CUNMONX, SrchNo, Pnl.CurMonIndx, Hmsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Current digit maximum value																				*/
/*--------------------------------------------------------------------------------------------------*/
		mondef_tmp = GET_UNMONDEF( pTbl, Pnl.CurMonIndx );				/* Get monitor definition */
		UnNo = CUNMONX->GetNumber( CUNMONX, Pnl.CurMonIndx, Hmsg );		/* Get monitor number */
		if(  CUNMONX->isLong( CUNMONX, Pnl.CurMonIndx, Hmsg ) )			/* Size is long	*/
		{
			Pnl.CurPosMax = 3;											/* Display switching available(Lower/Middle/Upper)	*/
		}
		else															/* Size is less than or equal to word	*/
		{
			Pnl.CurPosMax = 1;											/* Display switching not available	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		LED display processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		PnlDsp->LedCode[4] = LEDCODE_U;
		PnlDsp->LedCode[3] = LEDCODE_N;
		RpxLedSetValHex( UnNo, PnlDsp, 2, 3);
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < sizeof(PNL_N_LED); i++ )
		{
			PnlDsp->LedBlnkSw[i] = (i == Pnl.CurPos) ? 0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator UnData display mode															*/
/*																									*/
/****************************************************************************************************/
void	PnlMonDataMode( HMSGIF Hmsg )
{
ULONG	ldata_tmp;
CMONTBL *pTbl = GET_UNMONTBL( CUNMONX );							/* get monitor table */
CMONDEF	*mondef_tmp = GET_UNMONDEF( pTbl, Pnl.CurMonIndx );			/* get monitor definition */
LEDDSP *DspData = (LEDDSP *)(Hmsg->ResBuf);							/* @@ Panel Display Data */
/*--------------------------------------------------------------------------------------------------*/
/*		Display data acquisition																				*/
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->Read( (COBJECT *)CUNMONX, Pnl.CurMonIndx, &ldata_tmp, MCHK_ACCS_LVL, Hmsg );  /* <S073> */
/*--------------------------------------------------------------------------------------------------*/
/*		Key input processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if(OpKeyMk.data)
		{
			Pnl.CurPos = 0;
			Pnl.DispMode = PNLMODE_UNNUM;							/* Go to parameter number mode	*/
			return;
		}
		else if(OpKeyMk.shup)
		{
			Pnl.CurPos = ((Pnl.CurPos+1) == Pnl.CurPosMax)?  0 : Pnl.CurPos+1;
		}
		else if(OpKeyMk.rst)
		{
			/* TODO: RpxMonClearVarIdx( Pnl.CurMonIndx ); */  
			CUNMONX->Clear( (COBJECT *)CUNMONX, Pnl.CurMonIndx, Hmsg );	/* <S144> */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		switch( CHK_UNBASE( mondef_tmp ) )
		{
		case DSPDEF_BASEDEC:
			if( ! CUNMONX->isLong( CUNMONX, Pnl.CurMonIndx, Hmsg ) )	/* Word	*/
			{
				RpxLedDispMonDecWord(
					(USHORT)ldata_tmp, 
					DspData,										/* @@  Panel Display Data */
					CHK_UNDECIMALS( mondef_tmp ), 
					CHK_UN_SIGNED( mondef_tmp )
					);
			}
			else													/* Long	*/
			{
				RpxLedDispMonDecLong(
					ldata_tmp, 
					DspData, 										/* @@ Panel Display Data */
					Pnl.CurPos, 
					CHK_UNDECIMALS( mondef_tmp ),
					CHK_UN_SIGNED( mondef_tmp )
					);
			}
			break;
		case DSPDEF_BASEHEX:
			if( ! CUNMONX->isLong( CUNMONX, Pnl.CurMonIndx, Hmsg ) )	/* Word	*/
			{
				PnlLedDispMonHexWord((USHORT)ldata_tmp, DspData );	/* Currently there is only a word	*/
			}
			else 
			{
				/* TODO: */;
			}
			break;
		case DSPDEF_BASEBIT:
			PnlLedDispMonBit((UCHAR)ldata_tmp, DspData );			/* Lower8Display bit	*/
			break;
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDBlink process															*/
/*																									*/
/****************************************************************************************************/
void	PnlLedBlink( HMSGIF Hmsg )
{
LEDDSP *PnlDsp = (LEDDSP *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Blink end check																		*/
/*--------------------------------------------------------------------------------------------------*/
		if(*((UINT *)&OpBlnkFlg) != 0)
		{
			if(KpiGetLongTimer(&OpBlnkTime) > 1000)
			{															/* Blink period has ended	*/
				*((UINT *)&OpBlnkFlg) = 0;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Blink start processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if(OpBlnkKickFlg.PrmEewrite)									/* When writing parameters	*/
		{
			*((UINT *)&OpBlnkFlg) = *((UINT *)&OpBlnkKickFlg) = 0;
			OpBlnkFlg.PrmEewrite = 1;
			KpiRstLongTimer(&OpBlnkTime);
		}
		else if(OpBlnkKickFlg.no_op)									/* read/writeNo key	*/
		{
			*((UINT *)&OpBlnkFlg) = *((UINT *)&OpBlnkKickFlg) = 0;
			OpBlnkFlg.no_op = 1;
			KpiRstLongTimer(&OpBlnkTime);
		}
		else if(OpBlnkKickFlg.error)
		{
			*((UINT *)&OpBlnkFlg) = *((UINT *)&OpBlnkKickFlg) = 0;
			OpBlnkFlg.error = 1;
			KpiRstLongTimer(&OpBlnkTime);
		}
		else if(OpBlnkKickFlg.done)
		{
			*((UINT *)&OpBlnkFlg) = *((UINT *)&OpBlnkKickFlg) = 0;
			OpBlnkFlg.done = 1;
			KpiRstLongTimer(&OpBlnkTime);
		}
		else if(OpBlnkKickFlg.end)
		{
			*((UINT *)&OpBlnkFlg) = *((UINT *)&OpBlnkKickFlg) = 0;
			OpBlnkFlg.end = 1;
			KpiRstLongTimer(&OpBlnkTime);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Blink process																				*/
/*--------------------------------------------------------------------------------------------------*/
		if(OpBlnkFlg.PrmEewrite)									/* Blink when writing parameters	*/
		{
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
		else if(OpBlnkFlg.no_op)								/* The panel executes various operations from the panel operatorread/writeNo key	*/
		{
			/* "no_oP"	*/
			PnlDsp->LedCode[4] = LEDCODE_N;
			PnlDsp->LedCode[3] = LEDCODE_sO;
			PnlDsp->LedCode[2] = LEDCODE_B3;
			PnlDsp->LedCode[1] = LEDCODE_sO;
			PnlDsp->LedCode[0] = LEDCODE_P;
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
		else if(OpBlnkFlg.error)
		{
			/* "Error"	*/
			PnlDsp->LedCode[4] = LEDCODE_E;
			PnlDsp->LedCode[3] = LEDCODE_R;
			PnlDsp->LedCode[2] = LEDCODE_R;
			PnlDsp->LedCode[1] = LEDCODE_sO;
			PnlDsp->LedCode[0] = LEDCODE_R;
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
		else if(OpBlnkFlg.done)
		{
			/* " donE"	*/
			PnlDsp->LedCode[4] = LEDCODE_BLANK;
			PnlDsp->LedCode[3] = LEDCODE_D;
			PnlDsp->LedCode[2] = LEDCODE_sO;
			PnlDsp->LedCode[1] = LEDCODE_N;
			PnlDsp->LedCode[0] = LEDCODE_E;
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
		else if(OpBlnkFlg.end)
		{
			/* "  End"	*/
			PnlDsp->LedCode[4] = LEDCODE_BLANK;
			PnlDsp->LedCode[3] = LEDCODE_BLANK;
			PnlDsp->LedCode[2] = LEDCODE_E;
			PnlDsp->LedCode[1] = LEDCODE_N;
			PnlDsp->LedCode[0] = LEDCODE_D;
			PnlLedSetBlnkSw( PnlDsp, PNLLED_BLINK, PNL_N_LED, TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
		PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK, PNL_N_LED,
						 ((KpiGetLongTimer(&Pnl.BlnkTime)/100) & 0x02) );
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel Operator Initialize																	*/
/*																									*/
/****************************************************************************************************/
void	RpxPnlOpInit( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Structure variablePnl																		*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory( (void *)&Pnl, sizeof(PNL_STRUC) );
/*--------------------------------------------------------------------------------------------------*/
/*		Clear blink variable																		*/
/*--------------------------------------------------------------------------------------------------*/
		*((UINT *)&OpBlnkFlg)     = 0;
		*((UINT *)&OpBlnkKickFlg) = 0;
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		<S0B5> Panel Monitor Initialize																*/
/*																									*/
/****************************************************************************************************/
void	RpxPnlOpMonInit( HMSGIF Hmsg )
{
LONG	wkx;
AXIS_HANDLE *pAxRsc = Hmsg->pAxRsc;
LONG	SrchNo = pAxRsc->UniPrms->Prm->powon_mon;						/* Pn52F */
/*--------------------------------------------------------------------------------------------------*/
/*		Panel Monitor Initialize																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (SrchNo != PNLMON_NOSET) && (KpiGetAlarmInfo( ) == 0) )
		{
			if( CUNMONX->Search( (COBJECT *)CUNMONX, SrchNo, 0, &wkx, MCHK_ACCS_LVL, Hmsg ) == OK )
			{
				Pnl.CurMonIndx = (USHORT)wkx;
				Pnl.DispMode = PNLMODE_UNDATA;
				Pnl.CurPos = 0;
				if ( CUNMONX->isLong( CUNMONX, Pnl.CurMonIndx, Hmsg ) == TRUE )	/* Size is long	*/
				{
					Pnl.CurPosMax = 3;	/* Display switching available(Lower/Middle/Upper)	*/
				}
				else										/* Size is less than or equal to word	*/
				{
					Pnl.CurPosMax = 1;	/* Display switching not available	*/
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel Operator Finish																		*/
/*																									*/
/****************************************************************************************************/
void	RpxPnlOpFinish( HMSGIF Hmsg )
{
//LONG	rc_tmp;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing to terminate the auxiliary function that is being executed.					*/
/*--------------------------------------------------------------------------------------------------*/
		CPNLFUN->Finish( CPNLFUN, Hmsg ); 

/*--------------------------------------------------------------------------------------------------*/
/*		Get parameterEEPROM											*/
/*--------------------------------------------------------------------------------------------------*/
//TODO: EEPROM
//
//		rc_tmp = RpxPrmUpdateEepromIdx( Pnl.CurPrmIndx );
//		while( rc_tmp == PRM_EEPROM_WRITE_WAIT || rc_tmp == PRM_EEPROM_WRITE_FAIL )	/* Incomplete	*/
//		{
//			rc_tmp = RpxPrmUpdateEepromIdx( Pnl.CurPrmIndx );
//		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(Parameter unsigned10Hex word)										*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispPrmDecWordUnsigned(		/* Panel operator LEDDisplay(Parameter unsigned10Hex word)	*/
		USHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals,					/* Number of decimal places										*/
		LEDDSP	*PnlDsp )					/*														*/
{
USHORT	i;

/*--------------------------------------------------------------------------------------------------*/
/*		LEDDisplay(Parameter unsigned10Hex word)														*/
/*--------------------------------------------------------------------------------------------------*/
		if( cur_digit >= 5 ){ return;}
		RpxLedSetValDec( data, PnlDsp, 4, 5, FALSE, decimals );
		for( i = 0; i < PNL_N_LED; i++ )
		{
			PnlDsp->LedBlnkSw[i] = (i == cur_digit)? 0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(Parameter signed10Hex word)										*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispPrmDecWordSigned(			/* Panel operator LEDDisplay(Parameter signed10Hex word)	*/
		SHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals, 					/* Number of decimal places										*/
		SHORT	llimit,						/* Lower limit value												*/
		SHORT	ulimit,						/* Upper limit												*/
		LEDDSP	*PnlDsp )
{
USHORT	sdata_tmp, sdata_tmp1, sdata_tmp2;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( cur_digit >= 5 ){ return;}
		PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
		sdata_tmp = (data < 0)? -data: data;
/*--------------------------------------------------------------------------------------------------*/
/*		Sign included5digits are sufficient																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (-10000 < llimit) && (ulimit < 10000) )
		{
			RpxLedSetValDec(sdata_tmp, PnlDsp, 3, 4, FALSE, decimals);
			PnlDsp->LedCode[4] = (data < 0)? LEDCODE_MINUS: LEDCODE_BLANK;
			PnlDsp->LedBlnkSw[cur_digit] = 0xFF;							/* Cursor digit	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Sign included5digits are insufficient-->Switching display														*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if(Pnl.CurPos < 4)												/* Bottom4Digit	*/
			{
				sdata_tmp1 = sdata_tmp/10000;
				sdata_tmp2 = sdata_tmp - sdata_tmp1*10000;
				RpxLedSetValDec(sdata_tmp2, PnlDsp, 3, 4, FALSE, decimals);
				PnlDsp->LedBlnkSw[cur_digit] = 0xFF;						/* Cursor digit	*/
				PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B3;		/* Display digit display	*/
//				if( data < 0 ){ Pnl.DspData.LedCode[4] |= LEDCODE_POINT;}	/* Negative display'.'	*/
			}
			else															/* Top1Digit+Signed	*/
			{
				sdata_tmp1 = sdata_tmp/10000;
				RpxLedSetValDec(sdata_tmp1, PnlDsp, 0, 1, FALSE, (USHORT)(decimals-4));
				PnlDsp->LedCode[2] = PnlDsp->LedCode[1] = LEDCODE_BLANK;
				PnlDsp->LedCode[3] = (data < 0)? LEDCODE_MINUS: LEDCODE_BLANK;
				PnlDsp->LedBlnkSw[cur_digit-4] = 0xFF;						/* Cursor digit	*/
				PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B6;		/* Display digit display	*/
			}
			if( data < 0 ){ PnlDsp->LedCode[4] |= LEDCODE_POINT;}			/* Negative display'.'	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(Parameter16Hex word)												*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispPrmHexNblWord(			/* Panel operator LEDDisplay(Parameter16Hex word)		*/
		USHORT	data,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	base,						/* Bottom of display												*/
		LEDDSP 	*PnlDsp )					/* 														*/
{
USHORT	i;

/*--------------------------------------------------------------------------------------------------*/
/*		LEDDisplay(Parameter16Hex word)																*/
/*--------------------------------------------------------------------------------------------------*/
		if( cur_digit >= 4 ){ return;}
		PnlDsp->LedCode[4] = (base == DSPDEF_BASENBL)? 
			(LEDCODE_N | LEDCODE_POINT): (LEDCODE_H | LEDCODE_POINT);
		RpxLedSetValHex( data, PnlDsp, 3, 4 );
		for(i = 0; i < PNL_N_LED; i++)
		{
			PnlDsp->LedBlnkSw[i] = (i == cur_digit)? 0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(10Advanced long)														*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispPrmDecLong(				/* Panel operator LEDDisplay(10Advanced long)					*/
		ULONG	ldata,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign,						/* Sign presence/absence												*/
		LEDDSP	*PnlDsp )					/* 														*/
{
USHORT	sdata_tmp2;
ULONG	ldata_abs, ldata_tmp2;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( cur_digit >= 10 ){ return;}
		PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
		ldata_abs = ((sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0))?	-(LONG)ldata: ldata;
/*--------------------------------------------------------------------------------------------------*/
/*		Bottom4Digit																						*/
/*--------------------------------------------------------------------------------------------------*/
		if(Pnl.CurPos < 4)
		{
			ldata_tmp2 = ldata_abs/10000;
			sdata_tmp2 = (USHORT)(ldata_abs - ldata_tmp2*10000);
			RpxLedSetValDec(sdata_tmp2, PnlDsp, 3, 4, FALSE, decimals);
			PnlDsp->LedBlnkSw[cur_digit] = 0xFF;							/* Cursor digit	*/
			PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B3;			/* Display digit display	*/
//			if( (sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0) )
//			{
//				Pnl.DspData.LedCode[4] |= LEDCODE_POINT;					/* Negative display'.'	*/
//			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Medium4Digit																						*/
/*--------------------------------------------------------------------------------------------------*/
		else if(Pnl.CurPos < 8)
		{
			ldata_tmp2 = ldata_abs/100000000;
			sdata_tmp2 = (USHORT)((ldata_abs - ldata_tmp2*100000000)/10000);
			RpxLedSetValDec(sdata_tmp2, PnlDsp, 3, 4, FALSE, (USHORT)(decimals-4));
			PnlDsp->LedBlnkSw[cur_digit-4] = 0xFF;							/* Cursor digit	*/
			PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B6;			/* Display digit display	*/
//			if( (sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0) )
//			{
//				Pnl.DspData.LedCode[4] |= LEDCODE_POINT;					/* Negative display'.'	*/
//			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Top2Digit																						*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			PnlDsp->LedCode[3] = ((sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0))?
				LEDCODE_MINUS : LEDCODE_BLANK;
			PnlDsp->LedCode[2] = LEDCODE_BLANK;
			sdata_tmp2 = (USHORT)(ldata_abs/100000000);
			RpxLedSetValDec(sdata_tmp2, PnlDsp, 1, 2, FALSE, (USHORT)(decimals-8));
			PnlDsp->LedBlnkSw[cur_digit-8] = 0xFF;							/* Cursor digit	*/
			PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B0;			/* Display digit display	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Negative display'.'																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( (sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0) )
		{
			PnlDsp->LedCode[4] |= LEDCODE_POINT;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(16Advanced long)														*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispPrmHexLong(				/* Panel operator LEDDisplay(16Advanced long)					*/
		ULONG	ldata,						/* Value to display											*/
		USHORT	cur_digit,					/* Current cursor position										*/
		LEDDSP	*PnlDsp )
{
USHORT	sdata_tmp2;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( cur_digit > 8 ){ return;}
		PnlLedSetBlnkSw( PnlDsp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
/*--------------------------------------------------------------------------------------------------*/
/*		Bottom4Digit																						*/
/*--------------------------------------------------------------------------------------------------*/
		if(Pnl.CurPos < 4)
		{
			sdata_tmp2 = (USHORT)(ldata & 0xFFFF);
			RpxLedSetValHex(sdata_tmp2, PnlDsp, 3, 4);
			PnlDsp->LedBlnkSw[cur_digit] = 0xFF;							/* Cursor digit	*/
			PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B3;			/* Display digit display	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Top4Digit																						*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			sdata_tmp2 = (USHORT)((ldata >> 16) & 0xFFFF);
			RpxLedSetValHex(sdata_tmp2, PnlDsp, 3, 4);
			PnlDsp->LedBlnkSw[cur_digit-4] = 0xFF;							/* Cursor digit	*/
			PnlDsp->LedBlnkSw[4] = PnlDsp->LedCode[4] = LEDCODE_B6;			/* Display digit display	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(Monitor16Hex word)												*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispMonHexWord(				/* Panel operator LEDDisplay(Monitor16Hex word)			*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*DspData )					/* @@ Panel Display Data								*/
{
/*--------------------------------------------------------------------------------------------------*/
/*		LEDDisplay(Monitor16Hex word)																	*/
/*--------------------------------------------------------------------------------------------------*/
		DspData->LedCode[4] = LEDCODE_H | LEDCODE_POINT;
		RpxLedSetValHex(data, DspData, 3, 4);
		PnlLedSetBlnkSw( DspData, PNLLED_NOBLINK, PNL_N_LED, TRUE );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel operator LEDDisplay(Monitor bit byte)												*/
/*																									*/
/****************************************************************************************************/
void	PnlLedDispMonBit(					/* Panel operator LEDDisplay(Monitor bit byte)		*/
		UCHAR	data,						/* Value to display											*/
		LEDDSP	*leddsp )					/* LEDDisplay data address								*/
{
USHORT	i;

/*--------------------------------------------------------------------------------------------------*/
/*		Initializing variables																					*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < PNL_N_LED; i++ )
		{
			leddsp->LedBlnkSw[i] = leddsp->LedCode[i] = 0;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		8Display bit																				*/
/*--------------------------------------------------------------------------------------------------*/
		for(i = 0; i < 8; i++)
		{
			switch(i & 1)
			{
			case 0:																/* even numbered --> Right	*/
				leddsp->LedCode[i/2] |= (data & 1)? LEDCODE_B1 : LEDCODE_B2;
				break;
			case 1:																/* Odd numbered --> Left	*/
				leddsp->LedCode[i/2] |= (data & 1)? LEDCODE_B5 : LEDCODE_B4;
				break;
			default:
				break;
			}
			data >>= 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Hi/LoDisplay of indicators and breakpoints 																*/
/*--------------------------------------------------------------------------------------------------*/
		leddsp->LedCode[4] = LEDCODE_B1 | LEDCODE_B2 | LEDCODE_POINT;
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LEDOperator Monitor display(10Hex word with sign/No capacity)											*/
/*																									*/
/****************************************************************************************************/
void	RpxLedDispMonDecWord(				/* LEDOperator Monitor display(10Hex word with sign/None)		*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			)			/* Signed presence/absence											*/
{
USHORT	data_tmp;

/*--------------------------------------------------------------------------------------------------*/
/*		Monitor display(10Hex word with sign/No capacity)															*/
/*--------------------------------------------------------------------------------------------------*/
		data_tmp = ((sign == DSPDEF_W_SIGN) && ((SHORT)data < 0))?	-data: data;
	/*----------------------------------------------------------------------------------------------*/
		if( sign == DSPDEF_W_SIGN )
		{
			data_tmp = ((SHORT)data < 0)?	-data: data;
			RpxLedSetValDec(data_tmp, leddspp, 3, 4, FALSE, decimals);
			leddspp->LedCode[4] = ((SHORT)data < 0)?	LEDCODE_MINUS: LEDCODE_BLANK;
		}
		else
		{
			RpxLedSetValDec(data, leddspp, 4, 5, FALSE, decimals);
		}
	/*----------------------------------------------------------------------------------------------*/
		PnlLedSetBlnkSw( leddspp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LEDOperator Monitor display(10Advanced long/None)												*/
/*																									*/
/****************************************************************************************************/
void	RpxLedDispMonDecLong(				/* LEDOperator Monitor display(10Advanced long/None)		*/
		ULONG	ldata,						/* ldata:Value to display										*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	dsp_pos,					/* Current display position											*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			)			/* Sign presence/absence												*/
{
USHORT	sdata_tmp2;
ULONG	ldata_abs, ldata_tmp2;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( dsp_pos >= 3 ){ return;}
		PnlLedSetBlnkSw( leddspp, PNLLED_NOBLINK, PNL_N_LED, TRUE );
		ldata_abs = ((sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0))?	-(LONG)ldata: ldata;
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		switch(dsp_pos)
		{
		case 0:
			ldata_tmp2 = ldata_abs/10000;
			sdata_tmp2 = (USHORT)(ldata_abs - ldata_tmp2*10000);
			RpxLedSetValDec(sdata_tmp2, leddspp, 3, 4, FALSE, decimals);
			leddspp->LedBlnkSw[4] = leddspp->LedCode[4] = LEDCODE_B3;	/* Display digit display	*/
//			if( (sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0) )
//			{
//				leddspp->LedCode[4] |= LEDCODE_POINT;					/* Negative display'.'	*/
//			}
			break;
		case 1:
			ldata_tmp2 = ldata_abs/100000000;
			sdata_tmp2 = (USHORT)((ldata_abs - ldata_tmp2*100000000)/10000);
			RpxLedSetValDec(sdata_tmp2, leddspp, 3, 4, FALSE, (USHORT)(decimals-4));
			leddspp->LedBlnkSw[4] = leddspp->LedCode[4] = LEDCODE_B6;	/* Display digit display	*/
//			if((sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0))
//			{
//				leddspp->LedCode[4] |= LEDCODE_POINT;					/* Negative display'.'	*/
//			}
			break;
		case 2:
			leddspp->LedCode[3] = ((sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0))?
				LEDCODE_MINUS : LEDCODE_BLANK;
			leddspp->LedCode[2] = LEDCODE_BLANK;
			sdata_tmp2 = (USHORT)(ldata_abs/100000000);
			RpxLedSetValDec(sdata_tmp2, leddspp, 1, 2, FALSE, (USHORT)(decimals-8));
			leddspp->LedBlnkSw[4] = leddspp->LedCode[4] = LEDCODE_B0;	/* Display digit display	*/
			break;
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Negative display'.'																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( (sign == DSPDEF_W_SIGN) && ((LONG)ldata < 0) )
		{
			leddspp->LedCode[4] |= LEDCODE_POINT;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LEDOperator Status bit display information set processing											*/
/*																									*/
/****************************************************************************************************/
void	RpxLedSetStatusBits(				/* LEDOperator Status bit display information set processing		*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		HMSGIF	Hmsg )						/* Erase after												*/
{
AXIS_HANDLE	*pAxRsc = Hmsg->pAxRsc;			/* TODO: Erase after 										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Status bit display information set																*/
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_COIN() || CHK_VCMP() || CHK_TRQMODE() )						/* TODO: TRQMODE */
		{
			leddspp->LedCode[4] |= LEDCODE_B0;
		}
		if( CHK_BB() 						){ leddspp->LedCode[4] |= LEDCODE_B6;}
		if( CHK_TGON() 						){ leddspp->LedCode[3] |= LEDCODE_B0;}
		if( CHK_PREFON() || CHK_VREFON() 	){ leddspp->LedCode[3] |= LEDCODE_B6;}
		if( CHK_TREFON() || CHK_PERRCLR() 	){ leddspp->LedCode[3] |= LEDCODE_B3;}
		if( CHK_CHG() 						){ leddspp->LedCode[3] |= LEDCODE_B7;}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LEDOperator Display value(10Base)Set processing														*/
/*																									*/
/****************************************************************************************************/
void	RpxLedSetValDec(					/* LEDOperator Display value(10Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit,						/* Number of digits processed												*/
		USHORT	sup_flg,					/* 0Display suppression flag										*/
		USHORT	decimals		)			/* Number of decimal places										*/
{
USHORT	i, var1_tmp, var2_tmp;
UCHAR	*ledbuf_ptr;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( digit > 5 ){ return;}
		ledbuf_ptr = &(leddspp->LedCode[startdgt]);
/*--------------------------------------------------------------------------------------------------*/
/*		Drop high-order digits																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( data >= MlibDecPowerTbl[digit] )
		{
			var1_tmp = 	(USHORT)(data/MlibDecPowerTbl[digit]);
			data -= (USHORT)(var1_tmp*MlibDecPowerTbl[digit]);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display value(10Base)Set processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		for(i = 0; i < digit; i++)
		{
			var1_tmp = (USHORT)MlibDecPowerTbl[digit-1-i];		/* Number to divide: Example 1000, 100, 10, 1	*/
			var2_tmp = data/var1_tmp;
			if( sup_flg )										/* The upper order0is displayed blank	*/
			{
				if( var2_tmp == 0 )
				{
					*(ledbuf_ptr - i) = LEDCODE_BLANK;
				}
				else
				{
					*(ledbuf_ptr - i) = ledcodetbl[var2_tmp];
					sup_flg = FALSE;
				}
			}
			else
			{
				*(ledbuf_ptr - i) = ledcodetbl[var2_tmp];
			}
			data -= (var1_tmp * var2_tmp);
		}
/*--------------------------------------------------------------------------------------------------*/
		if( (0 < (SHORT)decimals) && ((SHORT)decimals < (SHORT)digit) )
		{
			*(ledbuf_ptr-(digit-1)+decimals) |= LEDCODE_POINT;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LEDOperator Display value(16Base)Set processing														*/
/*																									*/
/****************************************************************************************************/
void	RpxLedSetValHex(					/* LEDOperator Display value(16Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit			)			/* Number of digits processed												*/
{
USHORT	i, var1_tmp, var2_tmp;
UCHAR	*ledbuf_ptr;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( digit > 4 ){ return;}
		ledbuf_ptr = &(leddspp->LedCode[startdgt]);
/*--------------------------------------------------------------------------------------------------*/
/*		Drop high-order digits																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( data >= (0x1 << (digit*4)) )
		{
			data &= ((0x1 << (digit*4)) -1);			/* Drop high-order digits	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display value(10Base)Set processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		for(i = 0; i < digit; i++)
		{
			var1_tmp = 0x1 << ((digit-1-i)*4);			/* Number to divide: Example 0x1000, 0x100, 0x10, 0x1	*/
			var2_tmp = data/var1_tmp;
			*(ledbuf_ptr - i) = ledcodetbl[var2_tmp];
			data -= (var1_tmp * var2_tmp);
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LED Operator String Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLedSetStr(						/* LED Operator String Set								*/
		UCHAR	*str_ptr,					/* Character string address to display								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	length			)			/* Number of display characters											*/
{
USHORT	str_val;
USHORT	idx_tmp;
USHORT	str_idx, dgt_tmp;
UCHAR	*ledbuf_ptr;

/*--------------------------------------------------------------------------------------------------*/
/*		Length0 or Start position is outside display buffer															*/
/*--------------------------------------------------------------------------------------------------*/
		if( (length == 0) || (startdgt >= PNL_N_LED) )
		{
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		ledbuf_ptr = &(leddspp->LedCode[0]);
		str_idx = 0;
		dgt_tmp = startdgt;
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		while(*(str_ptr + str_idx) != 0)
		{
			str_val = (USHORT)*(str_ptr + str_idx);
			idx_tmp = str_val - '-';
		/*------------------------------------------------------------------------------------------*/
		/*	Character data < '-' In the case of																	*/
		/*------------------------------------------------------------------------------------------*/
			if(str_val < '-')
			{
				*(ledbuf_ptr + dgt_tmp) = LEDCODE_BLANK;
			}
		/*------------------------------------------------------------------------------------------*/
		/*	'-' <= Character data <= 'Z' In the case of															*/
		/*------------------------------------------------------------------------------------------*/
			else if( str_val <= 'Z' )
			{
				if( (str_idx != 0) && 					/* Not at the beginning of the string		*/
					('.' == *(str_ptr + str_idx)) && 	/* '.',			*/
					('.' != *(str_ptr + str_idx -1))	/* Previous character'.'	*/
				)
				{	/* '.', write something other than'.', it is added to the previous character.[5][.] -> [5.]	*/
					dgt_tmp++;
					*(ledbuf_ptr + dgt_tmp) |= LEDCODE_POINT;
				}
				else	*(ledbuf_ptr + dgt_tmp) = ledconvtbl[idx_tmp];
			}
		/*------------------------------------------------------------------------------------------*/
		/*	If the character data is lower case																*/
		/*------------------------------------------------------------------------------------------*/
			else if( ('a' <= str_val) && (str_val <= 'z') )
			{
				if(      'h' == str_val ){ *(ledbuf_ptr + dgt_tmp) = LEDCODE_sH;}
				else if( 'o' == str_val ){ *(ledbuf_ptr + dgt_tmp) = LEDCODE_sO;}
				else if( 'u' == str_val ){ *(ledbuf_ptr + dgt_tmp) = LEDCODE_sU;}
				else { *(ledbuf_ptr + dgt_tmp) = ledconvtbl[idx_tmp - ('a' - 'A')];}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Other than the above																			*/
		/*------------------------------------------------------------------------------------------*/
			else
			{
				if(      '_' == str_val ){ *(ledbuf_ptr + dgt_tmp) = LEDCODE_B3;}
				else if( '~' == str_val ){ *(ledbuf_ptr + dgt_tmp) = LEDCODE_B0;}
				else					 { *(ledbuf_ptr + dgt_tmp) = LEDCODE_BLANK;}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Right end of buffer reached																		*/
		/*------------------------------------------------------------------------------------------*/
			if( dgt_tmp == 0 )
			{
				if( ((str_idx+1) < length) && 				/* There is the next input character		*/
					('.' == *(str_ptr + str_idx + 1)) && 	/* The next character is'.',	*/
					('.' != *(str_ptr + str_idx)) 	)		/* This character is'.'When not	*/
				{
					*(ledbuf_ptr + dgt_tmp) |= LEDCODE_POINT;	/* '.'		*/
				}
				break;
			}
		/*------------------------------------------------------------------------------------------*/
			dgt_tmp--;
			str_idx++;
			if( str_idx >= length ){ break;}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LED Operator Blink Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLedSetBlnk(						/* LED Operator Blink Set								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		UCHAR	blnkptn			)			/* blinking pattern(1bit=1Character minutes)							*/
{
SHORT	i;

/*--------------------------------------------------------------------------------------------------*/
/*		LED Operator Blink Set																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < PNL_N_LED; i++ )
		{
			leddspp->LedBlnkSw[i] = (blnkptn & (1 << i))? 0xFF: 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel LED Set to Kernel Variables															*/
/*																									*/
/****************************************************************************************************/
void	PnlLedSetKnlBuffer(					/* Panel LED Set to Kernel Variables					*/
		LEDDSP	*leddsp,					/*														*/
		UCHAR	*dist,						/*														*/
		LONG	size			)			/*														*/
{
LONG	i;

/*--------------------------------------------------------------------------------------------------*/
/*		Panel LED Set to Kernel Variables															*/
/*--------------------------------------------------------------------------------------------------*/
		for ( i = 0; i < size; i++ )
		{
			dist[i] = (UCHAR)(leddsp->LedCode[i]) & (~((UCHAR)(leddsp->LedBlnkSw[i])));
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Panel LED Set to Same Data																	*/
/*																									*/
/****************************************************************************************************/
void	PnlLedSetBlnkSw(					/* Panel LED Set to Same Data							*/
		LEDDSP	*leddsp,					/*														*/
		UCHAR	data,						/*														*/
		LONG	size,						/*														*/
		LONG	flag			)			/*														*/
{
LONG	i;

/*--------------------------------------------------------------------------------------------------*/
/*		Panel LED Set to Same Data																	*/
/*--------------------------------------------------------------------------------------------------*/
		if ( flag != FALSE )
		{
			for ( i = 0; i < size; i++ )
			{
				leddsp->LedBlnkSw[i] = data;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/* <S0CB> Start */
/****************************************************************************************************/
/*																									*/
/*		Push button & 7SegLed Confirmation test execution processing													*/
/*																									*/
/****************************************************************************************************/
void	PbAndLedChkTest( HMSGIF Hmsg )
{
	AXIS_HANDLE	*pAxRsc = Hmsg->pAxRsc;
	LEDDSP *PnlDsp = (LEDDSP *)(Hmsg->ResBuf);
	PNLCMDMSG *PnlCmd = (PNLCMDMSG *)Hmsg->CmdBuf;
	UINT	PnlBtn = PnlCmd->PnlBtn;					/* Get Panel Button */
	FUN_CMN_CONTROL	*FnCmnCtrl =  pAxRsc->FnCmnCtrl;
	UINT	LedPos;										/* Turn on7SegLedlocation */

	ULONG IntervalTimeMs;
	USHORT i;	/* <S0FB> */

	IntervalTimeMs = FnCmnCtrl->LightIntervalTime * 100;	/* The lighting interval time setting value is100[ms]unit,[ms]Change to unit */

	if( FnCmnCtrl->AnlgLedChkInitFlag == 0x00 )
	{
		PnlDsp->LedCode[4] = LEDCODE_BLANK;
		PnlDsp->LedCode[3] = LEDCODE_BLANK;
		PnlDsp->LedCode[2] = LEDCODE_BLANK;
		PnlDsp->LedCode[1] = LEDCODE_BLANK;
		PnlDsp->LedCode[0] = LEDCODE_BLANK;

		pAxRsc->FnCmnCtrl->AnlgLedChkInitFlag = 0x01;
		KpiRstLongTimer(&Pnl.ChkTime);
	}
	else
	{
		if(KlibGetLongTimerMs(Pnl.ChkTime) > IntervalTimeMs)
		{

			KpiRstLongTimer(&Pnl.ChkTime);

			if( FnCmnCtrl->AnlgLedChkLstPnlBtn != PnlBtn )	/* If the push button pressed in the previous scan is different from the push button pressed in the current scan */
			{
				/* Counter reset */
				FnCmnCtrl->AnlgLedChkCnt = 0U;
			}

			/* Push button supportLEDSet location */
			switch(PnlBtn)
			{
				case PNLKEY_SHUP:
					/* Getting current elapsed time */
					KpiRstLongTimer(&Pnl.ChkTime);
					PnlDsp->LedCode[4] = LEDCODE_BLANK;
					PnlDsp->LedCode[3] = LEDCODE_BLANK;
					PnlDsp->LedCode[2] = LEDCODE_BLANK;
					PnlDsp->LedCode[1] = LEDCODE_BLANK;
					PnlDsp->LedCode[0] = LedLightPatternTbl[FnCmnCtrl->AnlgLedChkCnt];
					break;

				case PNLKEY_DOWN:
					/* Getting current elapsed time */
					KpiRstLongTimer(&Pnl.ChkTime);

					PnlDsp->LedCode[4] = LEDCODE_BLANK;
					PnlDsp->LedCode[3] = LEDCODE_BLANK;
					PnlDsp->LedCode[2] = LEDCODE_BLANK;
					PnlDsp->LedCode[1] = LedLightPatternTbl[FnCmnCtrl->AnlgLedChkCnt];
					PnlDsp->LedCode[0] = LEDCODE_BLANK;
					break;

				case PNLKEY_UP:
					/* Getting current elapsed time */
					KpiRstLongTimer(&Pnl.ChkTime);

					PnlDsp->LedCode[4] = LEDCODE_BLANK;
					PnlDsp->LedCode[3] = LEDCODE_BLANK;
					PnlDsp->LedCode[2] = LedLightPatternTbl[FnCmnCtrl->AnlgLedChkCnt];
					PnlDsp->LedCode[1] = LEDCODE_BLANK;
					PnlDsp->LedCode[0] = LEDCODE_BLANK;
					break;

				case PNLKEY_MODE:
					/* Getting current elapsed time */
					KpiRstLongTimer(&Pnl.ChkTime);

					PnlDsp->LedCode[4] = LEDCODE_BLANK;
					PnlDsp->LedCode[3] = LedLightPatternTbl[FnCmnCtrl->AnlgLedChkCnt];
					PnlDsp->LedCode[2] = LEDCODE_BLANK;
					PnlDsp->LedCode[1] = LEDCODE_BLANK;
					PnlDsp->LedCode[0] = LEDCODE_BLANK;
					break;

				default:
					/* Getting current elapsed time */
					KpiRstLongTimer(&Pnl.ChkTime);

					PnlDsp->LedCode[4] = LedLightPatternTbl[FnCmnCtrl->AnlgLedChkCnt];
					PnlDsp->LedCode[3] = LEDCODE_BLANK;
					PnlDsp->LedCode[2] = LEDCODE_BLANK;
					PnlDsp->LedCode[1] = LEDCODE_BLANK;
					PnlDsp->LedCode[0] = LEDCODE_BLANK;
					break;
			}

			/* Counter update */
			FnCmnCtrl->AnlgLedChkCnt++;

			if(FnCmnCtrl->AnlgLedChkCnt > LED_CHK_CNT_LMIT)
			{
				/* Counter reset */
				FnCmnCtrl->AnlgLedChkCnt = 0U;
			}

			KpiRstLongTimer(&Pnl.ChkTime);

			FnCmnCtrl->AnlgLedChkLstPnlBtn = PnlBtn;	/* Update the lower button of the previous scan */

		}
	}

/* <S0FB> Start */
		for( i = 0; i < PNL_N_LED; i++ )
		{
			PnlDsp->LedBlnkSw[i] = 0;
		}
/* <S0FB> End */

	return;
}
/* <S0CB> End */

#endif /* SVFSEL_PANELOP_USE == 1 */
/***************************************** end of file **********************************************/
