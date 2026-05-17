/****************************************************************************
Description	: Function list of the Analog Monitor <S016>
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
K.Sakamoto   2013.10.03     created

****************************************************************************/
#ifndef __ANALOG_MONITOR_CALC_H
#define __ANALOG_MONITOR_CALC_H

#include "Basedef.h"
#include "MercuryGlobals.h"

/* Analog monitor variable acquisition function	*/
LONG	GetVarData_AmonMotSpd( void *Axis );				/* 00 :Motor rotation speed							*/
LONG	GetVarData_AmondPosCmdSmp( void *Axis );			/* 05:Position command speed								*/
LONG	GetVarData_AmonFencSpdsmp( void *Axis );			/* 0D:External encoder speed						*/
LONG	GetVarData_TrqRefBeforeComp( void *Axis );			/* 43:Torque command before filter						*/
LONG	GetVarData_AmonIdRef( void *Axis );					/* 44:dAxis current command								*/

/*Analog monitor gain calculation function	*/
KSGAIN	AmonGcalMotorSpeed( void *Axis );					/* Motor speed,Speed command (2^24/OvrSpd --> 1V/1000min-1)	*/
KSGAIN	AmonGcalMotPfbkSpd( void *Axis );					/* Motor rotation speed (dMotPos/ScanB --> 1V/1000min-1)		*/
KSGAIN	AmonGcalPosCmdaSpd( void *Axis );					/* Position command speed (dPosCmda/ScanB --> 1V/1000min-1)		*/
KSGAIN	AmonGcalTorqueRef( void *Axis );					/* Torque command (2^24/MaxTrq --> 1V/100%)					*/
KSGAIN	AmonGcalTrqRef15K( void *Axis );					/* Torque command (15000/MaxTrq --> 1V/100%)				*/
KSGAIN	AmonGcalPulseError( void *Axis );					/* Position deviation (0.05V/1Command unit)							*/
KSGAIN	AmonGcalPulseErr01( void *Axis );					/* Motor load positional deviation (0.01V/1Command unit)				*/
KSGAIN	AmonGcalBitSignal( void *Axis );					/* Bit signal (ON:5V, OFF:0V)							*/
KSGAIN	AmonGcalActGain( void *Axis );						/* Valid gain (No.1:1V, No.2:2V, No.3:3V, No.4:4V)			*/


void	PcalAnalogMonitorGain( AXIS_HANDLE *Axis, LONG chx );	/* Analog monitor output gain calculation			*/
void	PcalAnalogMonitorOffset( AXIS_HANDLE *Axis, LONG chx );	/* Analog monitor output offset calculation		*/
LONG	LpxSearchAmonDefTable( LONG SelNo );					/* Analog monitor definition table search		*/
void	LpxGcalPrmVariable( AXIS_HANDLE *Axis, AMONOUT *AmonOutx, LONG Hgain );	/* PrmSpecified variable			*/


void	BpxOutputAnalogMonitor( AXIS_HANDLE *Axis );		/* Analog monitor output processing						*/

//void	FnOutputAnalogMonitor( AXIS_HANDLE *Axis, LONG Value, LONG Ch_No );		/* Analog monitor output processing(Register access) *//* <S0E7> */
void	FnOutputAnalogMonitor( AXIS_HANDLE *Axis, SHORT Value, LONG Ch_No );	/* Analog monitor output processing(Register access) *//* <S0FD> */

#endif //__ANALOG_MONITOR_CALC_H

