/****************************************************************************
Description	: Function list of the Data Trace
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.Kira   2010.11.01     created

****************************************************************************/
#ifndef __DATA_TRACE_CALC_H
#define __DATA_TRACE_CALC_H

#include "Basedef.h"
#include "PrmAccessIf.h"

/* Numeric trace data acquisition function	*/
LONG	GetVarData_AmonTrqRef( void *Axis );				/* 0x01,0x81: Torque/Thrust monitor					*/
LONG	GetVarData_MotSpdSmp( void *Axis );					/* 0x02,0x82: Motor speed						*/
LONG	GetVarData_AmonSpdRef( void *Axis );				/* 0x03,0x83: Speed command							*/
LONG	GetVarData_dPosCmdSmp( void *Axis );				/* 0x04,0x84: Position command speed						*/
LONG	GetVarData_AmonPosErra( void *Axis );				/* 0x05,0x85: Position deviation							*/
LONG	GetVarData_AmonDisTrq( void *Axis );				/* 0x13	    : Friction compensation torque/thrust				*/
LONG	GetVarData_MonResVib( void *Axis );					/* 0x14		: Damping differential speed						*/
LONG	GetVarData_TrqGravity( void *Axis );				/*  0x15	: TrqGravity						*/	/*<S0CD>*/
LONG	GetVarData_AmonPosErrx( void *Axis );				/* 0x16,0x96: Position amplifier deviation					*/
LONG	GetVarData_sfperra_per32s( void *Axis );			/* 0x17,0x97: Motor load positional deviation				*/
LONG	GetVarData_SpdFFCx( void *Axis );					/* 0x18,0x98: Speed feedforward			*/
LONG	GetVarData_TrqFFCx( void *Axis );					/* 0x19,0x99: Torque/thrust feedforward	*/
LONG	GetVarData_TrefBeforeDtrq( void *Axis );			/* 0x1E		: Torque command(Before disturbance)				*/
LONG	GetVarData_TrcCtrlTrqRef( void *Axis );				/* 0x1F		: Torque command - Friction compensation torque		*/
LONG	GetVarData_TrqRefi( void *Axis );					/* 0x20		: Torque command(Before disturbance/before filter)	*/
LONG	GetVarData_AmonActGain( void *Axis );				/* 0x21     : Valid gain						*/
LONG	GetVarData_dPosRefi( void *Axis );					/* 0x22,0xA2: Position command speed before position loop			*/	/* <S187> */
LONG	GetVarData_DcVoltx( void *Axis );					/* 0x23		: Main circuit bus voltage					*/
LONG	GetVarData_UnCrntDet( void *Axis );					/* 0x24     : Current detection value						*/
LONG	GetVarData_UnCrntRef( void *Axis );					/* 0x26	    : Cumulative load rate						*/
LONG	GetVarData_TrcRgPower( void *Axis );				/* 0x27		: Regeneration load rate						*/
LONG	GetVarData_TrcDbPower( void *Axis );				/* 0x28		: DBResistor power consumption					*/
LONG	GetVarData_FencSpdSmp( void *Axis );				/* 0x29		: External encoder speed				*/
LONG	GetVarData_DeltaPos( void *Axis );					/* 0x40		: Position difference							*/
LONG	GetVarData_dPcmdEgear( void *Axis );				/* 0x41     : Command difference							*/
LONG	GetVarData_DeltaPos( void *Axis );					/* 0x42		: Motor position difference					*/
LONG	GetVarData_dFencPos( void *Axis );					/* 0x43		: Load position difference						*/
LONG	GetVarData_MotPhase( void *Axis );					/* 0x44		: Current phase							*/
LONG	GetVarData_InterSignalX_L( void *Axis );			/* 0x60		: Internal signal monitor(Lower)				*/
LONG	GetVarData_InterSignalX_H( void *Axis );			/* 0x61		: Internal signal monitor(Upper level)				*/
LONG	GetVarData_InternalInputSignals_L( void *Axis );	/* 0x62		: Internal input signal monitor(Lower)			*/
LONG	GetVarData_InternalInputSignals_H( void *Axis );	/* 0x63		: Internal input signal monitor(Upper level)			*/
LONG	GetVarData_InternalOutputSignals_L( void *Axis );	/* 0x64		: Internal output signal monitor(Lower)			*/
LONG	GetVarData_InternalOutputSignals_H( void *Axis );	/* 0x65		: Internal output signal monitor(Upper level)			*/
LONG	GetVarData_VelRippleTrqRefBeforeComp( void *Axis );	/* 0xAA		: Speed ripple Torque before compensation/Thrust command*/	/* <S14B> */
LONG	GetVarData_CurrentPhase( void *Axis );				/* 0xC4		: Current phase							*/	/* <S14B> */
LONG	GetVarData_TrcDcVoltFili( void *Axis );				/* 0xC5		:Main circuit bus voltage(Before filter)			*/	/* <S199> */
LONG	GetVarData_M3_Apos( void *Axis );					/* 0x100	: M-ⅢFeedback position(APOS)		*/			/* <S164> : Setting value change 0xD0⇒0x100 */
LONG	GetVarData_M3_Cpos( void *Axis );					/* 0x101	: M-ⅢCurrent command position(CPOS)			*/			/* <S164> : Setting value change 0xD1⇒0x101 */
LONG	GetVarData_M3_Tpos( void *Axis );					/* 0x102	: M-ⅢTarget position(TPOS)				*/			/* <S164> : Setting value change 0xD2⇒0x102 */
LONG	GetVarData_M3_Lpos1( void *Axis );					/* 0x103	: M-ⅢLatch position1(LPOS1)			*/			/* <S164> : Setting value change 0xD3⇒0x103 */
LONG	GetVarData_M3_Lpos2( void *Axis );					/* 0x104	: M-ⅢLatch position2(LPOS2)			*/			/* <S164> : Setting value change 0xD4⇒0x104 */
LONG	GetVarData_M3_Fspd( void *Axis );					/* 0x105	: M-ⅢFeedback speed(FSPD)		*/			/* <S164> : Setting value change 0xD5⇒0x105 */
LONG	GetVarData_M3_Cspd( void *Axis );					/* 0x106	: M-ⅢCurrent position command speed(CSPD)		*/			/* <S164> : Setting value change 0xD6⇒0x106 */
LONG	GetVarData_M3_Trq( void *Axis );					/* 0x107	: M-ⅢTorque/Thrust command(TRQ)			*/			/* <S164> : Setting value change 0xD7⇒0x107 */
LONG	GetVarData_M3_TrqLim( void *Axis );					/* 0x108	: M-ⅢTorque limit command(TRQ_LIM)		*/			/* <S164> : Setting value change 0xD8⇒0x108 */
LONG	GetVarData_M3_VLim( void *Axis );					/* 0x109	: M-ⅢSpeed limit command(V_LIM)			*/			/* <S164> : Setting value change 0xD9⇒0x109 */
LONG	GetVarData_M3_M3_Alam( void *Axis );				/* 0x10A	: M-ⅢCurrently occurring alarm code(ALARM)	*/			/* <S164> : Setting value change 0xDA⇒0x10A */
LONG	GetVarData_M3_RCmd( void *Axis );					/* 0x10B	: M-ⅢCurrently executing command code(RCMD)	*/			/* <S164> : Setting value change 0xDB⇒0x10B */
LONG	GetVarData_M3_SubRCmd( void *Axis );				/* 0x10C	: M-ⅢCurrently executing subcommand code(SUB_RCMD)*/	/* <S164> : Setting value change 0xDC⇒0x10C */
LONG	GetVarData_M3_Cmd_0( void *Axis );					/* 0x110	: M-ⅢCommand data( 0~ 3byte)	*/			/* <S164> : Setting value change 0xE0⇒0x110 */
LONG	GetVarData_M3_Cmd_1( void *Axis );					/* 0x111	: M-ⅢCommand data( 4~ 7byte)	*/			/* <S164> : Setting value change 0xE1⇒0x111 */
LONG	GetVarData_M3_Cmd_2( void *Axis );					/* 0x112	: M-ⅢCommand data( 8~11byte)	*/			/* <S164> : Setting value change 0xE2⇒0x112 */
LONG	GetVarData_M3_Cmd_3( void *Axis );					/* 0x113	: M-ⅢCommand data(12~15byte)	*/			/* <S164> : Setting value change 0xE3⇒0x113 */
LONG	GetVarData_M3_Cmd_4( void *Axis );					/* 0x114	: M-ⅢCommand data(16~19byte)	*/			/* <S164> : Setting value change 0xE4⇒0x114 */
LONG	GetVarData_M3_Cmd_5( void *Axis );					/* 0x115	: M-ⅢCommand data(20~23byte)	*/			/* <S164> : Setting value change 0xE5⇒0x115 */
LONG	GetVarData_M3_Cmd_6( void *Axis );					/* 0x116	: M-ⅢCommand data(24~27byte)	*/			/* <S164> : Setting value change 0xE6⇒0x116 */
LONG	GetVarData_M3_Cmd_7( void *Axis );					/* 0x117	: M-ⅢCommand data(28~31byte)	*/			/* <S164> : Setting value change 0xE7⇒0x117 */
LONG	GetVarData_M3_Cmd_8( void *Axis );					/* 0x118	: M-ⅢCommand data(32~35byte)	*/			/* <S164> : Setting value change 0xE8⇒0x118 */
LONG	GetVarData_M3_Cmd_9( void *Axis );					/* 0x119	: M-ⅢCommand data(36~39byte)	*/			/* <S164> : Setting value change 0xE9⇒0x119 */
LONG	GetVarData_M3_Cmd_a( void *Axis );					/* 0x11A	: M-ⅢCommand data(40~43byte)	*/			/* <S164> : Setting value change 0xEA⇒0x11A */
LONG	GetVarData_M3_Cmd_b( void *Axis );					/* 0x11B	: M-ⅢCommand data(44~47byte)	*/			/* <S164> : Setting value change 0xEB⇒0x11B */
LONG	GetVarData_M3_Rsp_0( void *Axis );					/* 0x120	: M-ⅢResponse data( 0~ 3byte)	*/			/* <S164> : Setting value change 0xF0⇒0x120 */
LONG	GetVarData_M3_Rsp_1( void *Axis );					/* 0x121	: M-ⅢResponse data( 4~ 7byte)	*/			/* <S164> : Setting value change 0xF1⇒0x121 */
LONG	GetVarData_M3_Rsp_2( void *Axis );					/* 0x122	: M-ⅢResponse data( 8~11byte)	*/			/* <S164> : Setting value change 0xF2⇒0x122 */
LONG	GetVarData_M3_Rsp_3( void *Axis );					/* 0x123	: M-ⅢResponse data(12~15byte)	*/			/* <S164> : Setting value change 0xF3⇒0x123 */
LONG	GetVarData_M3_Rsp_4( void *Axis );					/* 0x124	: M-ⅢResponse data(16~19byte)	*/			/* <S164> : Setting value change 0xF4⇒0x124 */
LONG	GetVarData_M3_Rsp_5( void *Axis );					/* 0x125	: M-ⅢResponse data(20~23byte)	*/			/* <S164> : Setting value change 0xF5⇒0x125 */
LONG	GetVarData_M3_Rsp_6( void *Axis );					/* 0x126	: M-ⅢResponse data(24~27byte)	*/			/* <S164> : Setting value change 0xF6⇒0x126 */
LONG	GetVarData_M3_Rsp_7( void *Axis );					/* 0x127	: M-ⅢResponse data(28~31byte)	*/			/* <S164> : Setting value change 0xF7⇒0x127 */
LONG	GetVarData_M3_Rsp_8( void *Axis );					/* 0x128	: M-ⅢResponse data(32~35byte)	*/			/* <S164> : Setting value change 0xF8⇒0x128 */
LONG	GetVarData_M3_Rsp_9( void *Axis );					/* 0x129	: M-ⅢResponse data(36~39byte)	*/			/* <S164> : Setting value change 0xF9⇒0x129 */
LONG	GetVarData_M3_Rsp_a( void *Axis );					/* 0x12A	: M-ⅢResponse data(40~43byte)	*/			/* <S164> : Setting value change 0xFA⇒0x12A */
LONG	GetVarData_M3_Rsp_b( void *Axis );					/* 0x12B	: M-ⅢResponse data(44~47byte)	*/			/* <S164> : Setting value change 0xFB⇒0x12B */
LONG	GetVarData_AdrShortData( void *Axis );				/* 0x7F,0xFF: Real address specification(SHORT)				*/
LONG	GetVarData_AdrLongData( void *Axis );				/* 0x7F,0xFF: Real address specification(LONG)				*/
LONG	GetVarData_Dummy( void *Axis );						/* ****		: Dummy variable						*/
LONG	GetVarData_SqnetAdrData( void *Axis );				/* ****		: Real address specification(SynqNet)			*/
LONG	GetVarData_SqnetAdrData2( void *Axis );			/* ****		: Real address specification(SynqNet)			*/

LONG GetVarData_AmonAdrOutCH0H( void *Axis );				/* ****<S074>: Real address specification(AMON_CH:0 SHORT)  */
LONG GetVarData_AmonAdrOutCH0W( void *Axis );				/* ****<S074>: Real address specification(AMON_CH:0 LONG)   */
LONG GetVarData_AmonAdrOutCH1H( void *Axis );				/* ****<S074>: Real address specification(AMON_CH:1 SHORT)  */
LONG GetVarData_AmonAdrOutCH1W( void *Axis );				/* ****<S074>: Real address specification(AMON_CH:1 LONG)   */


/* Numeric trace data gain calculation function	*/
LONG	LpxPcalMotSpeedCalGain( void* Axis );				/* 0x02,0x82: Motor speed						*/
LONG	LpxPcalMotSpeedCalGain2( void* Axis );				/* 0x04,0x84: Position command speed						*/
LONG	LpxPcalPcmdSpeedCalGain( void* Axis );				/* 0x22		: Command speed before position loop				*/

/* Bit trace data acquisition function	*/
BOOL	GetBitData_SvonReq( void *Axis );					/* 0x01	: Servo-on command(/S-ON)					*/
BOOL	GetBitData_Pcon( void *Axis );						/* 0x02	: PControl command(/P-CON)						*/
BOOL	GetBitData_Pot( void *Axis );						/* 0x03	: Forward rotation prohibited(P-OT)						*/
BOOL	GetBitData_Not( void *Axis );						/* 0x04	: Reverse rotation prohibited(N-OT)						*/
BOOL	GetBitData_AlmRst( void *Axis );					/* 0x05	: Alarm reset(/ALMRST)				*/
BOOL	GetBitData_Pcl( void *Axis );						/* 0x06	: Positive torque limit(/P-CL)					*/
BOOL	GetBitData_Ncl( void *Axis );						/* 0x07	: Negative side torque limit(/N-CL)					*/
/* <S164> Start */
BOOL	GetBitData_SpdD( void *Axis );						/* 0x08	: SPD-D signal(/SPD-D)				*/
BOOL	GetBitData_SpdA( void *Axis );						/* 0x09	: SPD-A signal(/SPD-A)				*/
BOOL	GetBitData_SpdB( void *Axis );						/* 0x0A	: SPD-B signal(/SPD-B)				*/
BOOL	GetBitData_Csel( void *Axis );						/* 0x0B	: C-SEL signal(/C-SEL)				*/
BOOL	GetBitData_Zclamp( void *Axis );					/* 0x0C	: Zero clamp signal(/ZCLAMP)				*/
BOOL	GetBitData_Inhibit( void *Axis );					/* 0x0D	: Pulse command ignored signal(/INHIBIT)			*/
/* <S164> End */
BOOL	GetBitData_Gsel( void *Axis );						/* 0x0E	: G-SEL1Signal(/G-SEL1)					*/
BOOL	GetBitData_Pdet( void *Axis );						/* 0x0F : Magnetic pole detection command(/P-DET)					*/
BOOL	GetBitData_Estp( void *Axis );						/* 0x11	: Emergency stop(/E-STP)						*/	/* <S164> */
BOOL	GetBitData_Dec( void *Axis );						/* 0x12 : Deceleration signal(/DEC)						*/
BOOL	GetBitData_Ext1( void *Axis );						/* 0x13 : EXT1Interrupt request(/EXT1)					*/
BOOL	GetBitData_Ext2( void *Axis );						/* 0x14 : EXT2Interrupt request(/EXT2)					*/
BOOL	GetBitData_Ext3( void *Axis );						/* 0x15 : EXT3Interrupt request(/EXT3)					*/
BOOL	GetBitData_Fstp( void *Axis);						/* 0x16 : FSTPInterrupt request(/FSTP)  <S1F5>					*/
BOOL	GetBitData_Sen( void *Axis );						/* 0x19 : SENSignal(SEN)							*/
/* <S164> Start */
BOOL	GetBitData_Puls( void *Axis );						/* 0x1A	: Pulse command input presence/absence(PULS)				*/
BOOL	GetBitData_Sign( void *Axis );						/* 0x1B	: Sign command input presence/absence(SIGN)				*/
BOOL	GetBitData_Clr( void *Axis );						/* 0x1C	: Deviation clear signal presence/absence(CLR)				*/
BOOL	GetBitData_Psel( void *Axis );						/* 0x1D	: Command pulse input magnification switching signal(/PSEL)		*/
/* <S164> End */
BOOL	GetBitData_HWBB1( void *Axis );						/* 0x20	: HWBB1Signal(/HWBB1)						*/
BOOL	GetBitData_HWBB2( void *Axis );						/* 0x20	: HWBB2Signal(/HWBB2)						*//* <S064> */
BOOL	GetBitData_AlmFlag( void *Axis );					/* 0x80	: Alarm occurrence(ALM)						*/
BOOL	GetBitData_Coin( void *Axis );						/* 0x81	: Positioning complete(/COIN)					*/
BOOL	GetBitData_Vcmp( void *Axis );						/* 0x82	: Speed match(/V-CMP)						*/
BOOL	GetBitData_Tgon( void *Axis );						/* 0x83	: Motor rotation detection(/TGON)					*/
BOOL	GetBitData_SvRdy( void *Axis );						/* 0x84	: Servo ready(/S-RDY)					*/
BOOL	GetBitData_Clt( void *Axis );						/* 0x85 : Current limiting(CLT)						*/
BOOL	GetBitData_Vlt( void *Axis );						/* 0x86 : Speed limit(/VLT)						*/
BOOL	GetBitData_Bk( void *Axis );						/* 0x87	: Brake hold(/BK)						*/
BOOL	GetBitData_WrnFlag( void *Axis );					/* 0x88	: Warning occurs(/WARN)					*/
BOOL	GetBitData_Near( void *Axis );						/* 0x89	: Positioning vicinity(/NEAR)					*/
BOOL	GetBitData_Cplue( void *Axis );						/* 0x8A : CPhase output(/C-PLUS)						*/
/* <S164> Start */
BOOL	GetBitData_Alo1( void *Axis );						/* 0x8E	: Alarm code output 1(AL01)			*/
BOOL	GetBitData_Alo2( void *Axis );						/* 0x8F	: Alarm code output 2(AL02)			*/
BOOL	GetBitData_Alo3( void *Axis );						/* 0x90	: Alarm code output 3(AL03)			*/
BOOL	GetBitData_Pao( void *Axis );						/* 0x91	: Divided pulse output A phase(PAO)				*/
BOOL	GetBitData_Pbo( void *Axis );						/* 0x92	: Divided pulse output B phase(PBO)				*/
BOOL	GetBitData_Pco( void *Axis );						/* 0x93	: Divided pulse output C phase(PCO)				*/
BOOL	GetBitData_Psela( void *Axis );						/* 0x94	: Command pulse input magnification switching response(/PSELA)	*/
/* <S164> End */
BOOL	GetBitData_Acon( void *Axis );						/* 0xC0	: Main circuit power on(ACON)					*/
BOOL	GetBitData_Refz( void *Axis );						/* 0xC1	: Command dispensing zero(REFZ)				*/
BOOL	GetBitData_Pdetcmp( void *Axis );					/* 0xC2 : Magnetic pole detection complete(PDETCMP)					*/
BOOL	GetBitData_Den( void *Axis );						/* 0xC8	: Command dispensing complete(DEN)					*/
BOOL	GetBitData_Pset( void *Axis );						/* 0xC9	: Positioning complete(PSET)					*/
BOOL	GetBitData_M3CmdRdy( void *Axis );					/* 0xCA : M-ⅢCommand ready(CMDRDY)			*/
//BOOL	GetBitData_Dummy( void *Axis );						/* ****	: Dummy variable							*/

#endif //__DATA_TRACE_CALC_H

