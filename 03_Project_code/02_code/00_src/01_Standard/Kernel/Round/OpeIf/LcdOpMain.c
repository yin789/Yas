/****************************************************************************************************/
/*																									*/
/*																									*/
/*		LcdOpMain.c : LCDOperator processing module main												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : SigmaMessagecommunicates with the LCD operator and executes various operations from the operator.		*/
/*																									*/
/*			1)Monitor function : Specified monitor variable(UnXXX)												*/
/*			2)Parameter : Specified parameters(PnXXX),Editing,Change									*/
/*			3)Auxiliary function	 : Specification auxiliary function(FnXXX)and display execution results								*/
/*			4)ParamCopy  : Built-in operatorEEPROM				*/
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
/*		Rev.1.00 : 2002.11.14  M.Suga																*/
/*		Rev.1.10 : 2008.04.11  T.Taniguchi  Under Modification										*/
/*		Rev.1.20 : 2011.03.19  K.Ozaki      Adaptation of Converter LCD Operator <0BJ14>			*/
/*		Rev.1.30 : 2012.04.xx  K.Ozaki      Revised for Mercury 									*/
/*		Rev.1.40 : 2013.10.24  K.Ozaki      <S031> Parameter Copy 									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"			/* Kernel API */
#include "LcdOp.h"			/* LcdOpIf Common Header */
#include "Monitor.h"		/* Un Monitor */
#include "Parameter.h"		/* Parameter */
#include "Function.h"		/* Fn Funstion */


/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Constant Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCDFont loaded to operator															*/
/*--------------------------------------------------------------------------------------------------*/
const	UCHAR	LcdExtFont[] = {				/* LCDFont loaded to operator				*/
		0xe0, 0xe0, 0xe0, 0xe0, 0xe0,			/* Code 0x10	Bit(0)							*/
		0x07, 0x07, 0x07, 0x07, 0x07,			/* Code 0x11	Bit(1)							*/
/*--------------------------------------------------------------------------------------------------*/
		0x24, 0x64, 0xe7, 0x26, 0x24,			/* Code 0x12	Sample character					*/
		0x97, 0x49, 0x5b, 0x0e, 0x00,			/* Code 0x13	Sample character					*/
/*--------------------------------------------------------------------------------------------------*/
		0x0e, 0x5b, 0x49, 0x57, 0xcf,			/* Code 0x14	Sample character					*/
		0x57, 0x49, 0x5b, 0x0e, 0x00,			/* Code 0x15	Sample character					*/
/*--------------------------------------------------------------------------------------------------*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x16										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x17										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x18										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x19										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x1A										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x1B										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x1C										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x1D										*/
		0x00, 0xff, 0x00, 0xff, 0x00,			/* Code 0x1E										*/
		0x1c, 0x3e, 0x3e, 0x3e, 0x1c			/* Code 0x1F										*/
};
/*--------------------------------------------------------------------------------------------------*/
/*		Display Information Definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
		LOPDIT	DspDef[4];
} LOPDITDEF;
typedef const LOPDITDEF CLOPDITDEF;
/*--------------------------------------------------------------------------------------------------*/
/*		Initial Display for Rotary Axis																*/
/*--------------------------------------------------------------------------------------------------*/
const	LOPDITDEF	LopDitDef_Rotary[] = {
		CINITDISP_PRMMON,						/* PRM/MON */
		CINITDISP_ZSET,                         /* ZSET */
		CINITDISP_JOG,                          /* JOG */
		CINITDISP_PJOG                          /* PJOG */
};
/*--------------------------------------------------------------------------------------------------*/
/*		Initial Display for Linear Axis																*/
/*--------------------------------------------------------------------------------------------------*/
const	LOPDITDEF	LopDitDef_Linear[] = {
		CINITDISP_PRMMON_SGDV_L,
		CINITDISP_ZSET_SGDV_L,
		CINITDISP_JOG_SGDV_L,
		CINITDISP_PJOG_SGDV_L
};

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Dummy Message Declaration														*/
/*--------------------------------------------------------------------------------------------------*/
DECL_DUMMY_MSG();

/*<11J42>Addition from here*/
#if CCSW_CMDID == CMDID_EXIST	
const DEGOP_PRMRNG	LcdOptPrmLmtTbl[4][4] =	/* Maximum upper and lower limit value table for editing command option parameters */
{
	{	/* Nibble display	*/
		{0xFFFF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (without word sign)	*/
		{0xFFFF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (with word sign)	*/
		{0xFFFFFFFF, 0x00000000},	/* Maximum upper limit value, Maximum lower limit value (without long sign)	*/
		{0xFFFFFFFF, 0x00000000}	/* Maximum upper limit value, Maximum lower limit value (with long sign)	*/
	},

	{	/* 10Hex display	*/
		{0xFFFF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (without word sign)	*/
		{0x7FFF, 0xFFFF8000},		/* Maximum upper limit value, Maximum lower limit value (with word sign)	*/
		{0xFFFFFFFF, 0x00000000},	/* Maximum upper limit value, Maximum lower limit value (without long sign)	*/
		{0x7FFFFFFF, 0x80000000}	/* Maximum upper limit value, Maximum lower limit value (with long sign)	*/
	},

	{	/* 16Hex display	*/
		{0xFFFF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (without word sign)	*/
		{0x7FFF, 0xFFFF8000},		/* Maximum upper limit value, Maximum lower limit value (with word sign)	*/
		{0xFFFFFFFF, 0x00000000},	/* Maximum upper limit value, Maximum lower limit value (without long sign)	*/
		{0x7FFFFFFF, 0x80000000}	/* Maximum upper limit value, Maximum lower limit value (with long sign)	*/
	},

	{	/* Bit display (not supported)	*/
		{0x00FF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (without word sign)	*/
		{0x00FF, 0x0000},			/* Maximum upper limit value, Maximum lower limit value (with word sign)	*/
		{0x000000FF, 0x00000000},	/* Maximum upper limit value, Maximum lower limit value (without long sign)	*/
		{0x000000FF, 0x00000000}	/* Maximum upper limit value, Maximum lower limit value (with long sign)	*/
	}
};
#endif
/*<11J42>Addition to here*/

/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Variable Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator DispInfo Variables																*/
/*--------------------------------------------------------------------------------------------------*/
LOPDSP		LopDsp;						/* LCD operator: Display information								*/
LCDEEP		LcdEep[MAX_AXIS_NUMBER];	/* LCD operator: DisplayEEPROMInformation					 <S208> */
LCDEEP		*pLcdEep;					/* LCD display information pointer							 <S208> */
USHORT		LcdWtReq;					/* LCD display informationEEPROMWrite request					 <S208> */
USHORT		LcdWtCnt;					/* LCD display information writing counter flag			 <S208> */
ULONG		LcdRomAdr;					/* LCD display information writingEEPROMAddress				 <S208> */
ULONG		LcdRomBaseAdr;				/* LCD display information writingEEPROMAddress				 <S208> */
USHORT		*LcdMemAdr;					/* LCD display information writing memory address			 <S208> */

//TODO : minaosi
//#if CCSW_CMDID == CMDID_EXIST			/*<11J42>*/
//DEGOP_OPTCOM	LcdOpt[LCDPMDSP_MAX];
//#endif
//#pragma section sdata end
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Common Variables																*/
/*--------------------------------------------------------------------------------------------------*/
extern MSGIF LopMsg;					/* TODO: EraseLcdOpe:Message I/F							*/

/*--------------------------------------------------------------------------------------------------*/
LOPV		LopV;						/* LCD operator: Common variable								*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
DRVSEL		DrvSel;						/* LCD operator: Drive (axis) selection	Structure						*/
#endif
/*--------------------------------------------------------------------------------------------------*/
OPKEYFLG	OpKeyMk;					/* KeyMAKE(OFF->ON)Flag									*/
OPKEYFLG	OpKeyBrk;					/* KeyBREAK(ON->OFF)Flag									*/
OPKEYFLG	OpKeyLvl;					/* Key level flag											*/
OPKEYFLG	OpKeyHld;					/* Key long press flag											*/
/*--------------------------------------------------------------------------------------------------*/
BLINKFLG	OpBlnkFlg;					/* Blink flag											*/
BLINKFLG	OpBlnkKickFlg;				/* Blink start flag										*/
ULONG		OpBlnkTime;					/* Blink flag for blink time measurement							*/
/*--------------------------------------------------------------------------------------------------*/
FUN_MANAGER FnManagerSto;				/* Auxiliary function continuation command storage									*/
/*--------------------------------------------------------------------------------------------------*/
AXIS_HANDLE	*pAxRsc;					/* TODO: Delete it later 										*/
/*--------------------------------------------------------------------------------------------------*/


// TODO: minaosi
#if CCSW_CMDID == CMDID_EXIST			/*<11J42>*/
DEGOP_OPTCOM	LcdOpt[LCDPMDSP_MAX];
BLINKFLG	   *LcdOptBlnkKick;
#endif
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Function List																				*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdOpMain(						/* Lcd Operator Main Program							*/
		MSGIF	*hMsg,						/* Message Handle 										*/
		DRVSEL	*pDrvSel		);			/* Drive Selection Data for Multi-Axis System			*/
#if		CCSW_HWID == HWID_JPCV
LONG	RpxLcdOpRemoteMst(					/* Lcd Operator Main Program for Remote Operation(Mst)	*/
		DRVSEL	*pDrvSel		);			/* Drive Selection Data for Multi-Axis System			*/
#endif
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdOpFinish( MSGIF *hMsg );		/* Lcd Operator Finish Procedure						*/
/*--------------------------------------------------------------------------------------------------*/
//void	RpxLcdOpInitDispInfo( void );		/* Lcd Operator Init. Display Info. Table				*/
void	RpxLcdOpInitDispInfo(  AXIS_HANDLE* );	/* Lcd Operator Init. Display Info. Table	 <S213> */
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdClrTxBuf(						/* Clear ResBuf(TxBuf)									*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetPnUnData(					/* Pn/Un Display Data Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	IsLong,						/* When using long parameterTRUE								*/
		USHORT	Base,						/* Bottom(Display attributes)											*/
		USHORT	Sign,						/* Sign presence/absence(Display attributes)									*/
		SHORT	Decimals,					/* Number of decimal places(Display attributes)								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetSignedDecData(				/* Lcd Operator Signed10Hex number display						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetStr(						/* Lcd Operator String Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	*pStr,						/* Character string address to display								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxLcdSetSignChar(					/* Lcd Operator Sign Char Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Data to display										*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetDecPnt(					/* Lcd Operator Decimal w/point Set						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetValHex(					/* Lcd Operator HEX Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display											*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetBit(						/* Lcd Operator BIT Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	Data,						/* Display data											*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetBlnk(						/* Lcd Operator LCD Blink Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		USHORT	Length,						/* Number of characters												*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdClrLineBlnk(					/* Lcd Operator LCD Blink Stop							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/

/*--------------------------------------------------------------------------------------------------*/
/*		Local Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
void	LcdOpInit( MSGIF *hMsg );
void	LcdVarInit( void );
/*--------------------------------------------------------------------------------------------------*/
void	LcdKeyTranslate( MSGIF *hMsg );
/*--------------------------------------------------------------------------------------------------*/
//void	LcdInitDispInfoTable( void );
void	LcdInitDispInfoTable( AXIS_HANDLE* );	/* <S213> */
SHORT	LcdSaveDispInfoTable( USHORT dispID );
/*--------------------------------------------------------------------------------------------------*/
void	LcdResSend( MSGIF *hMsg );
void	RpxLcdSetResStatus( UCHAR *ResBuf );					/* <S031> */
void	LcdSetResLedSts( USHORT *pLedSts );
void	LcdSetResBlink( UCHAR *ResBuf, USHORT *pLedSts );
/*--------------------------------------------------------------------------------------------------*/
void 	RpxLcdSetBlink (UCHAR *ResBuf);							/* <S031> */
/*--------------------------------------------------------------------------------------------------*/
LONG	LcdModeChg ( DRVSEL *pDrvSel );
/*--------------------------------------------------------------------------------------------------*/
//static LONG LpxStartDispModeCv( DRVSEL *pDrvSel, LONG *DrvSelEnd );			/* <S031> Lcd Prm Copy 	*/
static LONG LpxStartDispModeSv( DRVSEL *pDrvSel );							/* <S031> Lcd Prm Copy 	*/
void	LopDspDataLoad( LONG AxisNo );										/* <S208>				*/

/*--------------------------------------------------------------------------------------------------*/
/*		LCDOperator processing execution table 																*/
/*--------------------------------------------------------------------------------------------------*/
void (* const LcdOpeProcTable[LCDOM_DISPNUM])(MSGIF *hMsg, DRVSEL *pDrvSel) = {
	LcdStartDisp,							/* 0:Initial screen display										*/
	LcdPrmMonMain,							/* 1:Parameter/Monitor screen								*/
	LcdFun, 								/* 2:Auxiliary function execution screen									*/
	LcdSelectUnit,							/* 3:Unit selection screen									*/
	LcdSelectAxis,							/* 4:Axis selection screen											*/
	LcdPasswd,								/* 5:Password input screen mode							*/
	LcdError,								/* 6:Error status(No response)									*/
__LCD_PRM_COPY_MAIN( RpxLcdPrmCopyMain ),	/* 7:<S031> Parameter copy screen						*/
};


/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Main Program																	*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdOpMain( MSGIF *hMsg, DRVSEL *pDrvSel )
{
LONG	DrvSelEnd = FALSE;
UCHAR	*CmdBuf = hMsg->CmdBuf;
CSIF 	*Sif = hMsg->Sif;
/*--------------------------------------------------------------------------------------------------*/
/* <S208> */
	if( (LONG)pLcdEep != (LONG)( &LcdEep[ hMsg->pAxRsc->AxisNo ] ) )
	{
		LopDspDataLoad( hMsg->pAxRsc->AxisNo );
	}
/* <S208> */
/*--------------------------------------------------------------------------------------------------*/
	pAxRsc = hMsg->pAxRsc; /* Delete it later */	
/*--------------------------------------------------------------------------------------------------*/
	switch ( CmdBuf[1] )
	{
/*--------------------------------------------------------------------------------------------------*/
		case LCD_CMD_INIT: /*		Command : 0x64													*/
/*--------------------------------------------------------------------------------------------------*/
			CLCDFUN->Finish( CLCDFUN, hMsg );					/* Finish Executing Function */
			LcdOpInit( hMsg );
			LopV.CurMsgCmd = CmdBuf[1];							/* <S031> Current Command */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
			/* 	LCD online1Start	*/
			LopV.Online = LCD_ONLINE1;							/* 0x64Command reception,ONLINE1 	*/
#endif
			break;
/*--------------------------------------------------------------------------------------------------*/
		case LCD_CMD_NOR:  /*		Command : 0x60													*/
		case LCD_CMD_LOAD: /*		<S031> Command : 0x61											*/
		case LCD_CMD_SAVE: /*		<S031> Command : 0x62											*/
/*--------------------------------------------------------------------------------------------------*/
			LopV.CurMsgCmd = CmdBuf[1];							/* <S031> Current Command */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
			if( LopV.Online == LCD_ONLINE1 )
			{
				LcdVarInit( );
				/*	Set the initial value of the currently displayed axis number1(For now0if not2axis)	*/
				if(NumberOfAxes > 1)
				{	/*	2During axis motion1Start with axis selection	*/
					DrvSel.Unit[0].DispCurAxis = 1;
				}
				else
				{	/*	1During axis motion	*/
					DrvSel.Unit[0].DispCurAxis = 0;
				}
				/*	Select the currently displayed axis data	*/
				LcdDspDatSelAxs( DrvSel.Unit[0].DispCurAxis );
			}
#else /* <S0E4> */
			if( LopV.Online == LCD_OFFLINE )
			{
				LcdVarInit( );
			}
#endif /* <S0E4> */
			LopV.Online = LCD_ONLINE2;
	/*----------------------------------------------------------------------------------------------*/
	/*			Key Input																			*/
	/*----------------------------------------------------------------------------------------------*/
			LcdKeyTranslate( hMsg );
	/*----------------------------------------------------------------------------------------------*/
	/*			Alarm Reset 																		*/
	/*----------------------------------------------------------------------------------------------*/
			if( OpKeyMk.almrst )
			{
				KriResetSystemAlarm();
			}
	/*----------------------------------------------------------------------------------------------*/
	/*			Change Display Mode																	*/
	/*----------------------------------------------------------------------------------------------*/
			DrvSelEnd = LcdModeChg( pDrvSel );
	/*----------------------------------------------------------------------------------------------*/
	/*			Create Display Data 																*/
	/*----------------------------------------------------------------------------------------------*/
			LcdOpeProcTable[ LopV.OpMode ]( hMsg, pDrvSel );
			break;
	/*----------------------------------------------------------------------------------------------*/
		case LCD_CMD_DIAG:						/* 0x63: <S031>										*/
	/*----------------------------------------------------------------------------------------------*/
		default:
			Sif->StxRes( Sif, hMsg->ResBuf, 0 ); /* no response */
			return (DrvSelEnd);
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Send Response																				*/
/*--------------------------------------------------------------------------------------------------*/
	LcdResSend( hMsg );
/*--------------------------------------------------------------------------------------------------*/
/*	Drive Selection end																				*/
/*--------------------------------------------------------------------------------------------------*/
	if (DrvSelEnd) { LCD_TRANSMESSAGE(); }	
/*--------------------------------------------------------------------------------------------------*/
	return( DrvSelEnd );
}

#if		CCSW_HWID == HWID_JPCV
/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Main Program for Remote Operation												*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdOpRemoteMst( DRVSEL *pDrvSel )
{
LONG	DrvSelEnd = TRUE;
UCHAR	*CmdBuf = LopMsg.CmdBuf;
UCHAR	*ResBuf = LopMsg.ResBuf;
ULONG	idx = pDrvSel->DispCurIdx;
ULONG	TxSize = ResBuf[2];
UCHAR	*pBuf;
ULONG	Axis = pDrvSel->Unit[idx].DispCurAxis + 1;
USHORT	LedSts;
/*--------------------------------------------------------------------------------------------------*/
/*		Command analysis																				*/
/*--------------------------------------------------------------------------------------------------*/
	switch ( CmdBuf[1] )
	{
	/*----------------------------------------------------------------------------------------------*/
		case 0x64 : /*	Command : 0x64																*/
	/*----------------------------------------------------------------------------------------------*/
			LcdOpInit( );
	/*----------------------------------------------------------------------------------------------*/
	/*		Checksum set																		*/
	/*----------------------------------------------------------------------------------------------*/
			TxSize = LPX_GET_MSGLEN( ResBuf );					/* Get Msg Length					*/
			LPX_SET_MSGLEN( ResBuf, MlibCnvLittle16(TxSize) );	/* Convert Endian(Big/Little)		*/
			MlibSetSum16SM( ResBuf, TxSize );					/* Set ChkSum16 for SigmaMessage	*/
	/*----------------------------------------------------------------------------------------------*/
			DrvSelEnd = FALSE;
			Axis = 0xFF;
	/*----------------------------------------------------------------------------------------------*/
	/* 		Message transfer																			*/
	/*----------------------------------------------------------------------------------------------*/
			KriAddLbusMsgQueue ( LMQUEID_MSGTRANSFER, 
								 (Axis << 16) | pDrvSel->Unit[idx].Address,
								 sizeof(dummyMsg),
								 dummyMsg );
			break;

	/*----------------------------------------------------------------------------------------------*/
		case 0x60 :/*	Command : 0x60																*/
	/*----------------------------------------------------------------------------------------------*/
	/*			Key Input																			*/
	/*----------------------------------------------------------------------------------------------*/
			LcdKeyTranslate( hMsg );
	/*----------------------------------------------------------------------------------------------*/
	/* 		Response message confirmation																		*/
	/*----------------------------------------------------------------------------------------------*/
			if ( KriGetLbusMsg ( LMQUEID_MSGTRANSFER, &pBuf, &TxSize ) )
			{
				KpiRstLongTimer ( &LopV.StDispTime );
				MlibCopyByteMemory ( pBuf, ResBuf, TxSize);
			}
	/*----------------------------------------------------------------------------------------------*/
	/* 		Drive transfer mode switching																	*/
	/*----------------------------------------------------------------------------------------------*/
			switch ( LopV.DrvOpStat )
			{
	/*----------------------------------------------------------------------------------------------*/
				case DRVOPST_WAITKEYSEND: /* wait for mode key break */
	/*----------------------------------------------------------------------------------------------*/
					pBuf = dummyMsg;
					if ( OpKeyBrk.mode )
					{
						LopV.DrvOpStat = DRVOPST_SENDKEYINFO;
					}
					break;

	/*----------------------------------------------------------------------------------------------*/
				case DRVOPST_SENDKEYINFO: /* sending key information */
	/*----------------------------------------------------------------------------------------------*/
					pBuf = CmdBuf;
					if( OpKeyMk.read ) 
					{ 
						DrvSelEnd = FALSE; /* drive selection end */
						Axis = 0xFF; 
					}
					else if ( KpiGetLongTimer( &LopV.StDispTime ) > 5000 ) 
					{
						DrvSelEnd = FALSE; /* drive selection end */
						Axis = 0xFF; 
						LopV.OpMode = LCDOM_SelectUnit;
					} 
					break;

	/*----------------------------------------------------------------------------------------------*/
				case DRVOPST_FORCECON:  /* force connection */
	/*----------------------------------------------------------------------------------------------*/
					pBuf = CmdBuf;
					if ( (KpiGetLongTimer( &LopV.StDispTime ) > 5000) )
					{
						DrvSelEnd = FALSE;
						Axis = 0xFF;
						TxSize = 0;
					}
					else if ( OpKeyMk.read )
					{
						if ( RpxChkSelectedDrv(pDrvSel) )
						{
							LopV.OpMode = LCDOM_SelectUnit;
							DrvSelEnd = FALSE;
							Axis = 0xFF;
							LopMsg.LopAccessLvl = ACCLVL_SYSTEM; /* copy the access level to the converter */
						}
						else
						{
							OpBlnkKickFlg.no_op = 1;
						}
					}
					break;

	/*----------------------------------------------------------------------------------------------*/
				default: /* error */
	/*----------------------------------------------------------------------------------------------*/
					pBuf = dummyMsg;
					break;
			}
	/*----------------------------------------------------------------------------------------------*/
			LcdSetResBlink (ResBuf, &LedSts); 					/* Blink */
			MlibSetSum16SM( ResBuf, TxSize );					/* Set ChkSum16 for SigmaMessage	*/
	/*----------------------------------------------------------------------------------------------*/
	/* 		Message transfer																			*/
	/*----------------------------------------------------------------------------------------------*/
			KriAddLbusMsgQueue ( LMQUEID_MSGTRANSFER, 
								 (Axis << 16) | pDrvSel->Unit[idx].Address,
								 sizeof(dummyMsg),
								 pBuf );
			break;
	/*----------------------------------------------------------------------------------------------*/
		default:
			TxSize = 0; /* No Response */
			break;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Send Response																				*/
/*--------------------------------------------------------------------------------------------------*/
	LopMsg.StxResMsg( ResBuf, TxSize );						/* Transmission start command							*/
/*--------------------------------------------------------------------------------------------------*/
	return( DrvSelEnd );
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Finish Procedure																*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdOpFinish( MSGIF *hMsg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Finish FnXXX Function																		*/
/*--------------------------------------------------------------------------------------------------*/
		CLCDFUN->Finish( CLCDFUN, hMsg );		/* Finish Executing Function 						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set Online, PrmCopy, etc																	*/
/*--------------------------------------------------------------------------------------------------*/
		//LopV.Online = LCD_ONLINE1;			/* Since the font has been loaded onceONLINE1 	*/
		LopV.Online = LCD_OFFLINE;				/* 													*/
//TODO		RoutV.f.PrmCopy = FALSE;			/* Abort if parameter copying is in progress			*/
		pAxRsc->FnCmnCtrl->f.PrmCopy = FALSE;	/* <S031> Abort if parameter copying is in progress		*/
//TODO		LopMsg.LopAccessLvl = 0;			/* Access level clear							*/
#if		CCSW_HWID != HWID_JPCV
//TODO:		RoutV.f.ReadyBlink = FALSE;
#endif
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Init Display Info. Table														*/
/*																									*/
/****************************************************************************************************/
//void	RpxLcdOpInitDispInfo( void )
void	RpxLcdOpInitDispInfo( AXIS_HANDLE *Axis )	/* <S213> */
{
//		LcdInitDispInfoTable( );				/* Display information table initialization							*/
		LcdInitDispInfoTable( Axis );			/* Display information table initialization				 <S213> 	*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Initialize																				*/
/*																									*/
/****************************************************************************************************/
void	LcdOpInit( MSGIF *hMsg )
{
USHORT	i;
UCHAR	*pByte;
UCHAR	*CmdBuf = hMsg->CmdBuf;
UCHAR	*ResBuf = hMsg->ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Init Variables																				*/
/*--------------------------------------------------------------------------------------------------*/
		LcdVarInit( );
		RpxLcdPrmCopyInit( );
		LopV.Online = LCD_ONLINE1;
/*--------------------------------------------------------------------------------------------------*/
/*		64H Response message creation																		*/
/*--------------------------------------------------------------------------------------------------*/
		ResBuf[LCDMSGR_AXIDX]  = CmdBuf[0];					/* Axis address							*/
		ResBuf[LCDMSGR_FUNIDX] = CmdBuf[1];					/* Command								*/
		LPX_SET_MSGLEN( ResBuf, LCDMSGR_LEN64 );			/* Size(Byte length)						*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0, pByte=&(ResBuf[LCDMSGR_FONIDX64]); i < (5*16); i++ )
		{
			pByte[i] = LcdExtFont[i];
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LCD Operator variables Initialize															*/
/*																									*/
/****************************************************************************************************/
void	LcdVarInit( void )
{
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
	LOPV	*lcd_globals;
	lcd_globals = &LopV;
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Structure variableLcd																		*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory( (void *)&LopV, sizeof(LOPV) );

#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
		/*	Initialization of drive (axis) selection data pointer (LCD operation axis management data)	*/
		lcd_globals->pDrvSel = &DrvSel;
#endif

#if CCSW_CMDID == CMDID_EXIST			/*<11J42>*/
/*--------------------------------------------------------------------------------------------------*/
/* 		Structure variableLcdOpt																	*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory((void *)&LcdOpt, sizeof(LcdOpt));
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Non0set of values																				*/
/*--------------------------------------------------------------------------------------------------*/
		KpiRstLongTimer( &LopV.StDispTime );		/* For startup display time measurement							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Clear blink variable																		*/
/*--------------------------------------------------------------------------------------------------*/
		*((UINT *)&OpBlnkFlg) = 0;					/* Clear Blink Flag								*/
		*((UINT *)&OpBlnkKickFlg) = 0;				/* Clear Blink Kick Flag						*/
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Key Translation																*/
/*																									*/
/****************************************************************************************************/
void	LcdKeyTranslate( MSGIF *hMsg )
{
#define KEYHOLDCNTR (4)						/* @@ */
UCHAR	*CmdBuf = hMsg->CmdBuf;
USHORT	LcdRawKey = LPX_GET_LCDKEY( CmdBuf );

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		MlibCnvLittle16( LcdRawKey, LcdRawKey );
		*((UINT *)&OpKeyHld) = *((UINT *)&OpKeyMk) = *((UINT *)&OpKeyBrk) = *((UINT *)&OpKeyLvl) = 0;
/*--------------------------------------------------------------------------------------------------*/
		if( *((UINT *)&OpBlnkFlg) != 0 )	/* Ignore key input while blinking	*/
		{
			LopV.RawKeyLast = LcdRawKey; return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Key input level processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( LcdRawKey & LCDKEY_MODE		){ OpKeyLvl.mode   = TRUE;}
		if( LcdRawKey & LCDKEY_SHUP		){ OpKeyLvl.shup   = TRUE;}
		if( LcdRawKey & LCDKEY_UP		){ OpKeyLvl.up     = TRUE;}
		if( LcdRawKey & LCDKEY_DOWN		){ OpKeyLvl.down   = TRUE;}
		if( LcdRawKey & LCDKEY_SCRL		){ OpKeyLvl.scroll = TRUE;}
		if( LcdRawKey & LCDKEY_SVON		){ OpKeyLvl.svon   = TRUE;}
		if( LcdRawKey & LCDKEY_DATA		){ OpKeyLvl.data   = TRUE;}
		if( LcdRawKey & LCDKEY_SHDOWN	){ OpKeyLvl.shdown = TRUE;}
		if( LcdRawKey & LCDKEY_READ		){ OpKeyLvl.read   = TRUE;}
		if( LcdRawKey & LCDKEY_WRITE	){ OpKeyLvl.write  = TRUE;}
		if( LcdRawKey & LCDKEY_ALMRST	){ OpKeyLvl.almrst = TRUE;}
		if((LcdRawKey & LCDKEY_RST) == LCDKEY_RST ){ OpKeyLvl.rst = TRUE;}
/*--------------------------------------------------------------------------------------------------*/
/*		Set only when there is a change in key input(=Chattering prevention)										*/
/*--------------------------------------------------------------------------------------------------*/
		if( LcdRawKey != LopV.RawKeyLast )
		{
			LopV.KeyHoldCnt = 0;
		/*------------------------------------------------------------------------------------------*/
		/*		Key inputMAKE																	*/
		/*------------------------------------------------------------------------------------------*/
			switch( LcdRawKey )
			{
			case LCDKEY_MODE:	OpKeyMk.mode   = TRUE;	break;
			case LCDKEY_SHUP:	OpKeyMk.shup   = TRUE;	break;
			case LCDKEY_UP:		OpKeyMk.up     = TRUE;	break;
			case LCDKEY_DOWN:	OpKeyMk.down   = TRUE;	break;
			case LCDKEY_SCRL:	OpKeyMk.scroll = TRUE;	break;
			case LCDKEY_SVON:	OpKeyMk.svon   = TRUE;	break;
			case LCDKEY_DATA:	OpKeyMk.data   = TRUE;	break;
			case LCDKEY_SHDOWN:	OpKeyMk.shdown = TRUE;	break;
			case LCDKEY_READ:	OpKeyMk.read   = TRUE;	break;
			case LCDKEY_WRITE:	OpKeyMk.write  = TRUE;	break;
			case LCDKEY_ALMRST:	OpKeyMk.almrst = TRUE;	break;
			case LCDKEY_RST:	OpKeyMk.rst    = TRUE;	break;
			default: break;
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Key inputBREAK																	*/
		/*------------------------------------------------------------------------------------------*/
			switch( LopV.RawKeyLast )
			{
			case LCDKEY_MODE:	OpKeyBrk.mode   = TRUE;	break;
			case LCDKEY_SHUP:	OpKeyBrk.shup   = TRUE;	break;
			case LCDKEY_UP:		OpKeyBrk.up     = TRUE;	break;
			case LCDKEY_DOWN:	OpKeyBrk.down   = TRUE;	break;
			case LCDKEY_SCRL:	OpKeyBrk.scroll = TRUE;	break;
			case LCDKEY_SVON:	OpKeyBrk.svon   = TRUE;	break;
			case LCDKEY_DATA:	OpKeyBrk.data   = TRUE;	break;
			case LCDKEY_SHDOWN:	OpKeyBrk.shdown = TRUE; break;
			case LCDKEY_READ:	OpKeyBrk.read   = TRUE;	break;
			case LCDKEY_WRITE:	OpKeyBrk.write  = TRUE;	break;
			case LCDKEY_ALMRST:	OpKeyBrk.almrst = TRUE;	break;
			case LCDKEY_RST:	OpKeyBrk.rst    = TRUE;	break;
			default: break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		When there is no change in key input: Repeat, Hold processing											*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			switch( LcdRawKey )
			{
			case LCDKEY_SHUP:
				if(      LopV.KeyHoldCnt <  KEYHOLDCNTR ){ LopV.KeyHoldCnt++;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR ){ LopV.KeyHoldCnt++; OpKeyMk.shup = TRUE;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR+1  ){ LopV.KeyHoldCnt--;}
				break;
			case LCDKEY_UP:
				if(      LopV.KeyHoldCnt <  KEYHOLDCNTR ){ LopV.KeyHoldCnt++;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR ){ LopV.KeyHoldCnt++; OpKeyMk.up = TRUE;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR+1 ){ LopV.KeyHoldCnt--;}
				break;
			case LCDKEY_DOWN:
				if(      LopV.KeyHoldCnt <  KEYHOLDCNTR ){ LopV.KeyHoldCnt++;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR ){ LopV.KeyHoldCnt++; OpKeyMk.down = TRUE;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR+1 ){ LopV.KeyHoldCnt--;}
				break;
			case LCDKEY_SHDOWN:
				if(      LopV.KeyHoldCnt <  KEYHOLDCNTR ){ LopV.KeyHoldCnt++;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR ){ LopV.KeyHoldCnt++; OpKeyMk.shdown = TRUE;}
				else if( LopV.KeyHoldCnt == KEYHOLDCNTR+1 ){ LopV.KeyHoldCnt--;}
				break;
	/*----------------------------------------------------------------------------------------------*/
	/*		Set Key Hold Status (About 3 Second.)													*/
	/*----------------------------------------------------------------------------------------------*/
			case LCDKEY_READ:
				if(		LopV.KeyHoldCnt < 35 ){ LopV.KeyHoldCnt++;}
				else if ( LopV.KeyHoldCnt == 35){ LopV.KeyHoldCnt++; OpKeyHld.read = TRUE;}
				break;
			default:
				break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		LopV.RawKeyLast = LcdRawKey;
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Start Display																			*/
/*																									*/
/****************************************************************************************************/
void	LcdStartDisp( MSGIF *hMsg, DRVSEL *pDrvSel )
{
UCHAR	*ResBuf = hMsg->ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Display																						*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdClrTxBuf( ResBuf );
	RpxLcdSetStr( 1, 0, (UCHAR *)CSTR_INITDSP_L1,  ResBuf );
	RpxLcdSetStr( 2, 2, (UCHAR *)CSTR_INITDSP_L2,  ResBuf );
/*--------------------------------------------------------------------------------------------------*/
/*		<S031> Get Parameter Copy Information														*/
/*--------------------------------------------------------------------------------------------------*/
	LopV.PrmInfoGetDone = RpxLcdPrmInfoGet( );
/*--------------------------------------------------------------------------------------------------*/
	return;
}
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
/****************************************************************************************************/
/*																									*/
/*		Lcd Select Axis mode 																		*/
/*																									*/
/****************************************************************************************************/
void	LcdSelectAxis( MSGIF *hMsg, DRVSEL *pDrvSel )
{
AXIS_HANDLE		*pAxRsc;		/*	Axis unit structure	*/
LONG	i;
static	UCHAR	SelAxsNo = 1;	/*	Selected axis number (1 or 2) initial value	*/
UCHAR	*ResBuf = hMsg->ResBuf;
UCHAR	*pSetSts;	/* <S19B> */
/*--------------------------------------------------------------------------------------------------*/
/*		Lcd Select Axis mode																			*/
/*--------------------------------------------------------------------------------------------------*/
	/*	Caution: Number of units is set for now1pieces fixed	*/
	if( OpKeyBrk.data )
	{
		/*	Do nothing	*/
	}
	else if( OpKeyMk.almrst )
	{
		/*	Alarm clear processing	*/
		/*	Note: Only the currently selected axis is cleared for the time being	*/
//		KriResetSystemAlarm();
		pAxRsc = GetAxisHandle(SelAxsNo - 1);
		ALMSetAlarmResetRequest( pAxRsc->AlmManager );
	}
	else if( OpKeyMk.up )
	{
		--SelAxsNo;			/*Axis number-1*/
		if(SelAxsNo < 1)
		{	/*	To maximum axis number	*/
			SelAxsNo = NumberOfAxes;
		}
		/*	Select the currently displayed axis data	*/
		LcdDspDatSelAxs( SelAxsNo );
	}
	else if( OpKeyMk.down )
	{
		++SelAxsNo;			/*Axis number+1*/
		if(SelAxsNo > NumberOfAxes)
		{	/*	To the minimum axis number	*/
			SelAxsNo = 1;
		}
		/*	Select the currently displayed axis data	*/
		LcdDspDatSelAxs( SelAxsNo );
	}
	else if( OpKeyMk.scroll )
	{
		/*	Do nothing	*/
	}
	/*------------------------------------------------------------------------------------------*/
	/*		Display processing																			*/
	/*------------------------------------------------------------------------------------------*/
	RpxLcdClrTxBuf( ResBuf );
	RpxLcdSetStr( 0, 0, (UCHAR *)CSTR_SELAXS_TITLE, ResBuf );	/* TITLEDisplay	*/
	/*------------------------------------------------------------------------------------------*/
	/*	Axis selection screen display	*/
	for( i = 0; i < NumberOfAxes; i++ )					/* Display four lines */
	{
		RpxLcdSetStr( i+1, 0, (UCHAR *)"Axis-0", ResBuf );
		RpxLcdSetValHex( i+1, 6, i+1, 1, ResBuf );		/*	1Digital axis number display			*/
		DrvSel.Unit[0].DispCurAxis = i + 1;				/*	Alarm code axis number set	*/
		pAxRsc = GetAxisHandle(i);
		/*	Alarm code acquisition	*/
		DrvSel.Unit[0].Axis[i].AlmCode = ALMGetAlarmInfo( pAxRsc->AlmManager );
		if( DrvSel.Unit[0].Axis[i].AlmCode != 0x0000 )
		{	/*	1Display alarm codes for axes	*/
			RpxLcdSetStr(    i+1, 9, (UCHAR *)"A.", (UCHAR *)ResBuf);
			RpxLcdSetValHex( i+1, 11, DrvSel.Unit[0].Axis[i].AlmCode, 3, (UCHAR *)ResBuf);
		}else
		{	/* 1Display status codes for axes *//* <S19B> */
			/*--------------------------------------------------------------------------------------------------*/
			/*		Converter status display processing																*/
			/*--------------------------------------------------------------------------------------------------*/
					if( CHK_BB()				    ){ pSetSts = (UCHAR *)"*BB";	}
					else if( CHK_PDET() 			){ pSetSts = (UCHAR *)"*P DET";	}
					else							 { pSetSts = (UCHAR *)"*RUN";	}
				/*----------------------------------------------------------------------------------------------*/
					if( CHK_TUNING() 				){ pSetSts = (UCHAR *)"*ADJ";	}
				/*----------------------------------------------------------------------------------------------*/
					if( !CHK_JOGMODE() && !CHK_ZRETMODE() )
					{
						if( CHK_POT() && CHK_NOT()	){ pSetSts = (UCHAR *)"*PT NT";	}
						else if( CHK_POT()			){ pSetSts = (UCHAR *)"*P-OT";	}
						else if( CHK_NOT()			){ pSetSts = (UCHAR *)"*N-OT";	}
						else if( CHK_PLS()			){ pSetSts = (UCHAR *)"*P-LS";	}
						else if( CHK_NLS()			){ pSetSts = (UCHAR *)"*N-LS";	}
					}
				/*----------------------------------------------------------------------------------------------*/
					if( CHK_ESTOP()					){ pSetSts = (UCHAR *)"*ESTOP";	}
				/*----------------------------------------------------------------------------------------------*/
					if( CHK_FSTP()					){ pSetSts = (UCHAR *)"*FSTP";	} 		/* <S1F5> */
				/*----------------------------------------------------------------------------------------------*/
					if( CHK_HBB() 					){ pSetSts = (UCHAR *)"*HBB";	}
				/*----------------------------------------------------------------------------------------------*/
					/* If not in motorless test mode, close the pointer1Advance *//* <S1B7> */
//					if( 1 							){ pSetSts = pSetSts + 1;		}
					if( pAxRsc->MotorLessTest->MotorLessTestMode == FALSE ){ pSetSts = pSetSts + 1;		}
			/*--------------------------------------------------------------------------------------------------*/
			/*		Status display string setting process																*/
			/*--------------------------------------------------------------------------------------------------*/
//-					RpxLcdSetStr( 0, 2, pSetSts, ResBuf );
					RpxLcdSetStr(  i+1, 9, (UCHAR *)pSetSts, (UCHAR *)ResBuf );
			/*--------------------------------------------------------------------------------------------------*/
		}	/* <S19B> */
	}
	RpxLcdSetBlnk( SelAxsNo, 0, 17, (UCHAR *)ResBuf );		/* Blink current line */
/*-------------------------------------------------------------------------------------------------*/
/*	Blink & status display processing																*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdSetResStatus (ResBuf);
	RpxLcdSetBlink (ResBuf);
	/*	Set the final value of the currently displayed axis number	*/
	DrvSel.Unit[0].DispCurAxis = SelAxsNo;
	/*	Select the currently displayed axis data	*/
	LcdDspDatSelAxs( SelAxsNo );
	return;
}
/* <S19B> */
/****************************************************************************************************/
/*	Operator user constant display selection	Caution)"MsgMngr.c"insideLpxPrmGetOpeDspMode()						*/
/****************************************************************************************************/
USHORT PrmGetOpeDspMode( AXIS_HANDLE *pAxRsc)
{
	PRMDATA		*Prm;
	USHORT		rc;

	Prm = pAxRsc->UniPrms->Prm;		/* Parameter setting data */
	if( (Prm->b_prmB & 0x000F) == OPEDSP_SETUPPRM )
	{
		/* Display only user constants for setup */
		rc = OPEDSP_SETUPPRM;
	}
	else if( (Prm->b_prmB & 0x000F) == OPEDSP_ALLPRM )
	{
		/* Display all user constants */
		rc = OPEDSP_ALLPRM;
	}
	else
	{
		/* Do Nothing */
	}
	return(rc);
}	/* <S19B> */
/****************************************************************************************************/
/*		Lcd Display Data Select Axis 																*/
/*		Input -> axs_no  0/1:1Axis data selection2:2Axis data selection											*/
/****************************************************************************************************/
void	LcdDspDatSelAxs( UINT axs_no )
{
	/*	Select axis to display	*/
	if(axs_no != 0)
	{
		/*	Set data retrieval axis pointer	*/
		LopMsg.pAxRsc = GetAxisHandle(axs_no - 1);
		/*	Last axis index (=0 or 1) set of	*/
		LopMsg.LastAxisIdx = axs_no - 1;
	}else
	{
		/*	Data retrieval axis pointer1axis	*/
		LopMsg.pAxRsc = GetAxisHandle(0);
		/*	Last axis index (=0) set of	*/
		LopMsg.LastAxisIdx = 0;
	}
	/*	Selection is set, the parameters of the axis are set	*//* <S19B> */
	LopMsg.OpeDspMode = PrmGetOpeDspMode(LopMsg.pAxRsc);	/* Set User Paramete Display Mode	*/
}

#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*//* <S0E4> */
/****************************************************************************************************/
/*																									*/
/*		Lcd Select Drive Mode																		*/
/*																									*/
/****************************************************************************************************/
void	LcdSelectAxis( MSGIF *hMsg, DRVSEL *pDrvSel )
{
#if 0
UCHAR	*ResBuf = LopMsg.ResBuf;
LONG	idx = pDrvSel->DispCurIdx;
LONG 	i;
USHORT	LedSts;
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdClrTxBuf( ResBuf );
/*--------------------------------------------------------------------------------------------------*/
	if( OpKeyMk.up )
	{
		if( pDrvSel->Unit[idx].DispCurAxis > 0 )
		{
			pDrvSel->Unit[idx].DispCurAxis --;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	else if( OpKeyMk.down )
	{
		if( pDrvSel->Unit[idx].DispCurAxis < (pDrvSel->Unit[idx].AxisNumber - 1) )
		{
			pDrvSel->Unit[idx].DispCurAxis ++;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Display processing																					*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdSetStr( 0, 0, (UCHAR *)"-AXIS SELECTION-", ResBuf );
	for( i=0; i < 3; i++ )
	{
		if( i >= pDrvSel->Unit[idx].AxisNumber ){ break; }
/*------------------------------------------------------------------------------------------*/
		RpxLcdSetStr( i+1, 0, (UCHAR *)"Axis-", ResBuf );
		RpxLcdSetValHex( i+1, 5, i+1, 2, ResBuf );
/*------------------------------------------------------------------------------------------*/
		if( pDrvSel->Unit[idx].Axis[i].AlmCode != 0x0000 )
		{
			RpxLcdSetStr(    i+1, 9, (UCHAR *)"A.", (UCHAR *)ResBuf);
			RpxLcdSetValHex( i+1, 11, pDrvSel->Unit[idx].Axis[i].AlmCode, 3, (UCHAR *)ResBuf);
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Blink of selected drive																	*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdSetBlnk( (pDrvSel->Unit[idx].DispCurAxis + 1), 0, 8, ResBuf );
/*--------------------------------------------------------------------------------------------------*/
/*		blinking character display process																			*/
/*--------------------------------------------------------------------------------------------------*/
	LcdSetResBlink(ResBuf, &LedSts);
#if		CCSW_HWID == HWID_JPCV
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm reset																			*/
/*--------------------------------------------------------------------------------------------------*/
	if ( OpKeyMk.almrst )
	{
		ResBuf = LopMsg.CmdBuf;
	}
	else
	{
		ResBuf = dummyMsg;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		ReadyLedflash,Dummy											*/
/*--------------------------------------------------------------------------------------------------*/
	KriAddLbusMsgQueue ( LMQUEID_MSGTRANSFER,
				((pDrvSel->Unit[ idx ].DispCurAxis + 1) << 16) | pDrvSel->Unit[ idx ].Address,
				sizeof(dummyMsg),
				ResBuf );
#endif
#endif
	return;
}
#endif /* <S0E4S0E4> */


#if		CCSW_HWID == HWID_JPCV
void	LcdSelectUnit(MSGIF *hMsg, DRVSEL *pDrvSel )
{
UCHAR	*ResBuf = LopMsg.ResBuf;
LONG	idx = pDrvSel->DispCurIdx;
LONG i,j,k;
USHORT	LedSts;

/*--------------------------------------------------------------------------------------------------*/
	RpxLcdClrTxBuf( ResBuf );
/*--------------------------------------------------------------------------------------------------*/
	if( OpKeyMk.up )
	{
		if( pDrvSel->DispCurIdx > 0 )
		{
			pDrvSel->DispCurIdx--;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	else if( OpKeyMk.down )
	{
		if( pDrvSel->DispCurIdx < (pDrvSel->DispNumber - 1) )
		{
			pDrvSel->DispCurIdx++;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Update DispTopIdx																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( pDrvSel->DispCurIdx < pDrvSel->DispTopIdx )
	{
		pDrvSel->DispTopIdx--;
	}
	else if( pDrvSel->DispCurIdx >= (pDrvSel->DispTopIdx + 4) )
	{
		pDrvSel->DispTopIdx++;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Display processing																					*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdSetStr( 0, 0, (UCHAR *)"-UNIT SELECTION-", ResBuf );
	for( i=0; i < 4; i++ )
	{
		j = pDrvSel->DispTopIdx + i;
		if( j >= pDrvSel->DispNumber ){ break;}
/*------------------------------------------------------------------------------------------*/
		if( pDrvSel->Unit[j].Type == DRVSEL_TYPE_CONVERTER )
		{
			RpxLcdSetStr( i+1, 0, (UCHAR *)"Converter", ResBuf );
		}
		else if( pDrvSel->Unit[j].Type == DRVSEL_TYPE_COMMCPU )
		{
			RpxLcdSetStr( i+1, 0, (UCHAR *)"Comm. CPU", ResBuf );
		}
		else
		{
			RpxLcdSetStr( i+1, 0, (UCHAR *)"UNIT-Adr", ResBuf );
			RpxLcdSetValHex( i+1, 8, (pDrvSel->Unit[j].Address & (0x0F)), 2, ResBuf );
		}
		/*------------------------------------------------------------------------------------------*/
		for ( k=0; k < pDrvSel->Unit[j].AxisNumber; k++)
		{
			if( pDrvSel->Unit[j].Axis[k].AlmCode != 0x0000 )
			{
				RpxLcdSetStr(    i+1, 11, (UCHAR *)"A.", (UCHAR *)ResBuf);
				RpxLcdSetValHex( i+1, 13, pDrvSel->Unit[j].Axis[k].AlmCode, 3, (UCHAR *)ResBuf);
				break;
			}
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Blink of selected drive																	*/
/*--------------------------------------------------------------------------------------------------*/
	RpxLcdSetBlnk( (pDrvSel->DispCurIdx - pDrvSel->DispTopIdx + 1), 0, 10, ResBuf );
/*--------------------------------------------------------------------------------------------------*/
/*		blinking character display process																			*/
/*--------------------------------------------------------------------------------------------------*/
	LcdSetResBlink(ResBuf, &LedSts);
	return;
}
#endif

#if		CCSW_HWID == HWID_JPCV
/****************************************************************************************************/
/*																									*/
/*		Lcd Password input Mode																		*/
/*																									*/
/****************************************************************************************************/
void	LcdPasswd( MSGIF *hMsg, DRVSEL *pDrvSel )
{
USHORT	FnIdx = LopV.PasswdIdx;
UCHAR	*ResBuf = LopMsg.ResBuf;
USHORT	LedSts;

/*--------------------------------------------------------------------------------------------------*/
/*		Display processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		RpxLcdClrTxBuf( ResBuf );
		RpxFunExeLcd( OpKeyMk, OpKeyBrk, OpKeyLvl, FnIdx, ResBuf, &OpBlnkKickFlg, &LopMsg );
		RpxLcdSetStr( 0, 0, (UCHAR *)"-UNIT SELECTION-", ResBuf );		/* Display Title			*/
/*--------------------------------------------------------------------------------------------------*/
/*		blinking character display process																			*/
/*--------------------------------------------------------------------------------------------------*/
		LcdSetResBlink(ResBuf, &LedSts);
/*--------------------------------------------------------------------------------------------------*/
		return;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Lcd Error Mode																				*/
/*																									*/
/****************************************************************************************************/
void	LcdError( MSGIF *hMsg, DRVSEL *pDrvSel )
{
UCHAR	*ResBuf = LopMsg.ResBuf;
		LPX_SET_MSGLEN( ResBuf, 0);		/* size = zero	*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		TxBuf Clear																					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	[Caution]  0x60, after executing this routine and LcdResSend()Before calling					*/
/*			Size field(*(USHORT *)(&(ResBuf[LCDMSGR_LENIDX])))			*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdClrTxBuf(						/* Clear ResBuf(TxBuf)									*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
USHORT *pBuf;
/*--------------------------------------------------------------------------------------------------*/
/*		Message header,etc																		*/
/*--------------------------------------------------------------------------------------------------*/
		ResBuf[LCDMSGR_AXIDX]  = 0;						/* Axis address(Temporary)							*/
		ResBuf[LCDMSGR_FUNIDX] = 0x60;					/* Command(Temporary)								*/
		LPX_SET_MSGLEN( ResBuf, LCDMSGR_LEN60 );		/* 0x60Response size(Temporary)						*/
		LPX_SET_LEDSTS( ResBuf, 0 );					/* LEDDisplay information								*/
/*--------------------------------------------------------------------------------------------------*/
/*		Screen name : dotted line display, Display concentration, LCDBlinking settings, 2Set line font size								*/
/*--------------------------------------------------------------------------------------------------*/
		pBuf = (USHORT *)&ResBuf[LCDMSGR_DSPIDX];
		do { 
			*(pBuf++) = 0;                                         /* clear data */
		} while ( pBuf < ((USHORT *)&ResBuf[LCDMSGR_CHRIDX]) );
		ResBuf[LCDMSGR_DSPIDX] = 5;						           /* Screen display format	*/
/*--------------------------------------------------------------------------------------------------*/
/*		LCDCharacter settings																					*/
/*--------------------------------------------------------------------------------------------------*/
		do { 
			*(pBuf++) = 0x2020;                                    /* set blank (' ' == 0x20) */
		} while ( pBuf < ((USHORT *)&ResBuf[LCDMSGR_SUMIDX60]) );
/*--------------------------------------------------------------------------------------------------*/
		return;
}






/****************************************************************************************************/
/*																									*/
/*		Pn/Un Display Data Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetPnUnData(					/* Pn/Un Display Data Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	IsLong,						/* When using long parameterTRUE								*/
		USHORT	Base,						/* Bottom(Display attributes)											*/
		USHORT	Sign,						/* Sign presence/absence(Display attributes)									*/
		SHORT	Decimals,					/* Number of decimal places(Display attributes)								*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
UINT	digit;								/* <S18B> */
/*--------------------------------------------------------------------------------------------------*/
/*		Bit data display																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( Base == DSPDEF_BASEBIT )
		{
			RpxLcdSetBit( Lpos, Dpos, (UCHAR)Data, ResBuf );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Long data display																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( IsLong )
		{
			if( Base == DSPDEF_BASEDEC )	/* Comment out because the base detects10Base	*/
			{
				if( Sign == DSPDEF_W_SIGN )
				{
					Data = (ULONG)RpxLcdSetSignChar( Lpos, Dpos, (LONG)Data, ResBuf );
					digit = ((Decimals == 0) ? 10 : 9);			/* <S18B> */
					RpxLcdSetDecPnt( Lpos, (USHORT)(Dpos+1), Data, digit /*10*/, FALSE, Decimals, ResBuf );
				}
				else
				{
					RpxLcdSetDecPnt( Lpos, Dpos, Data, 10, FALSE, Decimals, ResBuf );
				}
			}
			else							/* Comment out because the base detects10Non-hexadecimal	*/
			{
				if( Base == DSPDEF_BASENBL )
				{
					RpxLcdSetStr( Lpos, Dpos, (UCHAR *)"n.", ResBuf );
				}
				else
				{
					RpxLcdSetStr( Lpos, Dpos, (UCHAR *)"H.", ResBuf );
				}
				RpxLcdSetValHex( Lpos, (USHORT)(Dpos+2), Data, 8, ResBuf );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Word data display																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( Base == DSPDEF_BASEDEC )	/* Comment out because the base detects10Base	*/
			{
				if(Sign == DSPDEF_W_SIGN)
				{
					Data = (ULONG)RpxLcdSetSignChar( Lpos, Dpos, (SHORT)Data, ResBuf );
					RpxLcdSetDecPnt( Lpos, (USHORT)(Dpos+1), Data, 5, FALSE, Decimals, ResBuf );
				}
				else
				{
					RpxLcdSetDecPnt( Lpos, Dpos, Data, 5, FALSE, Decimals, ResBuf );
				}
			}
			else							/* Comment out because the base detects10Non-hexadecimal	*/
			{
				if( Base == DSPDEF_BASENBL )
				{
					RpxLcdSetStr( Lpos, Dpos, (UCHAR *)"n.", ResBuf );
				}
				else
				{
					RpxLcdSetStr( Lpos, Dpos, (UCHAR *)"H.", ResBuf );
				}
				RpxLcdSetValHex( Lpos, (USHORT)(Dpos+2), Data, 4, ResBuf );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Signed10Hex number display																*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetSignedDecData(				/* Lcd Operator Signed10Hex number display						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{

/*--------------------------------------------------------------------------------------------------*/
/*		Set Sign and Dec String																		*/
/*--------------------------------------------------------------------------------------------------*/
		Data = RpxLcdSetSignChar( Lpos, Dpos, Data, ResBuf );
		RpxLcdSetDecPnt( Lpos, (USHORT)(Dpos+1), Data, Digit, SupFlg, Decimals, ResBuf );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator String Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetStr(						/* Lcd Operator String Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	*pStr,						/* Character string address to display								*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	Idx = LCDMSGR_CHRIDX + (Lpos*18) + Dpos;

/*--------------------------------------------------------------------------------------------------*/
/*		Set String																					*/
/*--------------------------------------------------------------------------------------------------*/
		while( *pStr != 0 )
		{
			ResBuf[Idx++] = *(pStr++);
			if( Idx >= LCDMSGR_SUMIDX60 ) break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Sign Char Set																	*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdSetSignChar(					/* Lcd Operator Sign Char Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Data to display										*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	Idx = LCDMSGR_CHRIDX + (Lpos*18) + Dpos;

/*--------------------------------------------------------------------------------------------------*/
/*		Set Sign String																				*/
/*--------------------------------------------------------------------------------------------------*/
		if(      Data >  0 ){ ResBuf[Idx] = '+';}
		else if( Data == 0 ){ ResBuf[Idx] = ' ';}
		else				{ ResBuf[Idx] = '-'; Data = -Data;}
/*--------------------------------------------------------------------------------------------------*/
		return( Data );
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Decimal w/point Set															*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetDecPnt(					/* Lcd Operator Decimal w/point Set						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	i,j;
ULONG	DivWk, DecWk;
LONG	Idx = LCDMSGR_CHRIDX + (Lpos*18) + Dpos;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Digit																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( (Digit == 0) || (10 < Digit) )
		{
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
		if( Digit < 10 )									/* Drop high-order digits						*/
		{
			if( Data > MlibDecPowerTbl[Digit] ){ Data %= MlibDecPowerTbl[Digit];}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set Dec String																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Digit; i++ )
		{
			j = Digit - 1 - i;								/* Digit Index							*/
			DivWk = MlibDecPowerTbl[j];						/* Number to divide: Example 1000, 100, 10, 1			*/
			DecWk = Data / DivWk;							/* Display Dec Value					*/
			Data  = Data - (DivWk * DecWk);					/* Update  Data for Next				*/
		/*------------------------------------------------------------------------------------------*/
			if( SupFlg && (DecWk == 0) )
			{
				ResBuf[Idx++] = ' ';						/* The upper order0is displayed blank			*/
			}
			else
			{
				SupFlg = FALSE;
				ResBuf[Idx++] = (UCHAR)('0' + DecWk);
			}
			if( (j != 0) && (j == Decimals) ){ ResBuf[Idx++] = '.';}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator HEX Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetValHex(					/* Lcd Operator HEX Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display											*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	i,j;
LONG	HexWk;
LONG	Idx = LCDMSGR_CHRIDX + (Lpos*18) + Dpos;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Digit																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( (Digit == 0) || (8 < Digit) )
		{
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
		if( Digit < 8 )
		{
			Data &= ((0x1L << (Digit*4)) -1);				/* Drop high-order digits						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set Hex String																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Digit; i++ )
		{
			j = Digit - 1 - i;								/* Digit Index							*/
			HexWk = (Data >> (j*4)) & 0x0F;					/* Display Hex Value					*/
			ResBuf[Idx+i] = (HexWk < 0xA)? (UCHAR)('0' + HexWk) : (UCHAR)('A' + (HexWk - 0xA));
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator BIT Set																		*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetBit(						/* Lcd Operator BIT Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	Data,						/* Display data											*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	i;
LONG	Idx = LCDMSGR_CHRIDX + (Lpos*18) + Dpos;

		for(i = 0; i < 8; i++)
		{
			ResBuf[Idx+i] = (Data & (0x1 << (8-i-1)))? 0x11: 0x10;
		}
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator LCD Blink Set																	*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetBlnk(						/* Lcd Operator LCD Blink Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		USHORT	Length,						/* Number of characters												*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
LONG	i;
ULONG	BlnkWk;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Arguments																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (Lpos >= 5) || (Dpos >= 17) || (Length == 0) )
		{
			return;		/* Argument is inappropriate	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set DposWk,LenWk																			*/
/*--------------------------------------------------------------------------------------------------*/
		Length = ((Dpos+Length) > 17)? (17-Dpos) : Length;		/* If it is too long, go to the right end			*/
		if( Dpos == 0 )
		{
			Length = (Length <= 2)? 1 : (Length - 1);			/* Number of blinking bits to set			*/
		}
		else
		{
			Dpos = Dpos - 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set Blink																					*/
/*--------------------------------------------------------------------------------------------------*/
		for( BlnkWk=0,i=0; i < Length; i++ )
		{
			BlnkWk |= ( 1 << (Dpos+i) );
		}
		MlibCnvLittle16( BlnkWk, BlnkWk );
		*(USHORT *)(&(ResBuf[LCDMSGR_DSPIDX+4+Lpos*2])) |= BlnkWk; 
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Operator LCD Blink Stop																	*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdClrLineBlnk(					/* Lcd Operator LCD Blink Stop							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
		if( Lpos >= 5 ){ return;}	/* Argument is inappropriate	*/
		*(USHORT *)(&(ResBuf[LCDMSGR_DSPIDX+4+Lpos*2])) = 0x0000;
		return;
}




#if	0	/* <S208> */
/****************************************************************************************************/
/*																									*/
/*		Initialize Display Inf. Table																*/
/*																									*/
/****************************************************************************************************/
void	LcdInitDispInfoTable( void )
{
LONG	i,rc;
LONG	dispID;
LONG	ChkSum;
LONG	IdxWrk;
//LCDEEP	LcdEep;
LCDEEP	LcdEep;			/* <S18F> */
USHORT	UnIdx;			/* <S18F> */
USHORT	PnIdx;			/* <S18F> */
#if		CCSW_HWID == HWID_JPCV
CLOPDITDEF	*pLopDitDef= LopDitDef_Rotary;
#else
CLOPDITDEF	*pLopDitDef= /* TODO: (Kprm.AmpType==AMPTYPE_LINEAR)? LopDitDef_Linear :*/ LopDitDef_Rotary;
#endif
		
#if 1 /* TODO */	
COBJECT *pObjMon = (COBJECT *)CUNMONX;
COBJECT *pObjPrm = (COBJECT *)CPARAMX;
		for(dispID = 0; dispID < LCDPMDSP_MAX; dispID++)
		{
			//KpiResetWatchdog( ); 
			for( i=0; i < 4; i++ )
			{
				LopDsp.InfTbl[dispID][i].PnUn = pLopDitDef[dispID].DspDef[i].PnUn;
			/*--------------------------------------------------------------------------------------*/
				pObjMon->Search( pObjMon, pLopDitDef[dispID].DspDef[i].UnIdx, 
									0, &IdxWrk, MCHK_NONE, &LopMsg );
				LopDsp.InfTbl[dispID][i].UnIdx = IdxWrk;
			/*--------------------------------------------------------------------------------------*/
				pObjPrm->Search( pObjPrm, pLopDitDef[dispID].DspDef[i].PnIdx, 
									0, &IdxWrk, PCHK_NONE, &LopMsg );
				LopDsp.InfTbl[dispID][i].PnIdx = IdxWrk;
			/*--------------------------------------------------------------------------------------*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		LopDsp.CurFunIndx = 0 /* TODO: RpxFunSearchIdxOp( LcdEep.Old_FnNo, 0, &LopMsg ) */;
/*--------------------------------------------------------------------------------------------------*/
#endif
//#if 0 /* minaosi */
#if 1	/* <S18F> */
/*--------------------------------------------------------------------------------------------------*/
/*		ROMInitialize with default value (HerePn/UnConvert the number to an index)							*/
/*--------------------------------------------------------------------------------------------------*/
		for(dispID = 0; dispID < LCDPMDSP_MAX; dispID++)
		{
			/* it takes about 600 us while the cpu processes loop below */
			//KpiResetWatchdog( ); /* <0BJ14> */	/* <S18F> */
			/*--------------------------------------------------------------------------------------*/
			for( i=0; i < 4; i++ )
			{
			/*--------------------------------------------------------------------------------------*/
				LopDsp.InfTbl[dispID][i].PnUn = pLopDitDef[dispID].DspDef[i].PnUn;
			/*--------------------------------------------------------------------------------------*/
				//rc = RpiMonSearch( pLopDitDef[dispID].DspDef[i].UnIdx, 0, &IdxWrk, MCHK_NONE, &LopMsg );
				UnIdx = pLopDitDef[dispID].DspDef[i].UnIdx;									/* <S18F> */
				rc = CUNMONX->Search( (COBJECT *)CUNMONX, UnIdx, 0, &IdxWrk, MCHK_NONE, &LopMsg );		/* <S18F> */
				LopDsp.InfTbl[dispID][i].UnIdx = (rc==OK)? IdxWrk : 0;
			/*--------------------------------------------------------------------------------------*/
				//rc = RpxPrmSearchIdx( pLopDitDef[dispID].DspDef[i].PnIdx, 0, &IdxWrk, PCHK_NONE, &LopMsg );
				PnIdx = pLopDitDef[dispID].DspDef[i].PnIdx;									/* <S18F> */
				rc = CPARAMX->Search( (COBJECT *)CPARAMX, PnIdx, 0, &IdxWrk, PCHK_NONE, &LopMsg );		/* <S18F> */
				LopDsp.InfTbl[dispID][i].PnIdx = (rc==OK)? IdxWrk : 0;
			/*--------------------------------------------------------------------------------------*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMReading																				*/
/*--------------------------------------------------------------------------------------------------*/
		//KriReadEepromData( EEPADR_LCDINF, &LcdEep, LCDEEP_WDSIZE );
		KriReadEepromData( EEPADR_LCDINF, (USHORT*)(&LcdEep), LCDEEP_WDSIZE );		/* <S18F> */
		for( ChkSum=0, i=0; i < LCDEEP_WDSIZE; i++ )
		{
			ChkSum += *((USHORT *)(&LcdEep) + i);
		}
		if( (USHORT)ChkSum != 0 ){ return;}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMOverwrite with information																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( dispID=0; dispID < LCDPMDSP_MAX; dispID++ )
		{
			if( (LcdEep.accesslvl[dispID] != 0) && 					/* Not invalid data				*/
				(LopMsg.AccessLvl >= LcdEep.accesslvl[dispID]) &&	/* Access level is correct	*/
				( ( (LopMsg.AccessLvl == ACCLVL_USER1) &&			/* User constant selection display possible		*/
				    /*( (Iprm.OpeDspMode == OPEDSP_ALLPRM) ||*/
				    ( (LopMsg.OpeDspMode == OPEDSP_ALLPRM) ||		/* <S18F> */
				      (LcdEep.Old_OpeDspMode == OPEDSP_SETUPPRM) ) ) ||
				  (LopMsg.AccessLvl >= ACCLVL_USER2) ) )
			{
				//KpiResetWatchdog( ); /* <0BJ14> */	/* <S18F> */
				/*----------------------------------------------------------------------------------*/
				for( i=0; i < 4; i++ )
				{
					LONG ValidFlag = FALSE;
					//if(	RpiMonSearch( LcdEep.dispinf[dispID].UnNo[i],
					//		0, &IdxWrk, MCHK_ACCS_LVL | MCHK_SP_USE, &LopMsg ) == OK	)
					UnIdx = LcdEep.dispinf[dispID].UnNo[i];									/* <S18F> */
					if( CUNMONX->Search( (COBJECT *)CUNMONX, UnIdx,							/* <S18F> */
							0, &IdxWrk, MCHK_ACCS_LVL | MCHK_SP_USE, &LopMsg ) == OK )		/* <S18F> */
					{
						ValidFlag = TRUE;
						LopDsp.InfTbl[dispID][i].UnIdx = IdxWrk;
					}
					/*------------------------------------------------------------------------------*/
					//if( RpxPrmSearchIdx( LcdEep.dispinf[dispID].PnNo[i],
					//		0, &IdxWrk, PCHK_ACCS_LVL | PCHK_SP_USE, &LopMsg ) == OK	)
					PnIdx = LcdEep.dispinf[dispID].PnNo[i];									/* <S18F> */
					if( CPARAMX->Search( (COBJECT *)CPARAMX, PnIdx,							/* <S18F> */
							0, &IdxWrk, PCHK_ACCS_LVL | PCHK_SP_USE, &LopMsg ) == OK )		/* <S18F> */
					{
						ValidFlag = TRUE;
						LopDsp.InfTbl[dispID][i].PnIdx = IdxWrk;
					}
					/*------------------------------------------------------------------------------*/
					if( ValidFlag )		/* Pn or Un is enabledPn/UnSet type	*/
					{
						LopDsp.InfTbl[dispID][i].PnUn = LcdEep.dispinf[dispID].PnUn[i];
					}
				}
				/*----------------------------------------------------------------------------------*/
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		//LopDsp.CurFunIndx = RpxFunSearchIdxOp( LcdEep.Old_FnNo, 0, &LopMsg );
		LopDsp.CurFunIndx = CLCDFUN->SearchB( CLCDFUN, LcdEep.Old_FnNo, 0, &LopMsg );	/* <S18F> */
/*--------------------------------------------------------------------------------------------------*/
#endif
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Save Display Inf. Table																		*/
/*																									*/
/****************************************************************************************************/
SHORT	LcdSaveDispInfoTable( USHORT dispID )
{
//#if 0 /* minaosi */
#if 1	/* <S18F> */
LONG	i;
LONG	ChkSum;
LCDEEP	LcdEep;
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
LOPDIT	*LcdInfTblDef;
USHORT	first_lcdinf_save = FALSE;
		/* First time after power-onWRITEWhen key is pressed */
		if( LcdOpt[dispID].f.LcdInfSaved == FALSE )
		{
			/* Default LcdInfTbl	*/
			if( Kprm.AmpType == AMPTYPE_LINEAR )
			{
				LcdInfTblDef = (LOPDIT *)(&(LopDitDef_Linear[dispID]));
			}
			else
			{
				LcdInfTblDef = (LOPDIT *)(&(LopDitDef_Rotary[dispID]));
			}
			/* First timeWRITEKey press flag clear */
			LcdOpt[dispID].f.LcdInfSaved = TRUE;
			first_lcdinf_save = TRUE;
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMReading																				*/
/*--------------------------------------------------------------------------------------------------*/
		//KriReadEepromData( EEPADR_LCDINF, &LcdEep, LCDEEP_WDSIZE );
		KriReadEepromData( EEPADR_LCDINF, (USHORT*)(&LcdEep), LCDEEP_WDSIZE );		/* <S18F> */
		for( ChkSum=0, i=0; i < LCDEEP_WDSIZE; i++ )
		{
			ChkSum += *((USHORT *)(&LcdEep) + i);
		}
		if( (USHORT)ChkSum != 0 )
		{
			for( i=0; i < LCDPMDSP_MAX; i++ )				/* If the sum does not match,				*/
			{												/* Information is treated as invalid					*/
				LcdEep.accesslvl[i] = 0;					/* (dsipIDshould be used later)				*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMWrite																				*/
/*--------------------------------------------------------------------------------------------------*/
		LcdEep.accesslvl[dispID] = LopMsg.AccessLvl;
		//LcdEep.Old_OpeDspMode = Iprm.OpeDspMode;
		LcdEep.Old_OpeDspMode = LopMsg.OpeDspMode;									/* <S18F> */
		for( i=0; i < 4; i++ )
		{
#if CCSW_CMDID == CMDID_EXIST
			if( LcdOpt[dispID].Prm[i].State.Editing == TRUE )
			{
				if( first_lcdinf_save == TRUE )
				{	/* The line where option parameters are being edited isROMSet default value */
					LcdEep.dispinf[dispID].PnUn[i] = LcdInfTblDef[i].PnUn;
					LcdEep.dispinf[dispID].UnNo[i] = LcdInfTblDef[i].UnIdx;	/* UnNumber */
					LcdEep.dispinf[dispID].PnNo[i] = LcdInfTblDef[i].PnIdx;	/* PnNumber */
				}
			}
			else
			{
#endif
#if 0			/* <S18F> */
				LcdEep.dispinf[dispID].PnUn[i] = LopDsp.InfTbl[dispID][i].PnUn;
				LcdEep.dispinf[dispID].UnNo[i] = UnMonTbl[LopDsp.InfTbl[dispID][i].UnIdx].UnNo;
				LcdEep.dispinf[dispID].PnNo[i] = PnPrmTbl[LopDsp.InfTbl[dispID][i].PnIdx].PnNo;
#else
				USHORT	Numx;
				USHORT	UnIdx = LopDsp.InfTbl[dispID][i].UnIdx;
				USHORT	PnIdx = LopDsp.InfTbl[dispID][i].PnIdx;
				LcdEep.dispinf[dispID].PnUn[i] = LopDsp.InfTbl[dispID][i].PnUn;
				Numx = CUNMONX->GetNumber( (CMONITOR *)CUNMONX, UnIdx, &LopMsg );
				LcdEep.dispinf[dispID].UnNo[i] = Numx;
				Numx = CPARAMX->GetNumber( (CPARAM *)CPARAMX, PnIdx, &LopMsg );
				LcdEep.dispinf[dispID].PnNo[i] = Numx;
#endif
#if CCSW_CMDID == CMDID_EXIST
			}
#endif
		}
	/*----------------------------------------------------------------------------------------------*/
		for( ChkSum=0, i=0; i < (LCDEEP_WDSIZE - 1); i++ )
		{
			ChkSum += *((USHORT *)(&LcdEep) + i);
		}
		LcdEep.chksum = (USHORT)(-ChkSum);
		//KriWriteEepromData( EEPADR_LCDINF, &LcdEep, LCDEEP_WDSIZE );
		KriWriteEepromData( EEPADR_LCDINF, (USHORT*)(&LcdEep), LCDEEP_WDSIZE );			/* <S18F> */
/*--------------------------------------------------------------------------------------------------*/
#endif
		return( OK );
}

#else	/* <S208> */
/****************************************************************************************************/
/*																									*/
/*		Initialize Display Inf. Table																*/
/*																									*/
/****************************************************************************************************/
void	LcdInitDispInfoTable( AXIS_HANDLE *Axis )
{
LONG	i;
LONG	rc;
LONG	ax;
LONG	dispID;
LONG	ChkSum;
LONG	IdxWrk;
LONG	ValidFlag;
USHORT	UnIdx;
USHORT	PnIdx;

//CLOPDITDEF	*pLopDitDef= LopDitDef_Rotary;
CLOPDITDEF	*pLopDitDef;									/* <S214> */
AXIS_HANDLE *AxisRsc;										/* <S214> */

	for( ax = 0 ; ax < MAX_AXIS_NUMBER ; ax++ )
	{
/* <S214> */
		AxisRsc = &Axis[ax];
		if( AxisRsc->MencV->AxisMotType == MOTTYPE_LINEAR )	{ pLopDitDef = LopDitDef_Linear; }
		else												{ pLopDitDef = LopDitDef_Rotary; }
/* <S214> */
/*--------------------------------------------------------------------------------------------------*/
/*		ROMInitialize with default value (HerePn/UnConvert the number to an index)							*/
/*--------------------------------------------------------------------------------------------------*/
		for( dispID = 0 ; dispID < LCDPMDSP_MAX ; dispID++ )
		{
			/*--------------------------------------------------------------------------------------*/
			for( i = 0 ; i < 4 ; i++ )
			{
			/*--------------------------------------------------------------------------------------*/
				LopDsp.InfTbl[dispID][i].PnUn = pLopDitDef[dispID].DspDef[i].PnUn;
			/*--------------------------------------------------------------------------------------*/
				UnIdx = pLopDitDef[dispID].DspDef[i].UnIdx;
				rc = CUNMONX->Search( (COBJECT *)CUNMONX, UnIdx, 0, &IdxWrk, MCHK_NONE, &LopMsg );
				LopDsp.InfTbl[dispID][i].UnIdx = ( rc == OK )? IdxWrk : 0;
			/*--------------------------------------------------------------------------------------*/
				PnIdx = pLopDitDef[dispID].DspDef[i].PnIdx;
				rc = CPARAMX->Search( (COBJECT *)CPARAMX, PnIdx, 0, &IdxWrk, PCHK_NONE, &LopMsg );
				LopDsp.InfTbl[dispID][i].PnIdx = ( rc == OK )? IdxWrk : 0;
			/*--------------------------------------------------------------------------------------*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMReading																				*/
/*--------------------------------------------------------------------------------------------------*/
		KriReadEepromData( EEP_NOSUM_ADDRESS( ax, EEP_LCDINF_OFS ), (USHORT*)(&LcdEep[ax]), LCDEEP_WDSIZE );
		for( ChkSum = 0, i = 0 ; i < LCDEEP_WDSIZE ; i++ )
		{
			ChkSum += *((USHORT *)(&LcdEep) + i);
		}
		if( (USHORT)ChkSum == 0 )
		{
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMOverwrite with information																			*/
/*--------------------------------------------------------------------------------------------------*/
			for( dispID = 0 ; dispID < LCDPMDSP_MAX ; dispID++ )
			{
				if(( LcdEep[ax].accesslvl[dispID] != 0 ) && 			/* Not invalid data			*/
				   ( LopMsg.AccessLvl >= LcdEep[ax].accesslvl[dispID] ) &&	/* Access level is correct	*/
				   ((( LopMsg.AccessLvl == ACCLVL_USER1 ) &&			/* User constant selection display possible		*/
				     (( LopMsg.OpeDspMode == OPEDSP_ALLPRM ) ||
				      ( LcdEep[ax].Old_OpeDspMode == OPEDSP_SETUPPRM ))) ||
				   ( LopMsg.AccessLvl >= ACCLVL_USER2 )))
				{
					/*------------------------------------------------------------------------------*/
					for( i = 0; i < 4 ; i++ )
					{
						ValidFlag = FALSE;
						UnIdx = LcdEep[ax].dispinf[dispID].UnNo[i];
						if( CUNMONX->Search( (COBJECT *)CUNMONX, UnIdx,
								0, &IdxWrk, MCHK_ACCS_LVL | MCHK_SP_USE, &LopMsg ) == OK )
						{
							ValidFlag = TRUE;
							LopDsp.InfTbl[dispID][i].UnIdx = IdxWrk;
						}
						/*--------------------------------------------------------------------------*/
						PnIdx = LcdEep[ax].dispinf[dispID].PnNo[i];
						if( CPARAMX->Search( (COBJECT *)CPARAMX, PnIdx,
								0, &IdxWrk, PCHK_ACCS_LVL | PCHK_SP_USE, &LopMsg ) == OK )
						{
							ValidFlag = TRUE;
							LopDsp.InfTbl[dispID][i].PnIdx = IdxWrk;
						}
						/*--------------------------------------------------------------------------*/
						if( ValidFlag )		/* Pn or Un is enabledPn/UnSet type	*/
						{
							LopDsp.InfTbl[dispID][i].PnUn = LcdEep[ax].dispinf[dispID].PnUn[i];
						}
					}
					/*------------------------------------------------------------------------------*/
				}
			}
		/*------------------------------------------------------------------------------------------*/
			LopDsp.CurFunIndx = CLCDFUN->SearchB( CLCDFUN, LcdEep[ax].Old_FnNo, 0, &LopMsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMInformation initialization																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( dispID = 0 ; dispID < LCDPMDSP_MAX ; dispID++ )
		{
			/*--------------------------------------------------------------------------------------*/
			for( i = 0 ; i < 4 ; i++ )
			{
				UnIdx = LopDsp.InfTbl[dispID][i].UnIdx;
				PnIdx = LopDsp.InfTbl[dispID][i].PnIdx;
				LcdEep[ax].dispinf[dispID].PnUn[i] = LopDsp.InfTbl[dispID][i].PnUn;
				IdxWrk = CUNMONX->GetNumber( (CMONITOR *)CUNMONX, UnIdx, &LopMsg );
				LcdEep[ax].dispinf[dispID].UnNo[i] = IdxWrk;
				IdxWrk = CPARAMX->GetNumber( (CPARAM *)CPARAMX, PnIdx, &LopMsg );
				LcdEep[ax].dispinf[dispID].PnNo[i] = IdxWrk;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		for( ChkSum = 0, i = 0 ; i < ( LCDEEP_WDSIZE - 1 ) ; i++ )
		{
			ChkSum += *((USHORT *)( &LcdEep[ax] ) + i );
		}
		LcdEep[ax].chksum = (USHORT)(-ChkSum);
	/*----------------------------------------------------------------------------------------------*/
	}
	/*----------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Save Display Inf. Table																		*/
/*																									*/
/****************************************************************************************************/
SHORT	LcdSaveDispInfoTable( USHORT dispID )
{
LONG	i;
LONG	ChkSum;
USHORT	Numx;
USHORT	UnIdx;
USHORT	PnIdx;

#if CCSW_CMDID == CMDID_EXIST
LOPDIT	*LcdInfTblDef;
USHORT	first_lcdinf_save = FALSE;
		/* First time after power-onWRITEWhen key is pressed */
		if( LcdOpt[dispID].f.LcdInfSaved == FALSE )
		{
			/* Default LcdInfTbl	*/
			if( Kprm.AmpType == AMPTYPE_LINEAR )
			{
				LcdInfTblDef = (LOPDIT *)(&(LopDitDef_Linear[dispID]));
			}
			else
			{
				LcdInfTblDef = (LOPDIT *)(&(LopDitDef_Rotary[dispID]));
			}
			/* First timeWRITEKey press flag clear */
			LcdOpt[dispID].f.LcdInfSaved = TRUE;
			first_lcdinf_save = TRUE;
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
		if( LcdWtReq == TRUE )	return( NG );					/* Writing in progress						*/
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMReading																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( ChkSum=0, i = 0 ; i < LCDEEP_WDSIZE ; i++ )
		{
			ChkSum += *((USHORT *)( pLcdEep ) + i );
		}
		if( (USHORT)ChkSum != 0 )
		{
			for( i = 0; i < LCDPMDSP_MAX ; i++ )			/* If the sum does not match,				*/
			{												/* Information is treated as invalid					*/
				pLcdEep->accesslvl[i] = 0;					/* (dsipIDshould be used later)				*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMWriting edit																			*/
/*--------------------------------------------------------------------------------------------------*/
		pLcdEep->accesslvl[dispID] = LopMsg.AccessLvl;
		pLcdEep->Old_OpeDspMode    = LopMsg.OpeDspMode;
		for( i = 0 ; i < 4 ; i++ )
		{
#if CCSW_CMDID == CMDID_EXIST
			if( LcdOpt[dispID].Prm[i].State.Editing == TRUE )
			{
				if( first_lcdinf_save == TRUE )
				{	/* The line where option parameters are being edited isROMSet default value */
					pLcdEep->dispinf[dispID].PnUn[i] = LcdInfTblDef[i].PnUn;
					pLcdEep->dispinf[dispID].UnNo[i] = LcdInfTblDef[i].UnIdx;	/* UnNumber */
					pLcdEep->dispinf[dispID].PnNo[i] = LcdInfTblDef[i].PnIdx;	/* PnNumber */
				}
			}
			else
			{
#endif
				UnIdx = LopDsp.InfTbl[dispID][i].UnIdx;
				PnIdx = LopDsp.InfTbl[dispID][i].PnIdx;
				pLcdEep->dispinf[dispID].PnUn[i] = LopDsp.InfTbl[dispID][i].PnUn;
				Numx = CUNMONX->GetNumber( (CMONITOR *)CUNMONX, UnIdx, &LopMsg );
				pLcdEep->dispinf[dispID].UnNo[i] = Numx;
				Numx = CPARAMX->GetNumber( (CPARAM *)CPARAMX, PnIdx, &LopMsg );
				pLcdEep->dispinf[dispID].PnNo[i] = Numx;
#if CCSW_CMDID == CMDID_EXIST
			}
#endif
		}
	/*----------------------------------------------------------------------------------------------*/
		for( ChkSum=0, i=0; i < (LCDEEP_WDSIZE - 1); i++ )
		{
			ChkSum += *((USHORT *)( pLcdEep ) + i );
		}
		pLcdEep->chksum = (USHORT)(-ChkSum);
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMWriting request																			*/
/*--------------------------------------------------------------------------------------------------*/
		LcdRomAdr = LcdRomBaseAdr;
		LcdMemAdr = (USHORT*)( pLcdEep );
		LcdWtCnt  = LCDEEP_WDSIZE;
		LcdWtReq  = TRUE;
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}


/****************************************************************************************************/
/*																									*/
/*		Display table load																			*/
/*																									*/
/****************************************************************************************************/
void	LopDspDataLoad( LONG AxisNo )
{
LONG	i;
LONG	dispID;
LONG	IdxWrk;
LONG	ValidFlag;
USHORT	UnIdx;
USHORT	PnIdx;
	
	pLcdEep       = &LcdEep[ AxisNo ];
	LcdRomBaseAdr = EEP_NOSUM_ADDRESS( AxisNo, EEP_LCDINF_OFS );

	for( dispID = 0 ; dispID < LCDPMDSP_MAX ; dispID++ )
	{
		/*------------------------------------------------------------------------------------------*/
		for( i = 0; i < 4 ; i++ )
		{
			ValidFlag = FALSE;
			UnIdx = pLcdEep->dispinf[dispID].UnNo[i];
			if( CUNMONX->Search( (COBJECT *)CUNMONX, UnIdx,
					0, &IdxWrk, MCHK_ACCS_LVL | MCHK_SP_USE, &LopMsg ) == OK )
			{
				ValidFlag = TRUE;
				LopDsp.InfTbl[dispID][i].UnIdx = IdxWrk;
			}
			/*--------------------------------------------------------------------------*/
			PnIdx = pLcdEep->dispinf[dispID].PnNo[i];
			if( CPARAMX->Search( (COBJECT *)CPARAMX, PnIdx,
					0, &IdxWrk, PCHK_ACCS_LVL | PCHK_SP_USE, &LopMsg ) == OK )
			{
				ValidFlag = TRUE;
				LopDsp.InfTbl[dispID][i].PnIdx = IdxWrk;
			}
			/*--------------------------------------------------------------------------*/
			if( ValidFlag )		/* Pn or Un is enabledPn/UnSet type	*/
			{
				LopDsp.InfTbl[dispID][i].PnUn = pLcdEep->dispinf[dispID].PnUn[i];
			}
		}
		/*------------------------------------------------------------------------------*/
	}
		/*------------------------------------------------------------------------------------------*/
	LopDsp.CurFunIndx = CLCDFUN->SearchB( CLCDFUN, pLcdEep->Old_FnNo, 0, &LopMsg );
}


/****************************************************************************************************/
/*																									*/
/*		EEPROMWrite																				*/
/*																									*/
/****************************************************************************************************/
void	LcdEep2EepRomWrite( UINT devId )
{
	if( LcdWtReq == TRUE )
	{
		if( LcdWtCnt != 0 )
		{
			EepdevWriteNoSumValues( devId, LcdRomAdr, LcdMemAdr, 1 );
			LcdMemAdr += 1;
			LcdRomAdr += 2;
			LcdWtCnt--;
		}
		if( LcdWtCnt == 0 )
		{
			LcdWtReq = FALSE;
		}
	}
}
/*--------------------------------------------------------------------------------------------------*/
#endif	/* <S208> */

/****************************************************************************************************/
/*																									*/
/*		Lcd Response Send																			*/
/*																									*/
/****************************************************************************************************/
void	LcdResSend( MSGIF *hMsg )
{
LONG	TxSize;
//USHORT	LedSts  = 0;//<S0C7>
UCHAR	*CmdBuf = hMsg->CmdBuf;
UCHAR	*ResBuf = hMsg->ResBuf;
CSIF *Sif = hMsg->Sif;
/*--------------------------------------------------------------------------------------------------*/
/*		Blink and status are created during each screen process.											*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		Header section set																				*/
/*--------------------------------------------------------------------------------------------------*/
		ResBuf[LCDMSGR_AXIDX]  = CmdBuf[0];					/* Axis address							*/
		ResBuf[LCDMSGR_FUNIDX] = LopV.CurMsgCmd /* CmdBuf[1] */;/* <S031> Command					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Checksum set																			*/
/*--------------------------------------------------------------------------------------------------*/
		TxSize = LPX_GET_MSGLEN( ResBuf );					/* Get Tx Size 							*/
		MlibCnvLittle16( TxSize, TxSize );					/* Convert Endian(Big/Little)			*/
		LPX_SET_MSGLEN( ResBuf, TxSize  );					/* Set Message Length 					*/
		MlibSetSum16SM( ResBuf, TxSize );					/* Set ChkSum16 for SigmaMessage		*/
/*--------------------------------------------------------------------------------------------------*/
/*		Transmission command																					*/
/*--------------------------------------------------------------------------------------------------*/
		Sif->StxRes( Sif, ResBuf, TxSize );					/* Transmission start command							*/
		KpiRstLongTimer( &LopMsg.ComChkTime );				/* For RES/CMD Timeout check			*/
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Send Response : Set Status																*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdSetResStatus( UCHAR *ResBuf )					/* <S031> */
{
USHORT	CurAlarm;
UCHAR	*pSetSts;
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
if(LopV.OpMode == LCDOM_SelectAxis)
{	/*	On axis selection screen1Maintain the current status of line title	*/
	RpxLcdSetStr(0, 0, (UCHAR *)CSTR_SELAXS_TITLE, (UCHAR *)ResBuf);
	return;
}
else
{	/*	Other than axis selection screen	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Axis number display																					*/
/*--------------------------------------------------------------------------------------------------*/
	/*	Convert axis number to ASCII code	*/
	ResBuf[LCDMSGR_CHRIDX+0] = DrvSel.Unit[0].DispCurAxis + 0x30 ;
	ResBuf[LCDMSGR_CHRIDX+1] = ':';
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm code/warning code display														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Set pointer of selected axis and obtain alarm information *//* <S1A4> */
	pAxRsc = (AXIS_HANDLE*)GetAxisHandle(DrvSel.Unit[0].DispCurAxis - 1);
	if( (CurAlarm = KpiGetAlarmInfo()) != 0x0000 )
	{
		RpxLcdSetStr(0, 2, (UCHAR *)"A.", (UCHAR *)ResBuf);
		RpxLcdSetValHex(0, 4, CurAlarm, 3, (UCHAR *)ResBuf);
		return;
	}
}
#else
/*--------------------------------------------------------------------------------------------------*/
/*		Axis number display																					*/
/*--------------------------------------------------------------------------------------------------*/
	ResBuf[LCDMSGR_CHRIDX+0] = CCHR_AXISNUM ;
	ResBuf[LCDMSGR_CHRIDX+1] = ':';
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm code/warning code display														*/
/*--------------------------------------------------------------------------------------------------*/
	if( (CurAlarm = KpiGetAlarmInfo()) != 0x0000 )
	{
		RpxLcdSetStr(0, 2, (UCHAR *)"A.", (UCHAR *)ResBuf);
		RpxLcdSetValHex(0, 4, CurAlarm, 3, (UCHAR *)ResBuf);
		return;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Converter status display processing																*/
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_BB()				    ){ pSetSts = (UCHAR *)"*BB";	}
		else if( CHK_PDET() 			){ pSetSts = (UCHAR *)"*P DET";	}
		else							 { pSetSts = (UCHAR *)"*RUN";	}
	/*----------------------------------------------------------------------------------------------*/
		if( CHK_TUNING() 				){ pSetSts = (UCHAR *)"*ADJ";	}
	/*----------------------------------------------------------------------------------------------*/
		if( !CHK_JOGMODE() && !CHK_ZRETMODE() )
		{
			if( CHK_POT() && CHK_NOT()	){ pSetSts = (UCHAR *)"*PT NT";	}
			else if( CHK_POT()			){ pSetSts = (UCHAR *)"*P-OT";	}
			else if( CHK_NOT()			){ pSetSts = (UCHAR *)"*N-OT";	}
			else if( CHK_PLS()			){ pSetSts = (UCHAR *)"*P-LS";	}
			else if( CHK_NLS()			){ pSetSts = (UCHAR *)"*N-LS";	}
		}
	/*----------------------------------------------------------------------------------------------*/
		if( CHK_ESTOP()					){ pSetSts = (UCHAR *)"*ESTOP";	}
	/*----------------------------------------------------------------------------------------------*/
		if( CHK_FSTP()					){ pSetSts = (UCHAR *)"*FSTP";	} 			/* <S1F5> */
	/*----------------------------------------------------------------------------------------------*/
		if( CHK_HBB() 					){ pSetSts = (UCHAR *)"*HBB";	}
	/*----------------------------------------------------------------------------------------------*/
		/* If not in motorless test mode, close the pointer1Advance *//* <S1B7> */
//		if( 1 							){ pSetSts = pSetSts + 1;		}
		if( pAxRsc->MotorLessTest->MotorLessTestMode == FALSE ){ pSetSts = pSetSts + 1;		}
/*--------------------------------------------------------------------------------------------------*/
/*		Status display string setting process																*/
/*--------------------------------------------------------------------------------------------------*/
		RpxLcdSetStr( 0, 2, pSetSts, ResBuf );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Send Response : Set LED Status															*/
/*																									*/
/****************************************************************************************************/
void	LcdSetResLedSts( USHORT *pLedSts )
{
/*--------------------------------------------------------------------------------------------------*/
/*		LEDDisplay processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_SVON()							){ *pLedSts |= LCDLED_SVON;}
		if( CHK_TGON() 							){ *pLedSts |= LCDLED_TGON;}
		if( CHK_CHG()							){ *pLedSts |= LCDLED_CHG; }
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_COIN()							){ *pLedSts |= LCDLED_COIN;}
		if( CHK_VCMP() 							){ *pLedSts |= LCDLED_COIN;}
		if( CHK_TRQMODE() 						){ *pLedSts |= LCDLED_COIN;}
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_PREFON()						){ *pLedSts |= LCDLED_REF;}
		if( CHK_VREFON()						){ *pLedSts |= LCDLED_REF;}
		if( CHK_TREFON()						){ *pLedSts |= LCDLED_REF;}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Lcd Send Response : Set Blink																*/
/*																									*/
/****************************************************************************************************/
void	LcdSetResBlink( UCHAR *ResBuf, USHORT *pLedSts )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Blink flag processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( *((UINT *)&OpBlnkFlg) != 0 )				/* Currently blinking						*/
		{
			if( KpiGetLongTimer( &OpBlnkTime ) > 1500 )
			{
				*((UINT *)&OpBlnkFlg) = 0;				/* Blink end								*/
			}
		}
		else if( *((UINT *)&OpBlnkKickFlg) != 0 )		/* Blink start request received						*/
		{
			OpBlnkFlg = OpBlnkKickFlg;					/* Set Blink Flag							*/
			*((UINT *)&OpBlnkKickFlg) = 0;				/* Reset Blink Kick Flag					*/
			KpiRstLongTimer( &OpBlnkTime );				/* Reset Blink Timer						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Blink display data setting process																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpBlnkFlg.PrmEewrite )
		{	/* Entire right side of active line	*/
			RpxLcdSetBlnk((USHORT)(LopV.ActiveLine+1), 2+3+1, 11, (UCHAR *)ResBuf);
		}
		else if( OpBlnkFlg.read || LopV.LedBlnkSw.read )			/* READKey LED		*/
		{
			*pLedSts |= LCDLED_READB;
		}
		else if( OpBlnkFlg.write || LopV.LedBlnkSw.write )		/* WRITEKey LED	*/
		{
			*pLedSts |= LCDLED_WRITEB;
		}
		else if( OpBlnkFlg.no_op )
		{
//			RpxLcdSetStr(0, 0, (UCHAR *)"NO-OP   ", (UCHAR *)ResBuf);	/*<S054>*/
//			RpxLcdSetBlnk(0, 0, 8, (UCHAR *)ResBuf);
			RpxLcdSetStr(0, 0, (UCHAR *)"NO-OP  ", (UCHAR *)ResBuf);		/*<S054>*//*<S098>*/
			RpxLcdSetBlnk(0, 0, 7, (UCHAR *)ResBuf);
		}
		else if( OpBlnkFlg.error )
		{
//			RpxLcdSetStr(0, 0, (UCHAR *)"ERROR   ", (UCHAR *)ResBuf);	/*<S054>*/
//			RpxLcdSetBlnk(0, 0, 8, (UCHAR *)ResBuf);
			RpxLcdSetStr(0, 0, (UCHAR *)"ERROR  ", (UCHAR *)ResBuf);		/*<S054>*//*<S098>*/
			RpxLcdSetBlnk(0, 0, 7, (UCHAR *)ResBuf);
		}
		else if( OpBlnkFlg.done )
		{
//			RpxLcdSetStr(0, 0, (UCHAR *)"DONE    ", (UCHAR *)ResBuf);	/*<S054>*/
//			RpxLcdSetBlnk(0, 0, 8, (UCHAR *)ResBuf);
			RpxLcdSetStr(0, 0, (UCHAR *)"DONE   ", (UCHAR *)ResBuf);		/*<S054>*//*<S098>*/
			RpxLcdSetBlnk(0, 0, 7, (UCHAR *)ResBuf);
		}
		else if( OpBlnkFlg.end )
		{
//			RpxLcdSetStr(0, 0, (UCHAR *)"END     ", (UCHAR *)ResBuf);	/*<S054>*/
//			RpxLcdSetBlnk(0, 0, 8, (UCHAR *)ResBuf);
			RpxLcdSetStr(0, 0, (UCHAR *)"END    ", (UCHAR *)ResBuf);		/*<S054>*//*<S098>*/
			RpxLcdSetBlnk(0, 0, 7, (UCHAR *)ResBuf);
		}
/*--------------------------------------------------------------------------------------------------*/
		if( LopV.LedBlnkSw.read  ){ *pLedSts |= LCDLED_READB;}
		if( LopV.LedBlnkSw.write ){ *pLedSts |= LCDLED_WRITEB;}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Set LED and Blink 																			*/
/*																									*/
/****************************************************************************************************/
void 	RpxLcdSetBlink (UCHAR *ResBuf)				/* <S031> 	*/
{
USHORT LedSts = 0;
	
	LcdSetResLedSts( &LedSts );						/* Set LED Status						*/
	LcdSetResBlink( ResBuf, &LedSts );				/* Set Blink							*/
	MlibCnvLittle16( LedSts, LedSts );				/* Convert Endian 						*/
	LPX_SET_LEDSTS( ResBuf, LedSts );				/* Set LED Status						*/
	*((UINT *)&LopV.LedBlnkSw) = 0;					/* Reset RED Blink Switch				*/
}

/****************************************************************************************************/
/*																									*/
/*		Change the Lcd operator display mode <S031>													*/
/*																									*/
/****************************************************************************************************/
LONG	LcdModeChg ( DRVSEL *pDrvSel )
{
LONG DrvSelEnd = FALSE; /* Drive Selection end flag */
#if		CCSW_HWID == HWID_JPCV
USHORT FnIdx; /* Fn index of fn010 */
ULONG TxSize;
UCHAR *pBuf;
#endif
/*--------------------------------------------------------------------------------------------------*/
	switch (LopV.OpMode)
	{
/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_StartDisp : 	/* Start display mode */
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
			LopV.OpMode = LpxStartDispModeCv( pDrvSel, &DrvSelEnd ); 
#else
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*/
			/* At the end of initial communication block information acquisition with Digiope *//* <S19B> */
			if( RpxChkPrmCopyInfoGet( ) )
			{
				/*	With the current displayed axis number2Determine axis motion	*/
				if(DrvSel.Unit[0].DispCurAxis != 0)
				{	/*	2For axis operation	*/
					LopV.OpMode = LCDOM_SelectAxis;
				}else
				{	/*	1For axis operation	*/
					LopV.OpMode = LCDOM_PrmMon;
				}
			}
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
			LopV.OpMode = LpxStartDispModeSv( pDrvSel );
#endif
#endif
			break;

/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_PrmMon : /* parameter/monitor mode */
/*--------------------------------------------------------------------------------------------------*/
			if ( /* OPWM_None */ 0 == LopV.EewriteMode ) 
			{ /* when eeprom is not written */
				if ( OpKeyMk.read ) 
				{
#if		CCSW_HWID == HWID_JPCV
					LopV.OpMode = (!pDrvSel) ? LCDOM_PrmMon : LCDOM_SelectUnit;
#else
					LopV.OpMode = (!pDrvSel) ? LCDOM_PrmMon : LCDOM_SelectAxis;
#endif
				}
				else if ( OpKeyMk.mode )
				{
					LopV.OpMode = LCDOM_Fun;
				}
				/*----------------------------------------------------------------------------------*/
				if ( LopV.OpMode != LCDOM_PrmMon) 
				{ 
					LcdRstPnUnDspData();  /* reset Pn/Un display data */
				}
			}
			break;

/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_Fun : /* Fn function mode */
/*--------------------------------------------------------------------------------------------------*/
			if ( LopV.FunMode == LCDFM_Num )
			{
				if( OpKeyMk.read )
				{
	#if		CCSW_HWID == HWID_JPCV
					LopV.OpMode = (!pDrvSel) ? LCDOM_Fun : LCDOM_SelectUnit;
	#else
					LopV.OpMode = (!pDrvSel) ? LCDOM_Fun : LCDOM_SelectAxis;
	#endif
				}
				else if ( OpKeyMk.mode )
				{
//					LopV.OpMode = LCDOM_PrmMon ;
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*/
					/*	With the current displayed axis number2Determine axis motion	*/
					if(DrvSel.Unit[0].DispCurAxis != 0)
					{	/*	2For axis operation Copy parameter or loop to axis selection (01/08)	*/
						LopV.OpMode = (( SVFSEL_LCDPRMCOPY_USE == 1 ) ? LCDOM_PrmCopy : LCDOM_SelectAxis);
					}else
					{	/*	1For axis operation	*/
						LopV.OpMode = (( SVFSEL_LCDPRMCOPY_USE == 1 ) ? LCDOM_PrmCopy : LCDOM_PrmMon);
					}
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
					LopV.OpMode = (( SVFSEL_LCDPRMCOPY_USE == 1 ) ? LCDOM_PrmCopy : LCDOM_PrmMon);
#endif
				}
			}
			else if ( LopV.FunMode == LCDFM_Exe )
			{
#if 0
				if( OpKeyMk.read )
				{
					RpxFunFinishLcd( LopDsp.CurFunIndx, &LopMsg );	/* finish the executiing Fn function */
					LopV.FunMode = LCDFM_Num;
	#if		CCSW_HWID == HWID_JPCV
					LopV.OpMode = (!pDrvSel) ? LCDOM_Fun : LCDOM_SelectUnit;
	#else
					LopV.OpMode = (!pDrvSel) ? LCDOM_Fun : LCDOM_SelectAxis;
	#endif
				}
				else if( RpxFunEndChkLcd( LopDsp.CurFunIndx, &LopMsg ) )
				{
					RpxFunFinishLcd( LopDsp.CurFunIndx, &LopMsg );  /* finish the executiing Fn function */
					LopV.FunMode = LCDFM_Num;
				}
#endif
			}
			break;

/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_SelectUnit : /* Unit selection mode */
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
			if ( OpKeyHld.read )
			{
				//if (RpxFunSearchIdx( LCDPASSWD_FNNUM, 0, &FnIdx, 0, &LopMsg ) == OK)
				if (CLCDFUN->Search( CLCDFUN, LCDPASSWD_FNNUM, 0, &FnIdx, 0, &LopMsg ) == OK)	/* <S18F> */
				{ /* found index of Fn010 */
					if (RpxFunExeBeginLcd( FnIdx, &LopMsg ) == TRUE )
					{ /* execution of Fn010 */
						LopV.OpMode = LCDOM_Passwd;
					}
					LopV.PasswdIdx = FnIdx;
				}
			}
			else if( OpKeyMk.mode )
			{
				if ( RpxChkSelectedDrv(pDrvSel) )
				{
					if ( pDrvSel->Unit[pDrvSel->DispCurIdx].Type == DRVSEL_TYPE_DRVAXIS )
					{
						if( pDrvSel->Unit[pDrvSel->DispCurIdx].AxisNumber > 1 ) 
						{ /* multi axis drive is selected */
							LopV.OpMode = LCDOM_SelectAxis;
							KpiRstLongTimer( &LopV.StDispTime ); /* reset timer */
						}
						else 
						{ /* single axis drive is selected */
							LopV.DrvOpStat = DRVOPST_WAITKEYSEND; /* waiting for mode key break */
							DrvSelEnd = TRUE;                     /* drive selection end */
						}
					}
					else if ( pDrvSel->Unit[pDrvSel->DispCurIdx].Type == DRVSEL_TYPE_CONVERTER)
					{ /* converter is selected */
						LopV.OpMode = LCDOM_PrmMon;
					}
				}
				else
				{
					OpBlnkKickFlg.no_op = 1;
				}
			}
#endif
			break;

/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_SelectAxis : /* axis selection mode */
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
			if( OpKeyMk.read )
			{
				/* finish drive lcd operator */
				KriAddLbusMsgQueue ( LMQUEID_MSGTRANSFER, 
						(0xFF << 16) | pDrvSel->Unit[ pDrvSel->DispCurIdx ].Address,
						sizeof(dummyMsg),
						dummyMsg );
				LopV.OpMode = LCDOM_SelectUnit;
			}
			else if( OpKeyMk.mode )
			{
				if ( RpxChkSelectedDrv(pDrvSel) )
				{
					LopV.DrvOpStat = DRVOPST_WAITKEYSEND; /* waiting for mode key break */
					DrvSelEnd = TRUE;                     /* drive selection end */
				}
				else
				{
					OpBlnkKickFlg.no_op = 1;
				}
			}
	#if 1
			else 
			{
				if ( KriGetLbusMsg ( LMQUEID_MSGTRANSFER, &pBuf, &TxSize ) )
				{ 
					KpiRstLongTimer( &LopV.StDispTime ); /* reset timer */
				}
				else if ( KpiGetLongTimer ( &LopV.StDispTime ) > 5000 )
				{
					LopV.OpMode = LCDOM_SelectUnit; /* timeout */
				}
			}
	#endif
			break;
#else
			if( OpKeyMk.mode )
			{
				LopV.OpMode = LCDOM_PrmMon;
				DrvSelEnd = TRUE;
			}
			break;
#endif
/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_Passwd : /* password input mode */
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
			FnIdx = LopV.PasswdIdx;
			if( RpxFunEndChkLcd( FnIdx, &LopMsg ) )
			{ 
//TODO				RpxFunFinishLcd( FnIdx, &LopMsg ); /* finish Fn010 */
				LopV.FunMode = LCDFM_Num;
				LopV.OpMode = LCDOM_SelectUnit;
			}
#endif
			break;
/*--------------------------------------------------------------------------------------------------*/
		case LCDOM_PrmCopy: 				/* Parameter Copy Mode */
/*--------------------------------------------------------------------------------------------------*/
			if ( ( OpKeyMk.mode ) && ( ! RpxChkPrmCopyExe() ) )
			{
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*/
				if(DrvSel.Unit[0].DispCurAxis != 0)
				{	/*	2Loop to axis selection for axis motion (01/08)	*/
					LopV.OpMode = LCDOM_SelectAxis;
				}else
				{	/*	1For axis operation go to parameter/monitor	*/
					LopV.OpMode = LCDOM_PrmMon;
				}
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
				LopV.OpMode = LCDOM_PrmMon;
#endif
			}

/*--------------------------------------------------------------------------------------------------*/
		default: /* error state */
/*--------------------------------------------------------------------------------------------------*/
			break;
	}
	return DrvSelEnd;
}

#if CCSW_CMDID == CMDID_EXIST		/*<11J42>Addition from here*/
/****************************************************************************************************/
/*		Lcd Operator for Command Option : Returns Next Option Parameter Number						*/
/****************************************************************************************************/
USHORT	LcdOptGetNextOpPrmNo(USHORT cmd, USHORT cur_svidx, USHORT digit, DEGOP_PRM *optprm)
{
USHORT	optprmno = 0x0000;
USHORT	svprmno = PnPrmTbl[cur_svidx].PnNo;

		switch(cmd)
		{
			case DEGOP_READ_UP:
				/* If it has fallen below the lower limit of the option parameter once */
				if( optprm->State.ExceedLwrLmt == TRUE )
				{
					optprmno = svprmno + (0x01 << (digit*4));
				}
				/* If it has never fallen below the lower limit of the option parameter */
				else
				{
					optprmno = optprm->CurrPnNo + (0x01 << (digit*4));
				}

				/* Servo parameter upper limit value check */
				if( optprmno >= 0x1000 ) {optprmno = 0xFFF;}
				break;

			case DEGOP_READ_DOWN:
				/* If it has exceeded the upper limit of the option parameter once */
				if( optprm->State.ExceedUprLmt == TRUE )
				{
					optprmno = svprmno - (0x01 << (digit*4));
				}
				/* If it has never exceeded the upper limit of the option parameter */
				else
				{
					/* Servo parameter lower limit value check */
					if( optprm->CurrPnNo >= (0x01 << (digit*4)))
					{
						optprmno = optprm->CurrPnNo - (0x01 << (digit*4));
					}
				}
				break;

			default:
				break;
		}

		return(optprmno);
}



/****************************************************************************************************/
/*		Lcd Operator for Command Option : Returns Next Servo Parameter Index						*/
/****************************************************************************************************/
USHORT	LcdOptGetNextSvPrmIdx(USHORT cmd, USHORT cur_svidx, DEGOP_OPTCOM *lcdopt)
{
USHORT	svprmidx;
DEGOP_PRM	*optprm = &lcdopt->Prm[LopV.ActiveLine];

		switch(cmd)
		{
			case DEGOP_READ_UP:
				/* First time beyond the area */
				if( optprm->State.ExceedUprLmt == FALSE )
				{
					/* Next option parameter number is0x0B00Since it will never be less thanOK */
					svprmidx = RpxPrmSearchIdxOp(optprm->NextPnNo, cur_svidx, &LopMsg);
				}
				/* Second time after crossing the area (editing the system parameter area) */
				else
				{
					svprmidx = optprm->NextSvIdx; /* Servo parameter index update */
				}
				break;

			case DEGOP_READ_DOWN:
				/* First time after leaving the area */
				if( optprm->State.ExceedLwrLmt == FALSE )
				{
					/* Next option parameter number is0x0B00, search for that number. */
					if( optprm->NextPnNo < OPT_PNNO_VIRTUAL )
					{
						svprmidx = RpxPrmSearchIdxOp(optprm->NextPnNo, cur_svidx, &LopMsg);
					}
					/* Next option parameter number is0x0B00or more is0x0AFF */
					else
					{
						svprmidx = RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL-1), cur_svidx, &LopMsg);
					}
				}
				/* Second time after leaving the area (editing the user parameter area) */
				else
				{
					svprmidx = optprm->NextSvIdx; /* Servo parameter index update */
				}
				break;

			default:
				break;
		}

		return(svprmidx);

}



/****************************************************************************************************/
/*		Lcd Operator for Command Option : Request Option Parameter Read								*/
/****************************************************************************************************/
LONG	LcdOptSetOpPrmReadReq(USHORT cmd, USHORT srchno, DEGOP_OPTCOM *lcdopt)
{
LONG	rc;

		/* If the next parameter number is less than or equal to the lower limit */
		if( srchno <= lcdopt->PrmRange.LwrLmt )
		{
			rc = OpiReqDegOpPrmRead( cmd, lcdopt->PrmRange.LwrLmt);	/* Limit at the lower limit value and read option parameters */
		}
		/* If the next parameter number is greater than or equal to the upper limit */
		else if( srchno >= lcdopt->PrmRange.UprLmt )
		{
			rc = OpiReqDegOpPrmRead( cmd, lcdopt->PrmRange.UprLmt);	/* Limit at upper limit and read option parameters */
		}
		/* Other commands */
		else
		{
			rc = OpiReqDegOpPrmRead( cmd, srchno);					/* Issue next option parameter read command */
		}

		if( rc == DEGOP_SUCCESS )
		{
			/* Response data wait set */
			lcdopt->Prm[LopV.ActiveLine].State.RcvWait = TRUE;
			/* Timeout reset */
			KpiRstLongTimer( &lcdopt->ComTimeLong );
		}

		return(rc);
}



/****************************************************************************************************/
/*		Lcd Operator for Command Option : Update Option Parameter Data from Option					*/
/****************************************************************************************************/
LONG	LcdOptUpdateResponseData(USHORT dispID, DEGOP_OPTCOM *lcdopt)
{
	USHORT prmno, idx_tmp;
	DEGOP_ATTR attr;
	LONG data, rc;
	DEGOP_PRM *optprm = &lcdopt->Prm[LopV.ActiveLine];

/*--------------------------------------------------------------------------------------------------*/
/*		Option card response analysis																	*/
/*--------------------------------------------------------------------------------------------------*/

		/* Parameter readout/Getting a response to a write request */
		rc = OpiGetDegOpPrmValue( &prmno, &attr, &data );

		/* Response result analysis */
		if( rc == DEGOP_SUCCESS )	/* Normal end */
		{
			/* Current option parameter update */
			optprm->CurrPnNo = prmno & 0xFFF;
			optprm->Attr = attr;
			optprm->Data = data;
			optprm->Size = optprm->Attr.ByteSize;

			/* Parameter editing set */
			optprm->State.Editing = TRUE;
			/* Response data wait clear */
			optprm->State.RcvWait = FALSE;

		}
		else if( rc == DEGOP_BUSY )	/* Continuing processing */
		{
			if ( KpiGetLongTimer( &lcdopt->ComTimeLong ) > OPT_COM_TIMEOUT )	/* 200msecElapsed time	*/
			{
				/* Response data wait clear */
				optprm->State.RcvWait = FALSE;
			}
		}
		else if( (rc == DEGOP_REQ_ERR) || (rc == DEGOP_RCV_ERR) )	/* Abnormal termination */
		{
			/* Response data wait clear */
			optprm->State.RcvWait = FALSE;
		}
		else	/* Impossible path */
		{
			/* Response data wait clear */
			optprm->State.RcvWait = FALSE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Processing after option card response completion															*/
/*--------------------------------------------------------------------------------------------------*/
		if( optprm->State.RcvWait == FALSE ) 
		{
	/*----------------------------------------------------------------------------------------------*/
	/*		Servo parameter index update (first time entering option area)							*/
	/*----------------------------------------------------------------------------------------------*/
			if( optprm->State.Editing == TRUE )
			{
			/* When there is a normal response when entering the option parameter area for the first time */
			/* Update current index to virtual option parameter index */
				if( optprm->State.ExceedUprLmt == TRUE )
				{
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = optprm->NextSvIdx;
					optprm->State.ExceedUprLmt = FALSE;
				}
				else if( optprm->State.ExceedLwrLmt == TRUE )
				{
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = optprm->NextSvIdx;
					optprm->State.ExceedLwrLmt = FALSE;
				}
			}
			else
			{
			/* Timeout when entering the option parameter area for the first time/When an error occurs */
			/* Update current index to next displayable servo index */
				if( optprm->State.ExceedUprLmt == TRUE )
				{
					/* Virtual option parameters (PnB00) Measures to prevent display */
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
						RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL-1), optprm->NextSvIdx, &LopMsg);
					optprm->State.ExceedUprLmt = FALSE;
				}
				else if( optprm->State.ExceedLwrLmt == TRUE )
				{
					/* Virtual option parameters (PnB00) Measures to prevent display */
					idx_tmp = RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL+1), optprm->NextSvIdx, &LopMsg);
					if( PnPrmTbl[idx_tmp].PnNo > OPT_PNNO_VIRTUAL )
					{
						LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
						optprm->State.ExceedLwrLmt = FALSE;
					}
					else
					{
						LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
							RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL-1), idx_tmp, &LopMsg);
					}
				}
			}
	/*----------------------------------------------------------------------------------------------*/
	/*		Blink start																			*/
	/*----------------------------------------------------------------------------------------------*/
			if( lcdopt->f.PrmEepWrite == TRUE )
			{
				if( rc == DEGOP_SUCCESS )
				{
					LcdOptBlnkKick->PrmEewrite = 1;	/* Right side blink activation					*/
				}
				else
				{
					LcdOptBlnkKick->no_op = 1;		/* NO-OPBlink start				*/
				}
				lcdopt->f.PrmEepWrite = FALSE;		/* EEPROMWrite request flag reset */
				LopV.EewriteMode = OPWM_UpdtBlnk;	/* State transition 						*/
			}
			else if( lcdopt->f.PrmRamWrite == TRUE )
			{
				if( rc != DEGOP_SUCCESS )
				{
					LcdOptBlnkKick->no_op = 1;		/* NO-OPBlink start				*/
				}
				lcdopt->f.PrmRamWrite = FALSE;		/* RAMWrite request flag reset	*/
			}
		}

		return(optprm->State.RcvWait);

}



/****************************************************************************************************/
/*		Lcd Operator for Command Option : Parameter Blink x position								*/
/****************************************************************************************************/
USHORT	LcdOptSetDataBlinkDPos(DEGOP_PRM *optprm)
{

	USHORT	pos_tmp = LopV.DataDigit - 1 - LopV.CurDigit;	/* "H.", the position without considering the decimal point	*/

	if(optprm->Attr.Base == DSPDEF_BASEDEC)	/* Comment out because the base detects10Base	*/
	{
		if(optprm->Attr.Sign == DSPDEF_W_SIGN)
		{
			pos_tmp++;	/* Signed amount	*/
		}
		if( optprm->Attr.Decimals > 0)
		{
			if(LopV.CurDigit < (USHORT)optprm->Attr.Decimals) pos_tmp++;	/* Decimal point	*/
		}
	}
	else										/* Comment out because the base detects10Non-hexadecimal	*/
	{
		pos_tmp += 2;	/* "n." or "H." minutes	*/
	}
	return(pos_tmp);
}
#endif
/*<11J42>Addition to here*/


/****************************************************************************************************/
/*																									*/
/*	<S031> Drive For 																				*/
/*																									*/
/****************************************************************************************************/
LONG LpxStartDispModeSv( DRVSEL *pDrvSel )
{
LONG CurMode = LCDOM_StartDisp;

	if( RpxChkPrmCopyInfoGet( ) )
	{
		CurMode = (!pDrvSel) ? LCDOM_PrmMon : LCDOM_SelectAxis;
	}

	return (CurMode);
}


/****************************************************************************************************/
/*																									*/
/*	Converter For 																					*/
/*																									*/
/****************************************************************************************************/
#if		CCSW_HWID == HWID_JPCV
LONG LpxStartDispModeCv( DRVSEL *pDrvSel, LONG *DrvSelEnd )
{
LONG rc;
LONG CurMode = LCDOM_StartDisp;
	if (pDrvSel->UpdateStep > DRVSEL_UPD_MKUNITLIST)
	{
		if (( rc = RpxGetForceConnectDrive(pDrvSel) ) >= 0)
		{
			pDrvSel->DispCurIdx = rc;          					/* set unit index */
			pDrvSel->Unit[rc].DispCurAxis = 0; 					/* set first axis */
			LopV.DrvOpStat = DRVOPST_FORCECON; 					/* force connection */
			*DrvSelEnd = TRUE;                  				/* drive selection end */
			CurMode = LCDOM_Error;             					/* do not return to unit selection mode */
		}
		else
		{
			CurMode = LCDOM_SelectUnit;
		}
	}
	else if ( KpiGetLongTimer(&LopV.StDispTime) > 10000 )
	{
		CUrMode = LCDOM_SelectUnit;
	}
	
	return (CurMode);
}
#endif

/***************************************** end of file **********************************************/
