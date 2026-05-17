/****************************************************************************************************/
/*																									*/
/*																									*/
/*		XlsTrcDef.c : Trace definition source file													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Numeric trace definition table																	*/
/*																									*/
/*		2. Bit trace definition table																*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "XlsTrcDef.h"
#include "MercuryGlobals.h"
#include "DataTraceCalc.h"

/*--------------------------------------------------------------------------------------------------*/
/*  Caution: Excel macro not supported.																	*/
/*  The following must be set.																		*/
/*  Sel					: Numerical trace selection number														*/
/*  SftL				: Left shift amount																*/
/*  SftR				: Right shift amount																*/
/*	AvrgSel				: Average:TRUE, No averaging:FALSE										*/
/*	GetVerData Function	: Function pointer(Function that returns the value of the variable to be traced)								*/
/*	GainCal Function	: Function pointer(SftL,SftR, set the following in addition to the function that returns the gain coefficient when gain conversion is required)	*/
/*--------------------------------------------------------------------------------------------------*/
/****************************************************************************************************/
/*																									*/
/*		Numeric trace definition table																	*/
/*																									*/
/****************************************************************************************************/
const	TVXDEF	TrcVarTbl[] = {
/*  Sel  SftL SftR AvrgSel	Get TraceVarData Func       Gain Calcurate Func							*/
/*--------------------------------------------------------------------------------------------------*/
  { 0x00, 0,   0,   FALSE,	NULL,						NULL                      },/* No trace specification					*/
  { 0x01, 0,  10,   FALSE,	&GetVarData_AmonTrqRef,		NULL                      },/* Torque command / Thrust command				*/
  { 0x02, 0,  10,	TRUE,	&GetVarData_MotSpdSmp,		&LpxPcalMotSpeedCalGain2  },/* Motor speed						*/
  { 0x03, 0,  10,	FALSE,	&GetVarData_AmonSpdRef,		NULL	                  },/* Speed command							*/
  { 0x04, 0,  10, 	TRUE,	&GetVarData_dPosCmdSmp,		&LpxPcalPcmdSpeedCalGain  },/* Position command speed						*/
  { 0x05, 0,   0, 	FALSE,	&GetVarData_AmonPosErra,	NULL                      },/* Position deviation							*/
//{ 0x10, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Observer speed					*//* Not supported */
//{ 0x11, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Model torque command/thrust command		*//* Not supported */
//{ 0x12, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Model position deviation					*//* Not supported */
  { 0x13, 0,  10,   FALSE,	&GetVarData_AmonDisTrq,		NULL					  },/* Friction compensation torque/thrust				*//* Undisclosed */
  { 0x14, 0,  10,   FALSE,	&GetVarData_MonResVib,		NULL					  },/* Damping differential speed						*//* Undisclosed */
  { 0x15, 0,  10,   FALSE,	&GetVarData_TrqGravity,		NULL					  },/* Gravity compensated torque command/thrust command	*/	/*<S0CD>*/	/* Not supported⇒Correspondence */
  { 0x16, 0,   0,   FALSE,	&GetVarData_AmonPosErrx,	NULL	                  },/* Position amplifier deviation					*/
  { 0x17, 0,   0,   FALSE,	&GetVarData_sfperra_per32s, NULL				      },/* Motor load positional deviation				*/
  { 0x18, 0,  10,   FALSE,	&GetVarData_SpdFFCx,		NULL					  },/* Speed feedforward			*/
  { 0x19, 0,  10,   FALSE,	&GetVarData_TrqFFCx,		NULL					  },/* Torque/thrust feedforward	*/
//{ 0x1A, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* MFCStop time after position command					*//* Not supported */
//{ 0x1B, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Additional torque command/Thrust command			*//* Not supported */
//{ 0x1C, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL				      },/* Additional disturbance torque command/Thrust command		*//* Not supported */
//{ 0x1D, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Estimated speed							*//* Not supported */
  { 0x1E, 0,  10,   FALSE,	&GetVarData_TrefBeforeDtrq, NULL					  },/* Torque command(Before disturbance)				*//* Undisclosed */
  { 0x1F, 0,  10,   FALSE,	&GetVarData_TrcCtrlTrqRef,	NULL					  },/* Torque command - Friction compensation torque		*//* Undisclosed */
  { 0x20, 0,  10,   FALSE,	&GetVarData_TrqRefi,		NULL					  },/* Torque command(Before disturbance/before filter)	*//* Undisclosed */
  { 0x21, 0,  10,   FALSE,	&GetVarData_AmonActGain,	NULL					  },/* Valid gain						*/
  { 0x22, 0,  10,   TRUE,	&GetVarData_dPosRefi,		&LpxPcalMotSpeedCalGain   },/* Command speed before position loop				*/
  { 0x23, 0,   0,   FALSE,	&GetVarData_DcVoltx,		NULL					  },/* Main circuit bus voltage					*/
  { 0x24, 0,   0,   FALSE,	&GetVarData_UnCrntDet,		NULL					  },/* Current detection value						*/
//{ 0x25, 0,  10,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Effective load factor						*/
  { 0x26, 0,   0,   FALSE,	&GetVarData_UnCrntRef,		NULL					  },/* Cumulative load rate						*/
  { 0x27, 0,   0,   FALSE,	&GetVarData_TrcRgPower,		NULL					  },/* Regeneration load rate						*/
  { 0x28, 0,   0,   FALSE,	&GetVarData_TrcDbPower,		NULL					  },/* DBResistor power consumption					*/
  { 0x29, 0,  10,   TRUE,	&GetVarData_FencSpdSmp,		NULL					  },/* External encoder speed				*/
	//0x40and0x42outputs the same thing!0x40to Fulk!kira_110309
  { 0x40, 1,   0,   FALSE,	&GetVarData_DeltaPos,		NULL                      },/* Position difference							*/
  { 0x41, 1,   0,   FALSE,	&GetVarData_dPcmdEgear,		NULL					  },/* Command difference							*/
  { 0x42, 1,   0,   FALSE,	&GetVarData_DeltaPos,		NULL                      },/* Motor position difference					*/
  { 0x43, 1,   0,   FALSE,	&GetVarData_dFencPos,		NULL					  },/* Load position difference						*/
  { 0x44, 0,   2,   FALSE,	&GetVarData_MotPhase,		NULL					  },/* Current phase							*/
//{ 0x50, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMotor speed					*//* Not supported */
//{ 0x51, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMotor travel amount					*//* Not supported */
//{ 0x52, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMonitoring speedA						*//* Not supported */
//{ 0x53, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMonitoring speedB						*//* Not supported */
//{ 0x54, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Active mode internal command speed		*//* Not supported */
//{ 0x55, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMInternal data1					*//* Not supported */
//{ 0x56, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMInternal data2					*//* Not supported */
  { 0x60, 0,   0,   FALSE,	&GetVarData_InterSignalX_L,	NULL					  },/* Internal signal monitor(Lower)forSynqNet	*/
  { 0x61, 0,   0,   FALSE,	&GetVarData_InterSignalX_H,	NULL					  },/* Internal signal monitor(Upper level)forSynqNet	*/
  { 0x62, 0,   0,   FALSE,	&GetVarData_InternalInputSignals_L,		NULL		  },/* Internal input signal monitor(Lower)forSynqNet*/
  { 0x63, 0,   0,   FALSE,	&GetVarData_InternalInputSignals_H,		NULL		  },/* Internal input signal monitor(Upper level)forSynqNet*/
  { 0x64, 0,   0,   FALSE,	&GetVarData_InternalOutputSignals_L,	NULL		  },/* Internal output signal monitor(Lower)forSynqNet*/
  { 0x65, 0,   0,   FALSE,	&GetVarData_InternalOutputSignals_H,	NULL		  },/* Internal output signal monitor(Upper level)forSynqNet*/
  { 0x81, 0,   0,   FALSE,	&GetVarData_AmonTrqRef,		NULL                      },/* Torque command / Thrust command				*/
  { 0x82, 0,   0,	TRUE,	&GetVarData_MotSpdSmp,		&LpxPcalMotSpeedCalGain2  },/* Motor speed						*/
  { 0x83, 0,   0,   FALSE,	&GetVarData_AmonSpdRef,		NULL                      },/* Speed command							*/
  { 0x84, 0,   0,   TRUE,	&GetVarData_dPosCmdSmp,		&LpxPcalPcmdSpeedCalGain  },/* Position command speed						*/
  { 0x85, 0,   0,   FALSE,	&GetVarData_AmonPosErra,	NULL                      },/* Position deviation							*/
//{ 0x90, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Observer speed					*//* Not supported */
//{ 0x91, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Model torque command/thrust command		*//* Not supported */
//{ 0x92, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Model position deviation					*//* Not supported */
//{ 0x93, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Friction compensation torque/thrust				*//* Not supported */
  { 0x94, 0,   0,   FALSE,	&GetVarData_MonResVib,		NULL					  },/* Damping differential speed						*/
  { 0x95, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL				  	  },/* Gravity compensated torque command/thrust command	*//* <S0CD> */	/* Not supported⇒Correspondence */
  { 0x96, 0,   0,   FALSE,	&GetVarData_AmonPosErrx,	NULL	                  },/* Position amplifier deviation					*/
  { 0x97, 0,   0,   FALSE,	&GetVarData_sfperra_per32s, NULL					  },/* Motor load positional deviation				*/
  { 0x98, 0,   0,   FALSE,	&GetVarData_SpdFFCx,		NULL					  },/* Speed feedforward			*/
  { 0x99, 0,   0,   FALSE,	&GetVarData_TrqFFCx,		NULL					  },/* Torque/thrust feedforward	*/
//{ 0x9A, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* MFCStop time after position command					*//* Not supported */
//{ 0x9B, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Additional torque command/Thrust command			*//* Not supported */
//{ 0x9C, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Additional disturbance torque command/Thrust command		*//* Not supported */
//{ 0x9D, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Estimated speed							*//* Not supported */
//{ 0x9E, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Torque command(Before disturbance)				*//* Not supported */
//{ 0x9F, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Torque command - Friction compensation torque		*//* Not supported */
//{ 0xA0, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Torque command(Before disturbance/before filter)	*//* Not supported */
//{ 0xA1, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Valid gain						*//* Not supported */
  { 0xA2, 0,   0,   TRUE,	&GetVarData_dPosRefi,		&LpxPcalMotSpeedCalGain   },/* Command speed before position loop				*/
  { 0xA9, 0,   0,   TRUE,	&GetVarData_FencSpdSmp,		NULL					  },/* External encoder speed				*/
  { 0xAA, 0,   0,   FALSE,	&GetVarData_VelRippleTrqRefBeforeComp,	NULL		  },/* Torque command (before ripple correction)		*/	/* <S14B><S18C> */
  { 0xC4, 0,   0,   FALSE,	&GetVarData_CurrentPhase,	NULL					  },/* Current phase 						*/	/* <S14B><S18C> */
  { 0xC5, 0,   0,   FALSE,	&GetVarData_TrcDcVoltFili,	NULL					  },/* Main circuit bus voltage (before filter)		*/	/* <S199> *//* Not supported⇒Correspondence */
//{ 0xD0, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMotor speed					*//* Not supported */
//{ 0xD1, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMotor travel amount					*//* Not supported */
//{ 0xD2, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMonitoring speedA						*//* Not supported */
//{ 0xD3, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMMonitoring speedB						*//* Not supported */
//{ 0xD4, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Active mode internal command speed		*//* Not supported */
//{ 0xD5, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMInternal data1					*//* Not supported */
//{ 0xD6, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* SOMInternal data2					*//* Not supported */
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
  { 0x100, 0,   0,   FALSE,	&GetVarData_M3_Apos,		NULL					  },/* Feedback position(APOS)			*/				/* <S164> : Setting value change 0xD0⇒0x100 */
  { 0x101, 0,   0,   FALSE,	&GetVarData_M3_Cpos,		NULL					  },/* Current command position(CPOS)				*/				/* <S164> : Setting value change 0xD1⇒0x101 */
  { 0x102, 0,   0,   FALSE,	&GetVarData_M3_Tpos,		NULL					  },/* Target position(TPOS)					*/				/* <S164> : Setting value change 0xD2⇒0x102 */
  { 0x103, 0,   0,   FALSE,	&GetVarData_M3_Lpos1,		NULL					  },/* Latch position1(LPOS1)				*/				/* <S164> : Setting value change 0xD3⇒0x103 */
  { 0x104, 0,   0,   FALSE,	&GetVarData_M3_Lpos2,		NULL					  },/* Latch position2(LPOS2)				*/				/* <S164> : Setting value change 0xD4⇒0x104 */
//{ 0x105, 0,   0,   FALSE,	&GetVarData_Dummy,			NULL					  },/* Latch position3(LPOS3)				*//* Not supported */	/* <S164> */
  { 0x106, 0,   0,   FALSE,	&GetVarData_M3_Fspd,		NULL					  },/* Feedback speed(FSPD)			*/			/* <S164> : Setting value change 0xD5⇒0x106 */
  { 0x107, 0,   0,   FALSE,	&GetVarData_M3_Cspd,		NULL					  },/* Current position command speed(CSPD)			*/			/* <S164> : Setting value change 0xD6⇒0x107 */
  { 0x108, 0,   0,   FALSE,	&GetVarData_M3_Trq,			NULL					  },/* Torque/Thrust command(TRQ)				*/			/* <S164> : Setting value change 0xD7⇒0x108 */
  { 0x109, 0,   0,   FALSE,	&GetVarData_M3_TrqLim,		NULL					  },/* Torque limit command(TRQ_LIM)			*/			/* <S164> : Setting value change 0xD8⇒0x109 */
  { 0x10A, 0,   0,   FALSE,	&GetVarData_M3_VLim,		NULL					  },/* Speed limit command(V_LIM)				*/			/* <S164> : Setting value change 0xD9⇒0x10A */
  { 0x10B, 0,   0,   FALSE,	&GetVarData_M3_M3_Alam,		NULL					  },/* Currently occurring alarm code(ALARM)	*/			/* <S164> : Setting value change 0xDA⇒0x10B */
  { 0x10C, 0,   0,   FALSE,	&GetVarData_M3_RCmd,		NULL					  },/* M-ⅢCurrently executing command code(RCMD)	*/			/* <S164> : Setting value change 0xDB⇒0x10C */
  { 0x10D, 0,   0,   FALSE,	&GetVarData_M3_SubRCmd,		NULL					  },/* M-ⅢCurrently executing subcommand code(SUB_RCMD)	*/	/* <S164> : Setting value change 0xDC⇒0x10D */
  { 0x110, 0,   0,   FALSE,	&GetVarData_M3_Cmd_0,		NULL					  },/* M-ⅢCommand data( 0~ 3byte)	*/			/* <S164> : Setting value change 0xE0⇒0x110 */
  { 0x111, 0,   0,   FALSE,	&GetVarData_M3_Cmd_1,		NULL					  },/* M-ⅢCommand data( 4~ 7byte)	*/			/* <S164> : Setting value change 0xE1⇒0x111 */
  { 0x112, 0,   0,   FALSE,	&GetVarData_M3_Cmd_2,		NULL					  },/* M-ⅢCommand data( 8~11byte)	*/			/* <S164> : Setting value change 0xE2⇒0x112 */
  { 0x113, 0,   0,   FALSE,	&GetVarData_M3_Cmd_3,		NULL					  },/* M-ⅢCommand data(12~15byte)	*/			/* <S164> : Setting value change 0xE3⇒0x113 */
  { 0x114, 0,   0,   FALSE,	&GetVarData_M3_Cmd_4,		NULL					  },/* M-ⅢCommand data(16~19byte)	*/			/* <S164> : Setting value change 0xE4⇒0x114 */
  { 0x115, 0,   0,   FALSE,	&GetVarData_M3_Cmd_5,		NULL					  },/* M-ⅢCommand data(20~23byte)	*/			/* <S164> : Setting value change 0xE5⇒0x115 */
  { 0x116, 0,   0,   FALSE,	&GetVarData_M3_Cmd_6,		NULL					  },/* M-ⅢCommand data(24~27byte)	*/			/* <S164> : Setting value change 0xE6⇒0x116 */
  { 0x117, 0,   0,   FALSE,	&GetVarData_M3_Cmd_7,		NULL					  },/* M-ⅢCommand data(28~31byte)	*/			/* <S164> : Setting value change 0xE7⇒0x117 */
  { 0x118, 0,   0,   FALSE,	&GetVarData_M3_Cmd_8,		NULL					  },/* M-ⅢCommand data(32~35byte)	*/			/* <S164> : Setting value change 0xE8⇒0x118 */
  { 0x119, 0,   0,   FALSE,	&GetVarData_M3_Cmd_9,		NULL					  },/* M-ⅢCommand data(36~39byte)	*/			/* <S164> : Setting value change 0xE9⇒0x119 */
  { 0x11A, 0,   0,   FALSE,	&GetVarData_M3_Cmd_a,		NULL					  },/* M-ⅢCommand data(40~43byte)	*/			/* <S164> : Setting value change 0xEA⇒0x11A */
  { 0x11B, 0,   0,   FALSE,	&GetVarData_M3_Cmd_b,		NULL					  },/* M-ⅢCommand data(44~47byte)	*/			/* <S164> : Setting value change 0xEB⇒0x11B */
  { 0x120, 0,   0,   FALSE,	&GetVarData_M3_Rsp_0,		NULL					  },/* M-ⅢResponse data( 0~ 3byte)	*/			/* <S164> : Setting value change 0xF0⇒0x120 */
  { 0x121, 0,   0,   FALSE,	&GetVarData_M3_Rsp_1,		NULL					  },/* M-ⅢResponse data( 4~ 7byte)	*/			/* <S164> : Setting value change 0xF1⇒0x121 */
  { 0x122, 0,   0,   FALSE,	&GetVarData_M3_Rsp_2,		NULL					  },/* M-ⅢResponse data( 8~11byte)	*/			/* <S164> : Setting value change 0xF2⇒0x122 */
  { 0x123, 0,   0,   FALSE,	&GetVarData_M3_Rsp_3,		NULL					  },/* M-ⅢResponse data(12~15byte)	*/			/* <S164> : Setting value change 0xF3⇒0x123 */
  { 0x124, 0,   0,   FALSE,	&GetVarData_M3_Rsp_4,		NULL					  },/* M-ⅢResponse data(16~19byte)	*/			/* <S164> : Setting value change 0xF4⇒0x124 */
  { 0x125, 0,   0,   FALSE,	&GetVarData_M3_Rsp_5,		NULL					  },/* M-ⅢResponse data(20~23byte)	*/			/* <S164> : Setting value change 0xF5⇒0x125 */
  { 0x126, 0,   0,   FALSE,	&GetVarData_M3_Rsp_6,		NULL					  },/* M-ⅢResponse data(24~27byte)	*/			/* <S164> : Setting value change 0xF6⇒0x126 */
  { 0x127, 0,   0,   FALSE,	&GetVarData_M3_Rsp_7,		NULL					  },/* M-ⅢResponse data(28~31byte)	*/			/* <S164> : Setting value change 0xF7⇒0x127 */
  { 0x128, 0,   0,   FALSE,	&GetVarData_M3_Rsp_8,		NULL					  },/* M-ⅢResponse data(32~35byte)	*/			/* <S164> : Setting value change 0xF8⇒0x128 */
  { 0x129, 0,   0,   FALSE,	&GetVarData_M3_Rsp_9,		NULL					  },/* M-ⅢResponse data(36~39byte)	*/			/* <S164> : Setting value change 0xF9⇒0x129 */
  { 0x12A, 0,   0,   FALSE,	&GetVarData_M3_Rsp_a,		NULL					  },/* M-ⅢResponse data(40~43byte)	*/			/* <S164> : Setting value change 0xFA⇒0x12A */
  { 0x12B, 0,   0,   FALSE,	&GetVarData_M3_Rsp_b,		NULL					  },/* M-ⅢResponse data(44~47byte)	*/			/* <S164> : Setting value change 0xFB⇒0x12B */
#endif //CSW_NETIF_TYPE == NETIF_M3_TYPE
};
/*--------------------------------------------------------------------------------------------------*/
const	USHORT	TrcVarTblEntNum = sizeof(TrcVarTbl)/sizeof(TrcVarTbl[0]);
/*--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------*/
/*  Caution: Excel macro not supported.																	*/
/*  The following must be set.																		*/
/*  Sel					: Numerical trace selection number														*/
/*	GetBitData Function	: Function pointer(Function that returns the value of the bit to be traced)							*/
/*--------------------------------------------------------------------------------------------------*/
/****************************************************************************************************/
/*																									*/
/*		Bit trace definition table																	*/
/*																									*/
/****************************************************************************************************/
const	TBXDEF	TrcBitTbl[] = {
/*  Sel  Get BitData Function																		*/
/*--------------------------------------------------------------------------------------------------*/
  { 0x00,  NULL						},	/* No trace specification						*/
  { 0x01,  &GetBitData_SvonReq		},	/* Servo-on command(/S-ON)				*/
  { 0x02,  &GetBitData_Pcon			},	/* PControl command(/P-CON)					*/
  { 0x03,  &GetBitData_Pot			},	/* Forward rotation prohibited(P-OT)						*/
  { 0x04,  &GetBitData_Not			},	/* Reverse rotation prohibited(N-OT)						*/
  { 0x05,  &GetBitData_AlmRst		},	/* Alarm reset(/ALMRST)			*/
  { 0x06,  &GetBitData_Pcl			},	/* Positive torque limit(/P-CL)				*/
  { 0x07,  &GetBitData_Ncl			},	/* Negative side torque limit(/N-CL)				*/
  { 0x08,  &GetBitData_SpdD			},	/* SPD-DSignal(/SPD-D)					*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x09,  &GetBitData_SpdA			},	/* SPD-ASignal(/SPD-A)					*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x0A,  &GetBitData_SpdB			},	/* SPD-BSignal(/SPD-B)					*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x0B,  &GetBitData_Csel			},	/* C-SELSignal(/C-SEL)					*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x0C,  &GetBitData_Zclamp		},	/* Zero clamp signal(/ZCLAMP)			*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x0D,  &GetBitData_Inhibit		},	/* Pulse command ignored signal(/INHIBIT)			*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x0E,  &GetBitData_Gsel			},	/* G-SEL1Signal(/G-SEL1)					*/
  { 0x0F,  &GetBitData_Pdet			},	/* Magnetic pole detection command(/P-DET)					*/
//{ 0x10,  &BinV.f.Sv				},	/* G-SEL2Signal(/GSEL-2)					*//* Not supported */
  { 0x11,  &GetBitData_Estp			},	/* Emergency stop(/EMG-STP)					*//* Not supported */	/* <S199> *//* Not supported⇒Correspondence */
  { 0x12,  &GetBitData_Dec			},	/* Deceleration signal(/DEC)						*/
  { 0x13,  &GetBitData_Ext1			},	/* EXT1Interrupt request(/EXT1)				*/
  { 0x14,  &GetBitData_Ext2			},	/* EXT2Interrupt request(/EXT2)				*/
  { 0x15,  &GetBitData_Ext3			},	/* EXT3Interrupt request(/EXT3)				*/
  { 0x16,  &GetBitData_Fstp			},	/* Forced stop/FSTP)						*/	/* <S1F5> */
//{ 0x16,  &ReadDummy.Ulong			},	/* Input signal1(/INPUT1)					*//* Not supported */
//{ 0x17,  &ReadDummy.Ulong			},	/* Input signal2(/INPUT2)					*//* Not supported */
//{ 0x18,  &ReadDummy.Ulong			},	/* Input signal3(/INPUT3)					*//* Not supported */
  { 0x19,  &GetBitData_Sen			},	/* SENSignal(SEN)							*/
  { 0x1A,  &GetBitData_Puls			},	/* Pulse command input presence/absence(PULS)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */	/* <S199> */
  { 0x1B,  &GetBitData_Sign			},	/* Sign command input presence/absence(SIGN)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */	/* <S199> */
  { 0x1C,  &GetBitData_Clr			},	/* Deviation clear signal presence/absence(CLR)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */	/* <S199> */
  { 0x1D,  &GetBitData_Psel			},	/* Command pulse input magnification switching signal(/PSEL)	*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */	/* <S199> */
  { 0x20,  &GetBitData_HWBB1		},	/* HWBB1Signal(/HWBB1)					*/
  { 0x21,  &GetBitData_HWBB2		},	/* HWBB2Signal(/HWBB2)					*//* <S064> */
//{ 0x22,  &CoutSom.UnSomIOSignal	},	/* Safety request input signalA1					*//* Not supported */
//{ 0x23,  &CoutSom.UnSomIOSignal	},	/* Safety request input signalA2					*//* Not supported */
//{ 0x24,  &CoutSom.UnSomIOSignal	},	/* Safety request input signalB1					*//* Not supported */
//{ 0x25,  &CoutSom.UnSomIOSignal	},	/* Safety request input signalB2					*//* Not supported */
//{ 0x40,  &CoutSom.UnSomIOSignal	},	/* Safety request input statusA					*//* Not supported */
//{ 0x41,  &CoutSom.UnSomIOSignal	},	/* Safety request input statusB					*//* Not supported */
  { 0x80,  &GetBitData_AlmFlag		},	/* Alarm occurrence(ALM)					*/
  { 0x81,  &GetBitData_Coin			},	/* Positioning complete(/COIN)					*/
  { 0x82,  &GetBitData_Vcmp			},	/* Speed match(/V-CMP)						*/
  { 0x83,  &GetBitData_Tgon			},	/* Motor rotation detection(/TGON)				*/
  { 0x84,  &GetBitData_SvRdy		},	/* Servo ready(/S-RDY)					*/
  { 0x85,  &GetBitData_Clt			},	/* Current limiting(CLT)						*/
  { 0x86,  &GetBitData_Vlt			},	/* Speed limit(/VLT)						*/
  { 0x87,  &GetBitData_Bk			},	/* Brake hold(/BK)					*/
  { 0x88,  &GetBitData_WrnFlag		},	/* Warning occurs(/WARN)				*/
  { 0x89,  &GetBitData_Near			},	/* Positioning vicinity(/NEAR)					*/
  { 0x8A,  &GetBitData_Cplue		},	/* CPhase output(/C-PLUS)						*/
//{ 0x8B,  &ReadDummy.Ulong			},	/* Output signal1(/OUTPUT1)					*//* Not supported */
//{ 0x8C,  &ReadDummy.Ulong			},	/* Output signal2(/OUTPUT2)					*//* Not supported */
//{ 0x8D,  &ReadDummy.Ulong			},	/* Output signal3(/OUTPUT3)					*//* Not supported */
  { 0x8E,  &GetBitData_Alo1			},	/* Alarm code output1(AL01)			*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x8F,  &GetBitData_Alo2			},	/* Alarm code output2(AL02)			*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x90,  &GetBitData_Alo3			},	/* Alarm code output3(AL03)			*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x91,  &GetBitData_Pao			},	/* Divided pulse outputAPhase(PAO)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x92,  &GetBitData_Pbo			},	/* Divided pulse outputBPhase(PBO)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x93,  &GetBitData_Pco			},	/* Divided pulse outputCPhase(PCO)				*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
  { 0x94,  &GetBitData_Psela		},	/* Command pulse input magnification switching response(/PSELA)	*//* Correspondence */	/* <S164> *//* Not supported⇒Correspondence */
//{ 0xA0,  &CoutSom.UnSomIOSignal	},	/* External equipment monitor signalA					*//* Not supported */
//{ 0xA1,  &CoutSom.UnSomIOSignal	},	/* External equipment monitor signalB					*//* Not supported */
  { 0xC0,  &GetBitData_Acon			},	/* Main circuit power on(ACON)					*/
  { 0xC1,  &GetBitData_Refz			},	/* Command dispensing zero(REFZ)				*/
  { 0xC2,  &GetBitData_Pdetcmp		},	/* Magnetic pole detection complete(PDETCMP)				*/
//{ 0xC3,  &CoutSom.UnSomState		},	/* Active mode operating status				*//* Not supported */
//{ 0xC4,  &CoutSom.UnSomState		},	/* Safety function operating status-Monitor				*//* Not supported */
//{ 0xC5,  &CoutSom.UnSomState		},	/* Safety function operating status-Safety				*//* Not supported */
//{ 0xC6,  &CoutSom.UnSomState		},	/* Safety function operating status-HWBB				*//* Not supported */
//{ 0xC7,  &CoutSom.UnSomState		},	/* Safety function operating status-Preliminary				*//* Not supported */
  { 0xC8,  &GetBitData_Den			},	/* Command dispensing complete(DEN)				*/
  { 0xC9,  &GetBitData_Pset			},	/* Positioning complete(PSET)					*/
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
  { 0xCA,  &GetBitData_M3CmdRdy		},	/* M-ⅢCommand ready(CMDRDY)			*/
#endif //CSW_NETIF_TYPE == NETIF_M3_TYPE
};
/*--------------------------------------------------------------------------------------------------*/
const	USHORT	TrcBitTblEntNum = sizeof(TrcBitTbl)/sizeof(TrcBitTbl[0]);
/*--------------------------------------------------------------------------------------------------*/

/***************************************** end of file **********************************************/
