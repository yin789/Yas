/**
 *******************************************************************************
 * @file  u2f_main.c
 * @brief program main for usb function
 * @attention 
 * @note 
 * 
 * $Rev:: 338                        $
 * $Date:: 2012-02-20 21:33:53 +0900#$
 *******************************************************************************
 */
/*
 *------------------------------------------------------------------------------
 * Notice 
 *  This is a sample program. 
 *  Renesas Electronics assumes no responsibility for any losses incurred. 
 *
 * Copyright(C) 2011 Renesas Electronics Corporation 
 * Copyright(C) 2011 Renesas Micro Systems Co., Ltd. 
 *------------------------------------------------------------------------------
 */

/*============================================================================*/
/* I N C L U D E                                                              */
/*============================================================================*/
#include "Basedef.h"
#include "errcodes.h"
//#include "arm.h"				//@		/*<S008>*/
//#include "nbpfahb32vic128.h"	//@		/*<S008>*/
#include "ARMPF_USB_FUNC.h"
#include "usb_f.h"
#include "u2f_desc.h"

/*===========================================================================*/
/* D E F I N E                                                               */
/*===========================================================================*/
// --- Max Packet Size ---
#define BULK_MAXPACKET_SIZE_HS	 512
#define BULK_MAXPACKET_SIZE_FS	  64
#define INT_MAXPACKET_SIZE_HS	1024
#define INT_MAXPACKET_SIZE_FS	  64


/*===========================================================================*/
/* S T R U C T                                                               */
/*===========================================================================*/

/*============================================================================*/
/* V A R I A B L E                                                            */
/*============================================================================*/
volatile UINT32			f_connect = U2F_PLUG_OUT;
volatile UINT32			f_setup = 0;
volatile UINT32			f_ep2_out = 0;
volatile UINT32			f_ep1_dmaend = 0;
volatile UINT32			f_ep2_dmaend = 0;

/*============================================================================*/
/* S T A T I C  F U N C                                                       */
/*============================================================================*/
static void set_maxpacket ( void );
static void clr_flag ( void );

/*============================================================================*/
/* P R O G R A M                                                              */
/*============================================================================*/
/**
 *******************************************************************************
  @brief USB Function Driver initialization
  @param  None
  @retval None
 *******************************************************************************
*/
void u2f_init( void )
{
	UINT32	state;

	clr_flag();												// Clear flag

//	init_u2f_control();

	//=========================================
	// EPC,PLL Reset release
	//=========================================
	ARMPF_EPCTR &= ~(D_SYS_EPC_RST | D_SYS_PLL_RST);				// Reset release

	//=========================================
	// System Bus-EPC Bridge Initial setting
	//=========================================
	ARMPF_SYSSCTR   = D_SYS_WAIT_MODE;							// Slave wait operation specification
	ARMPF_SYSMCTR   = 0x00000034;									// DMAArbitration during transfer: round robin
	ARMPF_SYSBINTEN = D_SYS_VBUS_INT;								// VBUSInterrupt enabling

	//=========================================
	// EPC Initial setting
	//=========================================
	// --- PLL_LOCKWaiting ---
	while ((ARMPF_EPCTR & D_SYS_PLL_LOCK) == 0) {					// PLL_LOCKWaiting
		// !!!!!!!! ToDo Processing at abnormality !!!!!!!!
	}

	// --- Interrupt setting ---
	ARMPF_USB_CONTROL   = D_USB_INT_SEL | D_USB_SOF_RCV;			// INTU2FEPCSet interrupt to level interrupt
															// SOF Use automatic recovery function when reception error occurs
	ARMPF_USB_INT_ENA   = D_USB_USB_RST_EN    |					// Interrupt enable: Bus reset
					D_USB_SPEED_MODE_EN |					//             : Bus speed change
					D_USB_EP0_EN        |					//             :EP0 Interrupt
					D_USB_EP1_EN        |					//             :EP1 Interrupt
					D_USB_EP2_EN;							//             :EP2 Interrupt

	// --- Max PacketSetting ---
	set_maxpacket();

	// --- EachEP Buffer clear ---
	ARMPF_EP0_CONTROL   = D_EP0_BCLR;								// EP0 : Buffer clear
//	ARMPF_EP1_CONTROL   = D_EPn_BCLR | D_EPn_AUTO;				// EP1 : Bulk-in,  Buffer clear, Automatic data transmission enabled
	ARMPF_EP1_CONTROL   = D_EPn_BCLR;								// EP1 : Bulk-in,  Buffer clear
	ARMPF_EP2_CONTROL   = D_EPn_BCLR | D_EPn_DIR0;				// EP2 : Bulk-out, Buffer clear

	// --- EachEP Interrupt enabling ---
	ARMPF_EP0_INT_ENA   = D_EP0_SETUP_EN;							// EP0 : Interrupt enable:SETUP_INT
	ARMPF_EP1_INT_ENA   = D_EPn_IN_END_EN;						// EP1 : Interrupt enable:EP1_IN_END_INT
	ARMPF_EP2_INT_ENA   = D_EPn_OUT_EN | D_EPn_OUT_END_EN;		// EP2 : Interrupt enable:EP2_OUT_INT, EP2_OUT_END

	// --- EachEP Enabling ---
	ARMPF_EP1_CONTROL  |= D_EPn_EN;								// EP1 : Enabling
	ARMPF_EP2_CONTROL  |= D_EPn_EN;								// EP2 : Enabling

	//=========================================
	// Plugin/out processing (Startup status confirmation)
	//=========================================
	if ((ARMPF_EPCTR & D_SYS_VBUS_LEVEL) == D_SYS_VBUS_LEVEL)	// VBUSLevel confirmation
	{
		// ====== Plugin processing ======
		state       =  ARMPF_USB_CONTROL;							//
		state      &= ~D_USB_CONNECTB;						// (Function PHY USB Signal Enable)
		state      |=  D_USB_PUE2;							// (D+SignalPull-up ) Increase by
		ARMPF_USB_CONTROL =  state;								// Reflect the above operation at the same time
		f_connect = U2F_PLUG_IN;
	}
	else
	{
		// ====== Plugout processing ======
		state       =  ARMPF_USB_CONTROL;							//
		state      |=  D_USB_CONNECTB;						// (Function PHY USB Disable the signal)
		state      &= ~D_USB_PUE2;							// (D+SignalPull-up Do not do this)
		ARMPF_USB_CONTROL =  state;								// Reflect the above operation at the same time
		f_connect = U2F_PLUG_OUT;
	}

	//=========================================
	// Interrupt setting
	//=========================================
//@	intc_set_handler( IDX_INTU2F, u2f_int_intu2f );			// Interrupt handler registration
//@	intc_set_prl( IDX_INTU2F, PRIORITY_LVL0 );				// Interrupt priority level	LEVEL0
//@	intc_clear_eiint( IDX_INTU2F );							// Clear interrupt
//@	intc_enable_eiint( IDX_INTU2F );						// Interrupt enabling
//@
//@	intc_set_handler( IDX_INTU2FEPC, u2f_int_intu2fepc );	// Interrupt handler registration
//@	intc_set_prl( IDX_INTU2FEPC, PRIORITY_LVL0 );			// Interrupt priority level	LEVEL0
//@	intc_clear_eiint( IDX_INTU2FEPC );						// Clear interrupt
//@	intc_enable_eiint( IDX_INTU2FEPC );						// Interrupt enabling
}


/**
 *******************************************************************************
  @brief INTU2F Interrupt processing
  @param  None
  @retval None
 *******************************************************************************
*/
void u2f_int_intu2f ( void )
{
	UINT32	state;

	//=========================================
	// Plugin/out processing (VBUS)
	//=========================================
	if ((ARMPF_SYSBINT & D_SYS_VBUS_INT) == D_SYS_VBUS_INT) {
		ARMPF_SYSBINT |= D_SYS_VBUS_INT;								// Clear interrupt source
		if ((ARMPF_EPCTR & D_SYS_VBUS_LEVEL) == D_SYS_VBUS_LEVEL) {	// VBUSLevel confirmation
			// ====== Plugin processing ======
			state       =  ARMPF_USB_CONTROL;							//
			state      &= ~D_USB_CONNECTB;						// (Function PHY USB Signal Enable)
			state      |=  D_USB_PUE2;							// (D+SignalPull-up ) Increase by
			ARMPF_USB_CONTROL =  state;								// Reflect the above operation at the same time
			f_connect = U2F_PLUG_IN;
		} else {												//
			// ====== Plugout processing ======
			state       =  ARMPF_USB_CONTROL;							//
			state      |=  D_USB_CONNECTB;						// (Function PHY USB Disable the signal)
			state      &= ~D_USB_PUE2;							// (D+SignalPull-up Do not do this)
			state      &= ~D_USB_DEFAULT;						// (endpoint0)
			state      &= ~D_USB_CONF;							// (endpoint0)
			ARMPF_USB_CONTROL =  state;								// Reflect the above operation at the same time
			f_connect = U2F_PLUG_OUT;
			clr_flag();											// Clear flag
		}
	}

//@	intc_clear_eiint( IDX_INTU2F );								// INTU2F Cause clear

}

/**
 *******************************************************************************
  @brief INTU2FEPC Interrupt processing
  @param  None
  @retval None
 *******************************************************************************
*/
void u2f_int_intu2fepc ( void )
{
	//=========================================
	// Bus reset Processing
	//=========================================
	if ((ARMPF_USB_INT_STA & D_USB_USB_RST_INT) == D_USB_USB_RST_INT) {
		ARMPF_USB_INT_STA = ~D_USB_USB_RST_INT;						// Clear interrupt source
		// --- Max PacketSetting ---
		set_maxpacket();
		// --- EP Buffer clear ---
		ARMPF_EP0_CONTROL   |= D_EP0_BCLR;							// EP0 : Buffer clear
		ARMPF_EP1_CONTROL   |= D_EPn_BCLR;							// EP1 : Buffer clear
		ARMPF_EP2_CONTROL   |= D_EPn_BCLR;							// EP2 : Buffer clear
		// --- Clear flag ---
		clr_flag();												// Clear flag
	}
	//=========================================
	// USBChange bus speed Processing
	//=========================================
	if ((ARMPF_USB_INT_STA & D_USB_SPEED_MODE_INT) == D_USB_SPEED_MODE_INT) {
		ARMPF_USB_INT_STA = ~D_USB_SPEED_MODE_INT;					// Clear interrupt source
		// --- Max PacketSetting ---
		set_maxpacket();
	}

	//=========================================
	// EP0 Processing
	//=========================================
	// ---  SETUPData reception ---
	if ((ARMPF_EP0_STATUS & D_EP0_SETUP_INT) == D_EP0_SETUP_INT) {
		ARMPF_EP0_STATUS = ~D_EP0_SETUP_INT;							// Clear interrupt source
		f_setup = 1;
	}
	//=========================================
	// EP1 Processing
	//=========================================
	// ---  Bulk-In DMATransfer end ---
	if ((ARMPF_EP1_STATUS & D_EPn_IN_END_INT) == D_EPn_IN_END_INT) {
		ARMPF_EP1_STATUS = ~D_EPn_IN_END_INT;						// Clear interrupt source
		f_ep1_dmaend = 1;
	}
	//=========================================
	// EP2 Processing
	//=========================================
	// ---  Bulk-Out Data reception ---
	if ((ARMPF_EP2_STATUS & D_EPn_OUT_INT) == D_EPn_OUT_INT) {
		ARMPF_EP2_STATUS = ~D_EPn_OUT_INT;							// Clear interrupt source
		f_ep2_out++;
	}
	// ---  Bulk-Out DMATransfer end ---
	if ((ARMPF_EP2_STATUS & D_EPn_OUT_END_INT) == D_EPn_OUT_END_INT) {
		ARMPF_EP2_STATUS = ~D_EPn_OUT_END_INT;						// Clear interrupt source
		f_ep2_dmaend = 1;
	}

//@	intc_clear_eiint( IDX_INTU2FEPC );							// INTU2FEPC Cause clear
}

/**
 ******************************************************************************
  @brief EP0 Transmission
  @param  pBuf      -- Transmission data storage area
  @param  size      -- Send size
  @retval 0or above -- Number of bytes transmitted
  @retval When the value is negative    -- Forced termination due to error occurrence
 ******************************************************************************
*/
INT u2f_write_EP0( VUCHAR* pBuf, INT size )
{
	INT	iSendLength		= size;				// Transmission data length
	INT	iRemainSize		= 0;				// Remaining data length

	UINT32	TmpData;
	UINT32	temp;
	INT		i;

	// Remaining data length
	iRemainSize = iSendLength;

	while (iRemainSize > 0) {
		//-----------------------------
		// Buffer empty check
		//-----------------------------
		while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY) {	//  Wait until the buffer is empty
			// --- Forcibly terminate when cable is disconnected ---
			if (f_connect == U2F_PLUG_OUT) {
				return ER_SYS;
			}
		}

		//-----------------------------
		// Write buffer
		//  (4byteunit, Max:64byte)
		//-----------------------------
		for (i = 0; ((i < 64) && (iRemainSize >= 4)); i += 4, iRemainSize -= 4) {
			// --- Data generation ---
			TmpData  = (UINT32)*pBuf++;
			TmpData |= (UINT32)*pBuf++ <<  8;
			TmpData |= (UINT32)*pBuf++ << 16;
			TmpData |= (UINT32)*pBuf++ << 24;
			// --- Write buffer ---
			ARMPF_EP0_WRITE = TmpData;
		}
		// --- Preparation for data transmission permission ---
		temp  = ARMPF_EP0_CONTROL;											// CONTROLRegister Read
		temp |= D_EP0_DEND;												// Transmission permission

		//-----------------------------
		// Fractional byte(3byte)Processing
		//-----------------------------
		if ((i < 64) && (iRemainSize > 0) && (iRemainSize < 4)) {
			// --- Fractional data generation ---
			TmpData = 0;
			for (i = 0; i < iRemainSize; i++) {
				TmpData |= (UINT32)*pBuf++ << (8*i);
			}
			// --- Write buffer ---
			ARMPF_EP0_WRITE = TmpData;
			// --- Data transmission permission ---
			temp |= ( ((UINT32)iRemainSize) << 5 ) & ( D_EP0_DW );			// Fractional byte setting
			iRemainSize = 0;
		}
		//-----------------------------
		// Data transmission permission
		//-----------------------------
		ARMPF_EP0_CONTROL = temp;												// CONTROLRegister Write
	}

	return size;
}


/**
 ******************************************************************************
  @brief EPn Receiving (PIOTransfer)
  @param  num       -- EPNumber
  @param  pBuf      -- Receive data storage area
  @param  size      -- Reception request size
  @retval 0or above -- Number of bytes received
  @retval When the value is negative    -- Forced termination due to error occurrence
 ******************************************************************************
*/

INT u2f_read_EPn( UINT num, VUCHAR* pBuf, INT size )
{
	INT i;
	INT cnt;

	INT	iReadLength;							// Receive data length
	INT	iRemainSize;							// Remaining data length
	UINT32	TmpData;

	// Remaining data length
	iRemainSize = size;
	cnt = 0;

	while (iRemainSize > 0) {
		// --- Receive byte count check ---
		iReadLength = *((VUINT32*)(EPn_LEN_DCNT_ADR + (num - 1) * 0x20)) & 0x3ff;

		// --- Receive wait timeout judgment(Receive wait stateSOF10times,) ---
		if (iReadLength == 0) {
			if ((ARMPF_USB_INT_STA & D_USB_SOF_INT) == D_USB_SOF_INT) {
				ARMPF_USB_INT_STA &= ~D_USB_SOF_INT;					// SOFReceive information clear
				if (cnt++ >= 10) {
					break;
				}
			}
		} else {
			cnt = 0;
		}

		// --- Data reception ---
		while (iReadLength > 0) {
			TmpData = *((VUINT32*)(EPn_READ_ADR + (num - 1) * 0x20));
			for (i = 0; ((i < 4) && (iReadLength > 0)); i++, iReadLength--, iRemainSize--) {
				*pBuf++ = (UINT8)(TmpData >> (8*i));
			}
		}
		// --- Forcibly terminate when cable is disconnected ---
		if (f_connect == U2F_PLUG_OUT) {
			return ER_SYS;
		}
	}

//	__DI();
	f_ep2_out--;
//	__EI();

	return (size - iRemainSize);
}

/**
 ******************************************************************************
  @brief EPn Receiving (DMATransfer)
  @param  num       -- EPNumber
  @param  pBuf      -- Receive data storage area
  @param  size      -- Reception size
  @retval 0or above -- Number of bytes transmitted
  @retval When the value is negative    -- Forced termination due to error occurrence
 ******************************************************************************
*/

INT u2f_read_EPn_DMA( UINT num, VUCHAR* pBuf, INT size )
{
	UINT32	MaxPacketSize;
//	UINT32	LastPacketSize;
	UINT32	TransPacketNum;

	//=================================
	// Calculate the number of transfer packets
	//=================================
	// --- EPn ---
	MaxPacketSize  = (*((VUINT32*)(EPn_PCKT_ADRS_ADR + (num - 1) * 0x20)) & 0x0000ffff);	// EPn
	// --- Calculate the number of transfer packets---
	TransPacketNum = size / MaxPacketSize;													// Calculate the number of transfer packets
	// --- Maximum number of packets limit & Calculate final packet transfer size ---
	if (TransPacketNum > 256) {																// Number of transfer packets256(H/WLimit),
		// --- Maximum number of packets exceeded ---
		TransPacketNum = 256;																//  -> Calculate the number of transfer packets256
//		LastPacketSize = MaxPacketSize;														//  -> Also transfer size of final packet0
	} else {																				// Number of transfer packets256or less,
		if ((TransPacketNum > 0) && ((size % MaxPacketSize) == 0)) {						// If full packet,
			// --- Full packet ---
//			LastPacketSize = MaxPacketSize;													//  -> Set final packet size to max packet
		} else {																			// If it is a short packet,
			// --- Short packet ---
//			LastPacketSize = ((size % MaxPacketSize) & 0x7fc);								//  -> Calculate final packet transfer size
		}
	}

	//=================================
	// BridgeSide DMATransfer initial setting
	//=================================
	// --- Max packet size setting ---
	*((VUINT32*)(EPnDCR2_ADR + (num - 1) * 0x10)) = MaxPacketSize;
	// --- Transfer destination address setting ---
	*((VUINT32*)(EPnTADR_ADR + (num - 1) * 0x10)) = (UINT32)pBuf;

	// --- Transfer direction Setting ---
	*((VUINT32*)(EPnDCR1_ADR + (num - 1) * 0x10)) = D_SYS_EPn_DIR0;

	//=================================
	// EPCSide DMATransfer initial setting
	//=================================
	*((VUINT32*)(EPn_DMA_CTRL_ADR + (num - 1) * 0x20)) = (D_EPn_STOP_MODE | 0x00000101);

	//=================================
	// Data reception completion
	//=================================
	while (f_ep2_out == 0) {
		// --- Forcibly terminate when cable is disconnected ---
		if (f_connect == U2F_PLUG_OUT) {
			return ER_SYS;
		}
	}
//	__DI();
	f_ep2_out--;
//	__EI();

	//=================================
	// BridgeSide DMATransfer start processing
	//=================================
	// --- Number of transfer packets, Transfer direction, DMATransfer permission Setting ---
	//  (Number of transferred packets256, the number of transferred packets is called when0x00)
	*((VUINT32*)(EPnDCR1_ADR + (num - 1) * 0x10)) = (((TransPacketNum & 0xff) << 16) | D_SYS_EPn_DIR0 | D_SYS_EPn_REQEN);

	//=================================
	// EPCSide DMATransfer start processing
	//=================================
	// --- Number of transfer packets Setting ---
	*((VUINT32*)(EPn_LEN_DCNT_ADR + (num - 1) * 0x20)) = (TransPacketNum << 16);
	// EPC Burst setting & DMATransfer permission
	*((VUINT32*)(EPn_DMA_CTRL_ADR + (num - 1) * 0x20)) |= (D_EPn_BURST_SET | D_EPn_DMA_EN);

	//=================================
	// DMAWait for transfer end
	//=================================
	while ((f_ep2_dmaend & 1) != 1) {
		// --- Forcibly terminate when cable is disconnected ---
		if (f_connect == U2F_PLUG_OUT) {
			return ER_SYS;
		}
	}
	f_ep2_dmaend = 0;
//	__DI();
	f_ep2_out = 0;
//	__EI();

	return size;
}


/**
 ******************************************************************************
  @brief EPn Transmission (PIOTransfer)
  @param  num       -- EPNumber
  @param  pBuf      -- Transmission data storage area
  @param  size      -- Send size
  @retval 0or above -- Number of bytes transmitted
  @retval When the value is negative    -- Forced termination due to error occurrence
 ******************************************************************************
*/

INT u2f_write_EPn( UINT num, VUCHAR* pBuf, INT size )
{
	INT	iRemainSize;						// Remaining data length

	UINT32	MaxPacketSize;
	UINT32	TmpData;
	UINT32	temp;
	INT		i;

	// Remaining data length
	iRemainSize = size;

	// --- Automatic data transmission disabled ---
	*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) &= ~D_EPn_AUTO;

	// --- EPn ---
	MaxPacketSize  = (*((VUINT32*)(EPn_PCKT_ADRS_ADR + (num - 1) * 0x20)) & 0x0000ffff);	// EPn

	while (iRemainSize > 0) {
		//-----------------------------
		// Buffer empty check
		//-----------------------------
		while((*((VUINT32*)(EPn_STATUS_ADR + (num - 1) * 0x20)) & D_EPn_IN_EMPTY) != D_EPn_IN_EMPTY) {	//  Wait until the buffer is empty
			// --- Forcibly terminate when cable is disconnected ---
			if (f_connect == U2F_PLUG_OUT) {
				return ER_SYS;
			}
		}

		//-----------------------------
		// Write buffer
		//  (4byteunit, Max:Max Packet Size)
		//-----------------------------
		for (i = 0; ((i < MaxPacketSize) && (iRemainSize >= 4)); i += 4, iRemainSize -= 4) {
			// --- Data generation ---
			TmpData  = (UINT32)*pBuf++;
			TmpData |= (UINT32)*pBuf++ <<  8;
			TmpData |= (UINT32)*pBuf++ << 16;
			TmpData |= (UINT32)*pBuf++ << 24;
			// --- Write buffer ---
			*((VUINT32*)(EPn_WRITE_ADR + (num - 1) * 0x20)) = TmpData;
		}
		// --- Preparation for data transmission permission ---
		temp  = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));											// CONTROLRegister Read
		temp |= D_EPn_DEND;												// Transmission permission

		//-----------------------------
		// Fractional byte(3byte)Processing
		//-----------------------------
		if ((i < MaxPacketSize) && (iRemainSize > 0) && (iRemainSize < 4)) {
			// --- Fractional data generation ---
			TmpData = 0;
			for (i = 0; i < iRemainSize; i++) {
				TmpData |= (UINT32)*pBuf++ << (8*i);
			}
			// --- Write buffer ---
			*((VUINT32*)(EPn_WRITE_ADR + (num - 1) * 0x20)) = TmpData;
			// --- Data transmission permission ---
			temp |= ( ((UINT32)iRemainSize) << 5 ) & ( D_EP0_DW );			// Fractional byte setting
			iRemainSize = 0;
		}
		//-----------------------------
		// Data transmission permission
		//-----------------------------
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;										// CONTROLRegister Write
	}

	return size;
}

/**
 ******************************************************************************
  @brief EPn Transmission (DMATransfer)
  @param  num       -- EPNumber
  @param  pBuf      -- Transmission data storage area
  @param  size      -- Send size
  @retval 0or above -- Number of bytes transmitted
  @retval When the value is negative    -- Forced termination due to error occurrence
 ******************************************************************************
*/

INT u2f_write_EPn_DMA( UINT num, VUCHAR* pBuf, INT size )
{
	UINT32	MaxPacketSize, LastPacketSize;
	UINT32	TransPacketNum;

	// --- Automatic data transmission enabled ---
	*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_AUTO;

	//=================================
	// Calculate the number of transfer packets
	//=================================
	// --- EPn ---
	MaxPacketSize  = (*((VUINT32*)(EPn_PCKT_ADRS_ADR + (num - 1) * 0x20)) & 0x0000ffff);	// EPn
	// --- Calculate the number of transfer packets---
	TransPacketNum = size / MaxPacketSize;													// Calculate the number of transfer packets
	// --- Maximum number of packets limit & Calculate final packet transfer size ---
	if (TransPacketNum > 256)
	{																// Number of transfer packets256(H/WLimit),
		// --- Maximum number of packets exceeded ---
		TransPacketNum = 256;																//  -> Calculate the number of transfer packets256
		LastPacketSize = MaxPacketSize;														//  -> Also transfer size of final packet0
	}
	else
	{																				// Number of transfer packets256or less,
		if ((TransPacketNum > 0) && ((size % MaxPacketSize) == 0))
		{// If full packet,
			// --- Full packet ---
			LastPacketSize = MaxPacketSize;													//  -> Set final packet size to max packet
		}
		else
		{// If it is a short packet,
			// --- Short packet ---
			LastPacketSize = ((size % MaxPacketSize) & 0x7fc);								//  -> Calculate final packet transfer size
		}
	}

	//=================================
	// BridgeSide DMATransfer initial setting
	//=================================
	// --- Max packet & Final packet transfer size setting ---
	*((VUINT32*)(EPnDCR2_ADR + (num - 1) * 0x10)) = ((LastPacketSize << 16) | MaxPacketSize);
	// --- Transfer destination address setting ---
	*((VUINT32*)(EPnTADR_ADR + (num - 1) * 0x10)) = (UINT32)pBuf;

	// --- Number of transfer packets, Transfer direction, DMATransfer permission Setting ---
	//  (Number of transferred packets256, the number of transferred packets is called when0x00)
	*((VUINT32*)(EPnDCR1_ADR + (num - 1) * 0x10)) = (((TransPacketNum & 0xff) << 16) | D_SYS_EPn_REQEN);

	//=================================
	// EPCSide DMATransfer initial setting (transfer start)
	//=================================
	// --- Number of transfer packets Setting ---
	*((VUINT32*)(EPn_LEN_DCNT_ADR + (num - 1) * 0x20)) = (TransPacketNum << 16);
	// EPC Burst setting & DMATransfer permission
	*((VUINT32*)(EPn_DMA_CTRL_ADR + (num - 1) * 0x20)) = (D_EPn_BURST_SET | D_EPn_DMA_EN | 0x00000101);

	//=================================
	// DMAWait for transfer end
	//=================================
	while ((f_ep1_dmaend & 1) != 1)
	{
		// --- Forcibly terminate when cable is disconnected ---
		if
		(f_connect == U2F_PLUG_OUT)
		{
			return ER_SYS;
		}
	}
	f_ep1_dmaend = 0;

	return size;
}

/**
 ******************************************************************************
  @brief EP0 Stall response
  @param  None
  @retval None
 ******************************************************************************
*/

void u2f_set_stall_EP0( void )
{
	ARMPF_EP0_CONTROL |= D_EP0_STL;
}

/**
 ******************************************************************************
  @brief EPn Stall response
  @param  num       -- EPNumber
  @retval None
 ******************************************************************************
*/

void u2f_set_stall_EPn( UINT num )
{
	// --- IN/OUT Judgment ---
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) != D_EPn_DIR0 ) {
		// --- BULK IN ---
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_ISTL;
	} else {
		// --- BULK OUT ---
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= (D_EPn_OSTL_EN | D_EPn_OSTL);
	}
}

/**
 ******************************************************************************
  @brief EPn Stall release
  @param  num       -- EPNumber
  @retval None
 ******************************************************************************
*/

void u2f_clear_stall_EPn( UINT num )
{
	UINT32	temp;

	// --- IN/OUT Judgment ---
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) != D_EPn_DIR0 ) {
		// --- BULK IN ---
		temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
		temp &= ~(D_EPn_ISTL);											// EPn : INToken STALLCancellation
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;

	} else {
		// --- BULK OUT ---
		temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
		temp |= D_EPn_OSTL_EN;											// EPn_OSTL Enable writing to bit
		temp &= ~(D_EPn_OSTL);											// EPn : OUT/PINGToken STALLCancellation
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;
	}
}

/**
 ******************************************************************************
  @brief EPn Buffer clear
  @param  num       -- EPNumber
  @retval None
 ******************************************************************************
*/

void u2f_clear_buf_EPn( UINT num )
{
	*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_BCLR;			// EPn : Buffer clear
}

/**
 ******************************************************************************
  @brief EPn PIDClear
  @param  num       -- EPNumber
  @retval None
 ******************************************************************************
*/

void u2f_clear_PID_EPn( UINT num )
{
	// --- IN/OUT Judgment ---
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) != D_EPn_DIR0 ) {
		// --- BULK IN ---
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_IPIDCLR;		// EPn : TransmissionPIDClear
	} else {
		// --- BULK OUT ---
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_OPIDCLR;		// EPn : ReceivingPIDClear
	}
}

/**
 ******************************************************************************
  @brief EP0 NAKResponse
  @param  None
  @retval None
 ******************************************************************************
*/

void u2f_set_nak_EP0( void )
{
	UINT32 temp;

	temp  = ARMPF_EP0_CONTROL;
	temp |= D_EP0_INAK_EN;
	temp |= (UINT32)(D_EP0_INAK | D_EP0_ONAK);
	ARMPF_EP0_CONTROL = temp;
}

/**
 ******************************************************************************
  @brief EP0 NAKCancellation
  @param  None
  @retval None
 ******************************************************************************
*/

void u2f_clear_nak_EP0( void )
{
	UINT32 temp;

	temp  = ARMPF_EP0_CONTROL;
	temp |= D_EP0_INAK_EN;
	temp &= ~(UINT32)(D_EP0_INAK | D_EP0_ONAK);
	ARMPF_EP0_CONTROL = temp;
}


/**
 ******************************************************************************
  @brief EP0 NULLTransmission
  @param  None
  @retval None
 ******************************************************************************
*/

ER_RET u2f_send_null_EP0( void )
{
	//-----------------------------
	// Buffer empty check
	//-----------------------------
	while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY) {	//  Wait until the buffer is empty
		// --- Forcibly terminate when cable is disconnected ---
		if (f_connect == U2F_PLUG_OUT) {
			return ER_SYS;
		}
	}

	//-----------------------------
	// Data transmission permission
	//-----------------------------
	ARMPF_EP0_CONTROL |= D_EP0_DEND;									// Transmission permission

	return ER_OK;
}

/**
 ******************************************************************************
  @brief USBConnection status confirmation
  @param  None
  @retval U2F_PLUG_OUT -- USB Hostwhen not connected
  @retval U2F_PLUG_IN  -- USB Hostwhen connected
 ******************************************************************************
*/

UINT u2f_check_plug_inout( void )
{
	return f_connect;
}

/**
 ******************************************************************************
  @brief Max Packet Size Setting
  @param  None
  @retval None
 ******************************************************************************
*/

static void set_maxpacket ( void )
{
	UINT32	bulk_size, interrupt_size;

	//---------------------------------
	// USBCheck bus speed
	//---------------------------------
	if ( (ARMPF_USB_STATUS & D_USB_SPEED_MODE) == D_USB_SPEED_MODE ) {
		// High Speed
		bulk_size      = BULK_MAXPACKET_SIZE_HS;
		interrupt_size = INT_MAXPACKET_SIZE_HS;
	} else {
		// Full Speed
		bulk_size      = BULK_MAXPACKET_SIZE_FS;
		interrupt_size = INT_MAXPACKET_SIZE_FS;
	}

	//---------------------------------
	// Max Packet Size Setting
	//---------------------------------
														// EPn : BaseAdr  MaxPkt(HS/FS This driver sets the standard maximum value.)
														// EP0 : 0x0000        64 byte x 2Side
	ARMPF_EP1_PCKT_ADRS = 0x00200000 | bulk_size;				// EP1 : 0x0020    512/64 byte x 2Side	--   Bulk    Transfer --
	ARMPF_EP2_PCKT_ADRS = 0x01200000 | bulk_size;				// EP2 : 0x0120    512/64 byte x 2Side	--   Bulk    Transfer --
	ARMPF_EP3_PCKT_ADRS = 0x02200000 | interrupt_size;		// EP3 : 0x0220   1024/64 byte x 1Side	-- Interrupt Transfer --
}

/**
 ******************************************************************************
  @brief Clear flag
  @param  None
  @retval None
 ******************************************************************************
*/
static void clr_flag ( void )
{
	f_setup = 0;
	f_ep2_out = 0;
	f_ep1_dmaend = 0;
	f_ep2_dmaend = 0;
}
