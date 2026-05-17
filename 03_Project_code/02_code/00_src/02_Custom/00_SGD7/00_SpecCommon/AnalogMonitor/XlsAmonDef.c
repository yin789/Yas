/****************************************************************************************************/
/*																									*/
/*																									*/
/*		XlsAmonDef.c : Analog monitor definition source file												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Analog monitor definition table																*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		2013.10.03 : Made by Excel Macro															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "XlsAmonDef.h"
#include "AnalogMonitorCalc.h"
#include "DataTraceCalc.h"
#include "MercuryGlobals.h"





/****************************************************************************************************/
/*																									*/
/*		Analog monitor definition table																	*/
/*																									*/
/****************************************************************************************************/
const	AMONDEF	AmonDefTbl[] = {
/*   Sel  Type  VarFuncPtr                      GainFuncPtr											*/
/*--------------------------------------------------------------------------------------------------*/
  { 0x00, 0x00, GetVarData_AmonMotSpd,			AmonGcalMotorSpeed	},	/* Motor rotation speed			*/
  { 0x01, 0x00, GetVarData_AmonSpdRef,			AmonGcalMotorSpeed	},	/* Speed command					*/
  { 0x02, 0x00, GetVarData_AmonTrqRef,			AmonGcalTorqueRef	},	/* Torque command				*/
  { 0x03, 0x00, GetVarData_AmonPosErra,			AmonGcalPulseError	},	/* Position deviation					*/
  { 0x04, 0x00, GetVarData_AmonPosErrx,			AmonGcalPulseError	},	/* Position amplifier deviation			*/
  { 0x05, 0x00, GetVarData_AmondPosCmdSmp,		AmonGcalPosCmdaSpd	},	/* Position command speed				*/
  { 0x06, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation(Unusable)			*/
  { 0x07, 0x00, GetVarData_sfperra_per32s,		AmonGcalPulseErr01	},	/* Position deviation between motor and load	*/
  { 0x08, 0x02, (LONG(*)(void*))GetBitData_Coin,AmonGcalBitSignal	},	/* Positioning completion command			*/
  { 0x09, 0x00, GetVarData_SpdFFCx,				AmonGcalMotorSpeed	},	/* Speed FF					*/
  { 0x0A, 0x00, GetVarData_TrqFFCx,				AmonGcalTorqueRef	},	/* Torque FF				*/
  { 0x0B, 0x00, GetVarData_AmonActGain,			AmonGcalActGain		},	/* Valid gain				*/
  { 0x0C, 0x02, (LONG(*)(void*))GetBitData_Den,	AmonGcalBitSignal	},	/* Position command payout completion command	*/
  { 0x0D, 0x00, GetVarData_AmonFencSpdsmp,		AmonGcalMotorSpeed	},	/* External encoder speed		*/
  { 0x0E, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x0F, 0xF0, GetVarData_Dummy,				AmonGcalDummy		},	/* Specified variable(PnE40,PnE42)	*/
  { 0x40, 0x00, GetVarData_dPosRefi,			AmonGcalMotPfbkSpd	},	/* Command speed before position loop		*/
  { 0x41, 0x00, GetVarData_AmonDisTrq,			AmonGcalTorqueRef	},	/* Friction compensation torque			*/
  { 0x42, 0x00, GetVarData_MonResVib,			AmonGcalMotorSpeed	},	/* Vibration suppression monitor				*/
  { 0x43, 0x00, GetVarData_TrqRefBeforeComp,	AmonGcalTorqueRef	},	/* Torque command before ripple compensation	*/
  { 0x44, 0x00, GetVarData_AmonIdRef,			AmonGcalTrqRef15K	},	/* dAxis current command				*/
  { 0x45, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x46, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x47, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x48, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x49, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4A, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4B, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4C, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4D, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4E, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
  { 0x4F, 0x00, GetVarData_Dummy,				AmonGcalDummy		},	/* Reservation						*/
};
/*--------------------------------------------------------------------------------------------------*/
const	USHORT	AmonDefTblEntNum = sizeof(AmonDefTbl)/sizeof(AmonDefTbl[0]);
/*--------------------------------------------------------------------------------------------------*/







/***************************************** end of file **********************************************/
