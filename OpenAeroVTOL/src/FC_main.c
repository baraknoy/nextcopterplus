// **************************************************************************
// OpenAero VTOL software for KK2.0 & KK2.1
// ========================================
// Version: Beta 11 - December 2013
//
// Some receiver format decoding code from Jim Drew of XPS and the Papparazzi project
// OpenAero code by David Thompson, included open-source code as per quoted references
//
// **************************************************************************
// * 						GNU GPL V3 notice
// **************************************************************************
// * Copyright (C) 2013 David Thompson
// * 
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// * 
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// * 
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see <http://www.gnu.org/licenses/>.
// * 
// * tl;dr - all derivative code MUST be released with the source code!
// *
// **************************************************************************
// Version History
// ===============
// VTOL 	Based on OpenAero2 V1.2 Beta 8 code
// Alpha 1	Added transition code
//			Added basic height dampening code (untested)
// Alpha 2	Fixed bug where the output mixer switcher channel was screwing up the gyro signal distribution.
// Alpha 3	Fixed all the missing "const" that annoy some compilers. 
// Alpha 4	Fixed partial transition and offset bugs. Also fixed case where power up state is wrong. 
// Beta 1	Changed flying wing mixing to the output mixers.
//			Removed Source Mix setting that confused everyone. Also removed RC input Source B mixing
//			as this is best done in the output mixers now. Dynamic gain effect reversed. Now max input
//			is maximum stability. Decoupled stick and gyro for P gain.
//			Factory reset now enterable by pressing just the middle two buttons. 
//			Removed output switcher and added per-output offset.
//			Added the ability to mix any RC source into the outputs.
// Beta 1 	Added arming
// Beta 2	Reversed arming and reduced the stick extremes required.
//			Changed default to ARMED. Fixed up some defaults.
//			Fixed OUT1 first input mixer bug.
// Beta 3 	Merged Channel and Output mixers. Removed presets.
//			Transition is default mode. Removed Profile 3 setting.
// Beta 4 	Completely changed mixers, removed unwanted features.
//			Now customised for transitioning. 
// Beta 5	Memory reduction via sensor switches
//			Expanded transition to all eight channels.	
//			Updated status display to show more transition info.
//			Output mixer menus for OUT1 to OUT8 now contain both P1 and P2 configuration.
// Beta 6	Added motor marking and new safety features for arming, trims, limits and low-throttle.
//			Fixed LVA setting and display. Removed failsafe functionality.
//			Fixed menu position behaviour when moving about main and sub menus.
//			Fixed flakey initial transition behaviour when changing transition settings
//			PID values for both profiles now calculated on the fly and only transitioned in the mixer.
//			Increased transition steps to 100 and interval to 10ms. I-terms reset at throttle cut.
//			Added experimental three-point offset handling. Removed redundant servo trim.
// Beta 7 	Re-added sensor reversing. Very code-size inefficient.
// Beta 8 	Changed display text for safety to "Armed", "Armable". Rearranged mixer items.
//			Rationalised version number. Changed motor marker to "Motor/Servo".
//			Changed servo rate to "Normal/fast". P1 to P2 transition point now hard-coded to 20% above center
//			Changed manual transition range to +/-100% (was closer to +/-125%). LMA time now "Disarm time".
//			LMA/Disarm timeout also switches to disarmed state if armable.
//			Height damping removed, Z-axis delta now included in mixer list.
//			Loop speed optimisations in the mixer. PWM sync logic simplified.
// Beta 9	Disarm timer now functions in 0 to 127 seconds and will auto-disarm if armable and not set to zero.
//			"No signal" state also disarms if set to "armable". Rearranged General menu. Fixed transition bug.
//			Disarming state sets all outputs marked as "Motor" to minimum output.
//			Acc Z Delta now working. Missing REV for P2 sensors restored.
// Beta 10	Added support for KK2.1 board. Logo displays for K2.1 boards.
//			Most tunable items changed fom 5 to 1 increment.
//			Added dedicated throttle sources to mixers with throttle curves.
//			Throttle now referenced from zero and has its own Config.ThrottleMinOffset for referencing. 
//			Rearranged mixer menus. Simplified includes.
// Beta 11	Fixed throttle high bug. Axis lock stick rate now calibrated to 10, 20, 40, 80, 160, 320 and 640 deg/sec.
//			Contrast recovered from saved setting correctly.
//			
//
//***********************************************************
//* Notes
//***********************************************************
//
// Bugs:
//	I-term constrainst not working on KK2.0 but fine on KK2.1...
//  Contrast value not loaded on power-up
//
// Todo:
//  Remove remaining uneeded menu items
//	Better way to integrate switching of sensors.
//	Copy RC soures and .values to an array so that the mixer can do less work?
//	
// Wish list:
//	Per-sensor, per output, per profile gain adjustment.
//
//***********************************************************
//* Includes
//***********************************************************

#include <avr/io.h>
#include <avr/pgmspace.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include <string.h>
#include "io_cfg.h"
#include "rc.h"
#include "servos.h"
#include "vbat.h"
#include "gyros.h"
#include "init.h"
#include "acc.h"
#include "isr.h"
#include "glcd_driver.h"
#include "pid.h"
#include "mixer.h"
#include "glcd_buffer.h"
#include "mugui.h"
#include "glcd_menu.h"
#include "menu_ext.h"
#include "main.h"
#include "imu.h"
#include "eeprom.h"

//***********************************************************
//* Fonts
//***********************************************************

#include "Font_Verdana.h" 		// 8 (text) and 14 (titles) points
#include "Font_WingdingsOE2.h"	// Cursor and markers

//***********************************************************
//* Defines
//***********************************************************

#define	RC_OVERDUE 9765				// Number of T2 cycles before RC will be overdue = 9765 * 1/19531 = 500ms
#define	SLOW_RC_RATE 41667			// Slowest RC rate tolerable for Plan A syncing = 2500000/60 = 41667
#define	SERVO_RATE_LOW 390			// Requested servo rate when in Normal mode. 19531 / 50(Hz) = 390

#define REFRESH_TIMEOUT 39060		// Amount of time to wait after last RX activity before refreshing LCD (2 seconds)
#define SECOND_TIMER 19531			// Unit of timing for seconds
#define ARM_TIMER_RESET 960			// RC position to reset timer
#define TRANSITION_TIMER 195		// Transition timer units (10ms * 100) (1 to 10 = 1s to 10s)
#define ARM_TIMER 97655				// Amount of time the sticks must be held to trigger arm/disarm. Currently five seconds.

//***********************************************************
//* Code and Data variables
//***********************************************************

// Flight variables
uint32_t ticker_32;					// Incrementing system ticker
int16_t	transition_value_16 = 0;
int16_t transition_counter = 0;
uint8_t Transition_state = TRANS_0;

// Flags
uint8_t	General_error = 0;
uint8_t	Flight_flags = 0;
uint8_t	Main_flags = 0;
uint8_t	Alarm_flags = 0;

// Global buffers
char pBuffer[PBUFFER_SIZE];			// Print buffer (16 bytes)

// Serial buffer
char sBuffer[SBUFFER_SIZE];			// Serial buffer (25 bytes)

//************************************************************
//* Main loop
//************************************************************

int main(void)
{
	uint16_t cycletime;				// Loop time

	bool Overdue = false;
	bool ServoTick = false;
	bool SlowRC = false;
	bool TransitionUpdated = false;

	// 32-bit timers
	uint32_t Arm_timer = 0;
	uint32_t RC_Rate_Timer = 0;

	// 16-bit timers
	uint16_t Status_timeout = 0;
	uint16_t UpdateStatus_timer = 0;
	uint16_t Ticker_Count = 0;
	uint16_t RC_Timeout = 0;
	uint16_t Servo_Rate = 0;
	uint16_t Transition_timeout = 0;
	uint16_t Disarm_timer = 0;

	// Timer incrementers
	uint16_t LoopStartTCNT1 = 0;
	uint16_t RC_Rate_TCNT1 = 0;
	uint8_t Transition_TCNT2 = 0;
	uint8_t Status_TCNT2 = 0;
	uint8_t Refresh_TCNT2 = 0;
	uint8_t Disarm_TCNT2 = 0;
	uint8_t Arm_TCNT2 = 0;
	uint8_t Ticker_TCNT2 = 0;
	uint8_t Servo_TCNT2 = 0;
	uint8_t ServoRate_TCNT2 = 0;

	// Locals
	uint8_t	Disarm_seconds = 0;
	uint8_t Status_seconds = 0;
	uint8_t Menu_mode = STATUS_TIMEOUT;
//	uint8_t i = 0;
	int8_t	old_flight = 3;			// Old flight profile
	int8_t	old_trans_mode = 0;		// Old transition mode
	
	// Transition
	uint8_t start = 0;
	uint8_t end = 1;

	init();							// Do all init tasks

	// Main loop
	while (1)
	{
		//************************************************************
		//* State machine for switching between screens safely
		//************************************************************

		switch(Menu_mode) 
		{
			// In IDLE mode, the text "Press for status" is displayed ONCE.
			// If a button is pressed the mode changes to STATUS
			case IDLE:
				if((PINB & 0xf0) != 0xf0)
				{
					Menu_mode = STATUS;
					// Reset the status screen timeout
					Status_seconds = 0;
					menu_beep(1);
				}
				break;

			// Status screen first display
			case STATUS:
				// Reset the status screen period
				UpdateStatus_timer = 0;
				// Update status screen
				Display_status();
				// Force resync on next RC packet
				Interrupted = false;	
				// Wait for timeout
				Menu_mode = WAITING_TIMEOUT_BD;
				break;

			// Status screen up, but button still down ;)
			// This is designed to stop the menu appearing instead of the status screen
			// as it will stay here until the button is released
			case WAITING_TIMEOUT_BD:
				if(BUTTON1 == 0)
				{
					Menu_mode = WAITING_TIMEOUT_BD;
				}
				else
				{
					Menu_mode = WAITING_TIMEOUT;
				}
				break;
												
			// Status screen up, waiting for timeout or action
			// but button is back up
			case WAITING_TIMEOUT:
				// In status screen, change back to idle after timing out
				if (Status_seconds >= Config.Status_timer)
				{
					Menu_mode = STATUS_TIMEOUT;
				}

				// Jump to menu if button pressed
				else if(BUTTON1 == 0)
				{
					Menu_mode = MENU;
					menu_beep(1);
					// Force resync on next RC packet
					Interrupted = false;
				}

				// Update status screen while waiting to time out
				else if (UpdateStatus_timer > (SECOND_TIMER >> 2))
				{
					Menu_mode = STATUS;
				}
				break;

			// In STATUS_TIMEOUT mode, the idle screen is displayed and the mode changed to IDLE
			case STATUS_TIMEOUT:
				// Pop up the Idle screen
				idle_screen();
				// Force resync on next RC packet
				Interrupted = false;
				// Switch to IDLE mode
				Menu_mode = IDLE;
				break;

			// In MENU mode, 
			case MENU:
				LVA = 0;	// Make sure buzzer is off :)
				menu_main();
				// Force resync on next RC packet
				Interrupted = false;
				// Switch back to status screen when leaving menu
				Menu_mode = STATUS;
				// Reset timeout once back in status screen
				Status_seconds = 0;
				break;

			default:
				break;
		}

		//************************************************************
		//* Status menu refreshing
		//************************************************************

		// Update status timeout
		Status_timeout += (uint8_t) (TCNT2 - Status_TCNT2);
		Status_TCNT2 = TCNT2;

		// Count elapsed seconds
		if (Status_timeout > SECOND_TIMER)
		{
			Status_seconds++;
			Status_timeout = 0;
		}

		// Update status provided no RX activity for REFRESH_TIMEOUT seconds (1s)
		UpdateStatus_timer += (uint8_t) (TCNT2 - Refresh_TCNT2);
		Refresh_TCNT2 = TCNT2;

		//************************************************************
		//* System ticker - based on TCNT2 (19.531kHz)
		//* 
		//* ((Ticker_Count >> 8) &8) 	= 4.77Hz (Disarm and LVA alarms)
		//************************************************************

		// Ticker_Count increments at 19.531 kHz, in loop cycle chunks
		Ticker_Count += (uint8_t) (TCNT2 - Ticker_TCNT2);
		Ticker_TCNT2 = TCNT2;

		if ((Ticker_Count >> 8) &8) 
		{
			Alarm_flags |= (1 << BUZZER_ON);	// 4.77Hz beep
		}
		else 
		{
			Alarm_flags &= ~(1 << BUZZER_ON);
		}

		//************************************************************
		//* Alarms
		//************************************************************

		// Disarm timer alarm
		Disarm_timer += (uint8_t) (TCNT2 - Disarm_TCNT2);
		Disarm_TCNT2 = TCNT2;

		// Reset auto-disarm count if any RX activity or set to zero
		if ((Flight_flags & (1 << RxActivity)) || (Config.Disarm_timer == 0))
		{														
			Disarm_timer = 0;
			Disarm_seconds = 0;
		}
		
		// Increment disarm timer (seconds)
		if (Disarm_timer > SECOND_TIMER)
		{
			Disarm_seconds++;
			Disarm_timer = 0;
		}

		// Disarm model if timeout enabled and due
		if ((Disarm_seconds >= Config.Disarm_timer) && (Config.Disarm_timer != 0))	
		{
			// If FC is set to "armable" and is currently armed, disarm the FC
			if ((Config.ArmMode == ARMABLE) && ((General_error & (1 << DISARMED)) == 0))
			{
				General_error |= (1 << DISARMED);	// Set flags to disarmed
				// Force update of status screen
				Menu_mode = STATUS_TIMEOUT;
				menu_beep(1);							// Signal that FC is now disarmed
			}
		}

		// If RC signals overdue, signal RX error message and disarm
		if (Overdue)
		{
			General_error |= (1 << NO_SIGNAL);		// Set NO_SIGNAL bit
			
			// If FC is set to "armable" and is currently armed, disarm the FC
			if ((Config.ArmMode == ARMABLE) && ((General_error & (1 << DISARMED)) == 0))
			{
				General_error |= (1 << DISARMED);	// Set flags to disarmed
				// Force update of status screen
				Menu_mode = STATUS_TIMEOUT;
				menu_beep(1);						// Signal that FC is now disarmed
			}
		}
		else
		{
			General_error &= ~(1 << NO_SIGNAL);	// Clear NO_SIGNAL bit
		}

		// Beep buzzer if Vbat lower than trigger
		// Vbat is measured in units of 10mV, so a PowerTrigger of 127 equates to 
		if (GetVbat() < Config.PowerTriggerActual)
		{
			Alarm_flags |= (1 << LVA_Alarm);	// Set LVA_Alarm flag
		}
		else 
		{
			Alarm_flags &= ~(1 << LVA_Alarm);	// Clear LVA_Alarm flag
		}

		// Turn on buzzer if in alarm state (BUZZER_ON is oscillating)
		if	(((Alarm_flags & (1 << LVA_Alarm)) ||
			  (General_error & (1 << NO_SIGNAL))) && 
			  (Alarm_flags & (1 << BUZZER_ON))) 
		{
			LVA = 1;
		}
		else 
		{
			LVA = 0;
		}

		//************************************************************
		//* Arm/disarm handling
		//************************************************************

		if (Config.ArmMode == ARMABLE)
		{
			// Increment timer only if Launch mode on to save cycles
			Arm_timer += (uint8_t) (TCNT2 - Arm_TCNT2); // TCNT2 runs at 19.531kHz. SECOND_TIMER amount of TCNT2 is 1 second
			Arm_TCNT2 = TCNT2;

			// If sticks not at extremes, reset timer
			// Sticks down and centered = armed. Down and outside = disarmed
			if (
				((-ARM_TIMER_RESET < RCinputs[AILERON]) && (RCinputs[AILERON] < ARM_TIMER_RESET)) ||
				((-ARM_TIMER_RESET < RCinputs[ELEVATOR]) && (RCinputs[ELEVATOR] < ARM_TIMER_RESET)) ||
				((-ARM_TIMER_RESET < RCinputs[RUDDER]) && (RCinputs[RUDDER] < ARM_TIMER_RESET)) ||
				((-ARM_TIMER_RESET < RCinputs[THROTTLE]) && (RCinputs[THROTTLE] < ARM_TIMER_RESET))
			   )

			{
				Arm_timer = 0;
			}

			// If arm timer times out, the sticks must have been at extremes for ARM_TIMER/SECOND_TIMER seconds
			if (Arm_timer > ARM_TIMER)
			{
				// If aileron is at min, arm the FC
				if (RCinputs[AILERON] < ARM_TIMER_RESET)
				{
					Arm_timer = 0;
					General_error &= ~(1 << DISARMED);		// Set flags to armed
					menu_beep(20);							// Signal that FC is now armed
				}

				// Else, disarm the FC
				else
				{
					Arm_timer = 0;
					General_error |= (1 << DISARMED);		// Set flags to disarmed
					menu_beep(1);							// Signal that FC is now disarmed
				}

				// Force update of status screen
				Menu_mode = STATUS_TIMEOUT;
			}
		}
		// Arm when ArmMode is OFF
		else 
		{
			General_error &= ~(1 << DISARMED);		// Set flags to armed
		}

		//************************************************************
		//* Get RC data
		//************************************************************

		// Update zeroed RC channel data
		RxGetChannels();

		// Check for throttle reset
		if (RCinputs[THROTTLE] < 50)
		{
			// Clear throttle high error
			General_error &= ~(1 << THROTTLE_HIGH);	

			// Reset I-terms at throttle cut
			IntegralGyro[P1][ROLL] = 0;	
			IntegralGyro[P1][PITCH] = 0;
			IntegralGyro[P1][YAW] = 0;
			IntegralGyro[P2][ROLL] = 0;	
			IntegralGyro[P2][PITCH] = 0;
			IntegralGyro[P2][YAW] = 0;

			//UpdateLimits(); // debug
		}

		//************************************************************
		//* Flight profile / transition state selection
		//*
		//* When transitioning, the flight profile is a moving blend of 
		//* Flight profiles 0 to 1. The transition speed is controlled 
		//* by the Config.TransitionSpeed setting.
		//************************************************************

		// P1 to P2 transition point now hard-coded to 20% above center
		if 	(RCinputs[Config.FlightChan] > 200)
		{
			Config.FlightSel = 1;			// Flight mode 1 (P2)
		}
		else
		{
			Config.FlightSel = 0;			// Flight mode 0 (P1)
		}

		// Reset update request each loop
		TransitionUpdated = false;

		//************************************************************
		//* Transition initial state setup/reset
		//*
		//* For the first startup, set up the right state for the current setup
		//* Check for initial startup - the only time that old_flight should be "3".
		//* Also, re-initialise if the transition setting is changed
		//************************************************************

		if ((old_flight == 3) || (old_trans_mode != Config.TransitionSpeed))
		{
			switch(Config.FlightSel)
			{
				case 0:
					Transition_state = TRANS_0;
					transition_counter = 0;
					break;
				case 1:
					Transition_state = TRANS_1;
					transition_counter = 100;
					break;
				default:
					break;
			}		 
			old_flight = Config.FlightSel;
			old_trans_mode = Config.TransitionSpeed;
		}

		// Update timed transition when changing flight modes
		if (Config.FlightSel != old_flight)
		{
			// When in a timed transition mode
			if (Config.TransitionSpeed != 0)
			{
				// Flag that update is required if mode changed
				TransitionUpdated = true;
			}
		}

		// Check to see if the transition channel value has changed when bound to an 
		// input channel to control transition (Config.TransitionSpeed = 0)
		// If so, set TransitionUpdated flag to trigger an update.
		// Note that by dividing the input value by 16 we reduce the steps from +/-1250 to about 156 steps 
		// or +/-1000 to +/- 62, 124 steps 

		if (Config.TransitionSpeed == 0)
		{
			if (transition_value_16 != (RCinputs[Config.FlightChan] >> 4))
			{
				TransitionUpdated = true;
			}
		}

		//************************************************************
		//* Transition state handling
		//************************************************************

		// Update transition timer
		Transition_timeout += (uint8_t) (TCNT2 - Transition_TCNT2);
		Transition_TCNT2 = TCNT2;

		// Update transition value. +/-1250 --> +/-steps, +/-800 --> +/-50 steps
		transition_value_16 = (RCinputs[Config.FlightChan] >> 4);

		// Update transition state change when control value or flight mode changes
		if (TransitionUpdated)
		{
			// Always in the TRANSITIONING state when Config.TransitionSpeed is 0
			// which means the transition is controlled by an RC channel 
			if (Config.TransitionSpeed == 0)
			{
				Transition_state = TRANSITIONING;
			}
			// For the change from 0 to 1
			else if ((Config.FlightSel == 1) && (old_flight == 0))
			{
				Transition_state = TRANS_0_to_1_start;
				old_flight = 1;
			}
			// For the change from 1 to 0
			else if ((Config.FlightSel == 0) && (old_flight == 1))
			{
				Transition_state = TRANS_1_to_0_start;
				old_flight = 0;
			}
		}

		// Update state, values and transition_counter every Config.TransitionSpeed if not zero. 195 = 10ms
		if (((Config.TransitionSpeed != 0) && (Transition_timeout > (TRANSITION_TIMER * Config.TransitionSpeed))) ||

		// If bound to a channel update once
			  TransitionUpdated)
		{
			Transition_timeout = 0;
			TransitionUpdated = false;

			switch(Transition_state)
			{
				case TRANS_0:
					transition_counter = 0;
					break;

				case TRANS_1:
					transition_counter = 100;
					break;

				case TRANS_0_to_1_start:
				case TRANS_1_to_0_start:
					// Set start and end profiles
					if (Transition_state == TRANS_0_to_1_start) // Hmm... yeap
					{
						start = 0;
						end = 1;
					}
					else
					{
						start = 1;
						end = 0;
					}
					
					// Fall through to transition handling
					Transition_state = TRANSITIONING;

				case TRANSITIONING:
					// Handle timed transition
					// Profile 1 to 0, so counter decrements to zero
					if (start)
					{
						transition_counter--;
						if (transition_counter <= 0)
						{
							transition_counter = 0;
							Transition_state = TRANS_0;
						}
					}
					// Profile 0 to 1, so counter increments to 100
					else
					{
						transition_counter++;
						if (transition_counter >= 99)
						{
							transition_counter = 99;
							Transition_state = TRANS_1;
						}
					}
					break;

				default:
					break;

			} // switch(Transition_state)

		} // Increment transition_counter

		// Save current flight mode
		old_flight = Config.FlightSel;

		//************************************************************
		//* Remaining loop tasks
		//************************************************************

		// Read sensors
		ReadGyros();
		ReadAcc();	
		getEstimatedAttitude();

		// Calculate PID
		Calculate_PID();

		// Calculate mix
		ProcessMixer();

		// Transfer Config.Channel[i].value data to ServoOut[i] and check limits
		UpdateServos();

		//************************************************************
		//* RC and servo timers
		//************************************************************

		// Work out the current RC rate by measuring between incoming RC packets
		RC_Rate_Timer += (uint16_t) (TCNT1 - RC_Rate_TCNT1);
		RC_Rate_TCNT1 = TCNT1;

		// Sets the desired SERVO_RATE by flagging ServoTick when PWM due
		// Servo_Rate increments at 19.531 kHz, in loop cycle chunks
		Servo_Rate += (uint8_t) (TCNT2 - ServoRate_TCNT2);
		ServoRate_TCNT2 = TCNT2;
		
		// Signal RC overdue after RC_OVERDUE time (500ms)
		// RC_Timeout increments at 19.531 kHz, in loop cycle chunks
		RC_Timeout += (uint8_t) (TCNT2 - Servo_TCNT2);
		Servo_TCNT2 = TCNT2;

		//************************************************************
		//* Check prescence of RC
		//************************************************************

		// Check to see if the RC input is overdue (500ms)
		if (RC_Timeout > RC_OVERDUE)
		{
			Overdue = true;	// This results in a "No Signal" error
		}

		//************************************************************
		//* Measure incoming RC rate
		//************************************************************		

		// RC input received
		if (Interrupted)
		{
			RC_Timeout = 0;					// Reset RC timeout
			Overdue = false;				// And no longer overdue

			// Measure incoming RC rate. Threshold is 60Hz.
			// Slow RC rates are synched on every pulse, faster ones are limited to 50Hz
			if (RC_Rate_Timer > SLOW_RC_RATE)
			{
				SlowRC = true;
			}
			else
			{
				SlowRC = false;
			}

			RC_Rate_Timer = 0;
		}

		//************************************************************
		//* Manage desired output update rate when limited by
		//* the PWM rate set to "Normal"
		//************************************************************

		// Flag update required based on SERVO_RATE_LOW
		if (Servo_Rate > SERVO_RATE_LOW)
		{
			ServoTick = true; // Slow device is ready for output generation
			Servo_Rate = 0;
		}

		//************************************************************
		//* Synchronise output update rate to the RC interrupts
		//* based on a specific set of conditions
		//************************************************************

		// Cases where we are ready to output
		if	(Interrupted &&						// Only when interrupted (RC receive completed)
				(
				  (SlowRC) || 					// Plan A (Run as fast as the incoming RC if slow RC detected)
				  (ServoTick && !SlowRC) ||		// Plan B (Run no faster than the preset rate (ServoTick) if fast RC detected)
				  (Config.Servo_rate == HIGH) 	// Plan C (Run as fast as the incoming RC if in HIGH mode)
				)
			)
		{
			Interrupted = false;				// Reset interrupted flag
			ServoTick = false;					// Reset output requested flag
			Servo_Rate = 0;						// Reset servo rate timer
			output_servo_ppm();					// Output servo signal
		}

		// Not ready for output, so cancel the current interrupt and wait for the next one
		else
		{
			Interrupted = false;				// Reset interrupted flag
		}

		//************************************************************
		//* Measure loop rate
		//************************************************************

		cycletime = TCNT1 - LoopStartTCNT1;	// Update cycle time
		LoopStartTCNT1 = TCNT1;				// Measure period of loop from here
		ticker_32 += cycletime;

	} // main loop
} // main()

