/****************************************************************************
Description	: ANALOG&PULSE Task Interface Procedure
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
	  2013.09.20     created <S00C>

****************************************************************************/
#include "ScanN.h"

/****************************************************************************
 Network module initialization processing
****************************************************************************/
VOID OpiInitialize(void *axRsc)
{
	;
}

/****************************************************************************
 Network module initial parameter calculation processing
****************************************************************************/
VOID OpiCalcInitParam( VOID *axRsc )
{
	;
}

/****************************************************************************
 Network module device initialization processing
****************************************************************************/
VOID OpiInitDevice(void *axRsc, void *DeviceAddress)
{
	;
}

/****************************************************************************
 Network module device startup processing
****************************************************************************/
VOID OpiStartDevice( VOID *axRsc, void *DeviceAddress )
{
	;
}


/****************************************************************************
 Network moduleRoundProcessing
****************************************************************************/
BOOL OpiRoundProcedure(VOID *axRsc)
{
	return FALSE;
}


BOOL OpiIntSync(VOID *axRsc, BOOL IntStatus)
{
	return FALSE;
}


/****************************************************************************
 Network module cyclic input processing
****************************************************************************/
BOOL OpiInputProcess( VOID *axRsc, BOOL IntStatus )
{
	return FALSE;
}

/****************************************************************************
 Network module fixed cycle processing
****************************************************************************/
VOID OpiRunTimeScanProcess( void *axRsc, BOOL irqSts )
{
int ax_no;
AXIS_HANDLE *pAxRsc = (AXIS_HANDLE *)axRsc;
SV_OPTION_IF *pSvOptionIf = &(pAxRsc->SvControlIf->CmdOptionIf);

	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		NpiScanNMain( pSvOptionIf++ );
	}
}

/****************************************************************************
 Network module cyclic output processing
****************************************************************************/
BOOL OpiOutputProcess( VOID *axRsc, BOOL IntStatus )
{
	return FALSE;
}

/****************************************************************************
 Network moduleScanCProcessing
****************************************************************************/
VOID OpiScanCProcedure(VOID)
{
	;
}

/****************************************************************************
 Network stable state acquisition processing		<S1DB>
****************************************************************************/
BOOL OpiCheckStableState( void *axRsc )
{
//	return TRUE;			/* 2msCorrection for processing time measurement <S1FE> */
	return FALSE;
}

/****************************************************************************
 Network stable state acquisition processing		<S1DB>
****************************************************************************/
void OpiResetSyncExe( void *axRsc )
{
	return;
}
