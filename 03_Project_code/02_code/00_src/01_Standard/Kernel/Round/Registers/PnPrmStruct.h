/****************************************************************************
Description	: Definition of the Parameter(Pn) Structure
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2010.5.22     created

****************************************************************************/
#ifndef __PN_PRM_STRUCT_H
#define __PN_PRM_STRUCT_H

#include "Basedef.h"
#include "EncData.h"
#include "MotorPoleFind.h"



#define VENDOR_IOPRM_NUM	4						/* SVCMD_IO(Output)VendorSpecificsI/ONumber of assigned parameters		*//* <S1C0> */

/****************************************************************************
	Unit common parameter structure definition
****************************************************************************/
typedef struct CMN_PRMDATA {
	USHORT	b_prm6;					/* Pn006 : Function selection application switch 6									*//* 2Axis analog monitor compatible<S103> */
	USHORT	b_prm7;					/* Pn007 : Function selection application switch 7									*//* 2Axis analog monitor compatible<S103> */
//	USHORT	b_prm8;					/* Pn008 : Function selection application switch 8									*/
	USHORT	b_prmE;					/* Pn00E : Function selection application switch E									*//* <S0A6> */
	USHORT	b_prmF;					/* Pn00F : Function selection application switch F									*//* <S0A6> */
	USHORT	AxisAddress[2];			/* Pn010 : Slot-specific axis address										*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	UniPrmSel;				/* Pn020 : Unit common parameter settings for each digit							*//* <Oka06> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	CycleSwitch;			/* Pn041 : Scan cycle switch								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	acoftm;					/* Pn509 : Momentary power failure holding time												*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	monoffset1;				/* Pn550 : Analog monitor 1 offset voltage							*//* <S0DF> */
	USHORT	monoffset2;				/* Pn551 : Analog monitor 2 offset voltage							*//* <S0DF> */
	USHORT	mongain1;				/* Pn552 : Analog monitor1magnification										*//* <S0DF> */
	USHORT	mongain2;				/* Pn553 : Analog monitor2magnification										*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	powmon_unitime ;		/* Pn55A : Power consumption monitor unit time									*//* <S0A6> *//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	regres;					/* Pn600 : Regenerative resistor capacity												*/
	USHORT	exreger;				/* Pn603 : Regenerative resistance value												*/

//	LONG	TrgioPosition[8];		/* Pn6xx : Trigger I/O */
//	USHORT	TrgioCfg[8];			/* Pn6xx : Trigger I/O */
//	USHORT	TrgioOnTime[8];			/* Pn6xx : Trigger I/O */
//	USHORT	TrgioAdjTime[8];		/* Pn6xx : Trigger I/O */
//	LONG	TrgIoDelayTime[8];		/* Pn6xx : Trigger I/O */

	USHORT	syssw2;					/* PnE0C : System switch 2										*/
	DBYTEX	InstEnvTmp_sw1;			/* PnE0F : Lower:Installation environment monitor reference temperature, Upper level:Function restriction switch 1		*//* <S03A> */
	USHORT	reger;					/* PnE1A : Regenerative resistance value(Default)									*//* <S0F4> */
	USHORT	wre;					/* PnE1C : Regenerative resistor capacity(Default)									*/
	USHORT	kvvfil;					/* PnE3E : Low-pass filter time constant for main circuit voltage correction					*/
	USHORT	mnrng0;					/* PnE42 : Monitor 1 gain											*//* <S016> *//* <S0DF> */
	USHORT	mnrng1;					/* PnE43 : Monitor 2 gain											*//* <S016> *//* <S0DF> */
	ULONG 	mnptr0l;				/* PnE48 : Monitor 1 address											*//* <S0DF> */
	ULONG 	mnptr1l;				/* PnE4A : Monitor 2 address											*//* <S0DF> */
	DBYTEX	mnaz0_1;				/* PnE58 : Monitor 1,2 zero adjustment										*//* <S016> */
	DBYTEX	mnag0_1;				/* PnE59 : Monitor 1,2 gain adjustment										*//* <S016> */
	DBYTEX	pgadj;					/* PnE5A : Lower:PGPower supply voltage adjustment, Upper level:Empty								*/
	DBYTEX	pnadj;					/* PnE5B : Lower:P-NVoltage detection level fine adjustment, Upper level:P-NVoltage detection level zero adjustment	*/

	USHORT	NonRegOvlvl;			/* PnE71 : OV detection level when regenerative is not used in models without built-in regenerative resistor			*//* <S0F7> */
	USHORT	pnvoltgn;				/* PnE70 : P SounitsumVoltage detection level									*/
	USHORT	ovlvl;					/* PnE71 : OV detection level											*/
	USHORT	regonlvl;				/* PnE72 : Regeneration on level											*/
	USHORT	uvlvl;					/* PnE73 : UV detection level											*/
	USHORT	uvfil;					/* PnE74 : UV detection filter time constant									*/
	USHORT	regoffhys;				/* PnE78 : Regeneration off hysteresis width									*/
	USHORT	vdetadfil;				/* PnE79 : Low-pass filter time constant for main circuit detection AD					*/
	USHORT	vdefilsw;				/* PnE7A : Main circuit detection filter selection switch							*/
	USHORT	UvWrnLvl;				/* PnE7B : UV warning detection level									*/

	USHORT	VdetADMax;				/* PnE7C : PnE70[V]At the time of inputA/Dvalue */
	USHORT	ConvW;					/* PnE7F : Converter capacity(System)									*/
#if 0 /* Σ-7Delete as unnecessary *//* <S0F3> */
	USHORT	CnvCrtADNormMax;		/* PnE80 : Converter current detectionAD=32767 */
	USHORT	CnvCrtRatAD;			/* PnE81 : Converter continuous overload detectionADvalue(Temporary) */
	USHORT	CnvCrtMaxAD;			/* PnE82 : Converter instantaneous overload detectionADvalue(Temporary) */
	USHORT	CnvCrtRatTime;			/* PnE83 : Converter continuous overload detection time(Temporary)*/
	USHORT	CnvCrtMaxTime;			/* PnE84 : Converter instantaneous overload detection time(Temporary) */
#endif /* Σ-7Delete as unnecessary */
	/* <S154> Start */
	USHORT FanTemperatureOffset;    /* PnE80 : 	Built-inFANAmbient temperature offset for lifespan calculation	*/
	USHORT FanTimeOffset;			/* PnE81 :  FANReplacement offset time					*/
	USHORT RushLifeTime;			/* PnE82 :  Rush protection circuit life							*/
	/*  Start */
	ULONG RushSwichCount;			/* PnE84 :  Current anti-fault relay operation count				*/
//	USHORT RushSwichCount;			/* PnE83 :  Current anti-fault relay operating count (lower16bit)		*/
//	USHORT DBLifeTime;				/* PnE84 :  DBCircuit life							*/
//	USHORT DBSwichCount;			/* PnE85 : 	Current currentDBNumber of operations (lower16bit)		*/
//	USHORT DRSwithingCount;			/* PnE86 :  Current anti-fault protection/DBRelay operation count (upper) 0,1digit:
//												Current anti-fault relay operation count (upper8bit) 2,3digit: CurrentDBNumber of operations (upper8bit)  */
	/* <S192> End */
	/* <S154> End */
	USHORT	ChgWaitTime;			/* PnE88 : DC charging completion waiting time	<S053>								*/
	USHORT	PrdyWaitTime;			/* PnE89 : Power ready waiting time	<S053>								*/
	USHORT	DischCompVlt;			/* PnE8A : Rapid discharge continuation threshold voltage	<S1D1>								*/
	/* <S21D> Start */
	USHORT	regsynonlvl;			/* PnE9A : Regeneration during regeneration synchronizationONLevel adjustment value							*/
	USHORT	regsynontime;			/* PnE9B : Regeneration during regeneration synchronizationONWaiting time								*/
	/* <S21D> End */
	USHORT	ctrlboardalmtmpr;		/* PnE9C : Control board abnormal temperature/Warning temperature								*//* <S0E2> */
	USHORT	pwrboardalmtmpr;		/* PnE9D : Power board abnormal temperature/Warning temperature								*//* <S0E2> */
	USHORT	ctrlboardalmclrtmpr;	/* PnE9E : Control board abnormality clearable temperature/Abnormal temperature detection time				*//* <S0E2> */
	USHORT	pwrboardalmclrtmpr;		/* PnE9F : Power board abnormality clearable temperature/Abnormal temperature detection time			*//* <S0E2> */

	USHORT	FanLockTime;			/* PnEE5 : Servopack time built-in fan stop time						*//* <S0A3> */
	USHORT	SvonFanLock_Time;		/* PnEE6 : Built-in fan stop alarm detection time when servo is on				*/	/* <S219> */

#if 0 /* Σ-7Delete as unnecessary *//* <S0F3> */
	USHORT	ohfil;					/* PnEF0 : Temperature overheating detection filter time constant								*/
	SHORT	mbohlvl;				/* PnEF1 : Unit internal temperature overheating alarm level						*/
	SHORT	convohlvl;				/* PnEF2 : Converter temperature overheat alarm level							*/
	SHORT	ifohlvl;				/* PnEF3 : Control board temperature overheating alarm level							*/
	USHORT	ohwrnlvl;				/* PnEF4 : Temperature overheating warning level									*/
#endif /* Σ-7Delete as unnecessary */
	USHORT	DcBaseLvl;				/* PnEF5 : For main circuit voltage detection correction Reference voltage							*/

	ULONG	SeqInputInf;			/* PnEF6 */
	ULONG	SeqOutputInf;			/* PnEF8 */
} CMN_PRMDATA;



/****************************************************************************
	Parameter data structure definition
****************************************************************************/
typedef struct PRMDATA
{
/*------------------------------------------------------------------------------------------------------*/
	USHORT	b_prm0;					/* Pn000 : Function selection basic switch 0									*/
	USHORT	b_prm1;					/* Pn001 : Function selection application switch 1									*/
	USHORT	b_prm2;					/* Pn002 : Function selection application switch 2									*/
//	USHORT	b_prm6;					/* Pn006 : Function selection application switch 6									*//* <S103> */
//	USHORT	b_prm7;					/* Pn007 : Function selection application switch 7									*//* <S103> */
	USHORT	b_prm8;					/* Pn008 : Function selection application switch 8									*/
	USHORT	b_prm9;					/* Pn009 : Function selection application switch 9									*/
	USHORT	b_prmA;					/* Pn00A : Function selection application switch A									*//* <S0A6> */
	USHORT	b_prmB;					/* Pn00B : Function selection application switch B									*/
	USHORT	b_prmC;					/* Pn00C : Function selection application switch C									*//* <S0A6> */
	USHORT	b_prmD;					/* Pn00D : Function selection application switch D									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	FullcSlot;				/* Pn040 : Full close encoder connection slot number					*/
	USHORT	b_prm80;				/* Pn080 : Function selection application switch 80									*/
	USHORT	b_prm81;				/* Pn081 : Function selection application switch 81									*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	loophz;					/* Pn100 : Speed loop gain											*/
	USHORT	pitime;					/* Pn101 : Speed loop integral time constant										*/
	USHORT	posgn;					/* Pn102 : Position loop gain											*/
	USHORT	jrate;					/* Pn103 : Moment of inertia ratio											*/
	USHORT	loophz2;				/* Pn104 : Second velocity loop gain										*/
	USHORT	pitime2;				/* Pn105 : Second velocity loop integral time constant									*/
	USHORT	posgn2;					/* Pn106 : Second position loop gain										*/
	USHORT	ffgn;					/* Pn109 : Feedforward										*/
	USHORT	fffil;					/* Pn10A : Feedforward filter time constant							*/
	USHORT	gnmode;					/* Pn10B : Gain-related application switch									*/
	USHORT	mdswlv;					/* Pn10C : Mode switch(Torque command)								*/
	USHORT	mswrf;					/* Pn10D : Mode switch(Speed command)									*/
	USHORT	mswacc;					/* Pn10E : Mode switch(Acceleration)									*/
	USHORT	msplvl;					/* Pn10F : Mode switch(Position deviation)									*/
	USHORT	pitfil;					/* Pn11F : Position integration time constant											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	dobgn;					/* Pn121 : Friction compensation gain											*/
	USHORT	dobgn2;					/* Pn122 : Second friction compensation gain										*/
	USHORT	dtrqgn;					/* Pn123 : Friction compensation coefficient												*/
	SHORT	dlpfil;					/* Pn124 : Friction compensation frequency correction										*/
	USHORT	dobjgn;					/* Pn125 : Friction compensation gain correction										*/
	USHORT	evobgn;					/* Pn127 : Speed observer gain										*/
	USHORT	evobjgn;				/* Pn128 : Speed observer position compensation gain								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	gnswfil1;				/* Pn131 : Gain switching time 1										*/
	USHORT	gnswfil2;				/* Pn132 : Gain switching time 2										*/
	USHORT	gnswwait1;				/* Pn135 : Gain switching waiting time 1									*/
	USHORT	gnswwait2;				/* Pn136 : Gain switching waiting time 2									*/
	USHORT	gnsw1;					/* Pn139 : Automatic gain switching related switch 1							*/
	USHORT	Pn13B;					/* Pn13B : Speed observer low-pass filter time constant						*/
	USHORT	KcGainRatio;			/* Pn13D : Current gain level											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	mdlsw;					/* Pn140 : Model following control related switches								*/
	USHORT	mdlgn;					/* Pn141 : Model following control gain [0.1/s]								*/
	USHORT	mdlzeta;				/* Pn142 : Model following control gain correction									*/
	USHORT	mdlptff;				/* Pn143 : Model following control bias(Forward rotation direction)							*/
	USHORT	mdlntff;				/* Pn144 : Model following control bias(Reverse direction)							*/
	USHORT	mdlantfrq;				/* Pn145 : Vibration suppression 1 frequency A										*/
	USHORT	mdlresfrq;				/* Pn146 : Vibration suppression 1 frequency B										*/
	USHORT	mdlvff;					/* Pn147 : Model following control speed feedforward compensation					*/
	USHORT	mdlgn2;					/* Pn148 : Second model following control gain [0.1/s]							*/
	USHORT	mdlzeta2;				/* Pn149 : Second model following control gain correction								*/
	USHORT	ff_frq;					/* Pn14A : Vibration suppression 2 frequency											*/
	USHORT	ff_fil;					/* Pn14B : Vibration suppression 2 correction											*/
	USHORT	mfctype;				/* Pn14F : Control-related switches											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	avibsw;					/* Pn160 : Damping control related switches										*/
	USHORT	avibfrq;				/* Pn161 : A-type vibration damping frequency											*/
	USHORT	avibgn;					/* Pn162 : A type vibration suppression gain correction										*/
	USHORT	avibdamp;				/* Pn163 : A type vibration suppression damping gain									*/
	SHORT	avibfil1;				/* Pn164 : A type vibration suppression filter time constant 1 correction								*/
	SHORT	avibfil2;				/* Pn165 : A type vibration suppression filter time constant 2 correction								*/
	USHORT	avibdamp2;				/* Pn166 : 3Inertia vibration suppression damping gain				<S03E>		*/
	SHORT	avibfrq2;				/* Pn167 : 3Inertia damping frequency						<S03E>		*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	DatLevel;				/* Pn170 : Dynamic auto tuning related switches				*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	jrateSw;				/* Pn17A : Reserved parameters (inertia ratio switching related switches)		*//* <S0A6> */
	USHORT	jrate2;					/* Pn17B : No.2Inertia ratio											*//* <S0A6> */
	USHORT	jrateChgTime1;			/* Pn17C : Inertia ratio switching time 1								*//* <S0A6> */
	USHORT	jrateChgTime2;			/* Pn17D : Inertia ratio switching time 2								*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	mswrf_l;				/* Pn181 : Mode switch(Speed command)									*/
	USHORT	mswacc_l;				/* Pn182 : Mode switch(Acceleration)									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	divlessSw;				/* Pn190 : Reserved parameters (deviation-less/related switches)					*//* <S0A6> */
	USHORT	velffgain;				/* Pn191 : Speed feedforward gain								*//* <S0A6> */
	USHORT	trqffgain;				/* Pn192 : Torque feedforward gain							*//* <S0A6> */
	USHORT	trqfftime;				/* Pn193 : Torque feedforward filter time constant					*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	prstsw;					/* Pn200 : Position control command form selection switch								*/
	USHORT	mtlmt;					/* Pn205 : Multiturn limit setting									*/
	USHORT	cmfilsw;				/* Pn207 : Position command function switch										*/
	USHORT	expbias1;				/* Pn209 : Position command acceleration/deceleration bias									*/
	ULONG	fcplsml;				/* Pn20A : Number of external scale pitches									*/
	ULONG	ratb2l;					/* Pn20E : Electronic gear ratio(Molecular)											*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	rata2l;					/* Pn210 : Electronic gear ratio(Denominator)											*/
	ULONG	pgrat2l;				/* Pn212 : PG division ratio(Pulse unit)									*/
	USHORT	expfil;					/* Pn216 : Position command acceleration/deceleration time constant										*/
	USHORT	mafil;					/* Pn217 : Position command moving average time										*/
	USHORT	MulFrq;					/* Pn218 : Command pulse input magnification										*/
	USHORT	highrad;				/* Pn21B : HIGH filter average rad                                 *//*<S0C3>*/
	/*------------------------------------------------------------------------------------------------------*/
	USHORT	prefHfilacctime;		/* Pn21B : Position command high order filter acceleration/deceleration time							*//* <S0A6> */
	USHORT	prefHfilsmooth;			/* Pn21C : Position command high order filter smoothing ratio						*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT  fcsel;					/* Pn22A : Full-closed control selection switch								*//* <S04D> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	cmctrlexsw;				/* Pn230 : Position control extended function switch									*/
	LONG	blcm2l;					/* Pn231 : Backlash correction amount										*/
	USHORT	blfiltime;				/* Pn233 : Backlash correction time constant									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	perrclrsigtime;			/* Pn240 : Position deviation clear signal input minimum time interval						*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	encbaudrate;			/* Pn250 : Encoder communication speed selection									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	scalepgrat;				/* Pn281 : Encoder output resolution										*/
	ULONG	scalepitch2l;			/* Pn282 : Linear scale pitch										*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	psencernum;				/* Pn2D0 : Reserved parameters (number of position sensor outputs)						*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	vrefgn;					/* Pn300 : Speed command input gain										*/
	USHORT	speed1;					/* Pn301 : Internal setting speed1(Rotation)										*/
	USHORT	speed2;					/* Pn302 : Internal setting speed2(Rotation)										*/
	USHORT	speed3;					/* Pn303 : Internal setting speed3(Rotation)										*/
	USHORT	jogspd;					/* Pn304 : JOG speed(Rotation)											*/
	USHORT	sfsacc;					/* Pn305 : Soft start acceleration time									*/
	USHORT	sfsdec;					/* Pn306 : Soft start deceleration time									*/
	USHORT	srfil;					/* Pn307 : Speed command filter time constant									*/
	USHORT	fbfil;					/* Pn308 : Speed F/B filter time constant									*/
	USHORT	svoffdectime;			/* 0x30A : Deceleration time when servo is off and forced stop						*//* <S0A6> */
	USHORT	vrefmafil;				/* 0x30B : Speed command moving average time										*//* <S0A6> */
	USHORT	vffmafil;				/* 0x30C : Speed feedforward moving average time						*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	vibsel;					/* Pn310 : Vibration detection switch											*/
	USHORT	vibsens;				/* Pn311 : Vibration detection sensitivity												*/
	USHORT	vibnorm;				/* Pn312 : Vibration detection level(Rotation)										*/
	USHORT	maxvel;					/* 0x316 : Motor maximum speed(Rotation)										*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	Pn324;					/* Pn324 : Moment of inertia identification start level								*/
	USHORT	Pn325;					/* Pn325 : Vibration detection level during moment of inertia identification(Rotation)				*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	speed1_l;				/* Pn380 : Internal setting speed1(Linear)									*/
	USHORT	speed2_l;				/* Pn381 : Internal setting speed2(Linear)									*/
	USHORT	speed3_l;				/* Pn382 : Internal setting speed3(Linear)									*/
	USHORT	jogspd_l;				/* Pn383 : JOG speed(Linear)										*/
	USHORT	vibnorm_l;				/* Pn384 : Vibration detection level(Linear)									*/
	USHORT	maxvel_l;				/* Pn385 : Motor maximum speed(Linear)									*/
	USHORT	Pn387;					/* Pn386 : Vibration detection level during moment of inertia identification(Linear)				*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	trefgn;					/* Pn400 : Torque command input gain										*/
	USHORT	trqfil11;				/* Pn401 : 1st stage 1st torque command filter time constant						*/
	USHORT	tlmtf;					/* Pn402 : Forward rotation torque limit											*/
	USHORT	tlmtr;					/* Pn403 : Reverse torque limit											*/
	USHORT	outlmf1;				/* Pn404 : Forward rotation side external torque limit										*/
	USHORT	outlmr1;				/* Pn405 : Reverse side external torque limit										*/
	USHORT	emgtrq;					/* Pn406 : Emergency stop torque											*/
	USHORT	tcrlmt;					/* Pn407 : Speed limit during torque control									*/
	USHORT	tfuncsw;				/* Pn408 : Torque related function switch									*/
	USHORT	ntchfil1;				/* Pn409 : 1st stage notch filter frequency								*/
	USHORT	ntchq1;					/* Pn40A : 1st stage notch filter Q value									*/
	USHORT	ntchdamp1;				/* Pn40B : 1st stage notch filter depth									*/
	USHORT	ntchfil2;				/* Pn40C : 2nd stage notch filter frequency								*/
	USHORT	ntchq2;					/* Pn40D : 2nd stage notch filter Q value									*/
	USHORT	ntchdamp2;				/* Pn40E : 2nd stage notch filter depth									*/
	USHORT	secfil;					/* Pn40F : 2nd stage secondary torque command filter frequency						*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	secq;					/* Pn410 : 2nd stage secondary torque command filter Q value							*/
	USHORT	trqfil12;				/* Pn412 : 1st stage 2nd torque command filter time constant						*/
	USHORT	treffil;				/* Pn415 : T-REFFilter time constant										*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	ntchfil3;				/* Pn416 : 3Stage notch filter frequency								*//* <S0A6> */
	USHORT	ntchq3;					/* Pn417 : 3Stage notch filterQvalue									*//* <S0A6> */
	USHORT	ntchdamp3;				/* Pn418 : 3Stage notch filter depth									*//* <S0A6> */
	USHORT	ntchfil4;				/* Pn419 : 4Stage notch filter frequency								*//* <S0A6> */
	USHORT	ntchq4;					/* Pn41A : 4Stage notch filterQvalue									*//* <S0A6> */
	USHORT	ntchdamp4;				/* Pn41B : 4Stage notch filter depth									*//* <S0A6> */
	USHORT	ntchfil5;				/* Pn41C : 5Stage notch filter frequency								*//* <S0A6> */
	USHORT	ntchq5;					/* Pn41D : 5Stage notch filterQvalue									*//* <S0A6> */
	USHORT	ntchdamp5;				/* Pn41E : 5Stage notch filter depth									*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	tfuncsw2;				/* Pn41F : Torque(Thrust)Related function switch 2							*//* <S0A6> */
	USHORT	RippleCompSw;			/* Pn423 : Torque compensation related function switch								*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	UvTrqLmt;				/* Pn424 : Torque limit when main circuit voltage drops [%]							*/
	USHORT	UvTrqLmtOffFil;			/* Pn425 : Torque limit release time when main circuit voltage drops						*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	tffmafil;				/* 0x426 : Torque feedforward moving average time						*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	VelRippleCompSpd;		/* Pn427 : Speed ripple compensation effective speed									*/	/* <S14B> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	swpminfrq;				/* Pn453 : Sweep start frequency											*/
	USHORT	swpmaxfrq;				/* Pn454 : Sweep end frequency											*/
	USHORT	detfrqlmt;				/* Pn455 : Detection resonance frequency lower limit										*/
	USHORT	ezffttrq;				/* Pn456 : Sweep torque command amplitude										*/
	USHORT	ezfftsw;				/* Pn457 : FFT-related function switch									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	anotchsw;				/* Pn460 : Notch filter adjustment switch								*/
	USHORT	anotchsw2;				/* Pn466 : Notch filter adjustment switch2								*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	gcomptrq;				/* Pn470 : Gravity compensation torque											*//* <S0A6> */
	USHORT	pqfriqcomptrq;			/* Pn471 : Coulomb friction compensation torque in the positive direction								*//* <S0A6> */
	USHORT	nqfriqcomptrq;			/* Pn472 : Negative direction Coulomb friction compensation torque								*//* <S0A6> */
	USHORT	vfrigcomptrq;			/* Pn473 : Viscous friction compensation torque										*//* <S0A6> */
	USHORT	spdhys;					/* Pn474 : Internal setting speedhysteresis									*//* <S0CD> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	tcrlmt_l;				/* Pn480 : Speed limit during thrust control										*/
	USHORT	tlmtf_l;				/* Pn483 : Forward rotation thrust limit												*/
	USHORT	tlmtr_l;				/* Pn484 : Reverse thrust force limit												*/

	MPFIND_UPRM PdetP;				/* Pn481 - Pn49B : Magnetic pole detection parameters 								*/

	/*------------------------------------------------------------------------------------------------------*/
	USHORT	clamp;					/* Pn501 : Zero clamp level										*/
	USHORT	tgonlv;					/* Pn502 : Rotation detection level											*/
	USHORT	vcmplv;					/* Pn503 : Speed match signal output width										*/
	USHORT	brktim;					/* Pn506 : Brake command MatsuchaT-Booff delay time							*/
	USHORT	brkspd;					/* Pn507 : Brake command output speed level								*/
	USHORT	brkwai;					/* Pn508 : Servo off BakuchauRake command waiting time							*/
	USHORT	insel1;					/* Pn50A : Input signal selection1											*/
	USHORT	insel2;					/* Pn50B : Input signal selection2											*/
	USHORT	insel3;					/* Pn50C : Input signal selection3											*/
	USHORT	insel4;					/* Pn50D : Input signal selection4											*/
	USHORT	outsel1;				/* Pn50E : Output signal selection1											*/
	USHORT	outsel2;				/* Pn50F : Output signal selection2											*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	outsel3;				/* Pn510 : Output signal selection3											*/
	USHORT	inselA;					/* Pn511 : Input signal selection5											*/
//	USHORT	sorev;					/* Pn512 : Output signal inversion setting											*//* <S04B> */
	USHORT	sorev1;					/* Pn512 : Output signal inversion setting1										*//* <S04B> */
//	USHORT	outselA;				/* Pn513 : Output signal selection4											*//* <S04B> */
	USHORT	sorev2;					/* Pn513 : Output signal inversion setting2										*//* <S04B> */
	USHORT	outsel4;				/* 0x514 : Output signal selection5											*//* <S0A6> */
	USHORT	insel5;					/* Pn515 : Input signal selection6											*/
	USHORT	insel6;					/* 0x516 : Input signal selection7											*//* <S0A6> */

	USHORT	outsel6;				/* Pn517 : Output signal selection6											*/
	USHORT	outsel7;				/* Pn518 : Output signal selection7											*/
	USHORT	insel7;					/* Pn519 : Input signal selection8											*//* <S0A6> */
	ULONG	fpgoverlv2l;			/* Pn51B : Motor-Excessive deviation between load positions detection level						*/
	USHORT	overwrnlv;				/* Pn51E : Excessive position deviation warning level								*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	overlv2l;				/* Pn520 : Excessive position deviation alarm level								*/
	ULONG	coinlv2l;				/* Pn522 : Positioning completion width											*/
	ULONG	nearlv2l;				/* Pn524 : NEAR signal width											*/
	ULONG	svonoverlv2l;			/* Pn526 : Excessive position deviation alarm level when servo is on					*/
	USHORT	svonoverwrnlv;			/* Pn528 : Excessive position deviation warning level when servo is on					*/
	USHORT	svonspdlmtlv;			/* Pn529 : Speed limit level when servo is on								*/
	USHORT	fpgcutrate;				/* Pn52A : Full close1Multiply value per rotation							*/
	USHORT	olwarlvl;				/* Pn52B : Overload warning level									*/
	USHORT	mtbastrq_drt;			/* Pn52C : Motor overload detection base current derating				*/
	USHORT	svbastrq_drt;			/* Pn52D : Servopack overload detection base current derating when single-phase power input is selected	*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	powon_mon	;			/* 0x52F : Monitor display at power-on									*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	pjogsw;					/* Pn530 : Program JOG operation related switches							*/
	ULONG	pjogdistl;				/* Pn531 : Program JOG movement distance									*/
	USHORT	pjogrotspd;				/* Pn533 : Program JOG movement speed									*/
	USHORT	pjogacctm;				/* Pn534 : Program JOG acceleration/deceleration time								*/
	USHORT	pjogwaittm;				/* Pn535 : Program JOG waiting time									*/
	USHORT	pjognum;				/* Pn536 : Program JOG movement count									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	outalm_no1 ;			/* Pn53A : Signal output alarm/warning number1						*//* <S0A6> */
	USHORT	outalm_no2 ;			/* Pn53B : Signal output alarm/warning number2						*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	adatgmax;				/* Pn540 : Gain limit											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	trcalm_no;				/* Pn548 : Trace specified alarm number									*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
//	USHORT	monoffset1;				/* Pn550 : Analog monitor 1 offset voltage							*//* <S0DF> */
//	USHORT	monoffset2;				/* Pn551 : Analog monitor 2 offset voltage							*//* <S0DF> */
//	USHORT	mongain1;				/* Pn552 : Analog monitor1magnification										*//* <S0DF> */
//	USHORT	mongain2;				/* Pn553 : Analog monitor2magnification										*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
//	USHORT	powmon_unitime ;		/* Pn55A : Power consumption monitor unit time									*//* <S0A6> *//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	remdetw;				/* Pn560 : Residual vibration detection width											*/
	USHORT	ovserrdetlvl;			/* Pn561 : Overshoot detection level								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	clamp_l;				/* Pn580 : Zero clamp level										*/
	USHORT	tgonlv_l;				/* Pn581 : Rotation detection level											*//* <S003> */
	USHORT	vcmplv_l;				/* Pn582 : Speed match signal output width										*//* <S003> */
	USHORT	brkspd_l;				/* Pn583 : Brake command output speed level								*/
	USHORT	svonspdlmtlv_l;			/* Pn584 : Speed limit level when servo is on								*/
	USHORT	pjoglinspd;				/* Pn585 : Program JOG maximum speed(Linear)							*/
	USHORT	msrcra;					/* Pn586 : Motor free-running cooling rate											*/
	USHORT	pldetsw;				/* Pn587 : Absolute value scale magnetic pole detection execution selection							*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	pot_selsw;				/* Pn590 : POTInput signal setting											*/
	USHORT	not_selsw;				/* Pn591 : NOTInput signal setting											*/
	USHORT	dec_selsw;				/* Pn592 : DECInput signal setting											*/
	USHORT	ext_selsw[3];			/* Pn593 : EXTInput signal setting											*/
//	USHORT	bk_selsw;				/* Pn596 : BKOutput signal setting											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	fstp_selsw;				/* Pn597 : <S1B5> FSTPInput signal setting									*/
	USHORT	pcl_selsw;				/* Pn598 : <S1B5> P-CLInput signal setting									*/
	USHORT	ncl_selsw;				/* Pn599 : <S1B5> N-CLInput signal setting									*/
	USHORT	clinkans_selsw;			/* Pn59A : <S1B5> CLINKANSInput signal setting								*/
	USHORT	cmcans_selsw;			/* Pn59B : <S1B5> CMCANSInput signal setting								*/
	USHORT	convready_selsw;		/* Pn59C : <S1B5> CONV_READYInput signal setting							*/
	USHORT	convmcon_selsw;			/* Pn59D : <S1B5> CONV_MCONInput signal setting								*/
	USHORT	coin_selsw;				/* Pn5B0 : <S1B5> COINOutput signal setting									*/
	USHORT	vcmp_selsw;				/* Pn5B1 : <S1B5> VCMPOutput signal setting									*/
	USHORT	tgon_selsw;				/* Pn5B2 : <S1B5> TGONOutput signal setting									*/
	USHORT	srdy_selsw;				/* Pn5B3 : <S1B5> S-RDYOutput signal setting									*/
	USHORT	clt_selsw;				/* Pn5B4 : <S1B5> CLTOutput signal setting									*/
	USHORT	vlt_selsw;				/* Pn5B5 : <S1B5> VLTOutput signal setting									*/
	USHORT	bk_selsw;				/* Pn5B6 : <S1B5> BKOutput signal setting									*/
	USHORT	warn_selsw;				/* Pn5B7 : <S1B5> WARNOutput signal setting									*/
	USHORT	near_selsw;				/* Pn5B8 : <S1B5> NEAROutput signal setting									*/
	USHORT	cpulse_selsw;			/* Pn5B9 : <S1B5> C-PULSEOutput signal setting								*/
	USHORT	clink_selsw;			/* Pn5BA : <S1B5> CLINKOutput signal setting									*/
	USHORT	cmcon_selsw;			/* Pn5BB : <S1B5> CMCONOutput signal setting									*/
	USHORT	pm_selsw;				/* Pn5BC : <S1B5> PMOutput signal setting									*/
	USHORT	dao_selsw[2];			/* Pn5BD : <S1B5> DAOOutput signal setting									*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	dbres;					/* Pn601 : DB resistance capacity												*/
//	USHORT	dber;					/* Pn604 : DB resistance value												*//* <S0A6> *//* <S0F5> */
	USHORT	exdbr;					/* Pn604 : DB resistance value												*//* <S0F5> */
	USHORT	bbondiff0;				/* Pn620 : HWBBSignal timing time									*/
	USHORT	saftyfuncSw;			/* Pn621 : Safety function application selection switch									*//* <S0A6> */
	USHORT	safeindecA;				/* Pn622 : Active mode internal deceleration constantA							*//* <S0A6> */
	USHORT	safeindecB;				/* Pn623 : Active mode internal deceleration constantB							*//* <S0A6> */
	USHORT	safeindecVel;			/* Pn624 : Active mode internal deceleration stop speed							*//* <S0A6> */
	USHORT	safeStopTime;			/* Pn625 : Active mode stop duration								*//* <S0A6> */
	ULONG	safePerrlvl;			/* Pn626 : Active mode release position deviation level						*//* <S0A6> */
	USHORT	safeVreflvl;			/* Pn628 : Active mode release speed command level						*//* <S0A6> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	sq_b_prm0;				/* Pn800 : SynqNetFunction selection switch 0								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	comCtrl;				/* Pn800 : Communication control function selection											*/
	USHORT	sLimitSw;				/* Pn801 : Soft limit function selection									*/
	USHORT	cmdCtrlSw;				/* Pn802 : Command operation setting											*/
	USHORT	zpWidth;				/* Pn803 : Origin position range												*/
#if 0/* <S07F>:Mold change */
	USHORT	p_sLimitL;				/* Pn804 : Positive soft limit value L									*/
	USHORT	p_sLimitH;				/* Pn805 : Positive soft limit value H									*/
	USHORT	n_sLimitL;				/* Pn806 : Negative side soft limit value L									*/
	USHORT	n_sLimitH;				/* Pn807 : Negative side soft limit value H									*/
	USHORT	absOffsetL;				/* Pn808 : Absolute PG origin position offset L							*/
	USHORT	absOffsetH;				/* Pn809 : Absolute PG origin position offset H							*/
#else
	LONG	p_sLimitL;				/* Pn804 : Positive soft limit value L									*/
	LONG	n_sLimitL;				/* Pn806 : Negative side soft limit value L									*/
	LONG	absOffsetL;				/* Pn808 : Absolute PG origin position offset L							*/
#endif/* <S07F>:Mold change */
	USHORT	accRate1;				/* Pn80A : 1st stage linear acceleration constant										*/
	USHORT	accRate2;				/* Pn80B : 2nd stage linear acceleration constant										*/
	USHORT	accChgSpd;				/* Pn80C : Acceleration constant switching speed										*/
	USHORT	decRate1;				/* Pn80D : 1st stage linear deceleration constant										*/
	USHORT	decRate2;				/* Pn80E : 2nd stage linear deceleration constant										*/
	USHORT	decChgSpd;				/* Pn80F : Deceleration constant switching speed										*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	filExpBias;				/* Pn810 : Exponential function acceleration/deceleration bias									*/
	USHORT	filExpTb;				/* Pn811 : Exponential acceleration/deceleration time constant										*/
	USHORT	filMaTb;				/* Pn812 : Moving average time												*/
	USHORT	resrv2;					/* Pn813 : Reservation														*/
#if 0/* <S07F>:Mold change */
	USHORT	exDistL;				/* Pn814 : External positioning final travel distance L								*/
	USHORT	exDistH;				/* Pn815 : External positioning final travel distance H								*/
#else
	LONG	exDistL;				/* Pn814 : External positioning final travel distance L								*/
#endif/* <S07F>:Mold change */
	USHORT	zretDir;				/* Pn816 : Return-to-origin mode setting										*/
	USHORT	zretSpd1;				/* Pn817 : Return to origin approach speed 1									*/
	USHORT	zretSpd2;				/* Pn818 : Return to origin approach speed 2									*/
#if 0/* <S07F>:Mold change */
	USHORT	zretDistL;				/* Pn819 : Home return final travel distance L									*/
	USHORT	zretDistH;				/* Pn81A : Home return final travel distance H									*/
#else
	LONG	zretDistL;				/* Pn819 : Home return final travel distance L									*/
#endif/* <S07F>:Mold change */
	USHORT	resrv3;					/* Pn81B : Reservation														*/
	USHORT	resrv4;					/* Pn81C : Reservation														*/
	USHORT	resrv5;					/* Pn81D : Reservation														*/
	USHORT	ioMonSel;				/* Pn81E : Input signal monitor selection										*/
	USHORT	optBitSw;				/* Pn81F : Command data allocation selection									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	p_ltLimitL;				/* Pn820 : Positive latchable area L										*/
	LONG	n_ltLimitL;				/* Pn822 : Reverse side latchable area L										*/
//	USHORT	n_ltLimitH;				/* Pn823 : Reverse side latchable area H										*/
	USHORT	opMon1;					/* Pn824 : Option monitor 1 selection									*/
	USHORT	opMon2;					/* Pn825 : Option monitor 2 selection									*/
	USHORT	decRate1Emg;			/* Pn826 : First stage linear deceleration constant for emergency stop								*/
	USHORT	decRate2Emg;			/* Pn827 : Second stage linear deceleration constant for emergency stop								*/
	USHORT	decChgSpdEmg;			/* Pn828 : Emergency stop deceleration constant switching speed								*/
	USHORT	svOffTime;				/* Pn829 : SVOFFWaiting time(Deceleration stopSVOFFHour)							*/
	USHORT	optbit1;				/* Pn82A : OPTIONFunction bit assignment 1									*/
	USHORT	optbit2;				/* Pn82B : OPTIONFunction bit assignment 2									*/
	USHORT	optbit3;				/* Pn82C : OPTIONFunction bit assignment 3									*/
	USHORT	optbit4;				/* Pn82D : OPTIONFunction bit assignment 4									*/
	USHORT	optbit5;				/* Pn82E : OPTIONFunction bit assignment 5									*/
	USHORT	resrv6;					/* Pn82F : Reservation														*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	rotrefL;				/* Pn830 : Command unit amount per machine revolutionL							*/
	USHORT	rotrefH;				/* Pn831 : Command unit amount per machine revolutionH							*/
	USHORT	st_num;					/* Pn832 : Number of stations per machine revolution							*/
	USHORT	rateChgSw;				/* Pn833 : POSINGConstant selection											*/
	ULONG	accRate12L;				/* Pn834 : 1st stage linear acceleration constant 2nd L									*/
	ULONG	accRate22L;				/* Pn836 : 2nd stage linear acceleration constant 2nd L									*/
	ULONG	accChgSpd2L;			/* Pn838 : Acceleration constant switching speed 2nd L									*/
	ULONG	decRate12L;				/* Pn83A : 1st stage linear deceleration constant 2nd L									*/
	ULONG	decRate22L;				/* Pn83C : 2nd stage linear deceleration constant 2nd L									*/
	ULONG	decChgSpd2L;			/* Pn83E : Deceleration constant switching speed 2nd L									*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	decRateEmg2L;			/* Pn840 : Second stage linear deceleration constant for emergency stop2nd L						*/
	LONG	zretSpd12L;				/* Pn842 : Return to origin approach speed 1 2nd L							*/
	LONG	zretSpd22L;				/* Pn844 : Return to origin approach speed 2 2nd L							*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	ltSeqNum;				/* Pn850 : Number of consecutive latch sequences									*/
	USHORT	ltLoopNum;				/* Pn851 : Continuous latch count											*/
	USHORT	ltSeqSet1;				/* Pn852 : Continuous latch sequence setting1-4								*/
	USHORT	ltSeqSet2;				/* Pn853 : Continuous latch sequence setting4-8								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	svcmdIoOut[VENDOR_IOPRM_NUM];	/* Pn855~: SVCMD_IOOutputVendor Specific I/OArea allocation functionX				*//* <S1C0> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	cmdIo1;					/* Pn860 : SVCMD_IO(Input signal monitor)Allocation function1						*/
	USHORT	cmdIo2;					/* Pn861 : SVCMD_IO(Input signal monitor)Allocation function2						*/
	USHORT	cmdIo3;					/* Pn862 : SVCMD_IO(Input signal monitor)Allocation function3						*/
	USHORT	cmdIo4;					/* Pn863 : SVCMD_IO(Input signal monitor)Allocation function4						*/
	USHORT	cmdIo5;					/* Pn864 : SVCMD_IO(Input signal monitor)Allocation function5						*/
	USHORT	cmdIo6;					/* Pn865 : SVCMD_IO(Input signal monitor)Allocation function6						*/
	USHORT	cmdIo7;					/* Pn866 : SVCMD_IO(Input signal monitor)Allocation function7						*/
	USHORT	cmdIoOut1;				/* Pn868 : SVCMD_IO(Output signal monitor)Allocation function1						*/
	USHORT	cmdIoOut2;				/* Pn869 : SVCMD_IO(Output signal monitor)Allocation function2						*/
	USHORT	cmdIoOut3;				/* Pn86A : SVCMD_IO(Output signal monitor)Allocation function2						*//* <S1C0> */
	USHORT	p_zone1L;				/* Pn870 : Positive sideZONE1Signal output boundary position								*/
	USHORT	n_zone1L;				/* Pn872 : Negative sideZONE1Signal output boundary position								*/
	USHORT	p_zone2L;				/* Pn874 : Positive sideZONE2Signal output boundary position								*/
	USHORT	n_zone2L;				/* Pn876 : Negative sideZONE2Signal output boundary position								*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	nodeAdr;				/* Pn880 : Station address setting											*/
	USHORT	trByte;					/* Pn881 : Transmission byte count												*/
	USHORT	comTCycle;				/* Pn882 : Transmission cycle													*/
	USHORT	comAplCycle;			/* Pn883 : Communication cycle												*/
	USHORT	commErrBehavior;
//	USHORT	comErrSts1;
//	USHORT	comErrSts2;
	USHORT	comDataCtrl;
	USHORT	comErrCnt;				/* Pn88A : Reception error counter										*/

/*------------------------------------------------------------------------------------------------------*/
 #if (CSW_NETIF_TYPE == NETIF_M2_TYPE)												/* MECATROLINK- M2	*/
	USHORT	comErrSts1;
	USHORT	comErrSts2;
/* <S17F>Change the number of elements with type */
	USHORT	almCmd[16];				/* Pn890 : Communication when an alarm occursCMDData								*//* <S0BD> */
	USHORT	almRsp[16];				/* Pn8A0 : Communication when an alarm occursRSPData								*//* <S0BD> */
	USHORT	comCmd[16];				/* Pn8B0 : Online, communicationCMDData								*//* <S0BD> */
	USHORT	comRsp[16];				/* Pn8C0 : Online, communicationRSPData								*//* <S0BD> */
/*------------------------------------------------------------------------------------------------------*/
 #elif (CSW_NETIF_TYPE == NETIF_M3_TYPE)					/*  MECATROLINK-M3	*/
	ULONG	comErrSts1L;
	ULONG	comErrSts2L;
/* <S0CA> Start *//* <S17F>Change the number of elements with type */
	USHORT	almCmd[24];				/* Pn890 : Communication when an alarm occursCMDData								*//* <S0BD> */
	USHORT	almRsp[24];				/* Pn8A8 : Communication when an alarm occursRSPData								*//* <S0BD> */
	USHORT	comCmd[24];				/* Pn8C0 : Online, communicationCMDData								*//* <S0BD> */
	USHORT	comRsp[24];				/* Pn8D8 : Online, communicationRSPData								*//* <S0BD> */
/* <S0CA> End */
 #endif
	/*--------------------------------------------------------------------------------------------------*/
	USHORT	bnkNum;					/* Pn900 : Number of banks													*/
	USHORT	bnkMemNum;				/* Pn901 : Number of bank members											*/
	/*--------------------------------------------------------------------------------------------------*/
	USHORT	bnkMember[15];			/* Pn902 : Bank member												*//* <S080> */
	USHORT	bnkData[0x3F];			/* Pn920~Pn95F :Each bank member value									*//* <S080> */
	/*--------------------------------------------------------------------------------------------------*/
	USHORT	prmVer;					/* Pn9E0 : ParameterVer.											*/
	USHORT	spdDivision;			/* Pn9E1 : Speed resolution setting											*/
	USHORT	CmdOpSw;				/* Pn9E2 : Command-OptionFunctionSW										*/
	USHORT	sys_rsrv2;				/* Pn9E3 : 															*/
	USHORT	sys_rsrv3;				/* Pn9E4 : 															*//*------------------------------------------------------------------------------------------------------*/
	USHORT	posing_mode;			/* Pn8FF : 	POSINGMode selection										*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	monAdr;					/* Pn980 : 	Other station monitor address 										*/
	USHORT	monDataSize;			/* Pn981 : 	Other station communication data size									*/
	USHORT	monFuncSel;				/* Pn982 : 	Other station monitor function selection switch								*/
	USHORT	monDataSel;				/* Pn983 : 	Stop operation input command monitor setting							*/
	USHORT	trceStpSpd;				/* Pn984 : 	Following stop speed											*/
	USHORT	monAdj;					/* Pn985 : 	Cooperative stop adjustment parameter									*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_encTypeL;			/* PnA02 : Encoder type selection L									*/
	ULONG	m3_motorTypeL;			/* PnA04 : Motor type selection L										*/
	ULONG	m3_fulcTypeL;			/* PnA06 : Semi-closed/full-closed selection L							*/
	ULONG	m3_ratSpdL;				/* PnA08 : Rated rotation speed L											*/
	ULONG	m3_permitSpdL;			/* PnA0A : Maximum output speed L										*/
	ULONG	m3_spdMultiplierL;		/* PnA0C : Speed multiplier L												*/
	ULONG	m3_ratTrqL;				/* PnA0E : Rated torque L												*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_permitTrqL;			/* PnA10 : Maximum output torque L										*/
	ULONG	m3_trqMultiplierL;		/* PnA12 : Torque multiplier L												*/
	ULONG	m3_reslutionL;			/* PnA14 : Resolution(Rotary) L										*/
	ULONG	m3_scalepitchL;			/* PnA16 : Linear scale pitch L									*/
	ULONG	m3_scalepitchH;			/* PnA17 : Linear scale pitch H									*/
	ULONG	m3_pulsePerPitchL;		/* PnA18 : Number of pulses per scale pitch (linear) L					*/
/*------------------------------------------------------------------------------------------------------*/
	LONG	m3_ratbL;				/* PnA42 : Electronic gear ratio (numerator)										*/
	LONG	m3_rataL;				/* PnA44 : Electronic gear ratio (denominator)										*/
	LONG	m3_absOffsetL;			/* PnA46 : AbsolutePGOrigin position offset L								*/
	ULONG	m3_mtLimitL;			/* PnA48 : Multiturn limit setting L								*/
	ULONG	m3_limitSwL;			/* PnA4A : Limit setting L											*/
	LONG	m3_psLimitL;			/* PnA4C : Positive soft limit value L									*/
	LONG	m3_reserve1L;			/* Preliminary	*/
/*------------------------------------------------------------------------------------------------------*/
	LONG	m3_nsLimitL;			/* PnA50 : Negative side soft limit value L									*/
	LONG	m3_reserve2L;			/* Preliminary	*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_velUnitL;			/* PnA82 : Speed unit selection L											*/
	ULONG	m3_velBase4L;			/* PnA84 : Speed basic unit selection L										*/
	ULONG	m3_posUnitL;			/* PnA86 : Position unit selection L											*/
	ULONG	m3_posBaseL;			/* PnA88 : Position basic unit selection L										*/
	ULONG	m3_accUnitL;			/* PnA8A : Acceleration unit selection L											*/
	ULONG	m3_accBase4L;			/* PnA8C : Acceleration basic unit selection L										*/
	ULONG	m3_trqUnitL;			/* PnA8E : Torque unit selection L											*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_trqBase4L;			/* PnA90 : Torque basic unit selection L										*/
	ULONG	m3_supportUnitL;		/* PnA92 : Compatible unit system L												*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_KvL;					/* PnAC2 : Speed loop gain L										*/
	ULONG	m3_TviL;				/* PnAC4 : Speed loop integral time constant L									*/
	ULONG	m3_KpL;					/* PnAC6 : Position loop gain L										*/
	ULONG	m3_FFGainL;				/* PnAC8 : Feedforward compensation L									*/
	ULONG	m3_TpiL;				/* PnACA : Position loop integration time constant L									*/
	ULONG	m3_psetWidthL;			/* PnACC : Positioning completion width L											*/
	ULONG	m3_nearWidthL;			/* PnACE : Positioning vicinity width  L										*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_filExpTbL;			/* PnB02 : Exponential acceleration/deceleration time constant L									*/
	ULONG	m3_filMaTbL;			/* PnB04 : Moving average time L											*/
	LONG	m3_exDistL;				/* PnB06 : External signal positioning final travel distance L							*/
	ULONG	m3_zretSpd1L;			/* PnB08 : Return to origin approach speed L									*/
	ULONG	m3_zretSpd2L;			/* PnB0A : Return to origin creep speed L									*/
	LONG	m3_zretDistL;			/* PnB0C : Home return final travel distance L									*/
	ULONG	m3_cPrmMon1L;			/* PnB0E : Fixed monitor selection 1 L										*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_cPrmMon2L;			/* PnB10 : Fixed monitor selection 2 L										*/
	ULONG	m3_selMon1L;			/* PnB12 : SEL_MONMonitor selection 1 L									*/
	ULONG	m3_selMon2L;			/* PnB14 : SEL_MONMonitor selection 2 L									*/
	ULONG	m3_zpWidthL;			/* PnB16 : Origin detection width L												*/
	ULONG	m3_pTrqLmtL;			/* PnB18 : Forward rotation torque limit value L										*/
	ULONG	m3_nTrqLmtL;			/* PnB1A : Reverse torque limit value L										*/
	ULONG	m3_zSpdLvL;				/* PnB1C : Zero speed detection width L											*/
	ULONG	m3_vCmpLvL;				/* PnB1E : Speed match signal detection width L										*/
/*------------------------------------------------------------------------------------------------------*/
	ULONG	m3_enaCmdFeildL;		/* PnB20 : Enable/disable selection in servo command control field L			*/
	ULONG	m3_enabStatFeildL;		/* PnB22 : Enable/disable selection in servo status control field L		*/
	ULONG	m3_enaOutbitL;			/* PnB24 : I/OBit definition enable/disable selection (output side) L				*/
	ULONG	m3_enaInbitL;			/* PnB26 : I/OBit definition enable/disable selection (input side) L				*/
	ULONG	m3_Pn50A;				/* PnB28 : Positive side overtravel signal value assignment value L						*/
	ULONG	m3_Pn50B;				/* PnB29 : Negative side overtravel signal value assignment value L						*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	almmsk1;				/* PnE05 : Alarm mask 1											*/
	USHORT	almmsk2;				/* PnE06 : Alarm mask 2											*/
	USHORT 	opdetsw;				/* PnE09 : Option card/safety function compatible restriction switch			*//* <S04D> */
	USHORT	syssw1;					/* PnE0B : System switch 1										*/
	USHORT	syssw3;					/* PnE0D : System switch 3										*/
	USHORT	comfil;					/* PnE0E : Command filter setting when selecting command input filter					*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	paswrd;					/* PnE10 : Parameter password										*/
	DBYTEX	styp;					/* PnE11 : Lower:Servopack type code, Upper level:Input voltage code			*/
	USHORT	ampw;					/* PnE12 : Servopack capacity											*/
	USHORT	yopt;					/* PnE13 : Y specification number												*/
	USHORT	irats;					/* PnE14 : Servopack rated current										*/
	USHORT	imaxs;					/* PnE15 : Servopack maximum current										*/
	DBYTEX	ibs_md;					/* PnE17 : Lower:Overload detection base current, Upper level:Overload detection intermediate current		*/
	DBYTEX	imdt_mxt;				/* PnE18 : Lower:Overload detection intermediate current time, Upper level:Overload detection maximum current time	*/
	USHORT	olwrat;					/* PnE19 : OL warning ratio												*/
	USHORT	dbr;					/* PnE1B : DB resistance value												*/
	USHORT	wdb;					/* PnE1D : DB resistance capacity(Default)									*/
	DBYTEX	spgfail;				/* PnE1E : Lower:S-PGPermissible number of reception failures, Upper level:FS-PGPermissible number of reception failures	*/
	DBYTEX	PnE1F;					/* PnE1F : Lower:Allowable number of power-on cycles for inrush resistance overload, Upper level:(Preliminary)		*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	ignd;					/* PnE20 : Current loop gain(D)										*/
	USHORT	ignq;					/* PnE21 : Current loop gain(Q)										*/
	USHORT	kid;					/* PnE22 : Current loop integral time constant(D)									*/
	USHORT	kiq;					/* PnE23 : Current loop integral time constant(Q)									*/
	USHORT	limintd;				/* PnE24 : Current loop integral limit value(D)									*/
	USHORT	limintq;				/* PnE25 : Current loop integral limit value(Q)									*/
	USHORT	ifil;					/* PnE26 : Current loop filter time constant									*/
	USHORT	shant;					/* PnE28 : Current detection gain											*/
	USHORT	vrevgn;					/* PnE2A : Voltage compensation gain											*/
	USHORT	lmdata;					/* PnE2B : Voltage limit value												*/
	USHORT	pwmf;					/* PnE2C : PWM frequency												*/
	DBYTEX	odt_c;					/* PnE2D : Lower:On-delay time, Upper level:On-delay compensation constant			*/
	DBYTEX	odlv_slp;				/* PnE2E : Lower:On-delay compensation change level, Upper level:Compensation slope			*/
	USHORT	dq_sw;					/* PnE2F : DQ switch											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	phscmp;					/* PnE34 : Phase compensation													*/
	USHORT	kdd;					/* PnE35 : Current loop differential time(D)									*/
	USHORT	kdq;					/* PnE36 : Current loop differential time(Q)									*/
	USHORT	dfild;					/* PnE37 : Current loop differential filter time constant(D)							*/
	USHORT	dfilq;					/* PnE38 : Current loop differential filter time constant(Q)							*/
	USHORT	dq_sw2;					/* PnE39 : Current control switch 2										*/
	USHORT	motresq;				/* PnE3B : Notch filter for motor resonance suppressionQvalue						*/
	USHORT	PnE3C;					/* PnE3C : Nside switchONMinimum time									*/
	USHORT	PnE3D;					/* PnE3D : Sample & hold time									*/
	USHORT	fltrange;				/* PnE3F : Motor overcurrent detection filter count								*/ /* <S1F5> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	tplsfil;				/* PnE40 : DRVInternal signal abnormality detection filter number of times							*//* <S1F6> */
//	USHORT	mnrng0;					/* PnE42 : Monitor 1 gain											*//* <S016> *//* <S0DF> */
//	USHORT	mnrng1;					/* PnE43 : Monitor 2 gain											*//* <S016> *//* <S0DF> */
	DBYTEX	ipg_rwt;				/* PnE44 : Lower:i-PControl ratio, Upper level:Runaway detection torque					*/
	USHORT	PnE45;					/* PnE45 : Low speed OL: Base current[%], OL characteristic inflection point[Electrical angle10r/min]	*/
	USHORT	PnE46;					/* PnE46 : Low speed OL: Intermediate current[10%], Maximum current[10%]					*/
	USHORT	PnE47;					/* PnE47 : Low speed OL: Intermediate current time[0.1s], Maximum current time[0.1s]			*/
//	ULONG /*USHORT*/	mnptr0l;	/* PnE48 : <S074> Monitor 1 address									*//* <S0DF> */
//	ULONG /*USHORT*/	mnptr1l;	/* PnE4A : <S074> Monitor 2 address									*//* <S0DF> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	zadjiu;					/* PnE50 : Current detection zero adjustment(UPhase)										*/
	USHORT	zadjiv;					/* PnE51 : Current detection zero adjustment(VPhase)										*/
	DBYTEX	gadjiu_v;				/* PnE52 : Current detection gain adjustment(U,VPhase)									*/
//	USHORT	zadjvl;					/* PnE53 : Speed command zero adjustment											*/	/* <S0A9> Data type change */
	LONG	zadjvl;					/* PnE53 : Speed command zero adjustment											*/	/* <S0A9> Data type change */
	USHORT	zadjvh;					/* PnE54 : Reserved constants(Do not handle)								*/
	USHORT	gadjv;					/* PnE55 : Speed command gain adjustment											*/
	DBYTEX	zadjtp_n;				/* PnE56 : Torque command zero adjustment(Positive input/Negative input)							*/
	DBYTEX	gadjtp_n;				/* PnE57 : Torque command gain adjustment(Positive input/Negative input)						*/
/*------------------------------------------------------------------------------------------------------*/
	DBYTEX	ratbt_mt;				/* PnE60 : Lower:Overload detection base current, Upper level:Overload detection intermediate current		*/
	DBYTEX	ratmtt_2;				/* PnE61 : Lower:Overload detection intermediate time, Upper level:Overload detection intermediate current 2		*/
	DBYTEX	ratmt2_os;				/* PnE62 : Lower:Overload detection intermediate time 2, Upper level:OS detection level			*/
	USHORT	PnE63;					/* PnE63 : Alarm trace function selection switch							*/
	USHORT	PnE64;					/* PnE64 : Alarm trace function sampling interval						*/
	USHORT	PnE65;					/* PnE65 : Alarm trace function trace 1 target selection					*/
	USHORT	PnE66;					/* PnE66 : Alarm trace function trace 2 target selection					*/
	USHORT	PnE67;					/* PnE67 : Alarm trace function trace 3 target selection					*/
	USHORT	PnE68;					/* PnE68 : Alarm trace function trace 1 real address specification				*/
	USHORT	PnE6A;					/* PnE6A : Alarm trace function trace 2 real address specification				*/
	USHORT	PnE6C;					/* PnE6C : Alarm trace function trace 3 real address specification				*/
	/* <S192> Start */
/*------------------------------------------------------------------------------------------------------*/
	USHORT DBLifeTime;				/* PnE83 :  DBCircuit life							*/
	ULONG DBSwichCount;				/* PnE86 : 	CurrentDBNumber of operations				*//* <S192> */
	/* <S192> End */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	scnvorgoffset;			/* PnE8C : Serial conversion unit origin offset						*/
	USHORT	phsofst;				/* PnE8D : Current zero phase position											*/
/*------------------------------------------------------------------------------------------------------*/
	DBYTEX	olspdlvl;				/* PnE90 : Low speed overload detection speed level									*/
	DBYTEX	ollspdprm;				/* PnE91 : Low speed overload detection parameter									*/
	USHORT	dbol_warn;				/* PnE93 : DB overload detection power 1										*/
	USHORT	dbol_alm;				/* PnE94 : DB overload detection power 2										*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	ifil2;					/* PnEA4 : Quiet mode current loop filter time constant						*/
	USHORT	pwmf2;					/* PnEA5 : Quiet modePWMFrequency										*/
	USHORT	ilimlvl;				/* PnEA6 : Carrier frequency switching current threshold							*/
	USHORT	pwmf3;					/* PnEA7 : No.2PWMFrequency												*/
	USHORT	fbfil2;					/* PnEA8 : Second velocity F/B filter time constant								*/
	USHORT	kv;						/* PnEB0 : Voltage feedback loop proportional gain						*/
	USHORT	tv;						/* PnEB1 : Voltage feedback integration time constant								*/
	USHORT	idlim;					/* PnEB2 : Voltage feedback integral limit							*/
	USHORT	v1max;					/* PnEB3 : Voltage command limit level										*/
	USHORT	vmaxid;					/* PnEB4 : dVoltage limit level for axis current command limit						*/
	USHORT	vpnavg;					/* PnEB5 : Main circuit voltage average time										*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	PnED0;					/* PnED0 : Auto notch completion status										*/
	ULONG	PnED1;					/* PnED1 : Automatic notch maximum vibration value										*/
	USHORT	PnED3;					/* PnED3 : Notch frequency 1st correction coefficient 1								*/
	USHORT	PnED4;					/* PnED4 : Notch frequency 1st correction coefficient 2								*/
	USHORT	PnED5;					/* PnED5 : Notch frequency 1st correction range threshold							*/
	USHORT	PnED6;					/* PnED6 : Notch frequency 2nd correction coefficient 1								*/
	USHORT	PnED7;					/* PnED7 : Notch frequency 2nd correction coefficient 2								*/
	USHORT	PnED8;					/* PnED8 : Notch frequency 2nd correction range threshold							*/
	USHORT	PnED9;					/* PnED9 : Stop determination waiting time during automatic notch vibration check				*/
	USHORT	TunLessGain;			/* PnEDA : Adjustment less disturbance observer gain									*/
	USHORT	TunLessStiff;			/* PnEDB : Adjustment lessor correction gain										*/
	USHORT	TunLessTrqLpf;			/* PnEDC : Adjustment less disturbance torque filter time constant								*/
	USHORT	TunLessVfbLpf;			/* PnEDD : Adjustment less speedFBLoop filter time constant								*/
/*------------------------------------------------------------------------------------------------------*/
	DBYTEX	MLess_vrat_max;			/* PnEE0 : Motorless test function Virtual motor speed						*//* <S1B7> */
	DBYTEX	MLess_oslv_pol;			/* PnEE1 : Motorless test function Virtual motor OS detection level, Number of poles	*//* <S1B7> */
	USHORT	Mless_jmot;				/* PnEE2 : Motorless test function virtual motor rotor inertia			*//* <S1B7> */
	DBYTEX	Mless_exp_spd_jmot;		/* PnEE3 : Motorless test function virtual motor index parameter				*//* <S1B7> */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	dq_sw_L;				/* PnEE8 : DQ switch(Linear)									*/
	USHORT	v1max_L;				/* PnEE9 : Voltage command limit level(Linear)								*/
	USHORT	TunLessGain_L;			/* PnEEA : Adjustment less disturbance observer gain(Linear)							*/
	USHORT	TunLessStiff_L;			/* PnEEB : Adjustment lessor correction gain(Linear)								*/
/*------------------------------------------------------------------------------------------------------*/
	MENCPRM	MencP;					/* PnF00 - PnF1E : Encoder parameters */
/*------------------------------------------------------------------------------------------------------*/
	USHORT	RippleCompGainE;		/* PnFA0 : Speed ripple compensation gain(Electrical angle)							*/	/* <S14B> */
	USHORT	RippleCompCycleE[6];	/* PnFA1, PnFA4, …, PnFB0 : Speed ripple compensation component period(Electrical angle)		*/	/* <S14B> */
	SHORT	RippleCompAmplitudeE[6];/* PnFA2, PnFA5, …, PnFB1 : Speed ripple compensation component amplitude(Electrical angle)		*/	/* <S14B> */
	USHORT	RippleCompPhaseE[6];	/* PnFA3, PnFA6, …, PnFB2 : Speed ripple compensation component phase(Electrical angle)		*/	/* <S14B> */
/*------------------------------------------------------------------------------------------------------*/

} PRMDATA;



#endif //__PN_PRM_STRUCT_H

