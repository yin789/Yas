/****************************************************************************************************/
/*																									*/
/*																									*/
/*		LcdOpPrmMon.c : LCDOperator processing module:PnUnScreen										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : SigmaMessagecommunicates with the LCD operator and executes various operations from the operator.		*/
/*																									*/
/*			1)Monitor function : Specified monitor variable(UnXXX)												*/
/*			2)Parameter : Specified parameters(PnXXX),Editing,Change									*/
/*			3)																						*/
/*			4)																						*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.30 : 2012.04.xx  K.Ozaki      Revised for Mercury 									*/
/*		Rev.1.40 : 2013.09.26  K.Ozaki		<S00C>													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"			/* Kernel API */
#include "LcdOp.h"			/* LcdOpIf Common Header */
#include "Monitor.h"		/* Un (Monitor) Common Header */
#include "Parameter.h"		/* Pn (Parameter) Common Header */
#include "PrmAccessIf.h"	/* Parameter Access Management Header */

extern MSGIF LopMsg;					/* TODO: EraseLcdOpe:Message I/F										*/
#define RpxPrmUpdateEepromIdx( Idx ) (OK) /* TODO: */

/****************************************************************************************************/
/*																									*/
/*		Function List																				*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD operator IF function																	*/
/*--------------------------------------------------------------------------------------------------*/
void	LcdPrmMonMain( MSGIF *hMsg, DRVSEL *pDrvSel );		/* PRM/MON Display Main Function		*/
void	LcdRstPnUnDspData( void );							/* Reset Pn/UN Display Data 			*/

/*--------------------------------------------------------------------------------------------------*/
/*		Local Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
USHORT	LcdPnUnEditSub(										/* Pn/Un Edit Mode Procedure			*/
		USHORT	dispID,										/* DisplayID(LCDPMDSP_PM/JOG/etc)	*/
		OPKEYFLG *pKeyMk,									/* Key press information address				*/
		BLINKFLG *pBlnkKickFlg	);							/* Blink start flag pointer			*/
/*--------------------------------------------------------------------------------------------------*/
USHORT	LcdPnUnNumEdit( 
		USHORT dispID, 
		OPKEYFLG *pKeyMk, 
		BLINKFLG *pBlnkKickFlg );
/*--------------------------------------------------------------------------------------------------*/
USHORT	LcdPnUnSel( 
		USHORT dispID, 
		OPKEYFLG *pKeyMk,
		BLINKFLG *pBlnkKickFlg );
/*--------------------------------------------------------------------------------------------------*/
USHORT	LcdPnDataEdit( 
		USHORT dispID,
		OPKEYFLG *pKeyMk,
		BLINKFLG *pBlnkKickFlg );
/*--------------------------------------------------------------------------------------------------*/
void	LcdMkDspDataPnUnLines(			/* Make Display Data for Pn/Un Lines					*/
		USHORT	dispID,						/* DisplayID(LCDPMDSP_PM/JOG/etc)					*/
		USHORT	BlnkSupFlg,					/* Blink suppression flag									*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
USHORT	LcdPrmDataBlinkDpos( 
		CPRMDEF *pPrmDef );
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Lcd PRM/MON Display Mode Main Function														*/
/*																									*/
/****************************************************************************************************/
void	LcdPrmMonMain( MSGIF *hMsg, DRVSEL *pDrvSel )
{
UCHAR	*ResBuf = hMsg->ResBuf;
/*--------------------------------------------------------------------------------------------------*/
/*		Editing process																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( LCD_CHKCMD_RCVWAIT() ) 			/** Editing processing when not waiting for response data from an option **/
		{
			LcdPnUnEditSub(LCDPMDSP_PM, &OpKeyMk, &OpBlnkKickFlg);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display data creation process : Clear ResBuf															*/
/*--------------------------------------------------------------------------------------------------*/
		RpxLcdClrTxBuf( ResBuf );
/*--------------------------------------------------------------------------------------------------*/
/*		Display data creation process : TITLE																	*/
/*--------------------------------------------------------------------------------------------------*/
//		if( (hMsg->AccessLvl == ACCLVL_USER1) /* TODO: &&  (Iprm.OpeDspMode == OPEDSP_SETUPPRM) */)	/* <S113> */
		if( (hMsg->AccessLvl == ACCLVL_USER1) &&  (hMsg->OpeDspMode == OPEDSP_SETUPPRM) )			/* <S113> */
		{
//			RpxLcdSetStr(0, 8, (UCHAR *)"-PRM/MON-", (UCHAR *)ResBuf);		/* <S154> */
			RpxLcdSetStr(0, 10, (UCHAR *)"PRM/MON", (UCHAR *)ResBuf);		/* <S154> */
		}
		else
		{
//			RpxLcdSetStr(0, 8, (UCHAR *)"-ALL PRM-", (UCHAR *)ResBuf);				/* <S154> */
			RpxLcdSetStr(0, 10, (UCHAR *)"ALL PRM", (UCHAR *)ResBuf);				/* <S154> */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Display data creation process : Pn/Un Lines															*/
/*--------------------------------------------------------------------------------------------------*/
		LcdMkDspDataPnUnLines( LCDPMDSP_PM, FALSE, (UCHAR *)ResBuf );
/*--------------------------------------------------------------------------------------------------*/
/*		Blink & status display processing																*/
/*--------------------------------------------------------------------------------------------------*/
		RpxLcdSetResStatus (ResBuf);			/* <S031> */
		RpxLcdSetBlink (ResBuf);				/* <S031> */
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Pn/Un Edit mode (Return value:TRUE=Indicates that mode key press has been detected)							*/
/*																									*/
/****************************************************************************************************/
USHORT	LcdPnUnEditSub(						/* Pn/Un Edit mode										*/
		USHORT	dispID,						/* DisplayID(LCDPMDSP_PM/JOG/etc)					*/
		OPKEYFLG *pKeyMk,					/* Key press information address								*/
		BLINKFLG *pBlnkKickFlg	)			/* Blink start flag pointer							*/
{
USHORT	ModeChg = FALSE;
#if CCSW_CMDID == CMDID_EXIST				/*<11J42>*/
LONG	rc_tmp;
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Switch by EEPROM Write Mode																	*/
/*--------------------------------------------------------------------------------------------------*/
		switch( LopV.EewriteMode )
		{
	/*----------------------------------------------------------------------------------------------*/
		case OPWM_None:			/* Parameter value display/edit processing										*/
	/*----------------------------------------------------------------------------------------------*/
			switch( LopV.EditMode )
			{
			case LCDEM_PnUnNum:
				ModeChg = LcdPnUnNumEdit( dispID, pKeyMk, pBlnkKickFlg );
				break;
			case LCDEM_PnUnSel:
				ModeChg = LcdPnUnSel( dispID, pKeyMk, pBlnkKickFlg );
				break;
			case LCDEM_PnData:
				ModeChg = LcdPnDataEdit( dispID, pKeyMk, pBlnkKickFlg );
#if CCSW_CMDID == CMDID_EXIST				/*<11J42>*/
				if( ModeChg == TRUE )
				{
					/* When option parameters are being edited */
					if( LcdOpt[dispID].Prm[LopV.ActiveLine].State.Editing == TRUE )
					{
						/* Option parameter write request set(Non-volatile memory area) */
						rc_tmp = OpiReqDegOpPrmWrite( LcdOpt[dispID].Prm[LopV.ActiveLine].CurrPnNo,
													  LcdOpt[dispID].Prm[LopV.ActiveLine].Size,
													  LcdOpt[dispID].Prm[LopV.ActiveLine].Data );
						if( rc_tmp == DEGOP_SUCCESS )
						{
							/* Response data wait set */
							LcdOpt[dispID].Prm[LopV.ActiveLine].State.RcvWait = TRUE;
							/* Timeout reset */
							KpiRstLongTimer( &LcdOpt[dispID].ComTimeLong );
							/* Response data reception processing call(Note: Updated only upon normal completion) */
							rc_tmp = LcdOptUpdateResponseData(dispID, &LcdOpt[dispID]);
							while(rc_tmp == TRUE)
							{
								rc_tmp = LcdOptUpdateResponseData(dispID, &LcdOpt[dispID]);
							}
						}
					}
					else
					{
						rc_tmp = RpxPrmUpdateEepromIdx(LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx);
						while(rc_tmp == PRM_EEPROM_WRITE_WAIT || rc_tmp == PRM_EEPROM_WRITE_FAIL)
						{
							rc_tmp = RpxPrmUpdateEepromIdx(LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx);
						}
					}
				}
#endif
	 			/* 
				 * * Jupiter DataOnly when key is pressedEEPROM *
				 */
				break;
			default:
				break;
			}
			break;
	/*----------------------------------------------------------------------------------------------*/
		case OPWM_UpdtWait:		/* EEPROMWrite(Request registration)Waiting process									*/
	/*----------------------------------------------------------------------------------------------*/
			switch( RpxPrmUpdateEepromIdx( LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx ) )
			{
			case OK:	/* Normal	*/
				pBlnkKickFlg->PrmEewrite = 1;	/* Blink start	*/
				LopV.EewriteMode = OPWM_UpdtBlnk;
				break;
			default:
				break;
			}
			break;
	/*----------------------------------------------------------------------------------------------*/
		case OPWM_UpdtBlnk:		/* ParameterEEPROMBlink processing when writing							*/
	/*----------------------------------------------------------------------------------------------*/
			if( OpBlnkFlg.PrmEewrite == 0 )
			{
				LopV.EewriteMode = OPWM_None;
				LopV.EditMode = LCDEM_PnUnNum;
				LopV.CurDigit = 0;
			}
			break;
	/*----------------------------------------------------------------------------------------------*/
		case OPWM_InfWrWait:	/* Screen configuration informationEEPROMProcessing when writing is disabled								*/
	/*----------------------------------------------------------------------------------------------*/
			if( LcdSaveDispInfoTable( dispID ) == OK )
			{
				pBlnkKickFlg->write = TRUE;
				LopV.EewriteMode = OPWM_None;
			}
			break;
	/*----------------------------------------------------------------------------------------------*/
		default:
	/*----------------------------------------------------------------------------------------------*/
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( ModeChg );
}


/****************************************************************************************************/
/*																									*/
/*		Pn/Un Number Edit mode																		*/
/*																									*/
/****************************************************************************************************/
/*		dispID: DisplayID(LCDPMDSP_PM=PRM/MONScreen, LCDPMDSP_JOG=JOGScreen etc.), 				*/
/*		pKeyMk: Key press information address, pBlnkKickFlg: Blink start flag pointer					*/
/*		Return value: TRUE=Indicates that mode key press has been detected											*/
/****************************************************************************************************/
USHORT	LcdPnUnNumEdit( USHORT dispID, OPKEYFLG *pKeyMk, BLINKFLG *pBlnkKickFlg )
{
LONG	i;
CPRMDEF	*pPrmDef;
USHORT	Numx;
USHORT	PnUn  = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnUn;
USHORT	UnIdx = LopDsp.InfTbl[dispID][LopV.ActiveLine].UnIdx;
USHORT	PnIdx = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx;
#if CCSW_CMDID == CMDID_EXIST			/*<11J42>*/
USHORT	idx_tmp, cur_num_tmp, cur_idx_tmp;
DEGOP_PRM	*OpPrm = &LcdOpt[dispID].Prm[LopV.ActiveLine];
LONG rc;
#endif
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;										/* TODO: Erase after */

/*--------------------------------------------------------------------------------------------------*/
/*		Pn/Un Number Edit																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( pKeyMk->mode )
		{
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->up )
		{
			if( PnUn == UnSelected )
			{
				Numx = CUNMONX->GetNumber( CUNMONX, UnIdx, &LopMsg ) + (0x01 << (LopV.CurDigit*4));
				if( Numx >= 0x1000 ){ Numx = 0xFFF;}
				LopDsp.InfTbl[dispID][LopV.ActiveLine].UnIdx = 
					CUNMONX->SearchB( (COBJECT *)CUNMONX, Numx, UnIdx, &LopMsg );
			}
			else if( PnUn == PnSelected )
			{
/*<11J42>*/
#if CCSW_CMDID == CMDID_EXIST
/*--------------------------------------------------------------------------------------------------*/
/*			Get initial information																			*/
/*--------------------------------------------------------------------------------------------------*/
			/* Current servo parameter index */
			cur_idx_tmp = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx;
			/* Current servo parameter number */
			cur_num_tmp = PnPrmTbl[cur_idx_tmp].PnNo;
			/* Next servo parameter number */
			Numx = cur_num_tmp + (0x01 << (LopV.CurDigit*4));
			if(Numx >= 0x1000)	Numx = 0x0FFF;
			/* Next servo parameter index */
			idx_tmp = RpxPrmSearchIdxOp(Numx, cur_idx_tmp, &LopMsg);
			OpPrm->NextSvIdx = idx_tmp;

/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication status acquisition																	*/
/*--------------------------------------------------------------------------------------------------*/
			rc = OpiGetDegOpState(&LcdOpt[dispID].PrmRange);

/*--------------------------------------------------------------------------------------------------*/
/*			Set the parameter area currently being edited															*/
/*--------------------------------------------------------------------------------------------------*/
			/* User parameter area */
			if( cur_num_tmp < OPT_PNNO_VIRTUAL )
			{
				OpPrm->State.ExceedLwrLmt = TRUE;					/* Lower limit value over flag set		*/
				OpPrm->CurrPnNo = LcdOpt[dispID].PrmRange.LwrLmt;	/* Current optionPrmNumber update	*/
			}
			/* System parameter area */
			else if( cur_num_tmp > OPT_PNNO_VIRTUAL )
			{
				OpPrm->State.ExceedUprLmt = TRUE;					/* Set the upper limit value over flag		*/
				OpPrm->CurrPnNo = LcdOpt[dispID].PrmRange.UprLmt;	/* Current optionPrmNumber update	*/
			}

/*--------------------------------------------------------------------------------------------------*/
/*			When Digiope communication is enabled																		*/
/*--------------------------------------------------------------------------------------------------*/
			if( rc == DEGOP_ENABLE )
			{
			/*--------------------------------------------------------------------------------------*/
			/*	System/Editing option parameter area												*/
			/*--------------------------------------------------------------------------------------*/
				if( PnPrmTbl[idx_tmp].PnNo >= OPT_PNNO_VIRTUAL )
				{
					/* Get next option parameter number */
					OpPrm->NextPnNo = LcdOptGetNextOpPrmNo( DEGOP_READ_UP, cur_idx_tmp, LopV.CurDigit, OpPrm );

				/*----------------------------------------------------------------------------------*/
				/*	When exceeding option parameter area											*/
				/*----------------------------------------------------------------------------------*/
					if( OpPrm->NextPnNo >= LcdOpt[dispID].PrmRange.UprLmt )
					{
						/* When there are parameters above the option parameter area (system level) 	*/
						if( PnPrmTbl[idx_tmp].PnNo > OPT_PNNO_VIRTUAL )
						{
							/* Option parameter editing flag reset */
							OpPrm->State.Editing = FALSE;
							/* Servo parameter index update */
							LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
								LcdOptGetNextSvPrmIdx( DEGOP_READ_UP, cur_idx_tmp, &LcdOpt[dispID] );
						}
						/* When there are no parameters above the option parameter area (user level) 	*/
						else
						{
							/* Parameter readout request set */
							LcdOptSetOpPrmReadReq( DEGOP_READ_UP, OpPrm->NextPnNo, &LcdOpt[dispID] );
						}
					}
				/*----------------------------------------------------------------------------------*/
				/*	When within the option parameter area											*/
				/*----------------------------------------------------------------------------------*/
					else
					{
						/* Parameter readout request set */
						LcdOptSetOpPrmReadReq( DEGOP_READ_UP, OpPrm->NextPnNo, &LcdOpt[dispID] );
					}
				}
			/*--------------------------------------------------------------------------------------*/
			/*	Edit user parameter area															*/
			/*--------------------------------------------------------------------------------------*/
				else
				{
					/* Option parameter editing flag reset */
					OpPrm->State.Editing = FALSE;
					/* Servo parameter index update */
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
				}
			}
/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication processing in progress																		*/
/*--------------------------------------------------------------------------------------------------*/
			else if( rc == DEGOP_BUSY )
			{
				;
			}
/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication is disabled																		*/
/*--------------------------------------------------------------------------------------------------*/
			else
			{
				/* Option parameter status flag reset */
				MlibResetByteMemory((void *)&OpPrm->State, sizeof(OpPrm->State));

				/* Virtual option parameters (PnB00) Measures to prevent display */
				if( PnPrmTbl[idx_tmp].PnNo == OPT_PNNO_VIRTUAL )
				{
					idx_tmp = RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL+1), cur_idx_tmp, &LopMsg);
					if( PnPrmTbl[idx_tmp].PnNo > OPT_PNNO_VIRTUAL )
					{
						LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
					}
					else
					{
						LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
							RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL-1), idx_tmp, &LopMsg);
					}
				}
				else
				{
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
				}
			}
#else		//CCSW_CMDID != CMDID_EXIST 
				Numx = CPARAMX->GetNumber( CPARAMX, PnIdx, &LopMsg ) + (0x01 << (LopV.CurDigit*4));
				if( Numx >= 0x1000 ){ Numx = 0xFFF;}
				LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = CPARAMX->SearchB( (COBJECT *)CPARAMX, Numx, PnIdx, &LopMsg );
#endif
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->down )
		{
			if( PnUn == UnSelected )
			{
				if( CUNMONX->GetNumber( CUNMONX, UnIdx, &LopMsg ) < (0x01 << (LopV.CurDigit*4)))
				{
					Numx = 0x000;
				}
				else
				{
					Numx = CUNMONX->GetNumber( CUNMONX, UnIdx, &LopMsg ) - (0x01 << (LopV.CurDigit*4));
				}
				LopDsp.InfTbl[dispID][LopV.ActiveLine].UnIdx = 
						CUNMONX->SearchB( (COBJECT *)CUNMONX, Numx, UnIdx, &LopMsg );
			}
			else if( PnUn == PnSelected )
			{
/*<11J42>*/
#if CCSW_CMDID == CMDID_EXIST
/*--------------------------------------------------------------------------------------------------*/
/*			Get initial information																			*/
/*--------------------------------------------------------------------------------------------------*/
			/* Current servo parameter index */
			cur_idx_tmp = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx;
			/* Current servo parameter number */
			cur_num_tmp = PnPrmTbl[cur_idx_tmp].PnNo;
			/* Next servo parameter number */
			if(cur_num_tmp < (0x01 << (LopV.CurDigit*4)))
				Numx = 0x0000;
			else
				Numx = cur_num_tmp - (0x01 << (LopV.CurDigit*4));
			/* Next servo parameter index */
			idx_tmp = RpxPrmSearchIdxOp(Numx, cur_idx_tmp, &LopMsg);
			OpPrm->NextSvIdx = idx_tmp;

/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication status acquisition																	*/
/*--------------------------------------------------------------------------------------------------*/
			rc = OpiGetDegOpState(&LcdOpt[dispID].PrmRange);

/*--------------------------------------------------------------------------------------------------*/
/*			Set the parameter area currently being edited															*/
/*--------------------------------------------------------------------------------------------------*/
			/* User parameter area */
			if( cur_num_tmp < OPT_PNNO_VIRTUAL )
			{
				OpPrm->State.ExceedLwrLmt = TRUE;					/* Lower limit value over flag set		*/
				OpPrm->CurrPnNo = LcdOpt[dispID].PrmRange.LwrLmt;	/* Current optionPrmNumber update	*/
			}
			/* System parameter area */
			else if( cur_num_tmp > OPT_PNNO_VIRTUAL )
			{
				OpPrm->State.ExceedUprLmt = TRUE;					/* Set the upper limit value over flag		*/
				OpPrm->CurrPnNo = LcdOpt[dispID].PrmRange.UprLmt;	/* Current optionPrmNumber update	*/
			}

/*--------------------------------------------------------------------------------------------------*/
/*			When Digiope communication is enabled																		*/
/*--------------------------------------------------------------------------------------------------*/
			if( rc == DEGOP_ENABLE )
			{
			/*--------------------------------------------------------------------------------------*/
			/*	User/Editing option parameter area												*/
			/*--------------------------------------------------------------------------------------*/
				if( PnPrmTbl[idx_tmp].PnNo <= OPT_PNNO_VIRTUAL )
				{
					/* Get next option parameter number */
					OpPrm->NextPnNo = LcdOptGetNextOpPrmNo( DEGOP_READ_DOWN, cur_idx_tmp, LopV.CurDigit, OpPrm );

				/*----------------------------------------------------------------------------------*/
				/*	When exiting option parameter area											*/
				/*----------------------------------------------------------------------------------*/
					if( OpPrm->NextPnNo < LcdOpt[dispID].PrmRange.LwrLmt )
					{
						/* Option parameter editing flag reset */
						OpPrm->State.Editing = FALSE;
						/* Servo parameter index update */
						LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
							LcdOptGetNextSvPrmIdx( DEGOP_READ_DOWN, cur_idx_tmp, &LcdOpt[dispID] );
					}
				/*----------------------------------------------------------------------------------*/
				/*	When within the option parameter area											*/
				/*----------------------------------------------------------------------------------*/
					else
					{
						/* Parameter readout request set */
						LcdOptSetOpPrmReadReq( DEGOP_READ_DOWN, OpPrm->NextPnNo, &LcdOpt[dispID] );
					}
				}
			/*--------------------------------------------------------------------------------------*/
			/*	Edit system parameter area														*/
			/*--------------------------------------------------------------------------------------*/
				else
				{
					/* Option parameter editing flag reset */
					OpPrm->State.Editing = FALSE;
					/* Servo parameter index update */
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
				}
			}
/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication processing in progress																		*/
/*--------------------------------------------------------------------------------------------------*/
			else if( rc == DEGOP_BUSY )
			{
				;
			}
/*--------------------------------------------------------------------------------------------------*/
/*			Digiope communication is disabled																		*/
/*--------------------------------------------------------------------------------------------------*/
			else
			{
				/* Option parameter status flag reset */
				MlibResetByteMemory((void *)&OpPrm->State, sizeof(OpPrm->State));

				/* Virtual option parameters (PnB00) Measures to prevent display */
				if( PnPrmTbl[idx_tmp].PnNo == OPT_PNNO_VIRTUAL )
				{
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
						RpxPrmSearchIdxOp((OPT_PNNO_VIRTUAL-1), cur_idx_tmp, &LopMsg);
				}
				else
				{
					LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = idx_tmp;
				}
			}
#else		//CCSW_CMDID != CMDID_EXIST
				if( CPARAMX->GetNumber( CPARAMX, PnIdx, &LopMsg ) < (0x01 << (LopV.CurDigit*4)) )
				{
					Numx = 0x000;
				}
				else
				{
					Numx = CPARAMX->GetNumber( CPARAMX, PnIdx, &LopMsg ) - (0x01 << (LopV.CurDigit*4));
				}
				LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx = 
						CPARAMX->SearchB( (COBJECT *)CPARAMX, Numx, PnIdx, &LopMsg );
#endif
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->shup )
		{
			if(      LopV.CurDigit == (3-1) ){ LopV.EditMode = LCDEM_PnUnSel;}
			else if( LopV.CurDigit <  (3-1) ){ LopV.CurDigit++;}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->shdown )
		{
			if(      LopV.CurDigit == 0 ){ LopV.EditMode = LCDEM_PnUnSel;}
			else if( 0 < LopV.CurDigit  ){ LopV.CurDigit--;}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->data )
		{
			if( PnUn == UnSelected )
			{
				LopV.CurDigit = 0;
			}
			else if( PnUn == PnSelected )
			{
				if( CHK_WRITEINHIBIT() && (LopMsg.AccessLvl != ACCLVL_SYSTEM) )
				{
					OpBlnkKickFlg.no_op = 1;
				}
				else
				{
					LopV.EditMode = LCDEM_PnData;
					LopV.CurDigit = 0;
/*<11J42>*/
#if CCSW_CMDID == CMDID_EXIST		
					if( OpPrm->State.Editing == TRUE )
					{							/* Editing option parameter */
						if(OpPrm->IsLong)/* Long parameter	*/
						{
							LopV.DataDigit = (OpPrm->Attr.Base == DSPDEF_BASEDEC)? 10: 8;
						}
						else						/* Word parameter	*/
						{
							LopV.DataDigit = (OpPrm->Attr.Base == DSPDEF_BASEDEC)? 5: 4;
						}
					}
					else
					{
#endif
						pPrmDef = GET_PNPRMDEF( CPARAMX->pPrmTbl, PnIdx );		/* Parameter definition	*/
						if( CPARAMX->isLong( CPARAMX, PnIdx, &LopMsg ) )		/* Long parameter	*/
						{
							UINT dot = ((CHK_PNPRM_SIGNED( pPrmDef ) 			/* <S18B> */
										&& (GET_PNPRM_DECIMALS( CPARAMX, pPrmDef, &LopMsg )>0)) ? 1 : 0);
							LopV.DataDigit = ( GET_PNPRMBASE( pPrmDef ) == DSPDEF_BASEDEC)? (10-dot): 8;	/* <S18B> */
						}
						else													/* Word parameter	*/
						{
							LopV.DataDigit = ( GET_PNPRMBASE( pPrmDef ) == DSPDEF_BASEDEC)? 5: 4;
						}
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
					}
#endif
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->scroll )
		{
			LopV.ActiveLine = (LopV.ActiveLine == 0)? 3 : (--LopV.ActiveLine);
			LopV.CurDigit = 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->write )
		{
			if( LcdSaveDispInfoTable( dispID ) == OK )
			{
				pBlnkKickFlg->write = TRUE;
			}
			else
			{
				LopV.EewriteMode = OPWM_InfWrWait;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->rst )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Resettable monitors on the screen are reset										*/
		/*------------------------------------------------------------------------------------------*/
			for( i = 0; i < 4; i++ )
			{
				if ( LopDsp.InfTbl[dispID][i].PnUn == UnSelected )
				{
					CUNMONX->Clear( (COBJECT *)CUNMONX, LopDsp.InfTbl[dispID][i].UnIdx, &LopMsg );
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( FALSE );
}

/****************************************************************************************************/
/*																									*/
/*		Pn/Un Select mode																			*/
/*																									*/
/****************************************************************************************************/
/*		dispID: DisplayID(LCDPMDSP_PM=PRM/MONScreen, LCDPMDSP_JOG=JOGScreen etc.), 				*/
/*		pKeyMk: Key press information address, pBlnkKickFlg: Blink start flag pointer					*/
/*		Return value: TRUE=Indicates that mode key press has been detected											*/
/****************************************************************************************************/
USHORT	LcdPnUnSel( USHORT dispID, OPKEYFLG *pKeyMk, BLINKFLG *pBlnkKickFlg )
{
CPRMDEF		*pPrmDef; 														/* PRMDEF	*pPrmDef; */
USHORT	PnUn  = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnUn;
USHORT	PnIdx = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx;
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
	DEGOP_PRM	*OpPrm = &LcdOpt[dispID].Prm[LopV.ActiveLine];
#endif
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;										/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Pn/Un Select																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( pKeyMk->mode )
		{
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( (pKeyMk->up) || (pKeyMk->down) )
		{
			if( PnUn == UnSelected )
			{
				LopDsp.InfTbl[dispID][LopV.ActiveLine].PnUn = PnSelected;
			}
			else if( PnUn == PnSelected )
			{
				LopDsp.InfTbl[dispID][LopV.ActiveLine].PnUn = UnSelected;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->shup )
		{
			LopV.EditMode = LCDEM_PnUnNum;
			LopV.CurDigit = 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->shdown )
		{
			LopV.EditMode = LCDEM_PnUnNum;
			LopV.CurDigit = 3-1;
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->data )
		{
			if( PnUn == UnSelected )
			{
				LopV.EditMode = LCDEM_PnUnNum;
				LopV.CurDigit = 0;
			}
			else if( PnUn == PnSelected )
			{
				if( CHK_WRITEINHIBIT() /* Iprm.f.PrmWrtInhibit */ && (LopMsg.AccessLvl != ACCLVL_SYSTEM) )
				{
					OpBlnkKickFlg.no_op = 1;
				}
				else
				{
					LopV.EditMode = LCDEM_PnData;
					LopV.CurDigit = 0;
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
					if( OpPrm->State.Editing == TRUE )
					{							/* Editing option parameter */
						if(OpPrm->IsLong)/* Long parameter	*/
						{
							LopV.DataDigit = (OpPrm->Attr.Base == DSPDEF_BASEDEC)? 10: 8;
						}
						else						/* Word parameter	*/
						{
							LopV.DataDigit = (OpPrm->Attr.Base == DSPDEF_BASEDEC)? 5: 4;
						}
					}
					else
					{
#endif
						pPrmDef = GET_PNPRMDEF( CPARAMX->pPrmTbl, PnIdx ) /* (PRMDEF *)PnPrmTbl[PnIdx].PrmDefPtr */; /* Parameter definition	*/
						if( CPARAMX->isLong( CPARAMX, PnIdx, &LopMsg ) )	/* Long parameter	*/
						{
							UINT dot = ((CHK_PNPRM_SIGNED( pPrmDef ) 			/* <S18B> */
										&& (GET_PNPRM_DECIMALS( CPARAMX, pPrmDef, &LopMsg )>0)) ? 1 : 0);
							LopV.DataDigit = ( GET_PNPRMBASE( pPrmDef ) == DSPDEF_BASEDEC)? (10-dot): 8;	/* <S18B> */
						}
						else								/* Word parameter	*/
						{
							LopV.DataDigit = ( GET_PNPRMBASE( pPrmDef ) == DSPDEF_BASEDEC)? 5: 4;
						}
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
					}
#endif
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->scroll )
		{
			LopV.EditMode = LCDEM_PnUnNum;
			LopV.ActiveLine = (LopV.ActiveLine == 0)? 3 : (--LopV.ActiveLine);
			LopV.CurDigit = 0;
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( pKeyMk->write )
		{
			if( LcdSaveDispInfoTable(dispID) == OK )
			{
				pBlnkKickFlg->write = TRUE;
			}
			else
			{
				LopV.EewriteMode = OPWM_InfWrWait;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return(FALSE);
}


/****************************************************************************************************/
/*																									*/
/*		Pn Data Edit mode																			*/
/*																									*/
/****************************************************************************************************/
/*		dispID: DisplayID(LCDPMDSP_PM=PRM/MONScreen, LCDPMDSP_JOG=JOGScreen etc.)					*/
/*		pKeyMk: Key press information address, pBlnkKickFlg: Blink start flag pointer					*/
/*		Return value: TRUE=Indicates that mode key press has been detected											*/
/****************************************************************************************************/
USHORT	LcdPnDataEdit( USHORT dispID, OPKEYFLG *pKeyMk, BLINKFLG *pBlnkKickFlg )
{
LONG	rc;
ULONG	PnData;
USHORT	PnIdx = LopDsp.InfTbl[dispID][LopV.ActiveLine].PnIdx;
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
	DEGOP_PRM	*OpPrm = &LcdOpt[dispID].Prm[LopV.ActiveLine];
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Pn Data Edit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if(pKeyMk->mode)
		{
			return(TRUE);
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->up)
		{
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
		/* Editing option parameter */
			if( OpPrm->State.Editing == TRUE )
			{
				/* Data editing (upward) */
				PnData = RpiPrmDataDigitInc( OpPrm->Data, LopV.CurDigit,
												LcdOptPrmLmtTbl[OpPrm->Attr.Base][(OpPrm->IsLong<<1)|(OpPrm->Attr.Sign)].UprLmt,
												OpPrm->Attr.Base,
												OpPrm->IsLong,
												OpPrm->Attr.Sign );
				/* Temporary option parameter write request set */
				rc = OpiReqDegOpPrmWrite( (OpPrm->CurrPnNo|0x1000), OpPrm->Size, PnData );

				if( rc == DEGOP_SUCCESS )
				{
					/* RAMWrite request flag set */
					LcdOpt[dispID].f.PrmRamWrite = TRUE;
					/* Save pointer to blink kick flag */
					LcdOptBlnkKick = pBlnkKickFlg;
					/* Response data wait set */
					OpPrm->State.RcvWait = TRUE;
					/* Timeout reset */
					KpiRstLongTimer( &LcdOpt[dispID].ComTimeLong );
				}
				else
				{
					OpBlnkKickFlg.no_op = 1;
				}
			}
			else
			{
#endif
				rc = CPARAMX->DigitInc( CPARAMX, PnIdx, LopV.CurDigit, &LopMsg );
				if( rc == PERR_CALC ){ OpBlnkKickFlg.error = 1;}
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->down)
		{	/* Data editing (downward) */
#if CCSW_CMDID == CMDID_EXIST
			if( OpPrm->State.Editing == TRUE )
			{	/* Editing option parameter */
				PnData = RpiPrmDataDigitDec( OpPrm->Data, LopV.CurDigit,
												LcdOptPrmLmtTbl[OpPrm->Attr.Base][(OpPrm->IsLong<<1)|(OpPrm->Attr.Sign)].LwrLmt,
												OpPrm->Attr.Base,
												OpPrm->IsLong,
												OpPrm->Attr.Sign );
				/* Temporary option parameter write request set */
				rc = OpiReqDegOpPrmWrite( (OpPrm->CurrPnNo|0x1000), OpPrm->Size, PnData );

				if( rc == DEGOP_SUCCESS )
				{
					/* RAMWrite request flag set */
					LcdOpt[dispID].f.PrmRamWrite = TRUE;
					/* Save pointer to blink kick flag */
					LcdOptBlnkKick = pBlnkKickFlg;
					/* Response data wait set */
					OpPrm->State.RcvWait = TRUE;
					/* Timeout reset */
					KpiRstLongTimer( &LcdOpt[dispID].ComTimeLong );
				}
				else
				{
					OpBlnkKickFlg.no_op = 1;
				}
			}
			else
			{
#endif
				rc = CPARAMX->DigitDec( CPARAMX, PnIdx, LopV.CurDigit, &LopMsg );
				if( rc == PERR_CALC ){ OpBlnkKickFlg.error = 1;}
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->shup)
		{
			LopV.CurDigit = (LopV.CurDigit == (LopV.DataDigit-1))? 0 : (++LopV.CurDigit);
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->shdown)
		{
			LopV.CurDigit = (LopV.CurDigit == 0)? (LopV.DataDigit-1) : (--LopV.CurDigit);
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->data)
		{
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			if( OpPrm->State.Editing == TRUE )
			{			/* Editing option parameter */
				/* Option parameter write request set */
				rc = OpiReqDegOpPrmWrite( OpPrm->CurrPnNo, OpPrm->Size, OpPrm->Data );

				if( rc == DEGOP_SUCCESS )
				{
					/* EEPROMWrite request flag set */
					LcdOpt[dispID].f.PrmEepWrite = TRUE;
					/* Save pointer to blink kick flag */
					LcdOptBlnkKick = pBlnkKickFlg;
					/* Response data wait set */
					OpPrm->State.RcvWait = TRUE;
					/* Timeout reset */
					KpiRstLongTimer( &LcdOpt[dispID].ComTimeLong );
				}
				else
				{
					OpBlnkKickFlg.no_op = 1;
					LopV.EditMode = LCDEM_PnUnNum;
					LopV.CurDigit = 0;
				}
			}
			else
			{
#endif

				rc = PrmWriteValueToEeprom( LopMsg.pAxRsc->RegManager->hPnReg,						  /* <S099> */
						PnPrmTbl[PnIdx].Attribute,
						0 );
				switch( rc )																		  /* <S098> */
				{
				case PRM_RSLT_SUCCESS:			/* Normal												*/
					pBlnkKickFlg->PrmEewrite = 1;	/* Blink start	*/
					LopV.EewriteMode = OPWM_UpdtBlnk;
					break;
				/* Current statusQueueis not available, so comment out the following Y.Oka */
//				case PRM_EEPROM_WRITE_FAIL:		/* untilQueueis not vacant							*/
//					LopV.EewriteMode = OPWM_UpdtWait;
//					break;
				case PRM_RSLT_RAMONLY:			/* RAMonly parameter								*/
				case PRM_RSLT_NO_DIFF:			/* No need to write(EEPROMThe value is the same as the written value)	*/
					LopV.EditMode = LCDEM_PnUnNum;
					LopV.CurDigit = 0;
					break;
				default:
					break;
				}

#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->scroll)
		{
			/* Do nothing	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else if(pKeyMk->write)
		{
			if(LcdSaveDispInfoTable(dispID) == OK)
			{
				pBlnkKickFlg->write = TRUE;
			}
			else
			{
				LopV.EewriteMode = OPWM_InfWrWait;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return(FALSE);
}


/****************************************************************************************************/
/*																									*/
/*		Make Display Data for Pn/Un Lines															*/
/*																									*/
/****************************************************************************************************/
void	LcdMkDspDataPnUnLines(				/* Make Display Data for Pn/Un Lines					*/
		USHORT	dispID,						/* DisplayID(LCDPMDSP_PM/JOG/etc)					*/
		USHORT	BlnkSupFlg,					/* Blink suppression flag									*/
		UCHAR	*ResBuf			)			/* Response buffer address									*/
{
USHORT	i, Idx;
ULONG	Ldata;
CPRMDEF	*pPrmDef;
CMONDEF	*pMonDef; 							/* MONDEF	*pMonDef; */
LOPDIT	*pInf = (LOPDIT *)(&(LopDsp.InfTbl[dispID][0]));
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
USHORT j;
DEGOP_PRM *optprm;
DEGOP_PRM *act_optprm = &LcdOpt[dispID].Prm[LopV.ActiveLine];

		if( act_optprm->State.RcvWait == TRUE )
		{			/* Reception processing when waiting for response data from an option */
			LcdOptUpdateResponseData(dispID, &LcdOpt[dispID]);
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Make Display Data for Pn/Un Lines															*/
/*--------------------------------------------------------------------------------------------------*/
		for(i = 0; i < 4; i++)
		{
			if(pInf[i].PnUn == UnSelected)
			{
				Idx = pInf[i].UnIdx;
				pMonDef = GET_UNMONDEF( CUNMONX->pUnMonTbl, Idx );
				RpxLcdSetStr((USHORT)(i+1), 0, (UCHAR *)"Un", ResBuf);
				RpxLcdSetValHex((USHORT)(i+1), 2, (ULONG) CUNMONX->GetNumber( CUNMONX, Idx, &LopMsg ), 3, ResBuf);
				RpxLcdSetStr((USHORT)(i+1), 5, (UCHAR *)"=", ResBuf);
				CUNMONX->Read( (COBJECT *)CUNMONX, Idx, &Ldata, MCHK_ACCS_LVL, &LopMsg );   /* Ldata <-- Monitor value	*/ /* <S073> */
				RpxLcdSetPnUnData((USHORT)(i+1), 
					6, 
					Ldata, 
					CUNMONX->isLong( CUNMONX, Idx, &LopMsg),	
					CHK_UNBASE( pMonDef ),
					pMonDef->Sign, 
					CHK_UNDECIMALS( pMonDef ),
					ResBuf);
			}
			else if(pInf[i].PnUn == PnSelected)
			{
#if CCSW_CMDID == CMDID_EXIST
				optprm = &LcdOpt[dispID].Prm[i];
				/* When option parameters are being edited */
				if( optprm->State.Editing == TRUE )
				{
					/* Attribute check & set */
					optprm->IsLong = (optprm->Size == 4)? TRUE : FALSE;
					if( optprm->Attr.Base == DSPDEF_BASEBIT )	/* Currently, no support for bit display */
					{ optprm->Attr.Base = DSPDEF_BASEHEX; }		/* Forcibly16Hex display */
					optprm->Attr.Decimals = 0;					/* Currently no support for decimal point display */

					/* Editing on each linePnIf the number is the same as that of the active line, the active line'sPnUnify the number value */
					if( i != LopV.ActiveLine )
					{
						if( optprm->CurrPnNo == act_optprm->CurrPnNo )
						{
							optprm->Data = act_optprm->Data;
						}
					}

					/* Editing on each screenPnIf the number is the same as that of the active line, the active line'sPnUnify the number value */
					for(j = 0; j < LCDPMDSP_MAX; j++)
					{
						if( j != dispID )
						{
							if( LcdOpt[j].Prm[i].CurrPnNo == act_optprm->CurrPnNo )
							{
								LcdOpt[j].Prm[i].Data = act_optprm->Data;
							}
						}
					}

					/* Response data display */
					RpxLcdSetStr((USHORT)(i+1), 0, (UCHAR *)"Pn", ResBuf);
					RpxLcdSetValHex((USHORT)(i+1), 2, (ULONG)optprm->CurrPnNo, 3, ResBuf);
					RpxLcdSetStr((USHORT)(i+1), 5, (UCHAR *)"=", ResBuf);
					RpxLcdSetPnUnData( (USHORT)(i+1), 6, optprm->Data,
										(USHORT)optprm->IsLong, (USHORT)optprm->Attr.Base,
										(USHORT)optprm->Attr.Sign, optprm->Attr.Decimals, ResBuf);
				}
				else
				{
#endif
					Idx = pInf[i].PnIdx;
					pPrmDef = GET_PNPRMDEF( CPARAMX->pPrmTbl, Idx );
					RpxLcdSetStr((USHORT)(i+1), 0, (UCHAR *)"Pn", ResBuf);
					RpxLcdSetValHex((USHORT)(i+1), 2, CPARAMX->GetNumber( CPARAMX, Idx, &LopMsg ), 3, ResBuf);
					RpxLcdSetStr((USHORT)(i+1), 5, (UCHAR *)"=", ResBuf);
					CPARAMX->Read( (COBJECT *)CPARAMX, Idx, &Ldata, PCHK_OPE_READ, &LopMsg );	/* Ldata <-- Parameter value	*/  /* <S073> */
					RpxLcdSetPnUnData( 
						(USHORT)(i+1), 
						6, 
						Ldata,
						CPARAMX->isLong( CPARAMX, Idx, &LopMsg ),
						GET_PNPRMBASE( pPrmDef ),
						CHK_PNPRM_SIGNED( pPrmDef ), 
						/*GET_PNPRM_DECIMALS( pPrmDef ), */
						GET_PNPRM_DECIMALS( CPARAMX, pPrmDef, &LopMsg ),	/* <S14F> */ 
						ResBuf);
				}
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set Blink in the active line																*/
/*--------------------------------------------------------------------------------------------------*/
		if(!BlnkSupFlg)
		{
			switch(LopV.EditMode)
			{
			case LCDEM_PnUnNum:
				RpxLcdSetBlnk((USHORT)(LopV.ActiveLine+1), (USHORT)(2+3-1 - LopV.CurDigit), 1, ResBuf);
				break;
			case LCDEM_PnUnSel:
				RpxLcdSetBlnk((USHORT)(LopV.ActiveLine+1), 0, 2, ResBuf);
				break;
			case LCDEM_PnData:
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
				if( act_optprm->State.Editing == TRUE )
				{	/* When option parameters are being edited */
					RpxLcdSetBlnk((USHORT)(LopV.ActiveLine+1),
								  (USHORT)(2+3+1 + LcdOptSetDataBlinkDPos(act_optprm)), 1, ResBuf);
				}
				else
				{
#endif
					Idx = pInf[LopV.ActiveLine].PnIdx;
					pPrmDef =GET_PNPRMDEF( CPARAMX->pPrmTbl, Idx ); /* (PRMDEF *)PnPrmTbl[Idx].PrmDefPtr; */
					RpxLcdSetBlnk( (USHORT)(LopV.ActiveLine+1),
							   (USHORT)(2+3+1 + LcdPrmDataBlinkDpos(pPrmDef)), 1, ResBuf );
#if CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
				}
#endif
				break;
			default:
				break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Reset Pn/UN Display Data 																	*/
/*																									*/
/****************************************************************************************************/
void	LcdRstPnUnDspData( void )
{
		LopV.ActiveLine = 0;
		LopV.EditMode = LCDEM_PnUnNum;
		LopV.EewriteMode = 0 /* OPWM_None */;
		LopV.CurDigit = 0;
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Parameter Blink Data position 													*/
/*																									*/
/****************************************************************************************************/
USHORT	LcdPrmDataBlinkDpos( CPRMDEF *pPrmDef )
{
USHORT	Dpos;
USHORT	Decimals;

/*--------------------------------------------------------------------------------------------------*/
/*		Set Dpos, Decimals																			*/
/*--------------------------------------------------------------------------------------------------*/
		Dpos = LopV.DataDigit - 1 - LopV.CurDigit;			/* "H.", the position without considering the decimal point		*/
		/*Decimals = GET_PNPRM_DECIMALS( pPrmDef );*/		/* Getting decimal point position						*/
		Decimals = GET_PNPRM_DECIMALS( CPARAMX, pPrmDef, &LopMsg );		 /* <S14F> Getting decimal point position	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Modify Dpos by Sign,Decimals,etc															*/
/*--------------------------------------------------------------------------------------------------*/
		if( GET_PNPRMBASE( pPrmDef ) == DSPDEF_BASEDEC )	/* Comment out because the base detects10Base	*/
		{
			if( CHK_PNPRM_SIGNED( pPrmDef )  == DSPDEF_W_SIGN )
			{
				Dpos++;	/* Signed amount	*/
			}
			if( (Decimals > 0) && (LopV.CurDigit < (USHORT)Decimals) )
			{
				Dpos++;	/* Decimal point	*/
			}
		}
		else												/* Comment out because the base detects10Non-hexadecimal	*/
		{
			Dpos += 2;	/* "n." or "H." minutes	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return( Dpos );
}

