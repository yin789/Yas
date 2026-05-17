/****************************************************************************************************/
/*																									*/
/*	FnJatOffLine.c : Offline auto tuning module											*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*																									*/
/*	T.Kira 2011.03.01   First edition																		*/
/****************************************************************************************************/
#include "Basedef.h"
#include "FnJatOffLine.h"
#include "PnPrmListTbl.h"
#include "TuneLib.h"
#include "BaseCmnStruct.h"

/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
static void JatParamCal( JATHNDL *JatHdl, UNI_PRM_STR *UniPrms, 
														REG_MANAGER_HANDLE *hPnReg, LONG SvCycleNs);

/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification ROUND processing														*/
/*																									*/
/*	Overview:	inertia ratio is reflected in the parameters.													*/
/*																									*/
/*	arg		:																						*/
/*	g-ref	:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
void CpiSetEstimateJrat( JATHNDL *JatHdl, REG_ACC_T *AccessPrm )
{
	JATV	*jatv;
	PRMBK	*prmbk;

	jatv = &JatHdl->jatv;
	prmbk = &JatHdl->prmbk;

	if (jatv->enable == TRUE)				/* Inertia identification enabled			*/
	{
		/*------------------------------------------------------------------*/
		/*	identification valueKvReflection processing												*/
		/*------------------------------------------------------------------*/
		/* Pn103 */
		TunePrmWriteRam( AccessPrm, &pndef_jrate, (LONG)jatv->jrate, pndef_jrate.DataLength );
		/* Update backup value	*/
		prmbk->jrate = (ULONG)jatv->jrate;
	}
}


/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification Identification error status acquisition API										*/
/*																									*/
/*	Overview:	Obtain the identification error status. Clear error status							*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
CHAR CpiGetJatErrStatus( JATHNDL *JatHdl )
{
	CHAR	rc;
	JATV	*jatv;

	jatv = &JatHdl->jatv;

	if(jatv->status & JATERR)
	{
		rc = NG;
	}
	else
	{
		rc = OK;
	}
	jatv->status &= ~JATERR;

	return	rc;
}

/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification Identification execution error status acquisition API								*/
/*																									*/
/*	Overview:	Get the identification execution error status. Do not clear the status							*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
CHAR CpiGetJatExeErrStatus( JATHNDL	*JatHdl )
{
	CHAR	rc;
	JATV	*jatv;

	jatv = &JatHdl->jatv;

	if(jatv->status & JATEXEERR)
	{
		rc = NG;
	}
	else
	{
		rc = OK;
	}

	return	rc;
}

/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification Identification completion status acquisition API										*/
/*	Overview:	Obtain the identification completion status.															*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
CHAR CpiGetJatCompStatus( JATHNDL	*JatHdl )
{
	CHAR	rc;
	JATV	*jatv;

	jatv = &JatHdl->jatv;

	if(jatv->status & JATJRATECOMP)
	{
		rc = OK;
	}
	else
	{
		rc = NG;
	}

	return	rc;
}

/****************************************************************************************************/
/*																									*/
/*		OfflineATInitialize																			*/
/*																									*/
/*	Overview:OfflineATInitialize the related variables and calculate the parameters.										*/
/*		 Inertia identification enabled flagON															*/
/*		 Set the identification movement distance																		*/
/*		 Be sure to execute this function before starting identification as it also initializes flags.							*/
/*       Pn103to this timing identification initial value.												*/
/*																									*/
/*																									*/
/*	arg		:	SHORT	InitSw		:	1:Variable and parameter calculation execution									*/
/*										0:Variable initialization only											*/
/*				LONG	distance	:	Total movement amount	[Command unit]										*/
/*	out		:	SHORT	rc			:	OK: Identification possible	NG: Identification not possible						*/
/*																									*/
/****************************************************************************************************/
SHORT CpiInitJatOffLine( JATHNDL *JatHdl, UNI_PRM_STR *UniPrms, REG_MANAGER_HANDLE *RegManager, 
					GAIN_CHNG_HNDL GainChange, SHORT InitSw, LONG distance, UCHAR CtrlMode, LONG SvCycleNs)
{
	SHORT	rc;
	JATV	*jatv;

	jatv = &JatHdl->jatv;

/*--------------------------------------------------------------------------------------------------*/
/*	Initializing variables																						*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( jatv, sizeof( JatHdl->jatv )>>2 );

/*--------------------------------------------------------------------------------------------------*/
/*	Initial setting																						*/
/*--------------------------------------------------------------------------------------------------*/
	jatv->TargetDist = distance >> 1;				/* Movement distance setting:1/2is for setting the identification start distance.	*/

/*--------------------------------------------------------------------------------------------------*/
/*	Execution condition determination																					*/
/*--------------------------------------------------------------------------------------------------*/
	if( (CtrlMode == CTRLMODE_POS)									/* Position control						*/
	 && (jatv->TargetDist != 0)										/* Movement distance is not zero			*/
	 && (GainChange.AtGsel == FALSE))								/* Not automatic gain switching		*/
	{
		/*------------------------------------------------------------------------------------------*/
		/*	Parameter calculation																			*/
		/*------------------------------------------------------------------------------------------*/
		if(InitSw == TRUE)
		{
			JatParamCal( JatHdl, UniPrms, RegManager, SvCycleNs );
		}
		jatv->enable = TRUE;						/* Inertia identification enabled flag  ON					*/
		rc = OK;
	}
	else
	{
		jatv->enable = FALSE;						/* Inertia identification enabled flagOFF				*/
		rc = NG;
	}

	return rc;
}

/****************************************************************************************************/
/*																									*/
/*		Parameter calculation																				*/
/*																									*/
/*	Overview:Set constants related to offline inertia identification.												*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
static void JatParamCal( JATHNDL *JatHdl, UNI_PRM_STR *UniPrms, REG_MANAGER_HANDLE *RegManager, LONG SvCycleNs )
{
	ULONG	wrk_kv, wrk_jrat;
	LONG	wrk_kp;
	ULONG	wrk_ki,wrk_klpf;
	LONG	kx,sx;
	LONG	ky,sy;

	PRMDATA *Prm;
	BPRMDAT	*Bprm;
	REGIF_CONFIG_T *RegCfg;
	REG_ACC_T	AccessPrm;
	PRMBK	*prmbk;
	JATP	*jatp_wk;
	JATV	*jatv;

	Prm = UniPrms->Prm;
	Bprm = UniPrms->Bprm;
	RegCfg = RegManager->hPnReg;
	prmbk = &JatHdl->prmbk;
	jatp_wk = &JatHdl->jatp_wk;
	jatv = &JatHdl->jatv;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain-related parameter setting																	*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		Identification initial inertia ratio																		*/
/*--------------------------------------------------------------------------------------------------*/
	prmbk->jrate = Prm->jrate;/* Pn103Get current setting value				*/
	wrk_jrat = Prm->Pn324;
	/* Pn103	*/
	TunePrmWriteRam( &AccessPrm, &pndef_jrate, (LONG)wrk_jrat, pndef_jrate.DataLength );

/*--------------------------------------------------------------------------------------------------*/
/*		Kp gain balance																			*/
/*--------------------------------------------------------------------------------------------------*/
	prmbk->kp = Prm->posgn;										/* Pn102Get current setting value				*/
	wrk_kv = Prm->loophz;										/* Kv acquisition							*/
	wrk_kp = wrk_kv/10;											/* Kp=Kv[Hz]/10				*/
	/* Lower limit check	*/
	if( wrk_kp < pndef_posgn.LowerLimit )
	{
		wrk_kp = pndef_posgn.LowerLimit;
	}
	TunePrmWriteRam( &AccessPrm, &pndef_posgn, (LONG)wrk_kp, pndef_posgn.DataLength );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed feedforward																		*/
/*--------------------------------------------------------------------------------------------------*/
	prmbk->vffgn = Prm->ffgn;									/* Speed FF gain acquisition				*/
	TunePrmWriteRam( &AccessPrm, &pndef_ffgn, (LONG)JATVFFGN, pndef_ffgn.DataLength );/* VFF=100%	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Inertia identification related parameter calculation															*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->mjrate	: Model inertia(Motor alone)												*/
/*--------------------------------------------------------------------------------------------------*/
	jatp_wk->mjrate = 100;
/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->mkv	: Model speed loop gain													*/
/*--------------------------------------------------------------------------------------------------*/
	wrk_kv = Prm->loophz;
	jatp_wk->mkv = MlibScalKskxkx( Bprm->Kvx, (jatp_wk->mjrate * wrk_kv),  1000, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->mkvi	: Model speed loop integral gain												*/
/*--------------------------------------------------------------------------------------------------*/
	wrk_ki = Prm->pitime;
	jatp_wk->mkvi = MlibScalKskxkx( jatp_wk->mkv, (SvCycleNs / 1000), 10*wrk_ki, NULL, 24 );
/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->mlpf	: Model torque filter gain												*/
/*--------------------------------------------------------------------------------------------------*/
	wrk_klpf = Prm->trqfil11;
	jatp_wk->mklpf = MlibPcalKf1gain( 10*wrk_klpf, (SvCycleNs / 1000), 0 );

/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->mkj	: Model inertia gain													*/
/*																									*/
/*					      MaxTrq * 100 * CycleNs[ns] * ObsJGain										*/
/*				x = ------------------------------------------------								*/
/*					  OvrSpd * Jmot * wrkp.mjrate * 10^9 * 100										*/
/*																									*/
/*					  2*PAI * 100 * CycleNs * ObsJGain(=100)										*/
/*				  = --------------------------------------											*/
/*					  Kvx * wrkp.mjrate * 10^9 * 100												*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	kx = MlibPcalKxmulx( 100, SvCycleNs, C2PAIE7, &sx );
	ky = MlibScalKskxkx( Bprm->Kvx,C10POW7,1, &sy, 0 );
	ky = MlibPcalKxmulx( ky, C10POW9, jatp_wk->mjrate, &sy );
	jatp_wk->mkj = MlibPcalKxdivx( kx, sx, ky, sy, 24 );
/*--------------------------------------------------------------------------------------------------*/
/*		jatp_wk->jfil	: Inertia update filter gain												*/
/*--------------------------------------------------------------------------------------------------*/
	jatp_wk->jfil = MlibPcalKf1gain( JATFIL, JRATCALCYCUS, 0);

/*--------------------------------------------------------------------------------------------------*/
/*		Speed deviation filter for identification error detection															*/
/*--------------------------------------------------------------------------------------------------*/
	jatp_wk->jverrfil = MlibPcalKf1gain( JRATVERRFIL, JRATCALCYCUS, 1 );

/*--------------------------------------------------------------------------------------------------*/
/*		Assign to structure																				*/
/*--------------------------------------------------------------------------------------------------*/
	jatv->JatPrmSetReq = TRUE;

}

/****************************************************************************************************/
/*																									*/
/*		OfflineATTermination processing																		*/
/*																									*/
/*	Overview:OfflineATInitialize related variables and restore changed parameters.								*/
/*		 Inertia identification enabled flagOFF															*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
void CpiEndJatOffLine( JATHNDL *JatHdl, REG_ACC_T *AccessPrm )
{
	JATV *jatv;
	PRMBK	*prmbk;

	jatv = &JatHdl->jatv;
	prmbk = &JatHdl->prmbk;
/*--------------------------------------------------------------------------------------------------*/
/*	Parameter recalculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	if (jatv->enable == TRUE)									/* Inertia identification enabled				*/
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Identification initial inertia ratio																*/
		/*------------------------------------------------------------------------------------------*/
		/* Pn103  VFF=100%	*/
		TunePrmWriteRam( AccessPrm, &pndef_jrate, (LONG)JATVFFGN, pndef_jrate.DataLength );

		/*------------------------------------------------------------------------------------------*/
		/*		Kp gain balance																	*/
		/*------------------------------------------------------------------------------------------*/
		/* Pn102  VFF=100%	*/
		TunePrmWriteRam( AccessPrm, &pndef_posgn, (LONG)prmbk->kp, pndef_posgn.DataLength );

		/*------------------------------------------------------------------------------------------*/
		/*		Speed feedforward																*/
		/*------------------------------------------------------------------------------------------*/
		/* Pn109  VFF=100% */
		TunePrmWriteRam( AccessPrm, &pndef_ffgn, (LONG)prmbk->vffgn, pndef_ffgn.DataLength );
	}
/*--------------------------------------------------------------------------------------------------*/
/*	Initializing variables																						*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( jatv, sizeof( JatHdl->jatv )>>2 );

}

