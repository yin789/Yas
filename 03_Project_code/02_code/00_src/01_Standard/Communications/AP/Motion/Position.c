/****************************************************************************/
/*																			*/
/*																			*/
/*		POSITION MANAGER 													*/
/*																			*/
/*																			*/
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/************** Copyright Yaskawa Electric Corporation **********************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/

/* includes */
#include "ScanN.h"
//#include "Basedef.h"
//#include "MlibSvc.h"
//#include "ServoIf.h"
//#include "Motion.h"
#include "MtPRM.h"
#include "Position.h"


/* forward declarations */
/* Public functions */
VOID PsnInit(PSNHNDL* hdl, PSN_PRM* prm);
VOID PsnUpdateFBData(PSNHNDL* hdl, LONG pgcnt);
BOOL PsnGetPsetSts(PSNHNDL* hdl);
BOOL PsnGetNearSts(PSNHNDL* hdl);
BOOL PsnGetZPointSts(PSNHNDL* hdl);
LONG PsnSetTPOS(PSNHNDL* hdl,LONG tpos);
LONG PsnSetTPOSDist(PSNHNDL* hdl,LONG dpos);
VOID PsnResetCmdPosAll(PSNHNDL* hdl, LONG err);
UCHAR PsnCheckSoftLimit(PSNHNDL* hdl, LONG* dpos);
UCHAR PsnCheckSoftLimitFB(PSNHNDL* hdl, BOOL* p_ot, BOOL* n_ot);
VOID PsnSetCordinate(PSNHNDL* hdl, UCHAR mode, LONG pos);
VOID PsnResetApos(PSNHNDL* hdl, BOOL abs);
BOOL PsnMakeLpos(PSNHNDL* hdl, LONG ltcnt);


/****************************************************************************
 Initialization process
****************************************************************************/
#pragma arm section code = "IXX_EXCODE"
VOID PsnInit(PSNHNDL* hdl, PSN_PRM* prm)
{
	hdl->Tpos = 0;			/* Target position */
	hdl->Ipos = 0;			/* Command position (before filter) */
	hdl->Pos = 0;			/* Command position (after filter) */
	hdl->Mpos = 0;			/* Command position (after filter) */
	hdl->Apos = 0;			/* FBPosition */
	hdl->Dpos = 0;
	hdl->Perr = 0;			/* Deviation */
	hdl->Fbspd = 0;			/* FBSpeed */
	hdl->Pgcnt = 0;			/* FBSpeed */
	hdl->SoftLimit = 0;		/* Soft limit status */
	hdl->RefEnable = FALSE;	/* Reference point disabled */
	hdl->PosEnable = FALSE;	/* Position data invalid */
	hdl->Prm = prm;
}
#pragma arm section code


/****************************************************************************
 FBData creation processing
 APOS,PERR, FBSPDupdate
****************************************************************************/
VOID PsnUpdateFBData(PSNHNDL* hdl, LONG pgcnt)
{
	LONG data;

/* FBPosition and speed creation */
	data = pgcnt - hdl->Pgcnt;
	hdl->Fbspd = data;					/* FBSpeed */
	hdl->Apos += data;					/* FBPosition */
	hdl->Pgcnt = pgcnt;					/* PGCNTSave */
/* MPOSUpdate */
	if(hdl->Prm->TdlyIn <= 0)
	{
		data = (hdl->lstDpos * hdl->Prm->TdlyIn) / hdl->Prm->SegNum;
		hdl->Mpos = (hdl->Pos - hdl->Dpos) + data;
	}
	else
	{
		data = (hdl->Dpos * hdl->Prm->TdlyIn) / hdl->Prm->SegNum;
		hdl->Mpos = (hdl->Pos - hdl->Dpos) - data;
	}
/* PERRUpdate */
	hdl->Perr = hdl->Mpos - hdl->Apos;	/* PERR */

/* 32msAverage */
#if 0
	hdl->FbSpdSum += hdl->Fbspd;
	hdl->FbSpdSumCnt ++;
	if(hdl->FbSpdSumCnt >= KPI_SCANN_MS( 32 ) )
	{
		hdl->FbSpdx = (hdl->FbSpdSum >> 5);
		hdl->FbSpdSum= 0;
		hdl->FbSpdSumCnt = 0;
	}
#endif
}

/****************************************************************************
 PSETState acquisition
****************************************************************************/
BOOL PsnGetPsetSts(PSNHNDL* hdl)
{
	LONG data = hdl->Tpos - hdl->Apos;
	if((hdl->Den == TRUE) && ((ULONG)MlibABS(data) <= hdl->Prm->psetWidth))
	{/* DEN=1 && (Tpos-Apos)<=Positioning completion width */
		return TRUE;
	}
	return FALSE;
}

/****************************************************************************
 NEARState acquisition
****************************************************************************/
BOOL PsnGetNearSts(PSNHNDL* hdl)
{
	LONG data = hdl->Tpos - hdl->Apos;
	if((ULONG)MlibABS(data) <= hdl->Prm->nearWidth)
	{/* (Tpos-Apos)<=NEARWidth */
		return TRUE;
	}
	return FALSE;
}

/****************************************************************************
 ZPOINTState acquisition
****************************************************************************/
BOOL PsnGetZPointSts(PSNHNDL* hdl)
{
	if((hdl->RefEnable == TRUE) 
		&& ((ULONG)MlibABS(hdl->Apos) <= hdl->Prm->zPointWidth))
	{/* Referncepoints valid && APOS <= Origin width near origin */
		return TRUE;
	}
	return FALSE;
}


/****************************************************************************
 TPOSSetting processing(Absolute value)
 ****************************************************************************/
LONG PsnSetTPOS(PSNHNDL* hdl,LONG tpos)
{
	LONG dpos = tpos - hdl->Ipos;

	PsnCheckSoftLimit(hdl, &dpos);
	hdl->Tpos = dpos + hdl->Ipos;
	return dpos;
}

/****************************************************************************
 TPOSSetting processing(Relative distance)
 ****************************************************************************/
LONG PsnSetTPOSDist(PSNHNDL* hdl,LONG dpos)
{
	PsnCheckSoftLimit(hdl, &dpos);
	hdl->Tpos = dpos + hdl->Ipos;
	return dpos;
}

/****************************************************************************
 Positioning process for speed/torque control
 TPOS=IPOS=POS=APOS+err,
****************************************************************************/
VOID PsnResetCmdPosAll(PSNHNDL* hdl, LONG err)
{
	LONG data = hdl->Apos + err;

	hdl->Ipos = data;
	hdl->Pos = data;
	if(hdl->PosEnable)
	{/* When coordinate system is valid */
		hdl->Tpos = data;
		hdl->Mpos = data;
		hdl->Perr = hdl->Mpos - hdl->Apos;
	}
	else
	{/* When coordinate system is invalid */
		hdl->Tpos = hdl->Apos;
		hdl->Mpos = hdl->Apos;
		hdl->Perr = 0;
	}
}

/****************************************************************************
 Soft limit status check process
 1)When reference point is disabled:SOTStatus clear,TPOS=dpos+IPOSand return
 2)TPOS
   TPOSis more than the soft limit, it is the soft limitTPOSClamp the
   dpos Distance to soft limit(TPOS-IPOS)
****************************************************************************/
UCHAR PsnCheckSoftLimit(PSNHNDL* hdl, LONG* dpos)
{
	LONG dist, sdist;

/* Command limit check */	
	if(hdl->Prm->tposLimitEnable && hdl->RefEnable)
	{
		dist = *dpos;
		if(dist >= 0)
		{/* Positive side */
			sdist = hdl->Prm->softLimit_P - hdl->Ipos;
			if((dist >= sdist) && (hdl->Prm->sotEnableP == TRUE))
			{/* Soft limit enable && Outside limit */
				/* If limit value is not reached, set travel amount to limit */
				/* If limit value is reached, travel amount0set */
				*dpos = ((sdist > 0) ? sdist: 0);
			}
		}
		else
		{/* Negative side */
			sdist = hdl->Prm->softLimit_N - hdl->Ipos;
			if((dist <= sdist) && (hdl->Prm->sotEnableN == TRUE))
			{/* Soft limit enable && Outside limit */
				/* If limit value is not reached, set travel amount to limit */
				/* If limit value is reached, travel amount0set */
				*dpos = ((sdist < 0) ? sdist: 0);
			}
		}
	}

	return 0;
}

/****************************************************************************
 Soft limit status check process(APOS)
 1)When reference point is disabled:SOTStatus clear, return
 2)APOS
   APOSis more than the soft limitSOTUpdate statusSOTreturn status
****************************************************************************/
UCHAR PsnCheckSoftLimitFB(PSNHNDL* hdl, BOOL* p_ot, BOOL* n_ot)
{
	UCHAR	sts = 0;
	*p_ot = FALSE;
	*n_ot = FALSE;
/* When reference point is disabled, clear the status and force return */
	if(!hdl->RefEnable)
	{
		hdl->SoftLimit = 0;
		return 0;
	}
/* FBLimit check */	
	if((hdl->Apos >= hdl->Prm->softLimit_P) && (hdl->Prm->sotEnableP == TRUE))
	{/* Positive soft limit status */
		sts |= SOFT_LIMIT_P;
		*p_ot = TRUE;
	}
	if((hdl->Apos <= hdl->Prm->softLimit_N) && (hdl->Prm->sotEnableN == TRUE))
	{/* Negative side soft limit state */
		sts |= SOFT_LIMIT_N;
		*n_ot = TRUE;
	}

	hdl->SoftLimit = sts;			/* SoftLimitState update */
	return sts;
}


/****************************************************************************
 Coordinate system setting
 POSReset all positions to standardpos,
****************************************************************************/
VOID PsnSetCordinate(PSNHNDL* hdl, UCHAR mode, LONG pos)
{
	LONG offset;

	/* POS_SETMode:POSReset position with reference */
	offset = pos - hdl->Pos;
	hdl->Pos = pos;
	hdl->Mpos = pos;
	hdl->Ipos += offset;
	hdl->Apos += offset;
	hdl->Tpos += offset;
}

/****************************************************************************
 Coordinate system reset
 Reset all positions to standardPGCNTReset to standard
 Also,abs=1&&ABS_PGatABSOffset, enable the reference point
 abs=0, reference point is disabled
****************************************************************************/
VOID PsnResetApos(PSNHNDL* hdl, BOOL abs)
{
	LONG offset;
	LONG apos;

	if(abs == TRUE)
	{/* ABSPosition creation mode: Add origin position offset,APOSReset position with reference */
		apos = hdl->Pgcnt;
		if(hdl->Prm->absPG)
		{
			apos += hdl->Prm->absOfsset;
			hdl->RefEnable = TRUE;
		}
		offset = apos - hdl->Apos;
		hdl->Apos = apos;
		hdl->Ipos += offset;
		hdl->Pos += offset;
		hdl->Mpos += offset;
		hdl->Tpos += offset;
		hdl->PosEnable = TRUE;
	}
	else
	{/* APOSReset mode:APOSReset position with reference */
		offset = hdl->Pgcnt - hdl->Apos;
		hdl->Apos = hdl->Pgcnt;
		hdl->Ipos += offset;
		hdl->Pos += offset;
		hdl->Mpos += offset;
		hdl->Tpos += offset;
		hdl->RefEnable = FALSE;
		hdl->PosEnable = FALSE;
	}
}


/****************************************************************************
 LPOSAcquisition
	If inside latch area,Lpos,TRUE.
****************************************************************************/
BOOL PsnMakeLpos(PSNHNDL* hdl, LONG ltcnt)
{
	LONG lpos;
	BOOL ltcmp = FALSE;

	lpos = hdl->Apos + (ltcnt - hdl->Pgcnt);
	if(hdl->Prm->ltRangeP > hdl->Prm->ltRangeN) 
	{/* Latch upper limit value > Latch lower limit value In the case of
		Latch area is lower limit of latch <= lpos <= Latch upper limit value */
		if((lpos >= hdl->Prm->ltRangeN) && (lpos <= hdl->Prm->ltRangeP))
		{
			ltcmp = TRUE;
			hdl->LstLpos = hdl->Lpos;
			hdl->Lpos = lpos;
		}
	}
	else
	{/* Latch upper limit value <= Latch lower limit value In the case of
		Latch area islpos <= Latch lower limit value or Latch upper limit value <= lpos */
		if((lpos <= hdl->Prm->ltRangeP) || (lpos >= hdl->Prm->ltRangeN))
		{
			ltcmp = TRUE;
			hdl->LstLpos = hdl->Lpos;
			hdl->Lpos = lpos;
		}
	}
	return ltcmp;
}


