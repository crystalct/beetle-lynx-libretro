//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Mikey chip emulation class                                               //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates all of the Mikey hardware with the exception of the  //
// CPU and memory selector. Update() does most of the work and does screen  //
// DMA and counter updates, it also schecules in which cycle the next timer //
// update will occur so that the CSystem->Update() doesnt have to call it   //
// every cycle, massive speedup but big complexity headache.                //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define MIKIE_CPP

#include "system.h"
#include "mikie.h"
#include "lynxdef.h"


void CMikie::BlowOut(void)
{
	C6502_REGS regs;
	mSystem.GetRegs(regs);
	//sprintf(addr,"Runtime Error - System Halted\nCMikie::Poke() - Read/Write to counter clocks at PC=$%04x.",regs.PC);
	//gError->Warning(addr);
	gSystemHalt=true;
}


CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	mpDisplayCurrent=NULL;
	mpRamPointer=NULL;

	mUART_CABLE_PRESENT=false;
	mpUART_TX_CALLBACK=NULL;

	int loop;
	for(loop=0;loop<16;loop++) mPalette[loop].Index=loop;
	for(loop=0;loop<4096;loop++) mColourMap[loop]=0;

	Reset();
}

CMikie::~CMikie()
{
}


void CMikie::Reset(void)
{
	mAudioInputComparator=false;	// Initialises to unknown
	mDisplayAddress=0x00;			// Initialises to unknown
	mLynxLine=0;
	mLynxLineDMACounter=0;
	mLynxAddr=0;

	mTimerStatusFlags=0x00;		// Initialises to ZERO, i.e No IRQ's
	mTimerInterruptMask=0x00;

	mpRamPointer=mSystem.GetRamPointer();	// Fetch pointer to system RAM

	mTIM_0_BKUP=0;
	mTIM_0_ENABLE_RELOAD=0;
	mTIM_0_ENABLE_COUNT=0;
	mTIM_0_LINKING=0;
	mTIM_0_CURRENT=0;
	mTIM_0_TIMER_DONE=0;
	mTIM_0_LAST_CLOCK=0;
	mTIM_0_BORROW_IN=0;
	mTIM_0_BORROW_OUT=0;
	mTIM_0_LAST_LINK_CARRY=0;
	mTIM_0_LAST_COUNT=0;

	mTIM_1_BKUP=0;
	mTIM_1_ENABLE_RELOAD=0;
	mTIM_1_ENABLE_COUNT=0;
	mTIM_1_LINKING=0;
	mTIM_1_CURRENT=0;
	mTIM_1_TIMER_DONE=0;
	mTIM_1_LAST_CLOCK=0;
	mTIM_1_BORROW_IN=0;
	mTIM_1_BORROW_OUT=0;
	mTIM_1_LAST_LINK_CARRY=0;
	mTIM_1_LAST_COUNT=0;

	mTIM_2_BKUP=0;
	mTIM_2_ENABLE_RELOAD=0;
	mTIM_2_ENABLE_COUNT=0;
	mTIM_2_LINKING=0;
	mTIM_2_CURRENT=0;
	mTIM_2_TIMER_DONE=0;
	mTIM_2_LAST_CLOCK=0;
	mTIM_2_BORROW_IN=0;
	mTIM_2_BORROW_OUT=0;
	mTIM_2_LAST_LINK_CARRY=0;
	mTIM_2_LAST_COUNT=0;

	mTIM_3_BKUP=0;
	mTIM_3_ENABLE_RELOAD=0;
	mTIM_3_ENABLE_COUNT=0;
	mTIM_3_LINKING=0;
	mTIM_3_CURRENT=0;
	mTIM_3_TIMER_DONE=0;
	mTIM_3_LAST_CLOCK=0;
	mTIM_3_BORROW_IN=0;
	mTIM_3_BORROW_OUT=0;
	mTIM_3_LAST_LINK_CARRY=0;
	mTIM_3_LAST_COUNT=0;

	mTIM_4_BKUP=0;
	mTIM_4_ENABLE_RELOAD=0;
	mTIM_4_ENABLE_COUNT=0;
	mTIM_4_LINKING=0;
	mTIM_4_CURRENT=0;
	mTIM_4_TIMER_DONE=0;
	mTIM_4_LAST_CLOCK=0;
	mTIM_4_BORROW_IN=0;
	mTIM_4_BORROW_OUT=0;
	mTIM_4_LAST_LINK_CARRY=0;
	mTIM_4_LAST_COUNT=0;

	mTIM_5_BKUP=0;
	mTIM_5_ENABLE_RELOAD=0;
	mTIM_5_ENABLE_COUNT=0;
	mTIM_5_LINKING=0;
	mTIM_5_CURRENT=0;
	mTIM_5_TIMER_DONE=0;
	mTIM_5_LAST_CLOCK=0;
	mTIM_5_BORROW_IN=0;
	mTIM_5_BORROW_OUT=0;
	mTIM_5_LAST_LINK_CARRY=0;
	mTIM_5_LAST_COUNT=0;

	mTIM_6_BKUP=0;
	mTIM_6_ENABLE_RELOAD=0;
	mTIM_6_ENABLE_COUNT=0;
	mTIM_6_LINKING=0;
	mTIM_6_CURRENT=0;
	mTIM_6_TIMER_DONE=0;
	mTIM_6_LAST_CLOCK=0;
	mTIM_6_BORROW_IN=0;
	mTIM_6_BORROW_OUT=0;
	mTIM_6_LAST_LINK_CARRY=0;
	mTIM_6_LAST_COUNT=0;

	mTIM_7_BKUP=0;
	mTIM_7_ENABLE_RELOAD=0;
	mTIM_7_ENABLE_COUNT=0;
	mTIM_7_LINKING=0;
	mTIM_7_CURRENT=0;
	mTIM_7_TIMER_DONE=0;
	mTIM_7_LAST_CLOCK=0;
	mTIM_7_BORROW_IN=0;
	mTIM_7_BORROW_OUT=0;
	mTIM_7_LAST_LINK_CARRY=0;
	mTIM_7_LAST_COUNT=0;

	for(int y = 0; y < 4; y++)
	{
         mAUDIO_BKUP[y]=0;
         mAUDIO_ENABLE_RELOAD[y]=0;
         mAUDIO_ENABLE_COUNT[y]=0;
         mAUDIO_LINKING[y]=0;
         mAUDIO_CURRENT[y]=0;
         mAUDIO_TIMER_DONE[y]=0;
         mAUDIO_LAST_CLOCK[y]=0;
         mAUDIO_BORROW_IN[y]=0;
         mAUDIO_BORROW_OUT[y]=0;
         mAUDIO_LAST_LINK_CARRY[y]=0;
         mAUDIO_LAST_COUNT[y]=0;
         mAUDIO_VOLUME[y]=0;
         mAUDIO_INTEGRATE_ENABLE[y]=0;
         mAUDIO_WAVESHAPER[y]=0;

	 mAUDIO_OUTPUT[y] = 0;
	}
	mSTEREO=0xff;	// xored! All channels enabled
	mPAN=0x00;      // all channels panning OFF
    mAUDIO_ATTEN[0]=0xff; // Full volume
    mAUDIO_ATTEN[1]=0xff;
    mAUDIO_ATTEN[2]=0xff;
    mAUDIO_ATTEN[3]=0xff;
    
	// Start with an empty palette

	for(int loop=0;loop<16;loop++)
	{
		mPalette[loop].Index=loop;
	}

	// Initialise IODAT register

	mIODAT=0x00;
	mIODIR=0x00;
	mIODAT_REST_SIGNAL=0x00;

	//
	// Initialise display control register vars
	//
	mDISPCTL_DMAEnable=false;
	mDISPCTL_Flip=false;
	mDISPCTL_FourColour=0;
	mDISPCTL_Colour=0;

	//
	// Initialise the UART variables
	//
	mUART_RX_IRQ_ENABLE=0;
	mUART_TX_IRQ_ENABLE=0;

	mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
	mUART_RX_COUNTDOWN=UART_RX_INACTIVE;

	mUART_Rx_input_ptr=0;
	mUART_Rx_output_ptr=0;
	mUART_Rx_waiting=0;
	mUART_Rx_framing_error=0;
	mUART_Rx_overun_error=0;

	mUART_SENDBREAK=0;
	mUART_TX_DATA=0;
	mUART_RX_DATA=0;
	mUART_RX_READY=0;

	mUART_PARITY_ENABLE=0;
	mUART_PARITY_EVEN=0;
}

uint32 CMikie::GetLfsrNext(uint32 current)
{
	// The table is built thus:
	//	Bits 0-11  LFSR					(12 Bits)
	//  Bits 12-20 Feedback switches	(9 Bits)
	//     (Order = 7,0,1,2,3,4,5,10,11)
	//  Order is mangled to make peek/poke easier as
	//  bit 7 is in a seperate register
	//
	// Total 21 bits = 2MWords @ 4 Bytes/Word = 8MB !!!!!
	//
	// If the index is a combination of Current LFSR+Feedback the
	// table will give the next value.

	uint32 switches,lfsr,next,swloop,result;
	static const uint32 switchbits[9]={7,0,1,2,3,4,5,10,11};

	switches=current>>12;
	lfsr=current&0xfff;
	result=0;
	for(swloop=0;swloop<9;swloop++)
	{
		if((switches>>swloop)&0x001) result^=(lfsr>>switchbits[swloop])&0x001;
	}
	result=(result)?0:1;
	next=(switches<<12)|((lfsr<<1)&0xffe)|result;
	return next;
}

void CMikie::PresetForHomebrew(void)
{
	//
	// After all of that nice timer init we'll start timers running as some homebrew
	// i.e LR.O doesn't bother to setup the timers

	mTIM_0_BKUP=0x9e;
	mTIM_0_ENABLE_RELOAD=true;
	mTIM_0_ENABLE_COUNT=true;

	mTIM_2_BKUP=0x68;
	mTIM_2_ENABLE_RELOAD=true;
	mTIM_2_ENABLE_COUNT=true;
	mTIM_2_LINKING=7;

	mDISPCTL_DMAEnable=true;
	mDISPCTL_Flip=false;
	mDISPCTL_FourColour=0;
	mDISPCTL_Colour=true;
}

void CMikie::ComLynxCable(int status)
{
	mUART_CABLE_PRESENT=status;
}

void CMikie::ComLynxRxData(int data)
{
	// Copy over the data
	if(mUART_Rx_waiting<UART_MAX_RX_QUEUE)
	{
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

		// Receive the byte
		mUART_Rx_input_queue[mUART_Rx_input_ptr]=data;
		mUART_Rx_input_ptr = (mUART_Rx_input_ptr + 1) % UART_MAX_RX_QUEUE;
		mUART_Rx_waiting++;
	}
}

void CMikie::ComLynxTxLoopback(int data)
{

	if(mUART_Rx_waiting<UART_MAX_RX_QUEUE)
	{
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

		// Receive the byte - INSERT into front of queue
		mUART_Rx_output_ptr = (mUART_Rx_output_ptr - 1) % UART_MAX_RX_QUEUE;
		mUART_Rx_input_queue[mUART_Rx_output_ptr]=data;
		mUART_Rx_waiting++;
	}
}

void CMikie::ComLynxTxCallback(void (*function)(int data,uint32 objref),uint32 objref)
{
	mpUART_TX_CALLBACK=function;
	mUART_TX_CALLBACK_OBJECT=objref;
}


void CMikie::DisplaySetAttributes(int32 bpp)
{
	mpDisplayCurrent=NULL;

	//
	// Calculate the colour lookup tabes for the relevant mode
	//
	TPALETTE Spot;

	for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
	{
      uint8 r = ((Spot.Colours.Red * 15) + 30);
      uint8 g = ((Spot.Colours.Green * 15) + 30);
      uint8 b = ((Spot.Colours.Blue * 15) + 30);

	  switch (bpp)
	  {
	  case 16:
#if defined(ABGR1555)
		  mColourMap[Spot.Index] = MAKECOLOR_15_1(r, g, b, 0);
#else
		  mColourMap[Spot.Index] = MAKECOLOR_16(r, g, b, 0);
#endif
		  break;

	  case 32:
		  mColourMap[Spot.Index] = MAKECOLOR_32(r, g, b, 0);
		  break;

	  default:
		  break;
	  }
	}
}

void CMikie::CopyLineSurface(int32 bpp)
{
	if(mpDisplayCurrentLine > 102)
	{
	 printf("Lynx Line Overflow: %d\n", mpDisplayCurrentLine);
	 return;
	}

	switch (bpp)
	{
	case 16:
	{
		uint16 *bitmap_tmp = mpDisplayCurrent->pixels + mpDisplayCurrentLine * mpDisplayCurrent->pitch;
		for (uint32 loop = 0; loop < SCREEN_WIDTH / 2; loop++)
		{
			uint32 source = mpRamPointer[(uint16)mLynxAddr];
			if (mDISPCTL_Flip)
			{
				mLynxAddr--;
				*bitmap_tmp = mColourMap[mPalette[source & 0x0f].Index];
				bitmap_tmp++;
				*bitmap_tmp = mColourMap[mPalette[source >> 4].Index];
				bitmap_tmp++;
			}
			else
			{
				mLynxAddr++;
				*bitmap_tmp = mColourMap[mPalette[source >> 4].Index];
				bitmap_tmp++;
				*bitmap_tmp = mColourMap[mPalette[source & 0x0f].Index];
				bitmap_tmp++;
			}
		}
		break;
	}
	case 32:
	{
		uint32 *bitmap_tmp = (uint32 *)mpDisplayCurrent->pixels + mpDisplayCurrentLine * mpDisplayCurrent->pitch;
		for (uint32 loop = 0; loop < SCREEN_WIDTH / 2; loop++)
		{
			uint32 source = mpRamPointer[(uint16)mLynxAddr];
			if (mDISPCTL_Flip)
			{
				mLynxAddr--;
				*bitmap_tmp = mColourMap[mPalette[source & 0x0f].Index];
				bitmap_tmp++;
				*bitmap_tmp = mColourMap[mPalette[source >> 4].Index];
				bitmap_tmp++;
			}
			else
			{
				mLynxAddr++;
				*bitmap_tmp = mColourMap[mPalette[source >> 4].Index];
				bitmap_tmp++;
				*bitmap_tmp = mColourMap[mPalette[source & 0x0f].Index];
				bitmap_tmp++;
			}
		}
	}
	}
}

uint32 CMikie::DisplayRenderLine(void)
{
	uint32 work_done=0;

	if(!mpDisplayCurrent) return 0;
	if(!mDISPCTL_DMAEnable) return 0;
//	if(mLynxLine&0x80000000) return 0;

	// Set the timer interrupt flag
	if(mTimerInterruptMask&0x01)
		mTimerStatusFlags|=0x01;

// Logic says it should be 101 but testing on an actual lynx shows the rest
// period is between lines 102,101,100 with the new line being latched at
// the beginning of count==99 hence the code below !!

	// Emulate REST signal
	if(mLynxLine==mTIM_2_BKUP-2 || mLynxLine==mTIM_2_BKUP-3 || mLynxLine==mTIM_2_BKUP-4) mIODAT_REST_SIGNAL=true; else mIODAT_REST_SIGNAL=false;

	if(mLynxLine==(mTIM_2_BKUP-3))
	{
		if(mDISPCTL_Flip)
		{
			mLynxAddr=mDisplayAddress&0xfffc;
			mLynxAddr+=3;
		}
		else
		{
			mLynxAddr=mDisplayAddress&0xfffc;
		}
		// Trigger line rending to start
		mLynxLineDMACounter=102;
	}

	// Decrement line counter logic
	if(mLynxLine) mLynxLine--;

	// Do 102 lines, nothing more, less is OK.
	if(mLynxLineDMACounter)
	{
		mLynxLineDMACounter--;

		// Cycle hit for a 80 RAM access in rendering a line
		work_done+=(80+80)*DMA_RDWR_CYC;

		// Mikie screen DMA can only see the system RAM....
		// (Step through bitmap, line at a time)

		// Assign the temporary pointer;
		if(!mpSkipFrame)
		{
	        CopyLineSurface(mpDisplayCurrent->bpp);

			if(mpDisplayCurrentLine < 102)
			 LynxLineDrawn[mpDisplayCurrentLine] = true;

			mpDisplayCurrentLine++;
		}
	}
	return work_done;
}

uint32 CMikie::DisplayEndOfFrame(void)
{
	// Stop any further line rendering
	mLynxLineDMACounter=0;
	mLynxLine=mTIM_2_BKUP;

	// Set the timer status flag
	if(mTimerInterruptMask&0x04)
		mTimerStatusFlags|=0x04;

	mpDisplayCurrent = NULL;
	return 0;
}

// Peek/Poke memory handlers

void CMikie::Poke(uint32 addr,uint8 data)
{
	/* Sound register area */
	if(addr >= 0xFD20 && addr <= 0xFD3F)
	{
	 int which = (addr - 0xFD20) >> 3; // Each channel gets 8 ports/registers
	 switch(addr & 0x7)
	 {
                case (AUD0VOL&0x7):
                        mAUDIO_VOLUME[which]=(int8)data;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0SHFTFB&0x7):
                        mAUDIO_WAVESHAPER[which]&=0x001fff;
                        mAUDIO_WAVESHAPER[which]|=(uint32)data<<13;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0OUTVAL&0x7):
                        mAUDIO_OUTPUT[which]=data;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0L8SHFT&0x7):
                        mAUDIO_WAVESHAPER[which]&=0x1fff00;
                        mAUDIO_WAVESHAPER[which]|=data;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0TBACK&0x7):
                        mAUDIO_BKUP[which]=data;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0CTL&0x7):
                        mAUDIO_ENABLE_RELOAD[which]=data&0x10;
                        mAUDIO_ENABLE_COUNT[which]=data&0x08;
                        mAUDIO_LINKING[which]=data&0x07;
                        mAUDIO_INTEGRATE_ENABLE[which]=data&0x20;
                        if(data&0x40) mAUDIO_TIMER_DONE[which]=0;
                        mAUDIO_WAVESHAPER[which]&=0x1fefff;
                        mAUDIO_WAVESHAPER[which]|=(data&0x80)?0x001000:0x000000;
                        if(data&0x48)
                        {
                                mAUDIO_LAST_COUNT[which]=gSystemCycleCount;
                                gNextTimerEvent=gSystemCycleCount;
                        }
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0COUNT&0x7):
                        mAUDIO_CURRENT[which]=data;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
                case (AUD0MISC&0x7):
                        mAUDIO_WAVESHAPER[which]&=0x1ff0ff;
                        mAUDIO_WAVESHAPER[which]|=(data&0xf0)<<4;
                        mAUDIO_BORROW_IN[which]=data&0x02;
                        mAUDIO_BORROW_OUT[which]=data&0x01;
                        mAUDIO_LAST_CLOCK[which]=data&0x04;
                        CombobulateSound(gSystemCycleCount - startTS);
                        break;
	 }
	}
	else switch(addr&0xff)
	{
		case (TIM0BKUP&0xff): 
			mTIM_0_BKUP=data;
			break;
		case (TIM1BKUP&0xff): 
			mTIM_1_BKUP=data;
			break;
		case (TIM2BKUP&0xff): 
			mTIM_2_BKUP=data;
			break;
		case (TIM3BKUP&0xff): 
			mTIM_3_BKUP=data;
			break;
		case (TIM4BKUP&0xff): 
			mTIM_4_BKUP=data;
			break;
		case (TIM5BKUP&0xff): 
			mTIM_5_BKUP=data;
			break;
		case (TIM6BKUP&0xff): 
			mTIM_6_BKUP=data;
			break;
		case (TIM7BKUP&0xff):
			mTIM_7_BKUP=data;
			break;


		case (TIM0CTLA&0xff):
			mTimerInterruptMask&=(0x01^0xff);
			mTimerInterruptMask|=(data&0x80)?0x01:0x00;
			mTIM_0_ENABLE_RELOAD=data&0x10;
			mTIM_0_ENABLE_COUNT=data&0x08;
			mTIM_0_LINKING=data&0x07;
			if(data&0x40) mTIM_0_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_0_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM1CTLA&0xff): 
			mTimerInterruptMask&=(0x02^0xff);
			mTimerInterruptMask|=(data&0x80)?0x02:0x00;
			mTIM_1_ENABLE_RELOAD=data&0x10;
			mTIM_1_ENABLE_COUNT=data&0x08;
			mTIM_1_LINKING=data&0x07;
			if(data&0x40) mTIM_1_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_1_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM2CTLA&0xff): 
			mTimerInterruptMask&=(0x04^0xff);
			mTimerInterruptMask|=(data&0x80)?0x04:0x00;
			mTIM_2_ENABLE_RELOAD=data&0x10;
			mTIM_2_ENABLE_COUNT=data&0x08;
			mTIM_2_LINKING=data&0x07;
			if(data&0x40) mTIM_2_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_2_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM3CTLA&0xff): 
			mTimerInterruptMask&=(0x08^0xff);
			mTimerInterruptMask|=(data&0x80)?0x08:0x00;
			mTIM_3_ENABLE_RELOAD=data&0x10;
			mTIM_3_ENABLE_COUNT=data&0x08;
			mTIM_3_LINKING=data&0x07;
			if(data&0x40) mTIM_3_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_3_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM4CTLA&0xff): 
			// Timer 4 can never generate interrupts as its timer output is used
			// to drive the UART clock generator
			mTIM_4_ENABLE_RELOAD=data&0x10;
			mTIM_4_ENABLE_COUNT=data&0x08;
			mTIM_4_LINKING=data&0x07;
			if(data&0x40) mTIM_4_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_4_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM5CTLA&0xff): 
			mTimerInterruptMask&=(0x20^0xff);
			mTimerInterruptMask|=(data&0x80)?0x20:0x00;
			mTIM_5_ENABLE_RELOAD=data&0x10;
			mTIM_5_ENABLE_COUNT=data&0x08;
			mTIM_5_LINKING=data&0x07;
			if(data&0x40) mTIM_5_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_5_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM6CTLA&0xff): 
			mTimerInterruptMask&=(0x40^0xff);
			mTimerInterruptMask|=(data&0x80)?0x40:0x00;
			mTIM_6_ENABLE_RELOAD=data&0x10;
			mTIM_6_ENABLE_COUNT=data&0x08;
			mTIM_6_LINKING=data&0x07;
			if(data&0x40) mTIM_6_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_6_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;
		case (TIM7CTLA&0xff):
			mTimerInterruptMask&=(0x80^0xff);
			mTimerInterruptMask|=(data&0x80)?0x80:0x00;
			mTIM_7_ENABLE_RELOAD=data&0x10;
			mTIM_7_ENABLE_COUNT=data&0x08;
			mTIM_7_LINKING=data&0x07;
			if(data&0x40) mTIM_7_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_7_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			break;


		case (TIM0CNT&0xff): 
			mTIM_0_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM1CNT&0xff): 
			mTIM_1_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM2CNT&0xff): 
			mTIM_2_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM3CNT&0xff): 
			mTIM_3_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM4CNT&0xff): 
			mTIM_4_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM5CNT&0xff): 
			mTIM_5_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM6CNT&0xff): 
			mTIM_6_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;
		case (TIM7CNT&0xff): 
			mTIM_7_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			break;

		case (TIM0CTLB&0xff): 
			mTIM_0_TIMER_DONE=data&0x08;
			mTIM_0_LAST_CLOCK=data&0x04;
			mTIM_0_BORROW_IN=data&0x02;
			mTIM_0_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM1CTLB&0xff): 
			mTIM_1_TIMER_DONE=data&0x08;
			mTIM_1_LAST_CLOCK=data&0x04;
			mTIM_1_BORROW_IN=data&0x02;
			mTIM_1_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM2CTLB&0xff): 
			mTIM_2_TIMER_DONE=data&0x08;
			mTIM_2_LAST_CLOCK=data&0x04;
			mTIM_2_BORROW_IN=data&0x02;
			mTIM_2_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM3CTLB&0xff): 
			mTIM_3_TIMER_DONE=data&0x08;
			mTIM_3_LAST_CLOCK=data&0x04;
			mTIM_3_BORROW_IN=data&0x02;
			mTIM_3_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM4CTLB&0xff): 
			mTIM_4_TIMER_DONE=data&0x08;
			mTIM_4_LAST_CLOCK=data&0x04;
			mTIM_4_BORROW_IN=data&0x02;
			mTIM_4_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM5CTLB&0xff): 
			mTIM_5_TIMER_DONE=data&0x08;
			mTIM_5_LAST_CLOCK=data&0x04;
			mTIM_5_BORROW_IN=data&0x02;
			mTIM_5_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM6CTLB&0xff): 
			mTIM_6_TIMER_DONE=data&0x08;
			mTIM_6_LAST_CLOCK=data&0x04;
			mTIM_6_BORROW_IN=data&0x02;
			mTIM_6_BORROW_OUT=data&0x01;
//			BlowOut();
			break;
		case (TIM7CTLB&0xff):
			mTIM_7_TIMER_DONE=data&0x08;
			mTIM_7_LAST_CLOCK=data&0x04;
			mTIM_7_BORROW_IN=data&0x02;
			mTIM_7_BORROW_OUT=data&0x01;
//			BlowOut();
			break;

		case (ATTEN_A&0xff):
            mAUDIO_ATTEN[0] = data;
            CombobulateSound(gSystemCycleCount - startTS);
            break;
		case (ATTEN_B&0xff):
            mAUDIO_ATTEN[1] = data;
            CombobulateSound(gSystemCycleCount - startTS);
            break;
		case (ATTEN_C&0xff):
            mAUDIO_ATTEN[2] = data;
            CombobulateSound(gSystemCycleCount - startTS);
            break;
		case (ATTEN_D&0xff):
            mAUDIO_ATTEN[3] = data;
            CombobulateSound(gSystemCycleCount - startTS);
            break;
		case (MPAN&0xff):
			mPAN = data;
			CombobulateSound(gSystemCycleCount - startTS);
			break;

		case (MSTEREO&0xff):
			data^=0xff;
			mSTEREO=data;
			CombobulateSound(gSystemCycleCount - startTS);
			break;

		case (INTRST&0xff):
			data^=0xff;
			mTimerStatusFlags&=data;
			gNextTimerEvent=gSystemCycleCount;
			break;

		case (INTSET&0xff): 
			mTimerStatusFlags|=data;
			gNextTimerEvent=gSystemCycleCount;
			break;

		case (SYSCTL1&0xff):
			if(!(data&0x02))
			{
				C6502_REGS regs;
				mSystem.GetRegs(regs);
				MDFN_printf("Runtime Alert - System Halted\nCMikie::Poke(SYSCTL1) - Lynx power down occurred at PC=$%04x.\nResetting system.\n",regs.PC);
				mSystem.Reset();
				gSystemHalt=true;
			}
			mSystem.CartAddressStrobe((data&0x01)?true:false);
			break;

		case (MIKEYSREV&0xff):
			break;

		case (IODIR&0xff):
			mIODIR=data;
			break;

		case (IODAT&0xff):
			mIODAT=data;
			mSystem.CartAddressData((mIODAT&0x02)?true:false);
			// Enable cart writes to BANK1 on AUDIN if AUDIN is set to output
			if(mIODIR&0x10) mSystem.mCart->mWriteEnableBank1=(mIODAT&0x10)?true:false;
			break;

		case (SERCTL&0xff): 
			mUART_TX_IRQ_ENABLE=(data&0x80)?true:false;
			mUART_RX_IRQ_ENABLE=(data&0x40)?true:false;
			mUART_PARITY_ENABLE=(data&0x10)?true:false;
			mUART_SENDBREAK=data&0x02;
			mUART_PARITY_EVEN=data&0x01;

			// Reset all errors if required
			if(data&0x08)
			{
				mUART_Rx_overun_error=0;
				mUART_Rx_framing_error=0;
			}

			if(mUART_SENDBREAK)
			{
				// Trigger send break, it will self sustain as long as sendbreak is set
				mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
				// Loop back what we transmitted
				ComLynxTxLoopback(UART_BREAK_CODE);
			}
			break;

		case (SERDAT&0xff):
			//
			// Fake transmission, set counter to be decremented by Timer 4
			//
			// ComLynx only has one output pin, hence Rx & Tx are shorted
			// therefore any transmitted data will loopback
			//
			mUART_TX_DATA=data;
			// Calculate Parity data
			if(mUART_PARITY_ENABLE)
			{
				// Calc parity value
				// Leave at zero !!
			}
			else
			{
				// If disabled then the PAREVEN bit is sent
				if(mUART_PARITY_EVEN) data|=0x0100;
			}
			// Set countdown to transmission
			mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
			// Loop back what we transmitted
			ComLynxTxLoopback(mUART_TX_DATA);
			break;

		case (SDONEACK&0xff):
			break;
		case (CPUSLEEP&0xff):
			gSuzieDoneTime = gSystemCycleCount+mSystem.PaintSprites();
			SetCPUSleep();
			break;

		case (DISPCTL&0xff): 
			{
				TDISPCTL tmp;
				tmp.Byte=data;
				mDISPCTL_DMAEnable=tmp.Bits.DMAEnable;
				mDISPCTL_Flip=tmp.Bits.Flip;
				mDISPCTL_FourColour=tmp.Bits.FourColour;
				mDISPCTL_Colour=tmp.Bits.Colour;
			}
			break;
		case (PBKUP&0xff): 
			break;

		case (DISPADRL&0xff):
			mDisplayAddress&=0xff00;
			mDisplayAddress+=data;
			break;

		case (DISPADRH&0xff): 
			mDisplayAddress&=0x00ff;
			mDisplayAddress+=(data<<8);
			break;

		case (Mtest0&0xff): 
		case (Mtest1&0xff): 
			// Test registers are unimplemented
			// lets hope no programs use them.
		case (Mtest2&0xff): 
			// Test registers are unimplemented
			// lets hope no programs use them.
			//gError->Warning("CMikie::Poke() - Write to MTEST2");
			break;

		case (GREEN0&0xff): 
		case (GREEN1&0xff): 
		case (GREEN2&0xff): 
		case (GREEN3&0xff): 
		case (GREEN4&0xff): 
		case (GREEN5&0xff): 
		case (GREEN6&0xff): 
		case (GREEN7&0xff): 
		case (GREEN8&0xff): 
		case (GREEN9&0xff): 
		case (GREENA&0xff): 
		case (GREENB&0xff): 
		case (GREENC&0xff): 
		case (GREEND&0xff): 
		case (GREENE&0xff): 
		case (GREENF&0xff):
			mPalette[addr&0x0f].Colours.Green=data&0x0f;
			break;

		case (BLUERED0&0xff): 
		case (BLUERED1&0xff): 
		case (BLUERED2&0xff): 
		case (BLUERED3&0xff): 
		case (BLUERED4&0xff): 
		case (BLUERED5&0xff): 
		case (BLUERED6&0xff): 
		case (BLUERED7&0xff): 
		case (BLUERED8&0xff): 
		case (BLUERED9&0xff): 
		case (BLUEREDA&0xff): 
		case (BLUEREDB&0xff): 
		case (BLUEREDC&0xff): 
		case (BLUEREDD&0xff): 
		case (BLUEREDE&0xff): 
		case (BLUEREDF&0xff): 
			mPalette[addr&0x0f].Colours.Blue=(data&0xf0)>>4;
			mPalette[addr&0x0f].Colours.Red=data&0x0f;
			break;

// Errors on read only register accesses

		case (MAGRDY0&0xff): 
		case (MAGRDY1&0xff): 
		case (AUDIN&0xff): 
		case (MIKEYHREV&0xff): 
			break;

// Errors on illegal location accesses

		default:
			break;
	}
}



uint8 CMikie::Peek(uint32 addr)
{
        /* Sound register area */
        if(addr >= 0xFD20 && addr <= 0xFD3F)
        {
         int which = (addr - 0xFD20) >> 3; // Each channel gets 8 ports/registers
         switch(addr & 0x7)
         {
                case (AUD0VOL&0x7):
                        return (uint8)mAUDIO_VOLUME[which];
                        break;
                case (AUD0SHFTFB&0x7):
                        return (uint8)((mAUDIO_WAVESHAPER[which]>>13)&0xff);
                        break;
                case (AUD0OUTVAL&0x7):
                        return (uint8)mAUDIO_OUTPUT[which];
                        break;
                case (AUD0L8SHFT&0x7):
                        return (uint8)(mAUDIO_WAVESHAPER[which]&0xff);
                        break;
                case (AUD0TBACK&0x7):
                        return (uint8)mAUDIO_BKUP[which];
                        break;
                case (AUD0CTL&0x7):
                        {
                                uint8 retval=0;
                                retval|=(mAUDIO_INTEGRATE_ENABLE[which])?0x20:0x00;
                                retval|=(mAUDIO_ENABLE_RELOAD[which])?0x10:0x00;
                                retval|=(mAUDIO_ENABLE_COUNT[which])?0x08:0x00;
                                retval|=(mAUDIO_WAVESHAPER[which]&0x001000)?0x80:0x00;
                                retval|=mAUDIO_LINKING[which];
                                return retval;
                        }
                        break;
                case (AUD0COUNT&0x7):
                        return (uint8)mAUDIO_CURRENT[which];
                        break;
                case (AUD0MISC&0x7):
                        {
                                uint8 retval=0;
                                retval|=(mAUDIO_BORROW_OUT[which])?0x01:0x00;
                                retval|=(mAUDIO_BORROW_IN[which])?0x02:0x00;
                                retval|=(mAUDIO_LAST_CLOCK[which])?0x08:0x00;
                                retval|=(mAUDIO_WAVESHAPER[which]>>4)&0xf0;
                                return retval;
                        }
                        break;
	 }
	}
	else switch(addr&0xff)
	{

// Timer control registers

		case (TIM0BKUP&0xff): 
			return (uint8)mTIM_0_BKUP;
		case (TIM1BKUP&0xff): 
			return (uint8)mTIM_1_BKUP;
		case (TIM2BKUP&0xff): 
			return (uint8)mTIM_2_BKUP;
		case (TIM3BKUP&0xff): 
			return (uint8)mTIM_3_BKUP;
		case (TIM4BKUP&0xff): 
			return (uint8)mTIM_4_BKUP;
		case (TIM5BKUP&0xff): 
			return (uint8)mTIM_5_BKUP;
		case (TIM6BKUP&0xff): 
			return (uint8)mTIM_6_BKUP;
		case (TIM7BKUP&0xff):
			return (uint8)mTIM_7_BKUP;

		case (TIM0CTLA&0xff):
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x01)?0x80:0x00;
				retval|=(mTIM_0_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_0_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_0_LINKING;
				return retval;
			}
			break;
		case (TIM1CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x02)?0x80:0x00;
				retval|=(mTIM_1_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_1_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_1_LINKING;
				return retval;
			}
			break;
		case (TIM2CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x04)?0x80:0x00;
				retval|=(mTIM_2_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_2_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_2_LINKING;
				return retval;
			}
			break;
		case (TIM3CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x08)?0x80:0x00;
				retval|=(mTIM_3_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_3_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_3_LINKING;
				return retval;
			}
			break;
		case (TIM4CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x10)?0x80:0x00;
				retval|=(mTIM_4_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_4_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_4_LINKING;
				return retval;
			}
			break;
		case (TIM5CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x20)?0x80:0x00;
				retval|=(mTIM_5_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_5_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_5_LINKING;
				return retval;
			}
			break;
		case (TIM6CTLA&0xff): 
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x40)?0x80:0x00;
				retval|=(mTIM_6_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_6_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_6_LINKING;
				return retval;
			}
			break;
		case (TIM7CTLA&0xff):
			{
				uint8 retval=0;
				retval|=(mTimerInterruptMask&0x80)?0x80:0x00;
				retval|=(mTIM_7_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_7_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_7_LINKING;
				return retval;
			}
			break;

		case (TIM0CNT&0xff): 
			Update();
			return (uint8)mTIM_0_CURRENT;
		case (TIM1CNT&0xff): 
			Update();
			return (uint8)mTIM_1_CURRENT;
		case (TIM2CNT&0xff): 
			Update();
			return (uint8)mTIM_2_CURRENT;
		case (TIM3CNT&0xff): 
			Update();
			return (uint8)mTIM_3_CURRENT;
		case (TIM4CNT&0xff): 
			Update();
			return (uint8)mTIM_4_CURRENT;
		case (TIM5CNT&0xff): 
			Update();
			return (uint8)mTIM_5_CURRENT;
		case (TIM6CNT&0xff): 
			Update();
			return (uint8)mTIM_6_CURRENT;
		case (TIM7CNT&0xff): 
			Update();
			return (uint8)mTIM_7_CURRENT;

		case (TIM0CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_0_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_0_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_0_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_0_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM1CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_1_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_1_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_1_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_1_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM2CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_2_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_2_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_2_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_2_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM3CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_3_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_3_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_3_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_3_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM4CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_4_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_4_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_4_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_4_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM5CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_5_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_5_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_5_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_5_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM6CTLB&0xff): 
			{
				uint8 retval=0;
				retval|=(mTIM_6_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_6_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_6_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_6_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;
		case (TIM7CTLB&0xff):
			{
				uint8 retval=0;
				retval|=(mTIM_7_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_7_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_7_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_7_BORROW_OUT)?0x01:0x00;
				return retval;
			}
//			BlowOut();
			break;

		// Extra audio control registers

        case (ATTEN_A&0xff): 
            return (uint8) mAUDIO_ATTEN[0];
        case (ATTEN_B&0xff): 
            return (uint8) mAUDIO_ATTEN[1];
        case (ATTEN_C&0xff):
            return (uint8) mAUDIO_ATTEN[2];
        case (ATTEN_D&0xff): 
            return (uint8) mAUDIO_ATTEN[3];
        case (MPAN&0xff):
            return (uint8) mPAN;

		case (MSTEREO&0xff):
			return (uint8) mSTEREO^0xff;

// Miscellaneous registers

		case (SERCTL&0xff): 
			{
				uint32 retval=0;
				retval|=(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)?0xA0:0x00;	// Indicate TxDone & TxAllDone
				retval|=(mUART_RX_READY)?0x40:0x00;							// Indicate Rx data ready
				retval|=(mUART_Rx_overun_error)?0x08:0x0;					// Framing error
				retval|=(mUART_Rx_framing_error)?0x04:0x00;					// Rx overrun
				retval|=(mUART_RX_DATA&UART_BREAK_CODE)?0x02:0x00;			// Indicate break received
				retval|=(mUART_RX_DATA&0x0100)?0x01:0x00;					// Add parity bit
				return (uint8)retval;
			}
			break;

		case (SERDAT&0xff):
			mUART_RX_READY=0;
			return (uint8)(mUART_RX_DATA&0xff);

		case (IODAT&0xff): 
			{
				uint32 retval=0;
				retval|=(mIODIR&0x10)?mIODAT&0x10:0x10;									// IODIR  = output bit : input high (eeprom write done)
				retval|=(mIODIR&0x08)?(((mIODAT&0x08)&&mIODAT_REST_SIGNAL)?0x00:0x08):0x00;									// REST   = output bit : input low
				retval|=(mIODIR&0x04)?mIODAT&0x04:((mUART_CABLE_PRESENT)?0x04:0x00);	// NOEXP  = output bit : input low
				retval|=(mIODIR&0x02)?mIODAT&0x02:0x00;									// CARTAD = output bit : input low
				retval|=(mIODIR&0x01)?mIODAT&0x01:0x01;									// EXTPW  = output bit : input high (Power connected)
				return (uint8)retval;
			}
			break;

		case (INTRST&0xff):
		case (INTSET&0xff):
			return (uint8)mTimerStatusFlags;

		case (MAGRDY0&0xff): 
		case (MAGRDY1&0xff): 
			return 0x00;

		case (AUDIN&0xff):
			return 0x80;

		case (MIKEYHREV&0xff): 
			return 0x01;

// Pallette registers

		case (GREEN0&0xff): 
		case (GREEN1&0xff): 
		case (GREEN2&0xff): 
		case (GREEN3&0xff): 
		case (GREEN4&0xff): 
		case (GREEN5&0xff): 
		case (GREEN6&0xff): 
		case (GREEN7&0xff): 
		case (GREEN8&0xff): 
		case (GREEN9&0xff): 
		case (GREENA&0xff): 
		case (GREENB&0xff): 
		case (GREENC&0xff): 
		case (GREEND&0xff): 
		case (GREENE&0xff): 
		case (GREENF&0xff):
			return mPalette[addr&0x0f].Colours.Green;

		case (BLUERED0&0xff): 
		case (BLUERED1&0xff): 
		case (BLUERED2&0xff): 
		case (BLUERED3&0xff): 
		case (BLUERED4&0xff): 
		case (BLUERED5&0xff): 
		case (BLUERED6&0xff): 
		case (BLUERED7&0xff): 
		case (BLUERED8&0xff): 
		case (BLUERED9&0xff): 
		case (BLUEREDA&0xff): 
		case (BLUEREDB&0xff): 
		case (BLUEREDC&0xff): 
		case (BLUEREDD&0xff): 
		case (BLUEREDE&0xff): 
		case (BLUEREDF&0xff):
			return (mPalette[addr&0x0f].Colours.Red | (mPalette[addr&0x0f].Colours.Blue<<4));

// Errors on write only register accesses

		// For easier debugging

		case (DISPADRL&0xff): 
			return (uint8)(mDisplayAddress&0xff);
		case (DISPADRH&0xff): 
			return (uint8)(mDisplayAddress>>8)&0xff;

		case (DISPCTL&0xff): 
		case (SYSCTL1&0xff):
		case (MIKEYSREV&0xff): 
		case (IODIR&0xff): 
		case (SDONEACK&0xff): 
		case (CPUSLEEP&0xff): 
		case (PBKUP&0xff): 
		case (Mtest0&0xff): 
		case (Mtest1&0xff): 
		case (Mtest2&0xff): 
			break;

// Register to let programs know handy is running

		case (0xfd97&0xff):
			// return 0x42;
			break;

// Errors on illegal location accesses

		default:
			break;
	}
	return 0xff;
}


int CMikie::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT MikieRegs[] =
 {
        SFVAR(mDisplayAddress),
        SFVAR(mAudioInputComparator),
        SFVAR(mTimerStatusFlags),
        SFVAR(mTimerInterruptMask),

	// TPALETTE is a union'd 32-bit integer
	SFARRAY32N((uint32 *)mPalette, 16, "mPalette"),
        SFVAR(mIODAT),
        SFVAR(mIODAT_REST_SIGNAL),
        SFVAR(mIODIR),

        SFVAR(mDISPCTL_DMAEnable),
        SFVAR(mDISPCTL_Flip),
        SFVAR(mDISPCTL_FourColour),
        SFVAR(mDISPCTL_Colour),

        SFVAR(mTIM_0_BKUP),
        SFVAR(mTIM_0_ENABLE_RELOAD),
        SFVAR(mTIM_0_ENABLE_COUNT),
        SFVAR(mTIM_0_LINKING),
        SFVAR(mTIM_0_CURRENT),
        SFVAR(mTIM_0_TIMER_DONE),
        SFVAR(mTIM_0_LAST_CLOCK),
        SFVAR(mTIM_0_BORROW_IN),
        SFVAR(mTIM_0_BORROW_OUT),
        SFVAR(mTIM_0_LAST_LINK_CARRY),
        SFVAR(mTIM_0_LAST_COUNT),

        SFVAR(mTIM_1_BKUP),
        SFVAR(mTIM_1_ENABLE_RELOAD),
        SFVAR(mTIM_1_ENABLE_COUNT),
        SFVAR(mTIM_1_LINKING),
        SFVAR(mTIM_1_CURRENT),
        SFVAR(mTIM_1_TIMER_DONE),
        SFVAR(mTIM_1_LAST_CLOCK),
        SFVAR(mTIM_1_BORROW_IN),
        SFVAR(mTIM_1_BORROW_OUT),
        SFVAR(mTIM_1_LAST_LINK_CARRY),
        SFVAR(mTIM_1_LAST_COUNT),

        SFVAR(mTIM_2_BKUP),
        SFVAR(mTIM_2_ENABLE_RELOAD),
        SFVAR(mTIM_2_ENABLE_COUNT),
        SFVAR(mTIM_2_LINKING),
        SFVAR(mTIM_2_CURRENT),
        SFVAR(mTIM_2_TIMER_DONE),
        SFVAR(mTIM_2_LAST_CLOCK),
        SFVAR(mTIM_2_BORROW_IN),
        SFVAR(mTIM_2_BORROW_OUT),
        SFVAR(mTIM_2_LAST_LINK_CARRY),
        SFVAR(mTIM_2_LAST_COUNT),

        SFVAR(mTIM_3_BKUP),
        SFVAR(mTIM_3_ENABLE_RELOAD),
        SFVAR(mTIM_3_ENABLE_COUNT),
        SFVAR(mTIM_3_LINKING),
        SFVAR(mTIM_3_CURRENT),
        SFVAR(mTIM_3_TIMER_DONE),
        SFVAR(mTIM_3_LAST_CLOCK),
        SFVAR(mTIM_3_BORROW_IN),
        SFVAR(mTIM_3_BORROW_OUT),
        SFVAR(mTIM_3_LAST_LINK_CARRY),
        SFVAR(mTIM_3_LAST_COUNT),

        SFVAR(mTIM_4_BKUP),
        SFVAR(mTIM_4_ENABLE_RELOAD),
        SFVAR(mTIM_4_ENABLE_COUNT),
        SFVAR(mTIM_4_LINKING),
        SFVAR(mTIM_4_CURRENT),
        SFVAR(mTIM_4_TIMER_DONE),
        SFVAR(mTIM_4_LAST_CLOCK),
        SFVAR(mTIM_4_BORROW_IN),
        SFVAR(mTIM_4_BORROW_OUT),
        SFVAR(mTIM_4_LAST_LINK_CARRY),
        SFVAR(mTIM_4_LAST_COUNT),

        SFVAR(mTIM_5_BKUP),
        SFVAR(mTIM_5_ENABLE_RELOAD),
        SFVAR(mTIM_5_ENABLE_COUNT),
        SFVAR(mTIM_5_LINKING),
        SFVAR(mTIM_5_CURRENT),
        SFVAR(mTIM_5_TIMER_DONE),
        SFVAR(mTIM_5_LAST_CLOCK),
        SFVAR(mTIM_5_BORROW_IN),
        SFVAR(mTIM_5_BORROW_OUT),
        SFVAR(mTIM_5_LAST_LINK_CARRY),
        SFVAR(mTIM_5_LAST_COUNT),

        SFVAR(mTIM_6_BKUP),
        SFVAR(mTIM_6_ENABLE_RELOAD),
        SFVAR(mTIM_6_ENABLE_COUNT),
        SFVAR(mTIM_6_LINKING),
        SFVAR(mTIM_6_CURRENT),
        SFVAR(mTIM_6_TIMER_DONE),
        SFVAR(mTIM_6_LAST_CLOCK),
        SFVAR(mTIM_6_BORROW_IN),
        SFVAR(mTIM_6_BORROW_OUT),
        SFVAR(mTIM_6_LAST_LINK_CARRY),
        SFVAR(mTIM_6_LAST_COUNT),


        SFVAR(mTIM_7_BKUP),
        SFVAR(mTIM_7_ENABLE_RELOAD),
        SFVAR(mTIM_7_ENABLE_COUNT),
        SFVAR(mTIM_7_LINKING),
        SFVAR(mTIM_7_CURRENT),
        SFVAR(mTIM_7_TIMER_DONE),
        SFVAR(mTIM_7_LAST_CLOCK),
        SFVAR(mTIM_7_BORROW_IN),
        SFVAR(mTIM_7_BORROW_OUT),
        SFVAR(mTIM_7_LAST_LINK_CARRY),
        SFVAR(mTIM_7_LAST_COUNT),

        SFVAR(mAUDIO_BKUP[0]),
        SFVAR(mAUDIO_ENABLE_RELOAD[0]),
        SFVAR(mAUDIO_ENABLE_COUNT[0]),
        SFVAR(mAUDIO_LINKING[0]),
        SFVAR(mAUDIO_CURRENT[0]),
        SFVAR(mAUDIO_TIMER_DONE[0]),
        SFVAR(mAUDIO_LAST_CLOCK[0]),
        SFVAR(mAUDIO_BORROW_IN[0]),
        SFVAR(mAUDIO_BORROW_OUT[0]),
        SFVAR(mAUDIO_LAST_LINK_CARRY[0]),
        SFVAR(mAUDIO_LAST_COUNT[0]),
        SFVAR(mAUDIO_VOLUME[0]),
        SFVAR(mAUDIO_OUTPUT[0]),
        SFVAR(mAUDIO_INTEGRATE_ENABLE[0]),
        SFVAR(mAUDIO_WAVESHAPER[0]),


        SFVAR(mAUDIO_BKUP[1]),
        SFVAR(mAUDIO_ENABLE_RELOAD[1]),
        SFVAR(mAUDIO_ENABLE_COUNT[1]),
        SFVAR(mAUDIO_LINKING[1]),
        SFVAR(mAUDIO_CURRENT[1]),
        SFVAR(mAUDIO_TIMER_DONE[1]),
        SFVAR(mAUDIO_LAST_CLOCK[1]),
        SFVAR(mAUDIO_BORROW_IN[1]),
        SFVAR(mAUDIO_BORROW_OUT[1]),
        SFVAR(mAUDIO_LAST_LINK_CARRY[1]),
        SFVAR(mAUDIO_LAST_COUNT[1]),
        SFVAR(mAUDIO_VOLUME[1]),
        SFVAR(mAUDIO_OUTPUT[1]),
        SFVAR(mAUDIO_INTEGRATE_ENABLE[1]),
        SFVAR(mAUDIO_WAVESHAPER[1]),


        SFVAR(mAUDIO_BKUP[2]),
        SFVAR(mAUDIO_ENABLE_RELOAD[2]),
        SFVAR(mAUDIO_ENABLE_COUNT[2]),
        SFVAR(mAUDIO_LINKING[2]),
        SFVAR(mAUDIO_CURRENT[2]),
        SFVAR(mAUDIO_TIMER_DONE[2]),
        SFVAR(mAUDIO_LAST_CLOCK[2]),
        SFVAR(mAUDIO_BORROW_IN[2]),
        SFVAR(mAUDIO_BORROW_OUT[2]),
        SFVAR(mAUDIO_LAST_LINK_CARRY[2]),
        SFVAR(mAUDIO_LAST_COUNT[2]),
        SFVAR(mAUDIO_VOLUME[2]),
        SFVAR(mAUDIO_OUTPUT[2]),
        SFVAR(mAUDIO_INTEGRATE_ENABLE[2]),
        SFVAR(mAUDIO_WAVESHAPER[2]),

        SFVAR(mAUDIO_BKUP[3]),
        SFVAR(mAUDIO_ENABLE_RELOAD[3]),
        SFVAR(mAUDIO_ENABLE_COUNT[3]),
        SFVAR(mAUDIO_LINKING[3]),
        SFVAR(mAUDIO_CURRENT[3]),
        SFVAR(mAUDIO_TIMER_DONE[3]),
        SFVAR(mAUDIO_LAST_CLOCK[3]),
        SFVAR(mAUDIO_BORROW_IN[3]),
        SFVAR(mAUDIO_BORROW_OUT[3]),
        SFVAR(mAUDIO_LAST_LINK_CARRY[3]),
        SFVAR(mAUDIO_LAST_COUNT[3]),
        SFVAR(mAUDIO_VOLUME[3]),
        SFVAR(mAUDIO_OUTPUT[3]),
        SFVAR(mAUDIO_INTEGRATE_ENABLE[3]),
        SFVAR(mAUDIO_WAVESHAPER[3]),

        // Lynx 2 extra audio registers
        SFVAR(mSTEREO),
        SFVAR(mPAN),
        SFVAR(mAUDIO_ATTEN[0]),
        SFVAR(mAUDIO_ATTEN[1]),
        SFVAR(mAUDIO_ATTEN[2]),
        SFVAR(mAUDIO_ATTEN[3]),

        //
        // Serial related variables
        //
        SFVAR(mUART_RX_IRQ_ENABLE),
        SFVAR(mUART_TX_IRQ_ENABLE),

        SFVAR(mUART_TX_COUNTDOWN),
        SFVAR(mUART_RX_COUNTDOWN),

        SFVAR(mUART_SENDBREAK),
        SFVAR(mUART_TX_DATA),
        SFVAR(mUART_RX_DATA),
        SFVAR(mUART_RX_READY),

        SFVAR(mUART_PARITY_ENABLE),
        SFVAR(mUART_PARITY_EVEN),
	SFEND
	};

	int ret = MDFNSS_StateAction(sm, load, data_only, MikieRegs, "MIKY", false);

	if(load)
	{



	}
        return ret;
}

void CMikie::CombobulateSound(uint32 teatime)
{
                                int cur_lsample = 0;
                                int cur_rsample = 0;
                                static int last_lsample = 0;
                                static int last_rsample = 0;
                                int x;

                                teatime >>= 2;
                                for(x = 0; x < 4; x++){
                                   /// Assumption (seems there is no documentation for the Attenuation registers)
                                   /// a) they are linear from $0 to $f - checked!
                                   /// b) an attenuation of $0 is equal to channel OFF (bits in mSTEREO not set) - checked!
                                   /// c) an attenuation of $f is NOT equal to no attenuation (bits in PAN not set), $10 would be - checked!
                                   /// These assumptions can only checked with an oszilloscope... - done
                                   /// the values stored in mSTEREO are bit-inverted ...
                                   /// mSTEREO was found to be set like that already (why?), but unused
               
                                 if(mSTEREO & (0x10 << x))
                                 {
                                    if(mPAN & (0x10 << x))
                                      cur_lsample += (mAUDIO_OUTPUT[x]*(mAUDIO_ATTEN[x]&0xF0))/(16*16); /// NOT /15*16 see remark above
                                    else
                                      cur_lsample += mAUDIO_OUTPUT[x];
                                 }
                                 if(mSTEREO & (0x01 << x))
                                 {
                                    if(mPAN & (0x01 << x))
                                      cur_rsample += (mAUDIO_OUTPUT[x]*(mAUDIO_ATTEN[x]&0x0F))/16; /// NOT /15 see remark above
                                    else
                                      cur_rsample += mAUDIO_OUTPUT[x];
                                 }
                                }
                                if(cur_lsample != last_lsample){
                                  miksynth.offset_inline(teatime, cur_lsample - last_lsample, mikbuf.left());
                                  last_lsample = cur_lsample;
                                }
                                if(cur_rsample != last_rsample){
                                  miksynth.offset_inline(teatime, cur_rsample - last_rsample, mikbuf.right());
                                  last_rsample = cur_rsample;
                                }
}

void CMikie::Update(void)
{
			int32 divide;
			int32 decval;
			uint32 tmp;
			uint32 mikie_work_done=0;

			//
			// To stop problems with cycle count wrap we will check and then correct the
			// cycle counter.
			//


			if(gSystemCycleCount>0xf0000000)
			{
				gSystemCycleCount-=0x80000000;
				mTIM_0_LAST_COUNT-=0x80000000;
				mTIM_1_LAST_COUNT-=0x80000000;
				mTIM_2_LAST_COUNT-=0x80000000;
				mTIM_3_LAST_COUNT-=0x80000000;
				mTIM_4_LAST_COUNT-=0x80000000;
				mTIM_5_LAST_COUNT-=0x80000000;
				mTIM_6_LAST_COUNT-=0x80000000;
				mTIM_7_LAST_COUNT-=0x80000000;
				mAUDIO_LAST_COUNT[0]-=0x80000000;
				mAUDIO_LAST_COUNT[1]-=0x80000000;
				mAUDIO_LAST_COUNT[2]-=0x80000000;
				mAUDIO_LAST_COUNT[3]-=0x80000000;
				startTS -= 0x80000000;
				// Only correct if sleep is active
				if(gSuzieDoneTime)
				{
					gSuzieDoneTime-=0x80000000;
				}
			}

			gNextTimerEvent=0xffffffff;

			if(gSuzieDoneTime)
			{
				if(gSystemCycleCount >= gSuzieDoneTime)
				{
					ClearCPUSleep();
					gSuzieDoneTime = 0;
				}
				else if(gSuzieDoneTime > gSystemCycleCount) gNextTimerEvent = gSuzieDoneTime;
			}

			//	Timer updates, rolled out flat in group order
			//
			//	Group A:
			//	Timer 0 -> Timer 2 -> Timer 4. 
			//
			//	Group B:
			//	Timer 1 -> Timer 3 -> Timer 5 -> Timer 7 -> Audio 0 -> Audio 1-> Audio 2 -> Audio 3 -> Timer 1. 
			//

			//
			// Within each timer code block we will predict the cycle count number of
			// the next timer event
			//
			// We don't need to count linked timers as the timer they are linked
			// from will always generate earlier events.
			//
			// As Timer 4 (UART) will generate many events we will ignore it
			//
			// We set the next event to the end of time at first and let the timers
			// overload it. Any writes to timer controls will force next event to
			// be immediate and hence a new preidction will be done. The prediction
			// causes overflow as opposed to zero i.e. current+1
			// (In reality T0 line counter should always be running.)
			//

			
			//
			// Timer 0 of Group A
			//

			//
			// Optimisation, assume T0 (Line timer) is never in one-shot,
			// never placed in link mode
			//

			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
//			if(mTIM_0_ENABLE_COUNT && (mTIM_0_ENABLE_RELOAD || !mTIM_0_TIMER_DONE))
			if(mTIM_0_ENABLE_COUNT)
			{
				// Timer 0 has no linking
//				if(mTIM_0_LINKING!=0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_0_LINKING);
					decval=(gSystemCycleCount-mTIM_0_LAST_COUNT)>>divide;

					if(decval)
					{
						mTIM_0_LAST_COUNT+=decval<<divide;
						mTIM_0_CURRENT-=decval;

						if(mTIM_0_CURRENT&0x80000000)
						{
							// Set carry out
							mTIM_0_BORROW_OUT=true;
	
//							// Reload if neccessary
//							if(mTIM_0_ENABLE_RELOAD)
//							{
								mTIM_0_CURRENT+=mTIM_0_BKUP+1;
//							}
//							else
//							{
//								mTIM_0_CURRENT=0;
//							}

							mTIM_0_TIMER_DONE=true;

							// Interupt flag setting code moved into DisplayRenderLine()

							// Line timer has expired, render a line, we cannot incrememnt
							// the global counter at this point as it will screw the other timers
							// so we save under work done and inc at the end.
							mikie_work_done+=DisplayRenderLine();

						}
						else
						{
							mTIM_0_BORROW_OUT=false;
						}
						// Set carry in as we did a count
						mTIM_0_BORROW_IN=true;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_0_BORROW_IN=false;
						// Clear carry out
						mTIM_0_BORROW_OUT=false;
					}
				}

				// Prediction for next timer event cycle number

//				if(mTIM_0_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_0_CURRENT&0x80000000)?1:((mTIM_0_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}
	
			//
			// Timer 2 of Group A
			//

			//
			// Optimisation, assume T2 (Frame timer) is never in one-shot
			// always in linked mode i.e clocked by Line Timer
			//
			
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
//			if(mTIM_2_ENABLE_COUNT && (mTIM_2_ENABLE_RELOAD || !mTIM_2_TIMER_DONE))
			if(mTIM_2_ENABLE_COUNT)
			{
				decval=0;
		
//				if(mTIM_2_LINKING==0x07)
				{
					if(mTIM_0_BORROW_OUT) decval=1;
					mTIM_2_LAST_LINK_CARRY=mTIM_0_BORROW_OUT;
					divide = 0;
				}
//				else
//				{
//					// Ordinary clocked mode as opposed to linked mode
//					// 16MHz clock downto 1us == cyclecount >> 4 
//					divide=(4+mTIM_2_LINKING);
//					decval=(gSystemCycleCount-mTIM_2_LAST_COUNT)>>divide;
//				}
		
				if(decval)
				{
//					mTIM_2_LAST_COUNT+=decval<<divide;
					mTIM_2_CURRENT-=decval;
					if(mTIM_2_CURRENT&0x80000000)
					{
						// Set carry out
						mTIM_2_BORROW_OUT=true;
				
//						// Reload if neccessary
//						if(mTIM_2_ENABLE_RELOAD)
//						{
							mTIM_2_CURRENT+=mTIM_2_BKUP+1;
//						}
//						else
//						{
//							mTIM_2_CURRENT=0;
//						}
						mTIM_2_TIMER_DONE=true;

						// Interupt flag setting code moved into DisplayEndOfFrame(), also
						// park any CPU cycles lost for later inclusion
						mikie_work_done+=DisplayEndOfFrame();
					}
					else
					{
						mTIM_2_BORROW_OUT=false;
					}
					// Set carry in as we did a count
					mTIM_2_BORROW_IN=true;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_2_BORROW_IN=false;
					// Clear carry out
					mTIM_2_BORROW_OUT=false;
				}

				// Prediction for next timer event cycle number
// We dont need to predict this as its the frame timer and will always
// be beaten by the line timer on Timer 0
//				if(mTIM_2_LINKING!=7)
//				{
//					tmp=gSystemCycleCount+((mTIM_2_CURRENT+1)<<divide);
//					if(tmp<gNextTimerEvent)	gNextTimerEvent=tmp;
//				}
			}
		
			//
			// Timer 4 of Group A
			//
			// For the sake of speed it is assumed that Timer 4 (UART timer)
			// never uses one-shot mode, never uses linking, hence the code
			// is commented out. Timer 4 is at the end of a chain and seems
			// no reason to update its carry in-out variables
			//

			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
//			if(mTIM_4_ENABLE_COUNT && (mTIM_4_ENABLE_RELOAD || !mTIM_4_TIMER_DONE))
			if(mTIM_4_ENABLE_COUNT)
			{
				decval=0;
		
//				if(mTIM_4_LINKING==0x07)
//				{
////				if(mTIM_2_BORROW_OUT && !mTIM_4_LAST_LINK_CARRY) decval=1;
//					if(mTIM_2_BORROW_OUT) decval=1;
//					mTIM_4_LAST_LINK_CARRY=mTIM_2_BORROW_OUT;
//				}
//				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					// Additional /8 (+3) for 8 clocks per bit transmit
					divide=4+3+mTIM_4_LINKING;
					decval=(gSystemCycleCount-mTIM_4_LAST_COUNT)>>divide;
				}
		
				if(decval)
				{
					mTIM_4_LAST_COUNT+=decval<<divide;
					mTIM_4_CURRENT-=decval;
					if(mTIM_4_CURRENT&0x80000000)
					{
						// Set carry out
						mTIM_4_BORROW_OUT=true;
		
						//
						// Update the UART counter models for Rx & Tx
						//
		
						//
						// According to the docs IRQ's are level triggered and hence will always assert
						// what a pain in the arse
						//
						// Rx & Tx are loopedback due to comlynx structure

						//
						// Receive
						//
						if(!mUART_RX_COUNTDOWN)
						{
							// Fetch a byte from the input queue
							if(mUART_Rx_waiting>0)
							{
								mUART_RX_DATA=mUART_Rx_input_queue[mUART_Rx_output_ptr];
								mUART_Rx_output_ptr = (mUART_Rx_output_ptr + 1) % UART_MAX_RX_QUEUE;
								mUART_Rx_waiting--;
							}

							// Retrigger input if more bytes waiting
							if(mUART_Rx_waiting>0)
								mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD+UART_RX_NEXT_DELAY;
							else
								mUART_RX_COUNTDOWN=UART_RX_INACTIVE;

							// If RX_READY already set then we have an overrun
							// as previous byte hasnt been read
							if(mUART_RX_READY) mUART_Rx_overun_error=1;

							// Flag byte as being recvd
							mUART_RX_READY=1;
						}
						else if(!(mUART_RX_COUNTDOWN&UART_RX_INACTIVE))
						{
							mUART_RX_COUNTDOWN--;
						}

						if(!mUART_TX_COUNTDOWN)
						{
							if(mUART_SENDBREAK)
							{
								mUART_TX_DATA=UART_BREAK_CODE;
								// Auto-Respawn new transmit
								mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
								// Loop back what we transmitted
								ComLynxTxLoopback(mUART_TX_DATA);
							}
							else
							{
								// Serial activity finished 
								mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
							}

							// If a networking object is attached then use its callback to send the data byte.
							if(mpUART_TX_CALLBACK)
								(*mpUART_TX_CALLBACK)(mUART_TX_DATA,mUART_TX_CALLBACK_OBJECT);

						}
						else if(!(mUART_TX_COUNTDOWN&UART_TX_INACTIVE))
						{
							mUART_TX_COUNTDOWN--;
						}

						// Set the timer status flag
						// Timer 4 is the uart timer and doesn't generate IRQ's using this method
		
						// 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters
		
						// Reload if neccessary
//						if(mTIM_4_ENABLE_RELOAD)
//						{
							mTIM_4_CURRENT+=mTIM_4_BKUP+1;
							// The low reload values on TIM4 coupled with a longer
							// timer service delay can sometimes cause
							// an underun, check and fix
							if(mTIM_4_CURRENT&0x80000000)
							{
								mTIM_4_CURRENT=mTIM_4_BKUP;
								mTIM_4_LAST_COUNT=gSystemCycleCount;
							}
//						}
//						else
//						{
//							mTIM_4_CURRENT=0;
//						}
//						mTIM_4_TIMER_DONE=true;
					}
//					else
//					{
//						mTIM_4_BORROW_OUT=false;
//					}
//					// Set carry in as we did a count
//					mTIM_4_BORROW_IN=true;
				}
//				else
//				{
//					// Clear carry in as we didn't count
//					mTIM_4_BORROW_IN=false;
//					// Clear carry out
//					mTIM_4_BORROW_OUT=false;
//				}
//
//				// Prediction for next timer event cycle number
//
//				if(mTIM_4_LINKING!=7)
//				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_4_CURRENT&0x80000000)?1:((mTIM_4_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
//				}
			}

			// Emulate the UART bug where UART IRQ is level sensitive
			// in that it will continue to generate interrupts as long
			// as they are enabled and the interrupt condition is true

			// If Tx is inactive i.e ready for a byte to eat and the
			// IRQ is enabled then generate it always
			if((mUART_TX_COUNTDOWN&UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE)
				mTimerStatusFlags|=0x10;
			// Is data waiting and the interrupt enabled, if so then
			// what are we waiting for....
			if(mUART_RX_READY && mUART_RX_IRQ_ENABLE)
				mTimerStatusFlags|=0x10;
		
			//
			// Timer 1 of Group B
			//
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
			if(mTIM_1_ENABLE_COUNT && (mTIM_1_ENABLE_RELOAD || !mTIM_1_TIMER_DONE))
			{
				divide = 0;
				if(mTIM_1_LINKING!=0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_1_LINKING);
					decval=(gSystemCycleCount-mTIM_1_LAST_COUNT)>>divide;
		
					if(decval)
					{
						mTIM_1_LAST_COUNT+=decval<<divide;
						mTIM_1_CURRENT-=decval;
						if(mTIM_1_CURRENT&0x80000000)
						{
							// Set carry out
							mTIM_1_BORROW_OUT=true;
		
							// Set the timer status flag
							if(mTimerInterruptMask&0x02)
								mTimerStatusFlags|=0x02;
		
							// Reload if neccessary
							if(mTIM_1_ENABLE_RELOAD)
							{
								mTIM_1_CURRENT+=mTIM_1_BKUP+1;
							}
							else
							{
								mTIM_1_CURRENT=0;
							}
							mTIM_1_TIMER_DONE=true;
						}
						else
						{
							mTIM_1_BORROW_OUT=false;
						}
						// Set carry in as we did a count
						mTIM_1_BORROW_IN=true;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_1_BORROW_IN=false;
						// Clear carry out
						mTIM_1_BORROW_OUT=false;
					}
				}

				// Prediction for next timer event cycle number

				if(mTIM_1_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_1_CURRENT&0x80000000)?1:((mTIM_1_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}
		
			//
			// Timer 3 of Group A
			//
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
			if(mTIM_3_ENABLE_COUNT && (mTIM_3_ENABLE_RELOAD || !mTIM_3_TIMER_DONE))
			{
				decval=0;
		
				if(mTIM_3_LINKING==0x07)
				{
					if(mTIM_1_BORROW_OUT) decval=1;
					mTIM_3_LAST_LINK_CARRY=mTIM_1_BORROW_OUT;
					divide = 0;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_3_LINKING);
					decval=(gSystemCycleCount-mTIM_3_LAST_COUNT)>>divide;
				}
		
				if(decval)
				{
					mTIM_3_LAST_COUNT+=decval<<divide;
					mTIM_3_CURRENT-=decval;
					if(mTIM_3_CURRENT&0x80000000)
					{
						// Set carry out
						mTIM_3_BORROW_OUT=true;
		
						// Set the timer status flag
						if(mTimerInterruptMask&0x08)
							mTimerStatusFlags|=0x08;
		
						// Reload if neccessary
						if(mTIM_3_ENABLE_RELOAD)
						{
							mTIM_3_CURRENT+=mTIM_3_BKUP+1;
						}
						else
						{
							mTIM_3_CURRENT=0;
						}
						mTIM_3_TIMER_DONE=true;
					}
					else
					{
						mTIM_3_BORROW_OUT=false;
					}
					// Set carry in as we did a count
					mTIM_3_BORROW_IN=true;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_3_BORROW_IN=false;
					// Clear carry out
					mTIM_3_BORROW_OUT=false;
				}

				// Prediction for next timer event cycle number

				if(mTIM_3_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_3_CURRENT&0x80000000)?1:((mTIM_3_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}
		
			//
			// Timer 5 of Group A
			//
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
			if(mTIM_5_ENABLE_COUNT && (mTIM_5_ENABLE_RELOAD || !mTIM_5_TIMER_DONE))
			{
				decval=0;
		
				if(mTIM_5_LINKING==0x07)
				{
					if(mTIM_3_BORROW_OUT) decval=1;
					mTIM_5_LAST_LINK_CARRY=mTIM_3_BORROW_OUT;
					divide = 0;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_5_LINKING);
					decval=(gSystemCycleCount-mTIM_5_LAST_COUNT)>>divide;
				}
		
				if(decval)
				{
					mTIM_5_LAST_COUNT+=decval<<divide;
					mTIM_5_CURRENT-=decval;
					if(mTIM_5_CURRENT&0x80000000)
					{
						// Set carry out
						mTIM_5_BORROW_OUT=true;
		
						// Set the timer status flag
						if(mTimerInterruptMask&0x20)
							mTimerStatusFlags|=0x20;
		
						// Reload if neccessary
						if(mTIM_5_ENABLE_RELOAD)
						{
							mTIM_5_CURRENT+=mTIM_5_BKUP+1;
						}
						else
						{
							mTIM_5_CURRENT=0;
						}
						mTIM_5_TIMER_DONE=true;
					}
					else
					{
						mTIM_5_BORROW_OUT=false;
					}
					// Set carry in as we did a count
					mTIM_5_BORROW_IN=true;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_5_BORROW_IN=false;
					// Clear carry out
					mTIM_5_BORROW_OUT=false;
				}

				// Prediction for next timer event cycle number

				if(mTIM_5_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_5_CURRENT&0x80000000)?1:((mTIM_5_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}
		
			//
			// Timer 7 of Group A
			//
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
			if(mTIM_7_ENABLE_COUNT && (mTIM_7_ENABLE_RELOAD || !mTIM_7_TIMER_DONE))
			{
				decval=0;
		
				if(mTIM_7_LINKING==0x07)
				{
					if(mTIM_5_BORROW_OUT) decval=1;
					mTIM_7_LAST_LINK_CARRY=mTIM_5_BORROW_OUT;
					divide = 0;
				}
				else
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_7_LINKING);
					decval=(gSystemCycleCount-mTIM_7_LAST_COUNT)>>divide;
				}
		
				if(decval)
				{
					mTIM_7_LAST_COUNT+=decval<<divide;
					mTIM_7_CURRENT-=decval;
					if(mTIM_7_CURRENT&0x80000000)
					{
						// Set carry out
						mTIM_7_BORROW_OUT=true;
		
						// Set the timer status flag
						if(mTimerInterruptMask&0x80)
							mTimerStatusFlags|=0x80;
		
						// Reload if neccessary
						if(mTIM_7_ENABLE_RELOAD)
						{
							mTIM_7_CURRENT+=mTIM_7_BKUP+1;
						}
						else
						{
							mTIM_7_CURRENT=0;
						}
						mTIM_7_TIMER_DONE=true;
		
					}
					else
					{
						mTIM_7_BORROW_OUT=false;
					}
					// Set carry in as we did a count
					mTIM_7_BORROW_IN=true;
				}
				else
				{
					// Clear carry in as we didn't count
					mTIM_7_BORROW_IN=false;
					// Clear carry out
					mTIM_7_BORROW_OUT=false;
				}

				// Prediction for next timer event cycle number

				if(mTIM_7_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_7_CURRENT&0x80000000)?1:((mTIM_7_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}
		
			//
			// Timer 6 has no group
			//
			// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
			if(mTIM_6_ENABLE_COUNT && (mTIM_6_ENABLE_RELOAD || !mTIM_6_TIMER_DONE))
			{
//				if(mTIM_6_LINKING!=0x07)
				{
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4 
					divide=(4+mTIM_6_LINKING);
					decval=(gSystemCycleCount-mTIM_6_LAST_COUNT)>>divide;
		
					if(decval)
					{
						mTIM_6_LAST_COUNT+=decval<<divide;
						mTIM_6_CURRENT-=decval;
						if(mTIM_6_CURRENT&0x80000000)
						{
							// Set carry out
							mTIM_6_BORROW_OUT=true;
		
							// Set the timer status flag
							if(mTimerInterruptMask&0x40)
								mTimerStatusFlags|=0x40;
		
							// Reload if neccessary
							if(mTIM_6_ENABLE_RELOAD)
							{
								mTIM_6_CURRENT+=mTIM_6_BKUP+1;
							}
							else
							{
								mTIM_6_CURRENT=0;
							}
							mTIM_6_TIMER_DONE=true;
						}
						else
						{
							mTIM_6_BORROW_OUT=false;
						}
						// Set carry in as we did a count
						mTIM_6_BORROW_IN=true;
					}
					else
					{
						// Clear carry in as we didn't count
						mTIM_6_BORROW_IN=false;
						// Clear carry out
						mTIM_6_BORROW_OUT=false;
					}
				}

				// Prediction for next timer event cycle number
				// (Timer 6 doesn't support linking)

//				if(mTIM_6_LINKING!=7)
				{
					// Sometimes timeupdates can be >2x rollover in which case
					// then CURRENT may still be negative and we can use it to
					// calc the next timer value, we just want another update ASAP
					tmp=(mTIM_6_CURRENT&0x80000000)?1:((mTIM_6_CURRENT+1)<<divide);
					tmp+=gSystemCycleCount;
					if(tmp<gNextTimerEvent)
						gNextTimerEvent=tmp;
				}
			}

			//
			// Update the sound subsystem
			//
			{
			  int y;
			  for(y = 0; y < 4; y++)
			  {
				if(mAUDIO_ENABLE_COUNT[y] && (mAUDIO_ENABLE_RELOAD[y] || !mAUDIO_TIMER_DONE[y]))
				{
					decval=0;

					if(mAUDIO_LINKING[y]==0x07)
					{
						int bort;
						if(y) 
						 bort = mAUDIO_BORROW_OUT[y - 1];
						else
						 bort = mTIM_7_BORROW_OUT;

						if(bort) decval=1;
						mAUDIO_LAST_LINK_CARRY[y]=bort;
						divide = 0;
					}
					else
					{
						// Ordinary clocked mode as opposed to linked mode
						// 16MHz clock downto 1us == cyclecount >> 4 
						divide=(4+mAUDIO_LINKING[y]);
						decval=(gSystemCycleCount-mAUDIO_LAST_COUNT[y])>>divide;
					}

					if(decval)
					{
						mAUDIO_LAST_COUNT[y]  += decval<<divide;
						mAUDIO_CURRENT[y]-=decval;
						if(mAUDIO_CURRENT[y]&0x80000000)
						{
							// Set carry out
							mAUDIO_BORROW_OUT[y]=true;
		
							// Reload if neccessary
							if(mAUDIO_ENABLE_RELOAD[y])
							{
								mAUDIO_CURRENT[y]+=mAUDIO_BKUP[y]+1;
								if(mAUDIO_CURRENT[y]&0x80000000) mAUDIO_CURRENT[y]=0;
							}
							else
							{
								// Set timer done
								mAUDIO_TIMER_DONE[y]=true;
								mAUDIO_CURRENT[y]=0;
							}

							//
							// Update audio circuitry
							//
							if(mAUDIO_BKUP[y] || mAUDIO_LINKING[y])
							 mAUDIO_WAVESHAPER[y] = GetLfsrNext(mAUDIO_WAVESHAPER[y]);

							if(mAUDIO_INTEGRATE_ENABLE[y])
							{
								int32 temp=mAUDIO_OUTPUT[y];
								if(mAUDIO_WAVESHAPER[y]&0x0001) temp+=mAUDIO_VOLUME[y]; else temp-=mAUDIO_VOLUME[y];
								if(temp>127) temp=127;
								if(temp<-128) temp=-128;
								mAUDIO_OUTPUT[y]=(int8)temp;
							}
							else
							{
								if(mAUDIO_WAVESHAPER[y]&0x0001) mAUDIO_OUTPUT[y]=mAUDIO_VOLUME[y]; else mAUDIO_OUTPUT[y]=-mAUDIO_VOLUME[y];
							}
							CombobulateSound(gSystemCycleCount - startTS);
						}
						else
						{
							mAUDIO_BORROW_OUT[y]=false;
						}
						// Set carry in as we did a count
						mAUDIO_BORROW_IN[y]=true;
					}
					else
					{
						// Clear carry in as we didn't count
						mAUDIO_BORROW_IN[y]=false;
						// Clear carry out
						mAUDIO_BORROW_OUT[y]=false;
					}

					// Prediction for next timer event cycle number

					if(mAUDIO_LINKING[y]!=7)
					{
						// Sometimes timeupdates can be >2x rollover in which case
						// then CURRENT may still be negative and we can use it to
						// calc the next timer value, we just want another update ASAP
						tmp=(mAUDIO_CURRENT[y]&0x80000000)?1:((mAUDIO_CURRENT[y]+1)<<divide);
						tmp+=gSystemCycleCount;
						if(tmp<gNextTimerEvent)
							gNextTimerEvent=tmp;
					}
				}
			 }
			}

			//	if(gSystemCycleCount==gNextTimerEvent) gError->Warning("CMikie::Update() - gSystemCycleCount==gNextTimerEvent, system lock likely");

			// Update system IRQ status as a result of timer activity
			// OR is required to ensure serial IRQ's are not masked accidentally
		
			gSystemIRQ=(mTimerStatusFlags)?true:false;
			if(gSystemIRQ && gSystemCPUSleep) { ClearCPUSleep(); /*puts("ARLARM"); */ }
			//else if(gSuzieDoneTime) SetCPUSleep();

			// Now all the timer updates are done we can increment the system
			// counter for any work done within the Update() function, gSystemCycleCounter
			// cannot be updated until this point otherwise it screws up the counters.
			gSystemCycleCount+=mikie_work_done;
}
