/****************************************************************************************************/
/*																									*/
/*	JatOffLine.c : Offline auto tuning module											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Overview:	Perform offline inertia identification. Because it is executed using position control, identification is also possible on the vertical axis.		*/
/*			DD motor and linear motor can also be identified														*/
/*			The inertia ratio is set for forward/reverse rotation. Therefore, the									*/
/*			Operation during inertia identification is forward/reverse rotation(Identification)→Reverse/forward(Identification)→JrateUpdate			*/
/*																									*/
/*                         Identification execution(Correct)   Identification execution(Reverse)           JUpdate								*/
/*                       |--------------|---------------|       |-----|								*/
/*               _______________                                									*/
/*           ____|              |_______________                  ____  							*/
/*                                              |________________|									*/
/*																									*/
/*		Caution:																						*/
/*		  Identification command																					*/
/*			①Forward rotation→Reverse, reverse→The stopping time when switching to forward rotation must be almost zero.				*/
/*			  If the stopping time is long, the identification accuracy will deteriorate due to the effects of static friction.								*/
/*			②Speed=500min-1or more(Rated3000min-1), Acceleration time is30msThe above is ideal.					*/
/*			  In the case of a triangle command, the accuracy of identification execution judgment will be poor, so try to move as much as possible							*/
/*			  It is better to set a constant speed time.														*/
/*			③The reflection of the identification value is determined by “identification completed and zero speed”, and the command stop time is300msProvide approximately		*/
/*																									*/
/*		  Servo gain																				*/
/*			①This method updates the inertia after reciprocating operation, so with the factory gain						*/
/*			  Identification when there is large inertiaKp>>Kv, vibration of the position loop occurs.					*/
/*			  Therefore, it is executed using balance position control, but in order to identify the unknown inertia,Kp/Kv→Small is required		*/
/*			  ThusKp=Kv/10,VFF=100%only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.														*/
/*			②Servo gain isPn100,Pn102: 1stKv,Kp(Gain cannot be changed during identification.)			*/
/*			③Inertia ratio setting when starting inertia identification isPn103, make it a dedicated parameter			*/
/*			  Inertia ratio=300%, approximately100Identification possible up to double load(However, the torque must not be saturated.).		*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*																									*/
/*	T.Kira 2011.03.01   First edition																		*/
/****************************************************************************************************/
#include "Basedef.h"
#include "JatOffLine.h"
#include "ServoIf.h"
#include "PnPrmListTbl.h"

/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
static void jat_offline_st_detect( JATV *jatv, LONG ActPos);
static void jat_estimate( JATHNDL *JatHdl );
static void jat_modelcal( JATHNDL *JatHdl, CTRL_CMD_PRM *CtrlCmdPrm, LONG spdref );
static void jat_sumtrf_offline( JATHNDL *JatHdl, LONG TrqRef);


/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification scan C processing														*/
/*																									*/
/*	Overview:	Identification start determination, inertia ratio calculation, and vibration detection are performed.										*/
/*																									*/
/*	arg		:																						*/
/*	g-ref	:	LONG	CoutV.UnMotSpdx			:	ScanCMotor speed				[2^24/OvrSpd]		*/
/*				UINT	BoutV.f.Ctrl.TrqClamp	:	Torque limit flag								*/
/*				UINT	BinV.f.BaseEnable		:	Base enable flag							*/
/*				UINT	BoutV.f.PerClrCmd		:	Deviation clear flag								*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
void CpxJatOfflineCalc( JATHNDL *JatHdl, LONG UnMotSpdx, DETVIB *DetVib, BOOL PConActFlag, BOOL PerrClrReq )
{
	LONG	wk;
//	BOOL	PConActFlag;
	JATV	*jatv;
	JATP	*jatp;

	jatv = &JatHdl->jatv;
	jatp = &JatHdl->jatp;
//	PConActFlag = ((CmdCtrlBit >> ENBPCTRL_BITNO) & 0x01);

	/*--------------------------------------------------------------------------*/
	/*	Function restriction																*/
	/*--------------------------------------------------------------------------*/
	if (jatv->enable == TRUE)				/* Inertia identification enabled				*/
	{
		/*----------------------------------------------------------------------*/
		/*	Identification error detection														*/
		/*----------------------------------------------------------------------*/
		wk = jatv->mnfb - UnMotSpdx;			/* Model speed - FB speed		*/
		jatv->jverr = MlibLpfilter1(wk, jatp->jverrfil, jatv->jverr);

		if((DetVib->conf.JstVibCheckLevel << 1) < MlibABS(jatv->jverr))
		{	/* Error detection level exceeded	*/
			jatv->status |= JATERR;				/* Error status set		*/
		}

		if( ( PConActFlag == TRUE) || ( PerrClrReq == TRUE ))
		{	/* During proportional control or Deviation clearing */
			jatv->status |= JATEXEERR;			/* Execution error status set	*/
		}
		/*----------------------------------------------------------------------*/
		/*	Identification start determination														*/
		/*----------------------------------------------------------------------*/
		jat_offline_st_detect(jatv, jatv->fbsum);

		/*----------------------------------------------------------------------*/
		/*	Identification inertia ratio calculation												*/
		/*----------------------------------------------------------------------*/
		jat_estimate( JatHdl );
	}
}

/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification start/end determination														*/
/*																									*/
/*	Overview:Monitor FB position and create execution start/end flag for inertia identification process						*/
/*		 Also, detect the first vibration during identification when vibration is detected, and wait until				*/
/*		 Returns error status.																	*/
/*																									*/
/*	arg		:	LONG	ActPos				: Actual travel amount		[Command unit]								*/
/*	g-ref	:	UINT	CoutV.f.Sv.MotStop	: Motor stop flag										*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
static void jat_offline_st_detect( JATV *jatv, LONG ActPos )
{
	LONG	wk1, wk2;

	wk1 = MlibABS(jatv->TargetDist);				/* Setting movement amount	[Command unit]						*/
	wk2 = MlibABS(ActPos);							/* Actual travel amount		[Command unit]						*/

	switch(jatv->jarte_start)
	{
	case JATINITIAL:	/* Inertia identification(Torque integration)Not executed	*/
		if(wk1 <= wk2)								/* The movement amount from the start of identification is 1/2 or more of the set value		*/
		{
			jatv->jarte_start = JATCWSTART;			/* Forward rotation→Reverse inertia identification executedON				*/
		}
		break;
	case JATCWSTART:	/* Forward rotation→Reverse inertia identification(Torque integration)Executing	*/
		if(wk1 >= wk2)								/* The amount of movement from the start of identification is 1/2 or less of the set value	*/
		{
			jatv->jarte_start = JATCCWSTART;		/* Reverse running→Forward rotation inertia identification executedON				*/
		}
		break;
	case JATCCWSTART:	/* Reverse running→Forward rotation inertia identification(Torque integration)Executing	*/
		if(wk1 <= wk2)								/* The movement amount from the start of identification is 1/2 or more of the set value		*/
		{
			jatv->jarte_start = JATSUMEND;			/* Torque integration execution completed							*/
		}
		break;
	case JATSUMEND:		/* Torque integration completed	*/
		if(wk1 >= wk2)								/* The movement amount from the start of identification is 1/2 or less of the set value		*/
		{
			jatv->jarte_start = JATEXEEND;			/* Inertia identification completed						*/
		}
		break;
	case JATEXEEND:		/* Inertia identification completed		*/
		/* Do nothing	*/
	default:
		break;
	}
}

/****************************************************************************************************/
/*																									*/
/*		Inertia calculation																				*/
/*																									*/
/*	Overview:Estimate the inertia and set the inertia ratio appropriately.											*/
/*			This processing does not require real-time performance, so scan C is used.								*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
static void jat_estimate( JATHNDL *JatHdl )
{
	LONG	wk,wk1,wk2,s;
	JATV	*jatv;
	JATP	*jatp;

	jatv = &JatHdl->jatv;
	jatp = &JatHdl->jatp;

	if(jatv->jarte_start == JATEXEEND )
	{	/* Torque integration completed	*/
		/*----------------------------------------------------------------------*/
		/* Inertia identification value calculation 												*/
		/*		When there is a constant disturbance such as gravity axis,jat.j0,j2may take a negative value	*/
		/*		The influence of constant disturbance is removed by taking the average of forward and reverse operations			*/
		/*----------------------------------------------------------------------*/
		/* Forward rotation→Reverse direction	*/
		if(jatv->msum1 != 0)
		{
			s = 0;
			wk1 = MlibPcalKxgain(jatv->sum1, (jatp->mjrate << JSHBITNUM), jatv->msum1, &s, 24);
			jatv->j0 = MlibKsgain2Long(wk1);
			jatv->status |= JATCWCOMP;								/* Forward rotation→Reverse inertia identification executed	*/
		}
		/* Reverse running→Forward rotation direction	*/
		if(jatv->msum2 != 0)
		{
			s = 0;
			wk1 = MlibPcalKxgain(jatv->sum2, (jatp->mjrate << JSHBITNUM), jatv->msum2, &s, 24);
			jatv->j2 = MlibKsgain2Long(wk1);
			jatv->status |= JATCCWCOMP;								/* Reverse running→Forward rotation inertia identification executed	*/
		}
	}

	/* Update monitor Inertia ratio */
	if((jatv->status & JATCOMP) == JATCOMP)
	{	/* Bidirectional identification completed	*/
		wk = (jatv->j0 + jatv->j2) >> 1;							/* Total inertia average value calculation			*/

		/* Upper/lower limit check	*/
		wk1  = (LONG)( pndef_jrate.UpperLimit + 100 );
		wk2  = (LONG)( pndef_jrate.LowerLimit + 100 );
		wk  = MlibLimitul(wk, wk1, wk2);
		jatv->jrate = (USHORT)(wk - 100);							/* Inertia ratio calculation					*/

		JatHdl->EstimatJrat = jatv->jrate;							/* Set the identified value						*/

		jatv->status |= JATJRATECOMP;								/* Inertia identification completed status		*/
	}
}

/****************************************************************************************************/
/*																									*/
/*		Offline inertia identification scan B processing														*/
/*																									*/
/*	Overview:	Perform model calculation and torque integration.															*/
/*			Speed ​​command input to the model includes speed FF, torque command input uses filtered torque command		*/
/*																									*/
/*	arg		:																						*/
/*	g-ref	:	LONG	AoutV.SpdRefi	:	Speed command				[2^24/OvrSpd]					*/
/*				LONG	BoutV.SpdFFCx	:	Speed FF compensation			[2^24/OvrSpd]					*/
/*				LONG	BoutV.dPosFbki	:	Position FB difference			[Pulse/Scan]					*/
/*				LONG	AoutV.TrqRefo_a	:	Torque command after filter	[2^24/MaxTrq]					*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
void BpxJatOfflineCalc( JATHNDL	*JatHdl, EGEAR *Egear, CTRL_CMD_PRM *CtrlCmdPrm, 
														CTRL_CMD_MNG *CtrlCmdMngr, LONG	TrqRefo_a )
{
	LONG spdref;
	JATV *jatv;
	
	jatv = &JatHdl->jatv;

	/*--------------------------------------------------------------------------*/
	/*	Function restriction																*/
	/*--------------------------------------------------------------------------*/
	if (jatv->enable == TRUE)				/* Inertia identification enabled				*/
	{
		//BoutV.f.InertiaTuning = TRUE;		/* Inertia identification in progress					*/
		/*----------------------------------------------------------------------*/
		/*	Parameter set requestTRUE, set parameters				*/
		/*----------------------------------------------------------------------*/
		if( jatv->JatPrmSetReq == TRUE )
		{
			MlibCopyLongMemory( &JatHdl->jatp, &JatHdl->jatp_wk, sizeof( JatHdl->jatp )>>2 );
			jatv->JatPrmSetReq = FALSE;
		}

		spdref = CtrlCmdMngr->SpdRefi + CtrlCmdMngr->SpdFFCx;		/* <S160>	*/
//		spdref = CtrlCmdMngr->SpdRefo;

		/*----------------------------------------------------------------------*/
		/*	Motor model calculation													*/
		/*----------------------------------------------------------------------*/
		jat_modelcal( JatHdl, CtrlCmdPrm, spdref );

		/*----------------------------------------------------------------------*/
		/*	Torque integration															*/
		/*----------------------------------------------------------------------*/
		jat_sumtrf_offline( JatHdl, TrqRefo_a );
	}
	//else
	//{
		//BoutV.f.InertiaTuning = FALSE;	/* Inertia identification not executed				*/
	//}
	/*--------------------------------------------------------------------------*/
	/*	Feedback position calculation after identification start										*/
	/*--------------------------------------------------------------------------*/
	jatv->fbsum += MlibEgearRvscnv( CtrlCmdMngr->dPosFbki,
									Egear,
									&jatv->fbsumrem );	/*  Real position	[Command unit]	*/
}

/****************************************************************************************************/
/*																									*/
/*		Model calculation																					*/
/*																									*/
/*	Overview:Calculate the response of the current inertia model from the speed command.										*/
/*																									*/
/*	arg		:																						*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
static void jat_modelcal( JATHNDL *JatHdl, CTRL_CMD_PRM *CtrlCmdPrm, LONG spdref )
{
	LONG	wk1, wk2, wk3, wk4;
	JATV	*jatv;
	JATP	*jatp;

	jatv = &JatHdl->jatv;
	jatp = &JatHdl->jatp;

	jatv->mvref = spdref; 		/* Speed command input->Model speed command */
#if (FLOAT_USE==TRUE)
	/* SpeedFBCalculation */
	jatv->mnfb  = MlibIntegral( jatv->mtrf , jatp->mkj, jatv->mnfbwrk );
	/* Speed control */
	wk1 = (LONG)( (float)jatv->mvref * CtrlCmdPrm->PI_rate);
	jatv->mnerp = wk1 - jatv->mnfb;
	wk3 = jatv->mnerp * jatp->mkv;

	jatv->mneri = jatv->mvref - jatv->mnfb;
	wk2 = MlibIntegral(jatv->mneri, jatp->mkvi, jatv->mneribf);
	wk4	= wk3 + wk2;

	/* Model torque */
	jatv->mtrf = MlibLpfilter1(wk4, jatp->mklpf, jatv->mtrf);
#else
	/* SpeedFBCalculation */
	jatv->mnfb  = MlibIntegral( jatv->mtrf , jatp->mkj, jatv->mnfbwrk );
	/* Speed control */
	wk1 = MlibMulgainNolim( jatv->mvref, CtrlCmdPrm->PI_rate );
	jatv->mnerp = wk1 - jatv->mnfb;
	wk3 = MlibMulgain(jatv->mnerp, jatp->mkv);

	jatv->mneri = jatv->mvref - jatv->mnfb;
	wk2 = MlibIntegral(jatv->mneri, jatp->mkvi, jatv->mneribf);
	wk4	= wk3 + wk2;

	/* Model torque */
	jatv->mtrf = MlibLpfilter1(wk4, jatp->mklpf, jatv->mtrf);
#endif /* FLOAT_USE */
}

/****************************************************************************************************/
/*																									*/
/*		Torque integration for offline use																		*/
/*																									*/
/*	Overview:Perform model calculations and torque integration for inertia estimation.											*/
/*																									*/
/*	arg		:	LONG	TrqRef	: Torque command						[2^24/MaxTrq]					*/
/*	out		:													 									*/
/*																									*/
/****************************************************************************************************/
static void jat_sumtrf_offline( JATHNDL *JatHdl, LONG TrqRef )
{
	JATV	*jatv;
	jatv = &JatHdl->jatv;

	if(jatv->jarte_start == JATCWSTART)
	{	/* Forward rotation→Reverse side identification start	*/
		jatv->sum1  += (TrqRef >> JSHBITNUM);
		jatv->msum1 += jatv->mtrf;
		if( (MlibABS(jatv->sum1) > 0x7D000000) || (MlibABS(jatv->msum1) > 0x7D000000) )
		{
			jatv->sum1 = (jatv->sum1 >> 1);
			jatv->msum1 = (jatv->msum1 >> 1);
		}
	}
	else if(jatv->jarte_start == JATCCWSTART)
	{	/* Reverse running→Start of forward rotation identification	*/
		jatv->sum2  += (TrqRef >> JSHBITNUM);
		jatv->msum2 += jatv->mtrf;
		if( (MlibABS(jatv->sum2) > 0x7D000000) || (MlibABS(jatv->msum2) > 0x7D000000) )
		{
			jatv->sum2 = (jatv->sum2 >> 1);
			jatv->msum2 = (jatv->msum2 >> 1);
		}
	}
}

