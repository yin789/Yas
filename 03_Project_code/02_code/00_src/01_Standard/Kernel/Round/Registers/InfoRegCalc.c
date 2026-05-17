/****************************************************************************
  Description   : Operation Register Table
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corp., Copyright (c) 2010
                  All Rights Reserved

  Project       : INGRAM

  Create Date   : 2010-10-20

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "PrmAccessIf.h"
#include "SpecCommon.h"		/* <S0C9> */

/* <S0C9>	Start */
/****************************************************************************
0x0000	Servo modelID
****************************************************************************/
PRM_RSLT EInfReadServoID(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = SERVO_ID;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0001	Servo parameter setID(bit0~bit8)
****************************************************************************/
PRM_RSLT EInfReadServoPrmVer(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG ServoPrmVer;

	ServoPrmVer = (LONG)(ServoPrmVer | ((PRM_FORMAT_VERSION << 8)|(PRM_DATA_VERSION)));
	*pValue = ServoPrmVer;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0005	Control module firmware version(YSpecificationVer, YSpecificationVerRevision number)
0x0006	Control module firmware version(Firmware baseVer)
****************************************************************************/
PRM_RSLT EInfReadFirmVersion(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG ServoFirmVer;

//	ServoFirmVer = (LONG)SERVO_FIRM_VER>>16;	/* <S0D1> */
	ServoFirmVer = (LONG)(( (SERVO_FIRM_VER>>16)<<16 ) | SERVO_SPECIAL_SPEC_VER);	/* <S0D1> */
	*pValue = ServoFirmVer;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0007	Control moduleYSpecification number
****************************************************************************/
PRM_RSLT EInfReadYspecVer(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG YspecVer;

	YspecVer = (LONG)SERVO_SPECIAL_SPEC_VER;

	*pValue = YspecVer;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x000B	Servopack input voltage/output voltage
****************************************************************************/
PRM_RSLT EInfReadServoVoltage(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG ServoInVcode;
	LONG ServoOutVcode;
	LONG ServoVcode;

	ServoInVcode = (LONG)(Axis->UniPrms->Bprm->AmpInVcode); 
	ServoOutVcode = (LONG)(Axis->UniPrms->Bprm->AmpOutVcode); 

	ServoVcode = (ServoOutVcode << 8) | ServoInVcode;

	*pValue = ServoVcode;


	return PRM_RSLT_SUCCESS;
}

/* <S0C9>	End */

/****************************************************************************
0x000C	MotorID
****************************************************************************/
PRM_RSLT EInfReadMotorID(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.typm_e.b.l;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x000D	Encoder type/resolution
****************************************************************************/
PRM_RSLT EInfReadEncoderTypeAndBits(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Prm)->MencP.typm_e.b.h;
	x = (x & 0xF0) ? 0x0100L : 0x0000L;
	x |= (LONG)(Axis->UniPrms->Prm)->MencP.bit_dp.b.l;
	*pValue = x;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x000E	Encoder firmware version
****************************************************************************/
PRM_RSLT EInfReadEncoderVer(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.encver;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x000F	Motor capacity[W]
****************************************************************************/
PRM_RSLT EInfReadMotorWattage(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Bprm)->MotW;
	x = MlibScalKskxkx( x, 1, 1, NULL, -30 );

	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0011	Motor voltage[V]
****************************************************************************/
PRM_RSLT EInfReadMotorVoltage(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Bprm)->MotVcode;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0012	Absolute value encoder type
****************************************************************************/
PRM_RSLT EInfReadEncoderType(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Prm)->MencP.typm_e.b.h;
	*pValue = ((x >> 4) & 0x0F) - 1;		/* 2,1 --> 1,0					*/
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x001C	Motor rated speed [Rotary type:min-1,Linear type:mm/s]
****************************************************************************/
PRM_RSLT EInfReadMotorRatedSpeed(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.vrat_max.b.l;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x001D	Motor maximum speed [Rotary type:min-1,Linear type:mm/s]
****************************************************************************/
PRM_RSLT EInfReadMotorMaxSpeed(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.vrat_max.b.h;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x001E	Motor speed index [10]
****************************************************************************/
PRM_RSLT EInfReadMotorSpeedExp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.exp_motw_spd.b.h + 2;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x001F	Motor rated torque [Rotary type:N*m, Linear type:N]
****************************************************************************/
PRM_RSLT EInfReadMotorRatedTorque(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	i, unit;
	LONG	x,y,z;
	LONG	kx,sx;
	KSGAIN	MaxTrq, ExpTrq;
	LONG	PerMaxTrq;
	LONG	rattrq, maxtrq;

	MaxTrq = (Axis->UniPrms->Bprm)->MaxTrq;
	PerMaxTrq = (LONG)(Axis->UniPrms->Bprm)->PerMaxTrq;
	ExpTrq = (Axis->UniPrms->Bprm)->ExpTrq;
	rattrq = (LONG)(Axis->UniPrms->Prm)->MencP.rattrq;
	maxtrq = (LONG)(Axis->UniPrms->Prm)->MencP.maxtrq;

	x = MlibScalKskxkx( MaxTrq, 10000, PerMaxTrq, NULL, -30 );	/* 100*Rated torque	*/
	y = MlibScalKskxkx( MaxTrq, 100,   1,         NULL, -30 );	/* 100*Maximum torque	*/
	unit = ( Axis->MencV->AxisMotType == MOTTYPE_LINEAR ) ? 1 : 100;
	kx = MlibScalKxgain( rattrq, maxtrq, unit, &sx,  -1 );
	z = MlibScalKskskx( kx, ExpTrq, 1, NULL, -30 );				/* 100*Maximum torque	*/
	for( i=-2; y >= 10000; i++ ){ x /= 10; y /= 10; z /= 10; }	/* Scaling		*/

	*pValue = x;						/* Rated torque [Nm,N]						*/
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0020	Motor maximum torque [Rotary type:N*m, Linear type:N]
****************************************************************************/
PRM_RSLT EInfReadMotorMaxTorque(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	i,unit;
	LONG	x,y,z;
	LONG	kx,sx;
	KSGAIN	MaxTrq, ExpTrq;
	LONG	PerMaxTrq;
	LONG	rattrq, max_trq;

	MaxTrq = (Axis->UniPrms->Bprm)->MaxTrq;
	PerMaxTrq = (LONG)(Axis->UniPrms->Bprm)->PerMaxTrq;
	ExpTrq = (Axis->UniPrms->Bprm)->ExpTrq;
	rattrq = (LONG)(Axis->UniPrms->Prm)->MencP.rattrq;
	max_trq = (LONG)(Axis->UniPrms->Prm)->MencP.maxtrq;

	x = MlibScalKskxkx( MaxTrq, 10000, PerMaxTrq, NULL, -30 );	/* 100*Rated torque	*/
	y = MlibScalKskxkx( MaxTrq, 100,   1,         NULL, -30 );	/* 100*Maximum torque	*/
	unit = ( Axis->MencV->AxisMotType == MOTTYPE_LINEAR ) ? 1 : 100;
	kx = MlibScalKxgain( rattrq, max_trq, unit, &sx,  -1 );
	z = MlibScalKskskx( kx, ExpTrq, 1, NULL, -30 );				/* 100*Maximum torque	*/
	for( i=-2; y >= 10000; i++ ){ x /= 10; y /= 10; z /= 10; }	/* Scaling		*/

	*pValue = z;						/* Maximum torque [Nm,N]						*/
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0021	Motor torque index[10]
****************************************************************************/
PRM_RSLT EInfReadMotorTorqueExp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	i, unit;
	LONG	x,y,z;
	LONG	kx,sx;
	KSGAIN	MaxTrq, ExpTrq;
	LONG	PerMaxTrq;
	LONG	rattrq, max_trq;

	MaxTrq = (Axis->UniPrms->Bprm)->MaxTrq;
	PerMaxTrq = (LONG)(Axis->UniPrms->Bprm)->PerMaxTrq;
	ExpTrq = (Axis->UniPrms->Bprm)->ExpTrq;
	rattrq = (LONG)(Axis->UniPrms->Prm)->MencP.rattrq;
	max_trq = (LONG)(Axis->UniPrms->Prm)->MencP.maxtrq;

	x = MlibScalKskxkx( MaxTrq, 10000, PerMaxTrq, NULL, -30 );	/* 100*Rated torque	*/
	y = MlibScalKskxkx( MaxTrq, 100,   1,         NULL, -30 );	/* 100*Maximum torque	*/
	unit = ( Axis->MencV->AxisMotType == MOTTYPE_LINEAR ) ? 1 : 100;
	kx = MlibScalKxgain( rattrq, max_trq, unit, &sx,  -1 );
	z = MlibScalKskskx( kx, ExpTrq, 1, NULL, -30 );				/* 100*Maximum torque	*/
	for( i=-2; y >= 10000; i++ ){ x /= 10; y /= 10; z /= 10; }	/* Scaling		*/

	*pValue = i;						/* Torque index */
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0022	Encoder resolution[Rotary type:pulse/rev,Linear Fulk type:pulse/pitch]
****************************************************************************/
PRM_RSLT EInfReadEncoderResolution(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Bprm)->XfbPulse2;
	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0024	Multiturn limit value
****************************************************************************/
PRM_RSLT EInfReadMultiTurnLimit(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)((Axis->UniPrms->Prm)->MencP.limmlt);
	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0026	Maximum output torque[Rotary type:N*m, Linear type:N]
****************************************************************************/
PRM_RSLT EInfReadMaxTorqueRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	i, unit;
	LONG	x,y,z;
	LONG	kx,sx;
	KSGAIN	MaxTrq, ExpTrq;
	LONG	PerMaxTrq;
	LONG	rattrq, max_trq;

	MaxTrq = (Axis->UniPrms->Bprm)->MaxTrq;
	PerMaxTrq = (LONG)(Axis->UniPrms->Bprm)->PerMaxTrq;
	ExpTrq = (Axis->UniPrms->Bprm)->ExpTrq;
	rattrq = (LONG)(Axis->UniPrms->Prm)->MencP.rattrq;
	max_trq = (LONG)(Axis->UniPrms->Prm)->MencP.maxtrq;

	x = MlibScalKskxkx( MaxTrq, 10000, PerMaxTrq, NULL, -30 );	/* 100*Rated torque	*/
	y = MlibScalKskxkx( MaxTrq, 100,   1,         NULL, -30 );	/* 100*Maximum torque	*/
	unit = ( Axis->MencV->AxisMotType == MOTTYPE_LINEAR ) ? 1 : 100;
	kx = MlibScalKxgain( rattrq, max_trq, unit, &sx,  -1 );
	z = MlibScalKskskx( kx, ExpTrq, 1, NULL, -30 );				/* 100*Maximum torque	*/
	for( i=-2; y >= 10000; i++ ){ x /= 10; y /= 10; z /= 10; }	/* Scaling		*/

	*pValue = (y < z) ? y : z;	/* Maximum output torque[Nm,N]*/
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0027	Maximum operable motor speed [Rotary type:min-1,Linear type:mm/s]
****************************************************************************/
PRM_RSLT EInfReadMaxSpeedRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Bprm)->MaxVel;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0028	Fulque scale type
****************************************************************************/
PRM_RSLT EinfReadFullcScaleType(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Bprm)->FencUse & Axis->FencV->AbsoEncoder;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0029	Absolute value linear scale origin position setting
****************************************************************************/
PRM_RSLT EinfReadAbsScaleZeroSetting(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	MENCV	*MencV;

	MencV = ((Axis->UniPrms->Bprm)->FencUse != FALSE)
			? Axis->FencV: Axis->MencV;

	if((MencV->AbsoEncType != MENC_INCRE_TYPE)
		&& (MencV->SerialConvType == SCONVTYPE_MITUTOYO_ABS))
	{
		*pValue = 1;
	}
	else
	{
		*pValue = 0;
	}
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x002A	Linear scale pitch[pm]
****************************************************************************/
PRM_RSLT EInfReadLinearScalePitch(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Bprm)->MonScalePitch;
	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x002C	Linear scale pitch index[10]
****************************************************************************/
PRM_RSLT EInfReadLinearScalePitchExp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Bprm)->MonScaleExp;
	return PRM_RSLT_SUCCESS;
}

/* <S0C9>	Start */
/****************************************************************************
0x0030	Servopack firmware boot version
****************************************************************************/
PRM_RSLT EInfReadServoFirmwareBootVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG ServoFirmwareBootVer;
	*pValue = ServoFirmwareBootVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0040	Command option card modelID
****************************************************************************/
PRM_RSLT EInfReadCmdCardID(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdCardID;
	*pValue = CmdCardID;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0041	Directive option card firmware version
****************************************************************************/
PRM_RSLT EInfReadCmdCardFirmwareVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdCardFirmwareVer;
	*pValue = CmdCardFirmwareVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0043	Directive option cardYSpecification number
****************************************************************************/
PRM_RSLT EInfReadCmdCardYspecVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdCardYspecVer;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0044	Safety module modelID
****************************************************************************/
PRM_RSLT EInfReadSafetyID(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyID;
	*pValue = SafetyID;
#endif

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0045	Safety module firmware version
****************************************************************************/
PRM_RSLT EInfReadSafetyFirmwareVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyFirmwareVer;
	*pValue = SafetyFirmwareVer;
#endif

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0047	Safety moduleYSpecification number
****************************************************************************/
PRM_RSLT EInfReadSafetyYspecVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyYspecVer;
	*pValue = SafetyYspecVer;
#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0048	Feedback option card modelID
****************************************************************************/
PRM_RSLT EInfReadFbID(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbID;
	*pValue = FbID;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0049	Feedback option card firmware version
****************************************************************************/
PRM_RSLT EInfReadFbFirmwareVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbFirmwareVer;
	*pValue = FbFirmwareVer;
#endif
	return PRM_RSLT_SUCCESS;
}
/****************************************************************************
0x004B	feedback option cardYSpecification number
****************************************************************************/
PRM_RSLT EInfReadFbYspecVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbYspecVer;
	*pValue = FbYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x004C	Motor in feedback optionID
****************************************************************************/
PRM_RSLT EInfReadFbMotorID(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorID;
	*pValue = FbMotorID;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x004D	Motor capacity in feedback option[W]
****************************************************************************/
PRM_RSLT EInfReadFbMotorWattage(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorWattage;
	*pValue = FbMotorWattage;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x004F	Motor voltage in feedback options[V]
****************************************************************************/
PRM_RSLT EInfReadFbMotorVoltage(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorVoltage;
	*pValue = FbMotorVoltage;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0050	Encoder type/resolution in the feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderTypeAndBits(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderTypeAndBits;
	*pValue = FbEncoderTypeAndBits;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0051	Encoder resolution in feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderBits(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderBits;
	*pValue = FbEncoderBits;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0053	Absolute value encoder type in feedback options
****************************************************************************/
PRM_RSLT EInfReadFbAbsEncoderType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
	Long FbAbsEncoderType;
	*pValue = FbAbsEncoderType;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0054	Encoder firmware version in the feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderVer(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderVer;
	*pValue = FbEncoderVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/* <S0C9>	End */

/****************************************************************************
0x0800	Control module format string
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadModuleIDProductName(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->ModuleID->IfInfo->ProductName[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->ModuleID->IfInfo->ProductName[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}
/****************************************************************************
0x0808	Control module serial number string
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadModuleIDSerialNumber(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->ModuleID->IfInfo->SerialNumber[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->ModuleID->IfInfo->SerialNumber[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0810	Control module manufacturing date
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadModuleIDDate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)((Axis->ModuleID->IfInfo->Data & 0x00FF)<<8);
	usDate |= (USHORT)((Axis->ModuleID->IfInfo->Data & 0xFF00)>>8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0811	Motor type string
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadMotorProductName(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->MencV->MotInfo.Model[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->MencV->MotInfo.Model[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}
/****************************************************************************
0x081C	Motor serial number string
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadMotorSerialNumber(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->MencV->MotInfo.SNo[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->MencV->MotInfo.SNo[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x0824	Motor manufacturing date
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadMotorManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	UCHAR	*px;
	USHORT	usDate;

	usDate = 0;
	px = Axis->MencV->MotInfo.Date;
	if(px[0] != '\0')
	{
		usDate = (((px[0]-'0')*10 + (px[1]-'0'))<<8)
						+ ((px[3]-'0')*10 + (px[4]-'0'));
	}
	*pValue = (LONG)usDate;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0825	Encoder format area
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadEncoderProductName(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->MencV->EncInfo.Model[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->MencV->EncInfo.Model[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x082D	Encoder serialNoArea
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadEncoderSerialNumber(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	usDate;

	usDate = (USHORT)(Axis->MencV->EncInfo.SNo[ArrayIdx*2 + 1] & 0x00FF);
	usDate |= (USHORT)((Axis->MencV->EncInfo.SNo[ArrayIdx*2] & 0x00FF) << 8);
	
	*pValue = (LONG)usDate;
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0835	Encoder manufacturing date area
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadEncoderManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	UCHAR	*px;
	USHORT	usDate;

	usDate = 0;
	px = Axis->MencV->EncInfo.Date;
	if(px[0] != '\0')
	{
		usDate = (((px[0]-'0')*10 + (px[1]-'0'))<<8)
						+ ((px[3]-'0')*10 + (px[4]-'0'));
	}
	*pValue = (LONG)usDate;
	return PRM_RSLT_SUCCESS;
}

/* <S171> Start */
/****************************************************************************
0x0845	Number of control axes
****************************************************************************//* <S0D6> */
PRM_RSLT EinfReadNumberOfAxes(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = NumberOfAxes;
	return PRM_RSLT_SUCCESS;
}
/* <S171> End */

/* <S0C9>	Start */
/****************************************************************************
0x0864	Command option card format string
****************************************************************************/
PRM_RSLT EInfReadCmdType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdType;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0870	Directive option card serialNoArea
****************************************************************************/
PRM_RSLT EInfReadCmdSerialNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdSerialNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x087C	Command option card manufacturing date area
****************************************************************************/
PRM_RSLT EInfReadCmdManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG CmdManufactureDate;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x087D	Safety module format string
****************************************************************************/
PRM_RSLT EInfReadSafetyType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyType;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0889	Safety module serial number string
****************************************************************************/
PRM_RSLT EInfReadSafetySerialNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetySerialNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0895	Safety module manufacturing date
****************************************************************************/
PRM_RSLT EInfReadSafetyManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyManufactureDate;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0896	Feedback option card format string
****************************************************************************/
PRM_RSLT EInfReadFbType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbType;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08A2	Feedback option card serial number string
****************************************************************************/
PRM_RSLT EInfReadFbSerialNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbSerialNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08AE	Feedback option card manufacturing date
****************************************************************************/
PRM_RSLT EInfReadFbManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbManufactureDate;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08AF	Motor type string in feedback option
****************************************************************************/
PRM_RSLT EInfReadFbMotorType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorType;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08BA	Motor serial number string in feedback option
****************************************************************************/
PRM_RSLT EInfReadFbMotorSerialNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorSerialNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08C2	Motor manufacturing date in feedback options
****************************************************************************/
PRM_RSLT EInfReadFbMotorManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbMotorManufactureDate;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08C3	Encoder format string in feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderType(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderType;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08CB	Encoder serial number string in the feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderSerialNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderSerialNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08D3	Encoder manufacturing date in feedback option
****************************************************************************/
PRM_RSLT EInfReadFbEncoderManufactureDate(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG FbEncoderManufactureDate;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08E0	Input signal Enabled/Invalid information
****************************************************************************/
PRM_RSLT EInfReadInputSignalInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	*pValue = INPUT_SIGNAL_INFO;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08E2	Output signal Enabled/Invalid information
****************************************************************************/
PRM_RSLT EInfReadOutputSignalInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{

	*pValue = OUTPUT_SIGNAL_INFO;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08E4	SafetyI/O signal Enabled/Invalid information
****************************************************************************/
PRM_RSLT EInfReadSafetyIOSignalInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG SafetyIOSignalInfo;
	*pValue = CmdCardYspecVer;
#endif
/* <S174> Start */
	PRMDATA		*Prm;
	Prm = ((AXIS_HANDLE*)Axis)->UniPrms->Prm;

	if( (Prm->opdetsw & 0x0100) == 0 )	/* PnE09 bit8 = 0:HWBBFunction Not supported */
	{
			*pValue = 0;				/* /HBWW1, /HBWW2 : Unsupported(Disabled) */
	}
	else	/* PnE09 bit8 = 1:HWBBFunction Correspondence */
	{
			*pValue = 0x00000003;	/* /HBWW1, /HBWW2 : Support(Enabled) */
	}
/* <S174> End */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x08F0	ServopackBTONumber
****************************************************************************/
PRM_RSLT EInfReadServoBtoNo(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
#if 0
/* TODO */
	LONG ServoBtoNo;
	*pValue = CmdCardYspecVer;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x0910	Servopack special specification type
****************************************************************************/
PRM_RSLT EInfReadServoSpecialSpec(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	LONG ServoSpecialSpec;

	/*-------------------------------*/
	/* Special type type		Contents		 */
	/*-------------------------------*/
	/* 		0x00		Standard specifications	 */
	/* 		0x01		FTSpecification		 */
	/* 		0x02		EXSpecification		 */
	/* 		0x03		YSpecification		 */
	/*-------------------------------*/

	ServoSpecialSpec = (LONG)(SERVO_SPECIAL_SPEC_VER >> 16);
	*pValue = ServoSpecialSpec;

	return PRM_RSLT_SUCCESS;

}
/* <S0C9>	End */

/******************************* End of File *******************************/
