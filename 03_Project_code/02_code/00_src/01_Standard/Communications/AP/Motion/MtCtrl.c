/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MtCtrl.c : Motion Control source file 														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Create Motion Command 																	*/
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
/*		Rev.1.00 : 2012.09.05  K.Ozaki																*/
/*																									*/
/****************************************************************************************************/
#define ACCESSX_MONITOR
#include "ScanN.h"
#include "ServoIf.h"
#include "Motion.h"
#include "MtCtrl.h"
//#include "XlsAlmDefIxOpt.h"

/****************************************************************************************************/
/*																									*/
/*		Motion Control Definitions																	*/
/*																									*/
/****************************************************************************************************/
#define MOTOR_STP_SPD	(0x10000)			/* Motor stop speed 24bit/OS / 256						*/

/*--------------------------------------------------------------------------------------------------*/
#define LPX_SQIGETDEC()   (( SqiV.Sigs ) & SQI_SIGS_DEC )
#define LPX_SQIGETDECEDGE()  ((!(SqiV.LastSigs & SQI_SIGS_DEC )) &&  \
							(( SqiV.Sigs ) & SQI_SIGS_DEC ))

/****************************************************************************************************/
/*																									*/
/*		Motion Control Function Prototypes															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* Public functions 																				*/
/*--------------------------------------------------------------------------------------------------*/
VOID MtcInit(MTHNDL* hdl, SV_OPTION_IF* svData);
VOID MtcInputData(MTHNDL* hdl, SV_OPTION_IF  *psvif );
VOID MtcOutputData(MTHNDL* hdl, SV_OPTION_IF *psvif );

VOID MtcCreateInCmd(MTHNDL* hdl);							/* create motion in command */
INT32 MtcReqLatch(MTHNDL* hdl, UCHAR mode, UCHAR sgnl);
VOID MtcCancelLatch(MTHNDL* hdl, UCHAR mode);
/*--------------------------------------------------------------------------------------------------*/
void	MtcSetHomeMethod(                                   /* set homing method */
				long prm);                                  /* method selection parameter */


/*--------------------------------------------------------------------------------------------------*/
/* Private functions(Sequence processing related) 															*/
/*--------------------------------------------------------------------------------------------------*/
VOID mtcCheckLatchComp(MTHNDL* hdl);

/*--------------------------------------------------------------------------------------------------*/
/* Private functions(MotionCommand creation processing) 															*/
/*--------------------------------------------------------------------------------------------------*/
UCHAR mtcMakeNopRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeFollowRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakePosingRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeInterpolRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeFeedRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeExFeedRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeHoldRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeExPosingRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeHomingRef(MTHNDL* hdl, MG_INOUT* cmdOut);			/* Homing(/DEC + CPHASE) */
UCHAR mtcMakeHomingRefDec(MTHNDL* hdl, MG_INOUT* cmdOut);		/* Homing(/DEC only) */
UCHAR mtcMakeHomingRefCphs(MTHNDL* hdl, MG_INOUT* cmdOut);		/* Homing(CPHASE only) */
UCHAR mtcMakeHomingRefPush(MTHNDL* hdl, MG_INOUT* cmdOut);		/* Homing(Push) */
UCHAR mtcMakeVelRef(MTHNDL* hdl, MG_INOUT* cmdOut);
UCHAR mtcMakeTrqRef(MTHNDL* hdl, MG_INOUT* cmdOut);

/*--------------------------------------------------------------------------------------------------*/
/* Private functions(Latch sequence)	 															*/
/*--------------------------------------------------------------------------------------------------*/
#if SVFSEL_SEQUENTIAL_LATCH == 1
	VOID mtcControlLatchSeq(MTHNDL* hdl);
#else
	#define mtcControlLatchSeq(hdl) {;}
#endif

/*--------------------------------------------------------------------------------------------------*/
/* motion state machine function table 																*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
typedef UCHAR (*CTRLFUNC)(MTHNDL* hdl);
const CTRLFUNC _mtcControlState[] = { /* <V715> */
	mtcDisableState,
	mtcWaitPgRdyState,
	mtcReadyState,
	mtcRunState,
	mtcWaitSvOffState,
	mtcRampStopState,
};
#endif

/*--------------------------------------------------------------------------------------------------*/
/* Motion function table																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef UCHAR (*MAKECMDFUNC)(MTHNDL* hdl, MG_INOUT* cmdOut);
const MAKECMDFUNC _mtcMakeCmd[] = { /* <V715> */
	mtcMakeNopRef,					/* 0:NopMotion */
	mtcMakePosingRef,				/* 1:Posing */
	mtcMakeNopRef,//	mtcMakeExPosingRef,				/* 2:ExPosing */
	mtcMakeNopRef,//	mtcMakeHomingRef,				/* 3:Homing */
	mtcMakeFeedRef,					/* 4:Feed */
	mtcMakeNopRef,//	mtcMakeExFeedRef,				/* 5:ExFeed */
	mtcMakeHoldRef,					/* 6:Hold */
	mtcMakeInterpolRef,				/* 7:Interpolate */
	mtcMakeVelRef,					/* 8:Velctrl */
	mtcMakeTrqRef,					/* 9:Trqctrl */
	mtcMakeFollowRef,				/* A:Folloup */
};

/****************************************************************************************************/
/*																									*/
/*		MtCtrl Variables 																			*/
/*																									*/
/****************************************************************************************************/
MAKECMDFUNC mtcMakeCmd[sizeof(_mtcMakeCmd)/sizeof(MAKECMDFUNC)];

/*--------------------------------------------------------------------------------------------------*/
/* Latch monitor numberTable 																			*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR _LtMonTbl[] = {/* <V715> */
	MONSEL_LTPOS_C_L, MONSEL_LTPOS_EXT1_L, MONSEL_LTPOS_EXT2_L, MONSEL_LTPOS_EXT3_L
};

UCHAR LtMonTbl[sizeof(_LtMonTbl)/sizeof(UCHAR)];

/****************************************************************************************************/
/*																									*/
/*		Initialize Motion Control 																	*/
/*																									*/
/****************************************************************************************************/
#pragma arm section code = "IXX_EXCODE"
VOID MtcInit(MTHNDL* hdl, SV_OPTION_IF* svIf)
{
int i;
	MlibResetLongMemory( hdl,  sizeof(MTHNDL)/4 );					/* Reset Memory */
/*--------------------------------------------------------------------------------------------------*/
	PsnInit(&hdl->psnHdl, &hdl->mtPrm.PsnPrm);						/* Initialize Position */
	MtgInit(&hdl->mtgHdl, &hdl->mtPrm.BasePrm, &hdl->mtPrm.MgPrm);	/* Initialize Generator */
/*--------------------------------------------------------------------------------------------------*/
	hdl->crtMtype = MTYPE_NONE;
	hdl->ctrlMode = MT_NO_CTRLMD;
	hdl->seqState = 0;
	hdl->svCmd = &svIf->CycCmd;
	hdl->svRsp = &svIf->CycRsp;
	
/*--------------------------------------------------------------------------------------------------*/
/* ConstData tableRAM 														*//* <V715> */
/*--------------------------------------------------------------------------------------------------*/
	for( i=0; i < (sizeof(mtcMakeCmd)/4) ; i++) { mtcMakeCmd[i] = _mtcMakeCmd[i]; }

/*--------------------------------------------------------------------------------------------------*/
	return;
}
#pragma arm section code

/****************************************************************************************************/
/*																									*/
/*		Output Motion Data 																			*/
/*																									*/
/****************************************************************************************************/
VOID MtcOutputData(MTHNDL* hdl, SV_OPTION_IF *psvif)
{
SV_CYC_CMD	*psvcmd = &(psvif->CycCmd);
	psvcmd->CmdCtrl.ModSel = hdl->ctrlMode;             /* Set Control Mode */
	psvcmd->Position += hdl->outData.DposOut;			/* Set Position Command */
	psvcmd->Speed = hdl->outData.VrefOut;				/* Set Velocity Command */
	psvcmd->Torque = hdl->outData.TrefOut;				/* Set Torque Command */
}

/****************************************************************************************************/
/*																									*/
/*		Input Motion Data  																			*/
/*																									*/
/****************************************************************************************************/
VOID MtcInputData(MTHNDL* hdl, SV_OPTION_IF *psvif)
{
#define SviGetVPosErr( x )         ((x)->Mon4)
#define SviResetPos( x, data )     ((x)->Position = (data))
LONG fbSpd;
INT pls, nls;
LONG PosErr;	/* <S073> */
SV_CYC_RSP *psvrsp = &(psvif->CycRsp);
//UINT isMotorMoving = IXX_BITCHK( psvrsp->RspCtrl.RspCtrlBit, MOTOR_MOVING_BITNO );

	PosErr = hdl->svRsp->PosError;                                   /* <S073> */
	PsnUpdateFBData(&hdl->psnHdl, psvrsp->FbPosition);               /* update fb position */

	mtcCheckLatchComp(hdl);                                          /* update latch status */

	if(hdl->ctrlMode != MT_POS_CTRLMD)
	{
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		fbSpd = 0;
#else
		fbSpd = (hdl->otState != OTSTAT_NORMAL) ? 0 : PsnGetFBSPD(&hdl->psnHdl);
#endif
		MtgSetCspd(&hdl->mtgHdl, fbSpd);

		/* FollowUpofM2/M3 <S073> */
		if( (hdl->svRsp->RspCtrl.RspCtrlBit & MOTOR_MOVING_BITMSK) != FALSE )
		{
			PsnResetCmdPosAll( &hdl->psnHdl, PosErr );
			SviResetPos(hdl->svCmd, psvrsp->FbPosition + PosErr );
		}
		else
		{
			PsnResetCmdPosAll(&hdl->psnHdl, 0);
			SviResetPos(hdl->svCmd, psvrsp->FbPosition );
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	else if( psvrsp->RspCtrl.RspCtrlBit & (1<<PERCLREDGE_BITNO) )								
	{
		/* 
		 * @@ Perr Clear Edge 
		 * 1) If control mode is positioning and /clr signal edge detected,
		 *    reset command position to the latched position.
		 * 2) Command pulse is compensated by ScanB.
		 * */
		LONG err = ( IXX_GET_PERCLRPOS(psvif) - MtHdl.psnHdl.Pgcnt );
		PsnResetCmdPosAll( &(hdl->psnHdl), err );
	}
/*--------------------------------------------------------------------------------------------------*/
	PsnCheckSoftLimitFB( &(hdl->psnHdl), &pls, &nls );	/* check sot status */
}


/****************************************************************************
****************************************************************************/
/* Public functions (Operation function) */
/****************************************************************************
****************************************************************************/

/****************************************************************************************************/
/*																									*/
/*		Requesting Latch 																			*/
/*																									*/
/****************************************************************************************************/
INT32 MtcReqLatch(MTHNDL* hdl, UCHAR mode, UCHAR sgnl)
{
#define GET_LTSGNL_NO(no) (no)
#define SviGetLtReq(Cmd)  ((Cmd)->CmdLtBit)
#define SviSetLtReq(Cmd, data)  ((Cmd)->CmdLtBit = (data))
#define SviSetLtMonSel(Cmd, data) ((Cmd)->SelMon1 = LtMonTbl[(data)])
	UCHAR	ltReq = SviGetLtReq(hdl->svCmd);

	sgnl = GET_LTSGNL_NO(sgnl);

	hdl->latch.Mode = mode;
	hdl->latch.Cmp = 0;
	hdl->latch.Req = TRUE;
	hdl->latch.Sgnl = sgnl;

	if(IXX_BITCHK(ltReq, sgnl+4))
	{
		IXX_BITSET(ltReq, sgnl+4, FALSE);
	}
	else
	{
		IXX_BITSET(ltReq, sgnl+4, TRUE);
	}

	SviSetLtReq(hdl->svCmd, ltReq);
	SviSetLtMonSel(hdl->svCmd, sgnl);

	return 0;
}

/****************************************************************************************************/
/*																									*/
/*		Cancel Latch 																				*/
/*																									*/
/****************************************************************************************************/
VOID MtcCancelLatch(MTHNDL* hdl, UCHAR mode)
{
	hdl->latch.Cmp = 0;
	hdl->latch.Req = 0;
	hdl->latch.Mode = mode;

	hdl->latch.SeqReq = 0;
	hdl->latch.SeqCnt = 0;
	hdl->latch.SeqSts.wdCnt = 0;
}

/****************************************************************************************************/
/*																									*/
/*  Continuous latch request processing                                                                              */
/*																									*/
/****************************************************************************************************/
#if SVFSEL_SEQUENTIAL_LATCH == 1
VOID mtcControlLatchSeq(MTHNDL* hdl)
{
/*--------------------------------------------------------------------------------------------------*/
/* Latch sequence request check                                                                     */
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->latch.SeqReq != MTLT_SEQ_REQ) return;

/*--------------------------------------------------------------------------------------------------*/
/* Update sequence counter                                                                           */
/*--------------------------------------------------------------------------------------------------*/
	hdl->latch.SeqCnt++;
	hdl->latch.SeqSts.cnt[1] = hdl->latch.SeqCnt;
	if(hdl->latch.SeqCnt >= hdl->mtPrm.LtPrm.seqNum)
	{                                                                          /*Sequence complete */
		hdl->latch.SeqCnt = 0;
		hdl->latch.SeqSts.cnt[0]++;
		if((hdl->mtPrm.LtPrm.loopCnt != 0) &&
			(hdl->latch.SeqSts.cnt[0] >= hdl->mtPrm.LtPrm.loopCnt))
		{                                                                      /* Continuous latch completion */
			hdl->latch.SeqReq = MTLT_SEQ_DONE;
			return;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/* Latch request issue */
/*--------------------------------------------------------------------------------------------------*/
	MtcReqLatch(hdl, hdl->latch.Mode, 
		hdl->mtPrm.LtPrm.ltSgnl[hdl->latch.SeqCnt]);
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Check latch completion 																		*/
/*																									*/
/****************************************************************************************************/
VOID mtcCheckLatchComp(MTHNDL* hdl)
{
#define	SviChkLtMonSel(Rsp, sgnl)	((Rsp)->SelMon1 == LtMonTbl[sgnl])	/* Latch monitor setting */
#define SviGetLtPos(Rsp)            ((Rsp)->Mon1)                       /* Latch Position */
#define SviGetLtSts(Rsp)			((Rsp)->RspLtBit)
UINT req;
UINT cmp;
UINT ack;
/*--------------------------------------------------------------------------------------------------*/
	if(!hdl->latch.Req) return;	/* Latch request not present.Forced return */
/*--------------------------------------------------------------------------------------------------*/
	req = SviGetLtReq(hdl->svCmd);
	ack = SviGetLtSts(hdl->svRsp);

	req = IXX_BITCHK(req, hdl->latch.Sgnl+4);
	cmp = IXX_BITCHK(ack, hdl->latch.Sgnl+4);
	ack = IXX_BITCHK(ack, hdl->latch.Sgnl);
/*--------------------------------------------------------------------------------------------------*/
	if( (req == ack) && (cmp == TRUE)
		&& SviChkLtMonSel(hdl->svRsp, hdl->latch.Sgnl) )
	{                                                                           /* Latch completion */
		if((hdl->latch.LtDisable != FALSE)                                      /* Latch disabled or*/
			|| (PsnMakeLpos(&hdl->psnHdl, SviGetLtPos(hdl->svRsp)) == FALSE))   /* Outside latch area   */
		{                                                                       
			MtcReqLatch(hdl, hdl->latch.Mode, hdl->latch.Sgnl);                 /* Latch request issue */
		}
		else
		{                                                          /* Latch enabled && Within latch area */
			hdl->latch.Req = FALSE;			                       /* Latch requestOFF */
			mtcControlLatchSeq(hdl);		                       /* Latch sequence activation */
			hdl->latch.Cmp = MTLT_CMP_STS;	                       /* Latch completionON */
		}
	}
}


/****************************************************************************
****************************************************************************/
/* Private functions */
/****************************************************************************
****************************************************************************/

/****************************************************************************
 Motion state control(DisableStatus)
****************************************************************************/
#if 0
UCHAR mtcDisableState(MTHNDL* hdl)
{
	BOOL	senCmd;
	UCHAR	rc = MSTAT_DISABLE;
/* Motion command initialization */
	hdl->cmdData.Mtype = MTYPE_NONE;
	hdl->cmdData.ServoOn = FALSE;
	hdl->ctrlMode = MT_NO_CTRLMD;	//MT_VEL_CTRLMD;
	hdl->cmdData.CtrlMode = MT_POS_CTRLMD;
	SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
	SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
/* Motion valid check */
	if(hdl->cmdData.MtEenable)
	{
		rc = MSTAT_WAITPG_RDY;
		senCmd = (hdl->mtPrm.PsnPrm.absPG != FALSE)	/* absHour:SensOnCommand status, incrHour:1 */
						? hdl->cmdData.SensOn : TRUE;
		SviSetSensOn(hdl->svCmd, senCmd);
		if(senCmd && SviGetSensOnSts(hdl->svRsp) /*&& SviGetOpReadySts(hdl->svRsp)*/)
		{/* SensorONStatus */
			hdl->cmdData.SensOn = TRUE;
			hdl->cmdData.Mtype = MTYPE_FOLLOW;
			hdl->ctrlMode = MT_POS_CTRLMD;
			PsnResetApos(&hdl->psnHdl, TRUE);
			rc = MSTAT_READY;
		}
	}
	return rc;
}
#endif

/****************************************************************************
 Motion state control(WaitPGRdyStatus)
****************************************************************************/
#if 0
UCHAR mtcWaitPgRdyState(MTHNDL* hdl)
{
	BOOL	senCmd;
	UCHAR	rc = MSTAT_WAITPG_RDY;
/* Motion command initialization */
	hdl->cmdData.Mtype = MTYPE_NONE;
	hdl->cmdData.ServoOn = FALSE;
	hdl->ctrlMode = MT_VEL_CTRLMD;
	SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
	SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
/* If there is a motion invalid requestDisbleto the state */
	if(!hdl->cmdData.MtEenable)
		return MSTAT_DISABLE;
/* PGRDY,Followup=ON,APOSSetReadyto the state */
	senCmd = (hdl->mtPrm.PsnPrm.absPG != FALSE)	/* absHour:SensOnCommand status, incrHour:1 */
						? hdl->cmdData.SensOn : TRUE;
	SviSetSensOn(hdl->svCmd, senCmd);
	if(senCmd && SviGetSensOnSts(hdl->svRsp) /*&& SviGetOpReadySts(hdl->svRsp)*/)
	{
		hdl->cmdData.SensOn = TRUE;
		hdl->cmdData.Mtype = MTYPE_FOLLOW;
		hdl->cmdData.CtrlMode = MT_POS_CTRLMD;
		hdl->ctrlMode = MT_POS_CTRLMD;
		PsnResetApos(&hdl->psnHdl, TRUE);
		rc = MSTAT_READY;
	}
	return rc;
}
#endif

/****************************************************************************
 Motion state control(ReadyStatus)
****************************************************************************/
#if 0
UCHAR mtcReadyState(MTHNDL* hdl)
{
	UCHAR	rc = MSTAT_READY;
	BOOL	svOnCmp = SviGetServoOnSts(hdl->svRsp);

/* If there is a motion invalid requestDisbleto the state */
	if(!hdl->cmdData.MtEenable)
	{
		hdl->cmdData.Mtype = MTYPE_NONE;
		hdl->ctrlMode = MT_NO_CTRLMD;
		return MSTAT_DISABLE;
	}
/* SensOffCommandOFF WaitPGRdyto the state */
	if(!hdl->cmdData.SensOn)
	{
		SviSetSensOn(hdl->svCmd, FALSE);
		hdl->cmdData.Mtype = MTYPE_NONE;
		hdl->ctrlMode = MT_VEL_CTRLMD;
		PsnResetApos(&hdl->psnHdl, FALSE);
		return MSTAT_WAITPG_RDY;
	}
/* ServoONCommand issue*/
	SviSetServoOn(hdl->svCmd, hdl->cmdData.ServoOn);

	if(!SviGetOpReadySts(hdl->svRsp))
	{/* JOGIf in middle,JOGForcibly enter speed control mode */
		hdl->cmdData.Mtype = MTYPE_NONE;
		hdl->ctrlMode = MT_VEL_CTRLMD;
	}
	else
	{/* Follow-up & ServoONCommand issue*/
		if(hdl->ctrlMode == MT_POS_CTRLMD)
			hdl->cmdData.Mtype = MTYPE_FOLLOW;
	/* ServoONIf complete, go to completion stepRUNto the state */
		if(hdl->cmdData.ServoOn && svOnCmp)
		{
			return MSTAT_RUN;
		}
	}
	return rc;
}
#endif

/****************************************************************************
 Motion state control(RunStatus)
****************************************************************************/
#if 0
UCHAR mtcRunState(MTHNDL* hdl)
{
	BOOL	svOnCmp;
	UCHAR	rc = MSTAT_RUN;

/* ServoOFFReadyto the state */
	svOnCmp = SviGetServoOnSts(hdl->svRsp);
	if(!svOnCmp)
	{
		hdl->cmdData.ServoOn = FALSE;
		SviSetServoOn(hdl->svCmd, FALSE);
		if(hdl->ctrlMode != MT_TRQ_CTRLMD)
		{	hdl->cmdData.Mtype = MTYPE_NONE;	/* Command stop*/}
		SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
		SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
		return MSTAT_READY;
	}
/* ServoOnCommand=OFFIf servoOFFGo to waiting state */
	if(!hdl->cmdData.ServoOn)
	{
		SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
		SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
		if(hdl->mtPrm.MtSeqPrm.rampTimeOut == 0)
		{/* RampStopFunction disabled */
			SviSetServoOn(hdl->svCmd, FALSE);
			if(hdl->ctrlMode != MT_TRQ_CTRLMD)
			{	hdl->cmdData.Mtype = MTYPE_NONE;	/* Command stop*/}
			return MSTAT_WAIT_SVOFF;
		}
		else
		{/* RampStopFunction enabled */
			hdl->cmdData.Mtype = MTYPE_HOLD;
			hdl->cmdData.CtrlMode = MT_POS_CTRLMD;
			hdl->ctrlMode = MT_POS_CTRLMD;
			hdl->cmdData.HoldMod = 2;
			hdl->timer = 0;
			return MSTAT_RAMP_STOP;
		}
	}
	return rc;
}
#endif

/****************************************************************************
 Motion state control(WaitSvOffStatus)
****************************************************************************/
#if 0
UCHAR mtcWaitSvOffState(MTHNDL* hdl)
{
	BOOL	svOnCmp;
	UCHAR	rc = MSTAT_WAIT_SVOFF;
/* ServoOFFCommand issue */
	SviSetServoOn(hdl->svCmd, FALSE);
/* ServoOFFCommand stops until */
	if(hdl->ctrlMode != MT_TRQ_CTRLMD)
	{	hdl->cmdData.Mtype = MTYPE_NONE;}
	SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
	SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
/* ServoOFFReadyto the state */
	svOnCmp = SviGetServoOnSts(hdl->svRsp);
	if(!svOnCmp)
		return MSTAT_READY;

	return rc;
}
#endif

/****************************************************************************
 Motion state control(RampStopStatus)
****************************************************************************/
#if 0
UCHAR mtcRampStopState(MTHNDL* hdl)
{
	BOOL	svOnCmp;
	UCHAR	rc = MSTAT_RAMP_STOP;
	BOOL	den = PsnGetDenSts(&hdl->psnHdl);
/* Deceleration and stop command set */
	hdl->cmdData.Mtype = MTYPE_HOLD;
	hdl->ctrlMode = MT_POS_CTRLMD;
	hdl->cmdData.HoldMod = 2;
	SviSetTrqLimitP(hdl->svCmd, MAX_TRQ24);
	SviSetTrqLimitN(hdl->svCmd, MAX_TRQ24);
/* ServoOFFReadyto the state */
	svOnCmp = SviGetServoOnSts(hdl->svRsp);
	if(!svOnCmp)
	{
		hdl->cmdData.ServoOn = FALSE;
		SviSetServoOn(hdl->svCmd, FALSE);
		return MSTAT_READY;
	}
/* When deceleration to stop or timeoutWaitSvOffto the state */
	++hdl->timer;
	if( (hdl->timer > hdl->mtPrm.MtSeqPrm.rampTimeOut)	/* Timeout */
		|| (den && ( ABS( SviGetSpeed(hdl->svRsp)) < MOTOR_STP_SPD)) )/* den=1&&Motor stopped */
	{
		SviSetServoOn(hdl->svCmd, FALSE);
		return MSTAT_WAIT_SVOFF;
	}
	return rc;
}
#endif

/****************************************************************************
 OTState control(OTDuring stop operation rc = 0, Other commands rc = 1)
****************************************************************************/
#if 0
#ifndef OT_STOP_POSCTRL
BOOL mtcControlOTState(MTHNDL* hdl)
{
	BOOL p_ot, n_ot;
	LONG dpos, wk, remdist;	/* <V656,V679> */

/* APOSSoft limit check */
	PsnCheckSoftLimitFB(&hdl->psnHdl, &p_ot, &n_ot);
/* OTSignal check */
	p_ot |= SviGetPOTSig(hdl->svRsp);
	n_ot |= SviGetNOTSig(hdl->svRsp);
/* OTCommand set */
	SviSetPOT(hdl->svCmd, p_ot);
	SviSetNOT(hdl->svCmd, n_ot);

/* OTCheck release condition */
	if( ((p_ot | n_ot) == 0)							/* OTSignal release */
		|| (SviGetServoOnSts(hdl->svRsp) == FALSE)		/* ServoOFFStatus */
		|| (hdl->cmdData.CtrlMode != MT_POS_CTRLMD)) 	/* Speed/torque control */
	{/* OTCancellation */
		hdl->otState = OTSTAT_NORMAL;
		return TRUE;
	}

/* Checking pullback conditions */
	dpos = PsnGetAPOS(&hdl->psnHdl) - PsnGetPOS(&hdl->psnHdl);
/* Perform saturation addition processing as calculation result may overflow(Command filter accumulation31bit) *//* <V656> */
	wk = MlibSatAdd32(hdl->outData.DposIn, PsnGetIPOS(&hdl->psnHdl) - PsnGetPOS(&hdl->psnHdl));
//	dpos = (hdl->otState != OTSTAT_STOP) ? wk : (wk - dpos);
	dpos = (hdl->otState != OTSTAT_STOP) ? wk : MlibSatAdd32(wk, -dpos);/* <V776> */

	if( (p_ot && (!n_ot) && (dpos < 0))
		|| (n_ot && (!p_ot) && (dpos > 0)) )
	{/* With pullback */
//		if((hdl->outData.Tspd != 0) )
		if((hdl->outData.Tspd != 0) || (hdl->otState == OTSTAT_RETRACTION) )/* <V776> */
		{/* <V776> Target speed≠0 or Pulling back */
//			hdl->otState = OTSTAT_WAITNORMAL;
			hdl->otState = OTSTAT_RETRACTION;	/* <V679> */
			return TRUE;
		}
	}
	
/* Checking pullback end conditions <V679> */
	if((hdl->otState == OTSTAT_RETRACTION) && (dpos == 0))
	{
		remdist = PsnGetTPOS(&hdl->psnHdl) - PsnGetAPOS(&hdl->psnHdl);
		if( (p_ot && (!n_ot) && (remdist < 0))
			|| (n_ot && (!p_ot) && (remdist > 0)) )
		{
			return TRUE;
		}
	}

/* OTState control */
	hdl->ctrlMode = MT_VEL_CTRLMD;
	hdl->cmdData.Mtype = hdl->crtMtype = MTYPE_NONE;	/* Motion Type Clear */
	mtcMakeNopRef(hdl, &hdl->outData);			/* Command0 */
	//hdl->cmdData.AccFilSel = ACF_SW_NOFIL;		/* Filter clear */
	//AcfClear(&hdl->acfHdl);						/* Filter clear */
	PsnResetIPOS(&hdl->psnHdl);					/* IPOSReset */
	PsnSetTPOSNoLimit(&hdl->psnHdl, hdl->outData.DposIn);
	if(ABS(SviGetSpeed(hdl->svRsp)) > MOTOR_STP_SPD)
	{
//		if(hdl->otState != OTSTAT_WAITNORMAL) 
		if((hdl->otState != OTSTAT_WAITNORMAL) && (hdl->otState != OTSTAT_RETRACTION))	/* <V679> */
		{	hdl->otState = OTSTAT_STOP;}		/* OTSTAT_WAITNORMAL, leave the state unchanged */
		return FALSE;
	}
	hdl->otState = OTSTAT_WAITNORMAL;

	return TRUE;
}
#else
BOOL mtcControlOTState(MTHNDL* hdl)
{
	BOOL p_ot, n_ot;
	UCHAR f_cmp;
	LONG perr;
	BOOL rc = TRUE;

/* APOSSoft limit check */
	PsnCheckSoftLimitFB(&hdl->psnHdl, &p_ot, &n_ot);
/* OTSignal check */
	p_ot |= SviGetPOTSig(hdl->svRsp);
	n_ot |= SviGetNOTSig(hdl->svRsp);
/* OTCommand set */
	SviSetPOT(hdl->svCmd, p_ot);
	SviSetNOT(hdl->svCmd, n_ot);

/* OTCheck release condition */
	if( ((p_ot | n_ot) == 0)							/* OTSignal release */
		|| (SviGetServoOnSts(hdl->svRsp) == FALSE)		/* ServoOFFStatus */
		|| (hdl->cmdData.CtrlMode != MT_POS_CTRLMD)) 	/* Speed/torque control */
	{/* OTCancellation */
		hdl->otState = OTSTAT_NORMAL;
		return rc;
	}
	/* ★Caution★ When enabling this processing,<V656>horizontally */
	if( (p_ot && (!n_ot) && (hdl->outData.DposIn < 0))
		|| (n_ot && (!p_ot) && (hdl->outData.DposIn > 0)) )
	{/* With pullback */
		hdl->otState = OTSTAT_WAITNORMAL;
		return rc;
	}

/* OTState control */
	switch(hdl->otState)
	{
	case OTSTAT_NORMAL:	/* Normal operation status */
		hdl->otState = OTSTAT_STOP;
		/* break; ↓↓↓Go directly to the stop process↓↓↓ */

	case OTSTAT_STOP:	/* OTStopping */
		f_cmp = mtcMakeFollowRef(hdl, &hdl->outData);					/* Followup */
		if(ABS(SviGetSpeed(hdl->svRsp)) < MOTOR_STP_SPD)
		{/* Motor stopped */
			if((f_cmp != FALSE) && (PsnGetDenSts(&hdl->psnHdl) != FALSE))
			{/* DEN=ON, the nextStatefart */
				hdl->otState = OTSTAT_WAITNORMAL;
			}
		}
		else
		{/* Forced while the motor is rotatingOT, and sure to stop the motor */
			SviSetPOT(hdl->svCmd, TRUE);							/* P-OTForcedON */
			SviSetNOT(hdl->svCmd, TRUE);							/* N-OTForcedON */
			hdl->outData.DposIn = PsnGetFBSPD(&hdl->psnHdl);		/* Advance command */
			PsnSetTPOSNoLimit(&hdl->psnHdl, hdl->outData.DposIn);	/* TPOSReset */
			rc = FALSE;
		}
		break;

	case OTSTAT_WAITNORMAL:	/* OTWaiting for release */
		mtcMakeFollowRef(hdl, &hdl->outData);
		if(!hdl->mtPrm.BasePrm.ot_zstop)
		{/* After emergency stop, other than motor lock mode */
			perr = PsnGetPOS(&hdl->psnHdl) - PsnGetAPOS(&hdl->psnHdl);
			if(((perr > 0) & p_ot) || ((perr < 0) & n_ot))
			{/* Deviation direction=OTDirection */
				break;
			}
		}
		mtcMakeNopRef(hdl, &hdl->outData);		/* NOPCommand */
		PsnSetTPOSNoLimit(&hdl->psnHdl, 0);		/* TPOSReset */
		break;

	default : break;
	}
	return rc;
}
#endif
#endif


/****************************************************************************************************/
/*																									*/
/*		Create Motion Reference																		*/
/*																									*/
/****************************************************************************************************/
VOID MtcCreateInCmd(MTHNDL* hdl)
{
UCHAR	rc;
/*--------------------------------------------------------------------------------------------------*/
/* Motion switching→Motion restart                                                       */
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.Mtype != hdl->crtMtype)
	{
		hdl->seqState = 0;					                   /* Motion Sequence status clear */
	}

/*--------------------------------------------------------------------------------------------------*/
/* Acceleration/decelerationPRMSwitch                                                                                */
/*--------------------------------------------------------------------------------------------------*/
#if 0
	if(MBnkIsEnable(&hdl->mtPrm)) 
	{/* Bank enabled(BankPRMSet) */
		if(PsnGetDenSts(&hdl->psnHdl))
		{	MBnkChange(&hdl->mtPrm);	/* BankChange */}

		AcfAttachPRM(&hdl->acfHdl, MBnkGetAcfPrm(&hdl->mtPrm));
		if(hdl->cmdData.CmdRateOn)
		{/* Acceleration/deceleration command enabled(CommandPRMSet)<Rev2.01> */	
			MtgAttachPRM(&hdl->mtgHdl, &hdl->mtPrm.CmdRate);
		}
		else
		{/* Acceleration/deceleration command disabled(BankPRMSet) */
			MtgAttachPRM(&hdl->mtgHdl, MBnkGetMgPrm(&hdl->mtPrm));
		}
	}
	else
#endif
	{                                                                         /* Bank disabled */
		//AcfAttachPRM(&hdl->acfHdl, &hdl->mtPrm.AccFilPrm);
		if(0 /* hdl->cmdData.CmdRateOn */)
		{                                                    /* Acceleration/deceleration command enabled(CommandPRMSet)<Rev2.01> */	
			MtgAttachPRM(&hdl->mtgHdl, &hdl->mtPrm.CmdRate);
		}
		else
		{                                                    /* Acceleration/deceleration command disabled(DefaultPRMSet) */
			MtgAttachPRM(&hdl->mtgHdl, &hdl->mtPrm.MgPrm);
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/* Motion command creation processing                                                                           */
/*--------------------------------------------------------------------------------------------------*/
	rc = mtcMakeCmd[hdl->cmdData.Mtype](hdl, &hdl->outData);
	hdl->crtMtype = hdl->cmdData.Mtype; 

/*--------------------------------------------------------------------------------------------------*/
#if 0
   /* 
    * If there is a process below,FollowUpCommand1times.
    * (AxctrltoFollowUpIf you do not continue issuing commands, position deviation will accumulate.)
    * IndexerDisabling
    */
	if(rc != FALSE)
	{
		hdl->cmdData.Mtype = MTYPE_NONE;	/* Go to neutral*/
	}
#endif
	hdl->cmdData.CmdRstReq = FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Nop Referece																			*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeNopRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
	cmdOut->Mtype = MNONE;
	cmdOut->DposIn = 0;
	cmdOut->Tspd = 0;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return TRUE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Followup reference																	*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeFollowRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
	UCHAR rc = FALSE;

	cmdOut->Mtype = MFOLLOW;
	cmdOut->DposIn = PsnGetAPOS(&hdl->psnHdl) - PsnGetPOS(&hdl->psnHdl);
/*--------------------------------------------------------------------------------------------------*/
/*  7 pulseDo not use the following commands(Sigma-S)																*/
/*--------------------------------------------------------------------------------------------------*/
//	cmdOut->DposIn = (((MlibABS( cmdOut->DposIn ) >> 3) << 3) ? cmdOut->DposIn : 0);
/*--------------------------------------------------------------------------------------------------*/
	if(cmdOut->DposIn < (-FOLLOWUP_MAX_REF))
		cmdOut->DposIn = -FOLLOWUP_MAX_REF;	                 /* Clamp*/
	else if(cmdOut->DposIn > FOLLOWUP_MAX_REF)
		cmdOut->DposIn = FOLLOWUP_MAX_REF;	                 /* Clamp*/
	else
		rc = TRUE;

	cmdOut->Tspd = 0;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	PsnResetIPOS(&hdl->psnHdl);		                        /* IPOSReset */
	PsnSetTPOSNoLimit(&hdl->psnHdl, cmdOut->DposIn);

	return rc;
}

/****************************************************************************************************/
/*																									*/
/*		Create Posing reference																		*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakePosingRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
	if(hdl->cmdData.bHold)													/* Hold */
	{
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
	else if(hdl->cmdData.CmdRstReq)											/* cmd reset */
	{
		PsnSetTPOS(&hdl->psnHdl, hdl->cmdData.TPos);
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		//cmdOut->Tspd = hdl->cmdData.TSpeed;
		cmdOut->Tspd = MlibMulgain29( hdl->cmdData.TSpeed, hdl->cmdData.TspdKx );
	}
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		cmdOut->Tspd = MlibMulgain29( hdl->cmdData.TSpeed, hdl->cmdData.TspdKx );
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Interpolate reference																*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeInterpolRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{

	if(hdl->cmdData.CmdRstReq)
	{
		cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, hdl->cmdData.TPos);
		cmdOut->Vref = hdl->cmdData.Velocity;
		cmdOut->Tref = hdl->cmdData.Trq;
		hdl->segCnt = hdl->mtPrm.BasePrm.seg_num;
	}
	else if(hdl->segCnt == 0)
	{/* Remaining number of payouts=0, it is finished */
		return mtcMakeNopRef(hdl, cmdOut);
	}
	hdl->segCnt--;

	cmdOut->Mtype = MINTERPOLATE;
	cmdOut->Tspd = cmdOut->DposIn;	/* dummy(OTFor processing check) */

	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Feed Reference																		*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeFeedRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)													/* Hold */
	{
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
	else if(hdl->cmdData.TSpeed < 0)
	{                                                                       /* Negative direction */
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, -DISTANCE_MAX);
		cmdOut->Tspd = MlibMulgain29( -(hdl->cmdData.TSpeed), hdl->cmdData.TspdKx );
	}
	else
	{                                                                       /* forward direction */
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, DISTANCE_MAX);
		cmdOut->Tspd = MlibMulgain29( hdl->cmdData.TSpeed, hdl->cmdData.TspdKx );
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Hold Reference																		*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeHoldRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
LONG dpos;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.HoldMod == 1)                                   /* Rapid stop */
	{                                                               
		dpos = 0;
		cmdOut->Mtype = MNONE;
	}
/*--------------------------------------------------------------------------------------------------*/
#if SVFSEL_HOLD_MOD_2 == 1                                         /* Emergency stop deceleration enabled(Pn826Other) */
	else if(hdl->cmdData.HoldMod == 2)                             /* Deceleration stop2 */
	{                                                               
		cmdOut->Mtype = MHOLD;
		cmdOut->Ctrl = MG_HOLD_CTRL2;
		dpos = MtgGetDctDistance(&hdl->mtgHdl,
					MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
	else                                                           /* Deceleration stop1 */
	{                                                               
		cmdOut->Mtype = MHOLD;
		cmdOut->Ctrl = MG_HOLD_CTRL1;
		dpos = MtgGetDctDistance(&hdl->mtgHdl, 
					MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, dpos);
	cmdOut->Tspd = 0;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create ExPosing Reference																	*/
/*																									*/
/****************************************************************************************************/
#if 0
UCHAR mtcMakeExPosingRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
long wkx;
long seq;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                                /* Hold */
	{
		seq = 4;
	}
	else if(hdl->cmdData.CmdRstReq)
	{
		seq = hdl->seqState = 1;	                                      /* When resetting, start from the beginning */
	}
	else
	{
		seq = hdl->seqState;
	}

/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                            /* Latch completion state */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                            /* EX_POSINGStart */
/*--------------------------------------------------------------------------------------------------*/
		MtcCancelLatch(hdl, EXPOSING_LTMD);							   /* Latch cancel */
		MtcReqLatch(hdl, EXPOSING_LTMD, hdl->cmdData.LatchSgnl);	   /* Latch request issue */
		cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, hdl->cmdData.TPos);
		hdl->seqState++;
		break;

/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                            /* Latch waiting state */
/*--------------------------------------------------------------------------------------------------*/
		if(hdl->latch.Cmp && (hdl->latch.Mode == EXPOSING_LTMD))
		{                                                              /* Latch completion */
			wkx = MtgGetDctDistance( &(hdl->mtgHdl),                   /* calculate distance */
							MtgGetCspd(&(hdl->mtgHdl)),
							MG_HOLD_CTRL1 );

			if ( wkx > (hdl->cmdData.Rdst) ) 
			{
				IXX_ALARMSET( ALMBITS_REGDISTSHORT ); /* A.E23 Registration distance insufficient alarm */
				cmdOut->DposIn = 0;
				hdl->seqState = 3;
			}
			else
			{
				wkx = ( (hdl->mtgHdl.cMinusDir) ?                       /* wkx <- registration offset */
				    ((-1)*(hdl->cmdData.Rdst)) : (hdl->cmdData.Rdst) );
				wkx =  PsnGetLPOS(&hdl->psnHdl) + wkx;                  /* wkx <- updated tpos */
				cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, wkx);
				hdl->seqState = 0;
			}
		}
		else
		{                                                              /* Latch incomplete */
			cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, hdl->cmdData.TPos);
			if(cmdOut->DposIn == 0)	                                   /* End if positioning is complete */
				hdl->seqState = 0;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:                                                            /* Alarm status */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = 0;
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 4:                                                            /* HOLDStatus */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		break;
/*--------------------------------------------------------------------------------------------------*/
	default:     
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = 0;
		break;
/*--------------------------------------------------------------------------------------------------*/
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	//cmdOut->Tspd = hdl->cmdData.TSpeed;
	//cmdOut->Tspd = (hdl->seqState) ? (hdl->cmdData.TSpeed) : (hdl->cmdData.Rspd);
	wkx = (hdl->seqState) ? (hdl->cmdData.TSpeed) : (hdl->cmdData.Rspd);
	cmdOut->Tspd = (hdl->cmdData.bHold) ? 0 : MlibMulgain29( wkx, hdl->cmdData.TspdKx );
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Create Ex Feed Reference																	*/
/*																									*/
/****************************************************************************************************/
#if 0
UCHAR mtcMakeExFeedRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
/*--------------------------------------------------------------------------------------------------*/
long wkx;
long seq;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                                /* Hold */
	{
		seq = 4;
	}
	else if(hdl->cmdData.CmdRstReq)
	{
		seq = hdl->seqState = 1;	                                      /* When resetting, start from the beginning */
	}
	else
	{
		seq = hdl->seqState;
	}

/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                            /* Latch completion state */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                            /* EX_FEED Start */
/*--------------------------------------------------------------------------------------------------*/
		MtcCancelLatch(hdl, EXPOSING_LTMD);							   /* Latch cancel */
		MtcReqLatch(hdl, EXPOSING_LTMD, hdl->cmdData.LatchSgnl);	   /* Latch request issue */
		if(hdl->cmdData.TSpeed < 0)
		{                                                                       /* Negative direction */
			cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, -DISTANCE_MAX);
		}
		else
		{                                                                       /* forward direction */
			cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, DISTANCE_MAX);
		}
		hdl->seqState++;
		break;

/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                            /* Latch waiting state */
/*--------------------------------------------------------------------------------------------------*/
		if(hdl->latch.Cmp && (hdl->latch.Mode == EXPOSING_LTMD))
		{                                                              /* Latch completion */
			wkx = MtgGetDctDistance( &(hdl->mtgHdl),                   /* calculate distance */
							MtgGetCspd(&(hdl->mtgHdl)),
							MG_HOLD_CTRL1 );

			if ( wkx > (hdl->cmdData.Rdst) ) 
			{
				IXX_ALARMSET( ALMBITS_REGDISTSHORT ); /* A.E23 Registration distance insufficient alarm */
				cmdOut->DposIn = 0;
				hdl->seqState = 3;
			}
			else
			{
				wkx = ( (hdl->mtgHdl.cMinusDir) ?                       /* wkx <- registration offset */
				    ((-1)*(hdl->cmdData.Rdst)) : (hdl->cmdData.Rdst) );
				wkx =  PsnGetLPOS(&hdl->psnHdl) + wkx;                  /* wkx <- updated tpos */
				cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, wkx);
				hdl->seqState = 0;
			}
		}
		else
		{                                                              /* Latch incomplete */
			if(hdl->cmdData.TSpeed < 0)
			{                                                                       /* Negative direction */
				cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, -DISTANCE_MAX);
			}
			else
			{                                                                       /* forward direction */
				cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, DISTANCE_MAX);
			}
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:                                                            /* Alarm status */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = 0;
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 4:                                                            /* HOLDStatus */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		break;
/*--------------------------------------------------------------------------------------------------*/
	default:     
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = 0;
		break;
/*--------------------------------------------------------------------------------------------------*/
	}
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	//cmdOut->Tspd = (hdl->seqState) ? (MlibABS(hdl->cmdData.TSpeed)) : (hdl->cmdData.Rspd);
	wkx = (hdl->seqState) ? (MlibABS(hdl->cmdData.TSpeed)) : (hdl->cmdData.Rspd);
	cmdOut->Tspd = (hdl->cmdData.bHold) ? 0 : MlibMulgain29( wkx, hdl->cmdData.TspdKx );
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Create Homing Reference																		*/
/*																									*/
/*		use /DEC and CPHASE																			*/
/*																									*/
/****************************************************************************************************/
#if 0
UCHAR mtcMakeHomingRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
LONG tpos;
LONG seq;
LONG tdst;
LONG dummy;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                                /* Hold */
	{
		seq = 5;
	}
	else
	{
		seq = hdl->seqState;
	}
/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                                    /* Home return complete state */
/*--------------------------------------------------------------------------------------------------*/
		if(!hdl->cmdData.CmdRstReq)
		{                                                             /* If there is no reset request, command=0 */
			cmdOut->DposIn = 0;
			cmdOut->Tspd = 0;
			break;
		}
		MtcCancelLatch(hdl, ZRET_LTMD);				                  /* Latch cancel */
		hdl->seqState = 1;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                           /* HOMINGStart(DEC=ONWaiting state) */
/*--------------------------------------------------------------------------------------------------*/
		PsnSetRefPoint(&hdl->psnHdl, FALSE);				          /* Reference point clear */
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, hdl->mtPrm.MtSeqPrm.zret_dir);
		cmdOut->Tspd = MlibMulgain29(hdl->cmdData.TSpeed, hdl->cmdData.TspdKx);  /* TPOS,TSPEEDSet */

		//if(SviGetDecSig(hdl->svRsp))
		if( LPX_SQIGETDEC() )
		{                                                             /* DEC=ON */
			cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp1;		          /* Approach speed1fart */
			hdl->seqState++;								          /* DEC=OFFGo to waiting */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                           /* DEC=OFFWaiting state */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, 	              /* set direction */
						hdl->mtPrm.MtSeqPrm.zret_dir);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp1;	                  /* TPOS,TSPEED=Approach speed1Set */

		//if(!SviGetDecSig(hdl->svRsp))
		if( ! LPX_SQIGETDEC() )
		{                                                             /* DEC=OFF */
			MtcReqLatch(hdl, ZRET_LTMD, hdl->cmdData.LatchSgnl);	  /* Latch request issue */
			hdl->seqState++;										  /* Waiting for latch completion */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:                                                           /* Latch completion waiting state */
/*--------------------------------------------------------------------------------------------------*/
		if(hdl->latch.Cmp && (hdl->latch.Mode == ZRET_LTMD))
		{                                                             /* Latch completion */
			if(hdl->mtPrm.MtSeqPrm.zret_dir < 0)
				tpos = -hdl->mtPrm.MtSeqPrm.zret_dist;			      /*Reverse final mileage sign*/
			else
				tpos = hdl->mtPrm.MtSeqPrm.zret_dist;

			tpos = PsnGetLPOS(&hdl->psnHdl) + tpos;				      /* Set final position */
			cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, tpos);
			cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			      /* Approach speed2fart */
			hdl->seqState++;									      /* Go to waiting for positioning completion */
		}
		else
		{                                                             /* Latch incomplete */
			cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, hdl->mtPrm.MtSeqPrm.zret_dir);
			cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp1;				  /* Approach speed1 */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 4:                                                           /* Waiting for positioning completion */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2 */

		if(PsnGetPsetSts(&hdl->psnHdl))
		{                                                             /* Positioning complete */
	/*----------------------------------------------------------------------------------------------*/
	/* Interim: Origin position coordinate calculation 																		*/
	/*----------------------------------------------------------------------------------------------*/
			IxxSetOriginPos( &Npos, Npos.HomePos );                   /* set origin position        */
	/*----------------------------------------------------------------------------------------------*/
			MtcCancelLatch(hdl, NONE_LTMD);				              /* Latch cancel */
			hdl->seqState = 0;							              /* Return to home return complete state */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5:                                                           /* HOLDStatus */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}
#endif

#if 0/* SigmaSandDECSignal onlyHomingis not supported */
/****************************************************************************************************/
/*																									*/
/*		Create Homing Reference																		*/
/*																									*/
/*		use only /DEC 																				*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeHomingRefDec(MTHNDL* hdl, MG_INOUT* cmdOut)
{
LONG tpos;
LONG seq;
LONG tdst;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                                /* Hold */
	{
		seq = 5;
	}
	else
	{
		seq = hdl->seqState;
	}
/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                                    /* Home return complete state */
/*--------------------------------------------------------------------------------------------------*/
		if(!hdl->cmdData.CmdRstReq)
		{                                                             /* If there is no reset request, command=0 */
			cmdOut->DposIn = 0;
			cmdOut->Tspd = 0;
			break;
		}
		MtcCancelLatch(hdl, ZRET_LTMD);				                  /* Latch cancel */
		hdl->seqState = 1;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                           /* HOMINGStart(DEC=ONWaiting state) */
/*--------------------------------------------------------------------------------------------------*/
		PsnSetRefPoint(&hdl->psnHdl, FALSE);				          /* Reference point clear */
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, hdl->mtPrm.MtSeqPrm.zret_dir);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp1;                   /* TPOS,TSPEEDSet */

		if( LPX_SQIGETDECEDGE() )
		{                                                             /* DEC=ON */
			if(hdl->mtPrm.MtSeqPrm.zret_dir < 0)
				tpos = -hdl->mtPrm.MtSeqPrm.zret_dist;			      /*Reverse final mileage sign*/
			else
				tpos = hdl->mtPrm.MtSeqPrm.zret_dist;

			tpos = PsnGetPOS(&hdl->psnHdl) + tpos;				      /* Set final position */
			cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, tpos);
			cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			      /* Approach speed2fart */
			hdl->seqState++;									      /* Go to waiting for positioning completion */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                           /* Waiting for positioning completion */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2 */

		if(PsnGetPsetSts(&hdl->psnHdl))
		{                                                             /* Positioning complete */
			PsnSetRefPoint(&hdl->psnHdl, TRUE);			              /* Reference point enabled */
	/*----------------------------------------------------------------------------------------------*/
	/* Interim: Origin position coordinate calculation 																		*/
	/*----------------------------------------------------------------------------------------------*/
			Npos.SetDst( &Npos, Npos.HomePos, &tdst );                /* Apos=PnB25tdst */
			dummy = 0;                                                /* dummy */
			tdst = MlibPcmdEgear( tdst, &Iprm.EgearX, &dummy );       /* convert position (10->13) */
			PsnSetCordinate(&hdl->psnHdl,                             /* Origin position set  */
							0, PsnGetPOS(&hdl->psnHdl) + tdst );
			PsnSetRefPoint(&hdl->psnHdl, TRUE);			              /* Reference point enabled */
	/*----------------------------------------------------------------------------------------------*/
			hdl->seqState = 0;							              /* Return to home return complete state */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5:                                                           /* HOLDStatus */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}
#endif
/****************************************************************************************************/
/*																									*/
/*		Create Homing Reference																		*/
/*																									*/
/*		use only CPHASE																				*/
/*																									*/
/****************************************************************************************************/
#if 0
UCHAR mtcMakeHomingRefCphs(MTHNDL* hdl, MG_INOUT* cmdOut)
{
LONG tpos;
LONG seq;
LONG tdst;
LONG dummy;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                                /* Hold */
	{
		seq = 5;
	}
	else
	{
		seq = hdl->seqState;
	}
/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                           /* Home return complete state           */
/*--------------------------------------------------------------------------------------------------*/
		if(!hdl->cmdData.CmdRstReq)
		{                                                             /* If there is no reset request, command=0 */
			cmdOut->DposIn = 0;
			cmdOut->Tspd = 0;
			break;
		}
		MtcCancelLatch(hdl, ZRET_LTMD);				                  /* Latch cancel */
		hdl->seqState = 1;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                           /* HOMINGStart                 */
/*--------------------------------------------------------------------------------------------------*/
		PsnSetRefPoint(&hdl->psnHdl, FALSE);				          /* Reference point clear */
		MtcReqLatch(hdl, ZRET_LTMD, MT_LATCH_CPHASE);	              /* Latch request issue */
		hdl->seqState = 2;
/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                           /* wait latch completion      */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl, hdl->mtPrm.MtSeqPrm.zret_dir);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp1;                   /* TPOS,TSPEEDSet */
		/*------------------------------------------------------------------------------------------*/
		if(hdl->latch.Cmp && (hdl->latch.Mode == ZRET_LTMD))
		{                                                             /* Latch completion */
			if(hdl->mtPrm.MtSeqPrm.zret_dir < 0)
				tpos = -hdl->mtPrm.MtSeqPrm.zret_dist;			      /*Reverse final mileage sign*/
			else
				tpos = hdl->mtPrm.MtSeqPrm.zret_dist;

			tpos = PsnGetLPOS(&hdl->psnHdl) + tpos;				      /* Set final position */
			cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, tpos);
			cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			      /* Approach speed2fart */
			hdl->seqState++;									      /* Go to waiting for positioning completion */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:                                                           /* Waiting for positioning completion */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2 */

		if(PsnGetPsetSts(&hdl->psnHdl))
		{                                                             /* Positioning complete */
	/*----------------------------------------------------------------------------------------------*/
	/* Interim: Origin position coordinate calculation 																		*/
	/*----------------------------------------------------------------------------------------------*/
			IxxSetOriginPos( &Npos, Npos.HomePos );                   /* set origin position */
	/*----------------------------------------------------------------------------------------------*/
			hdl->seqState = 0;							              /* Return to home return complete state */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5:                                                           /* HOLDStatus */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = MPOSING;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;

}
#endif

/****************************************************************************************************/
/*																									*/
/*		Create Homing Reference																		*/
/*																									*/
/*		use /DEC and CPHASE																			*/
/*																									*/
/****************************************************************************************************/
#if 0
UCHAR mtcMakeHomingRefPush(MTHNDL* hdl, MG_INOUT* cmdOut)
{
#define SviGetSpeed(Rsp)   ((Rsp)->Mon3)	
#define SviGetTLmtSts(Rsp) (IXX_BITCHK((Rsp)->RspCtrl.RspCtrlBit, TRQ_CLAMP_BITNO))
LONG tpos, fbspd;
LONG seq;
LONG tdst;
LONG dummy;
CHAR followFlag = FALSE;
/*--------------------------------------------------------------------------------------------------*/
	if(hdl->cmdData.bHold)                                            /* Hold						*/
	{
		seq = 5;
	}
	else
	{
		seq = hdl->seqState;
	}
/*--------------------------------------------------------------------------------------------------*/
	hdl->ZretFspdSum += SviGetSpeed( hdl->svRsp );
	hdl->ZretFspdSumCnt++;
	//fbspd = hdl->psnHdl.FbSpdx << hdl->mtPrm.BasePrm.vel_shift;
	//fbspd = MlibAbs32(fbspd);
	
	if( hdl->ZretFspdSumCnt >= KPI_SCANN_MS( 32 ) )
	{
		fbspd = MlibAbs32( hdl->ZretFspdSum / KPI_SCANN_MS( 32 ) ); /* average of 32ms */
		if( fbspd >= (hdl->mtPrm.MtSeqPrm.zretOSLevel) )
		{
			IXX_ALARMSET( ALMBITS_ZRETOVERSPEED );				  	/* A.E25 : Return to origin overspeed	*/
		}
		hdl->ZretFspdSum = 0;
		hdl->ZretFspdSumCnt = 0;
	}
	//if(fbspd >= (hdl->mtPrm.MtSeqPrm.zretsp2 << 1))
	//{
	//	IXX_ALARMSET( ALMBITS_ZRETOVERSPEED );				  		/* A.E25 : Return to origin overspeed	*/
	//}
/*--------------------------------------------------------------------------------------------------*/
	switch( seq )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:                                                           /* Home return complete state			*/
/*--------------------------------------------------------------------------------------------------*/
		if(!hdl->cmdData.CmdRstReq)
		{                                                         /* If there is no reset request, command=0 */
			cmdOut->DposIn = 0;
			cmdOut->Tspd = 0;
			break;
		}
		hdl->seqState = 1;
/*--------------------------------------------------------------------------------------------------*/
	case 1:                                                           /* HOMINGStart					*/
/*--------------------------------------------------------------------------------------------------*/
		PsnSetRefPoint(&hdl->psnHdl, FALSE);				          /* Reference point clear		*/
		hdl->seqState++;											  /* TorqueLimitWaiting				*/
/*--------------------------------------------------------------------------------------------------*/
	case 2:                                                           /* TorqueLimitWaiting				*/
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl,
									 hdl->mtPrm.MtSeqPrm.zret_dir);	  /* Target position goes to infinity		*/
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2			*/
		hdl->tlimCnt = 0;											  /* Torque limit elapsed time reset		*/
		
		MlibBITFILTER1( hdl->tlimSig[0], hdl->tlimSig[1], (UCHAR)SviGetTLmtSts(hdl->svRsp) );
		if(hdl->tlimSig[0])
		{
			hdl->psnHdl.TlimPerr = PsnGetPERR(&hdl->psnHdl);		  /* Save position deviation when torque limit occurs */
			hdl->tlimCnt++;											  /* Torque limit elapsed timeCntUp		*/
			hdl->seqState++;										  /* Wait for push completion				*/
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:                                                           /* Wait for push completion				*/
/*--------------------------------------------------------------------------------------------------*/
																	  /* The target position differs depending on the conditions. */
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2 			*/
		
		MlibBITFILTER1( hdl->tlimSig[0], hdl->tlimSig[1], (UCHAR)SviGetTLmtSts(hdl->svRsp) );
		if(hdl->tlimSig[0])
		{
			if((hdl->mtPrm.MtSeqPrm.tlimTimeCnt1 <= hdl->tlimCnt)
									&&(hdl->tlimCnt < hdl->mtPrm.MtSeqPrm.tlimTimeCnt2))
			{/* Target positionAPOS+α */
				tpos = PsnGetAPOS(&hdl->psnHdl) + hdl->psnHdl.TlimPerr;		  /* Create target position				*/
				cmdOut->DposIn = tpos - PsnGetIPOS(&hdl->psnHdl);			  /* Create both payouts				*/
				followFlag = TRUE;
			}
			else if(hdl->mtPrm.MtSeqPrm.tlimTimeCnt2 <= hdl->tlimCnt)
			{
				if(hdl->mtPrm.MtSeqPrm.zret_dir < 0)
					tpos = -hdl->mtPrm.MtSeqPrm.zret_dist;
				else
					tpos = hdl->mtPrm.MtSeqPrm.zret_dist;			      /*Reverse final mileage sign*/
					
				tpos = PsnGetAPOS(&hdl->psnHdl) + tpos;				      /* Set final position */
				cmdOut->DposIn = PsnSetTPOS(&hdl->psnHdl, tpos);
				hdl->seqState++;									      /* Go to waiting for positioning completion */
			}
			else
			{
				cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl,
										 hdl->mtPrm.MtSeqPrm.zret_dir);	  /* Target position goes to infinity		*/
			}
			hdl->tlimCnt++;												  /* Torque limit elapsed timeCntUp		*/
		}
		else
		{
			cmdOut->DposIn = PsnSetTPOSDist(&hdl->psnHdl,
									 hdl->mtPrm.MtSeqPrm.zret_dir);	  /* Target position goes to infinity		*/
			if(hdl->mtPrm.MtSeqPrm.tlimTimeCnt1 <= hdl->tlimCnt)
			{/* tlimTimeCnt1or more, it will be considered an error. 					*/
				IXX_ALARMSET( ALMBITS_ZRETUNCOMPLETE );			 	 /* A.E24 : Return-to-origin failure		*/
				hdl->seqState = 5;
			}
			else
			{
				hdl->seqState = 2;									  /* TorqueLimitWaiting				*/
			}
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 4:                                                           /* Waiting for positioning completion */
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = PsnGetTPOS(&hdl->psnHdl) - PsnGetIPOS(&hdl->psnHdl);
		cmdOut->Tspd = hdl->mtPrm.MtSeqPrm.zretsp2;			          /* Approach speed2 */

		if(PsnGetPsetSts(&hdl->psnHdl))
		{                                                             /* Positioning complete */
	/*----------------------------------------------------------------------------------------------*/
	/* Interim: Origin position coordinate calculation 																		*/
	/*----------------------------------------------------------------------------------------------*/
			IxxSetOriginPos( &Npos, Npos.HomePos );                   /* set origin position */
	/*----------------------------------------------------------------------------------------------*/
			hdl->seqState = 0;							              /* Return to home return complete state */
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5:                                                           /* HOLDStatus 					*/
/*--------------------------------------------------------------------------------------------------*/
		cmdOut->DposIn = MtgGetDctDistance(&hdl->mtgHdl, 
						MtgGetCspd(&hdl->mtgHdl), cmdOut->Ctrl);
		cmdOut->Tspd = 0;
		hdl->seqState = (hdl->seqState != 4)?2:4;	/* Also in the next cyclebHold=TRUE, and thisCase	*/
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	cmdOut->Mtype = (!followFlag)?MPOSING:MFOLLOW;
	cmdOut->Vref = 0;
	cmdOut->Tref = 0;

	return FALSE;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Create Velocity Control Reference															*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeVelRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
	cmdOut->Mtype = MVELCTRL;
	cmdOut->DposIn = 0;
	cmdOut->Tspd = 0;
	cmdOut->Vref = hdl->cmdData.Velocity;
	cmdOut->Tref = hdl->cmdData.Trq;

	PsnResetIPOS(&hdl->psnHdl);		                          /* IPOSReset */
	PsnSetTPOSNoLimit(&hdl->psnHdl, 0);

	return FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Create Torque Control Reference																*/
/*																									*/
/****************************************************************************************************/
UCHAR mtcMakeTrqRef(MTHNDL* hdl, MG_INOUT* cmdOut)
{
	cmdOut->Mtype = MTRQCTRL;
	cmdOut->DposIn = 0;
	cmdOut->Tspd = 0;
	cmdOut->Vref = hdl->cmdData.Velocity;
	cmdOut->Tref = hdl->cmdData.Trq;

	PsnResetIPOS(&hdl->psnHdl);		                         /* IPOSReset */
	PsnSetTPOSNoLimit(&hdl->psnHdl, 0);

	return FALSE;
}


/****************************************************************************************************/
/*																									*/
/*		Set Homing Method 																			*/
/*																									*/
/****************************************************************************************************/
void	MtcSetHomeMethod( long prm )
{
	switch ( prm )
	{
#if 0
/*--------------------------------------------------------------------------------------------------*/
		case HOMEMETHOD_NOTUSE:
/*--------------------------------------------------------------------------------------------------*/
			NoutV.HomeUse = FALSE;
			NoutV.HomeMethod = HOMEMETHOD_NOTUSE;
			break;
/*--------------------------------------------------------------------------------------------------*/
		case HOMEMETHOD_DEC_C:
/*--------------------------------------------------------------------------------------------------*/
			mtcMakeCmd[MTYPE_HOMING] = mtcMakeHomingRef;
			NoutV.HomeUse = TRUE;
			NoutV.HomeMethod = HOMEMETHOD_DEC_C;
			break;
/*--------------------------------------------------------------------------------------------------*/
		case HOMEMETHOD_DEC:
/*--------------------------------------------------------------------------------------------------*/
			mtcMakeCmd[MTYPE_HOMING] = mtcMakeHomingRef; /* SigmaSandDECSignal onlyHomingis not supported */
			NoutV.HomeUse = TRUE;
			NoutV.HomeMethod = HOMEMETHOD_DEC_C;
			break;
/*--------------------------------------------------------------------------------------------------*/
		case HOMEMETHOD_C:
/*--------------------------------------------------------------------------------------------------*/
			mtcMakeCmd[MTYPE_HOMING] = mtcMakeHomingRefCphs;
			NoutV.HomeUse = TRUE;
			NoutV.HomeMethod = HOMEMETHOD_C;
			break;
/*--------------------------------------------------------------------------------------------------*/
		case HOMEMETHOD_PUSH:
/*--------------------------------------------------------------------------------------------------*/
			mtcMakeCmd[MTYPE_HOMING] = mtcMakeHomingRefPush;
			NoutV.HomeUse = TRUE;
			NoutV.HomeMethod = HOMEMETHOD_PUSH;
			break;
/*--------------------------------------------------------------------------------------------------*/
#endif
		default:
			NoutV.HomeUse = FALSE;
			NoutV.HomeMethod = HOMEMETHOD_NOTUSE;
//TODO			KPI_ALARMSET( ALM_OPTPRM_ERR );                   /* Interim: A.04A : Option parameter setting error */
			break;
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}
