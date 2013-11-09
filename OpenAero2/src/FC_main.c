// **************************************************************************
// OpenAero2 software for KK2.0
// ===========================
// Version 1.3 Beta 1 - October 2013
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
// * NB: Summary - all derivative code MUST be released with the source code!
// *
// **************************************************************************
// Version History
// ===============
// V1.0		Based on OpenAero V1.13 Beta 8 code
//			Initial code base.
// Alpha 1	First release to alpha testers
// Alpha 2  Fixed channel number bug, added second aileron to default mixer
// Alpha 3	Enabled autolevel and acc settings in mixer menus, gyro reversing works.
//			Contrast updates visually during modification. Status menu updated.
// Alpha 4	Battery auto-calculates LVA setting. Restore to default instead of clear.
//			*Everything* that can be in PROGMEM is in PROGMEM. JR/Spk notation.
//			Version added to Status menu. Added separate accs to mixer.
//			Increased GLCD write speed to maximum.
//			Working Status menu auto refresh!
// Alpha 5	Added audible and visual error messages for LVA, no RX, Gyro error, throttle high
//			and Lost Model.	Failsafe setting now possible from the RCinputs screen. 
//			Expo now works nicely. Changed min/max values to (us). Rearranged menu.
//			FlapChan now updates mixer so that M7 has the same source as FlapChan.
//			Enabled source volume - now adjustable from between 0% and 125%
//			Fixed niggling hiccup from servos due to status screen refreshing.
//			Fixed servo jitter in CPPM mode. Add four presets for use as source channels.
// Beta 1	First public release. Fixed failsafe bug. Removed unused menu items.
//			Small, ugly hack to modify battery defaults if NiMh selected.
// Beta 2	Change gyro setup in mixer. Change limits to percentages.
//			Large menu size reduction. Trash KK2 eeprom locations.
//			Added two, two-channel RC mixers - mixes to new channels "MIX1" and "MIX2".
//			Added adjustable Lost Model Alarm timeout. 0 = disabled, otherwise 1 to 10 minutes
//			without RC input. Fixed min/max travel bug. Startup beep now after last gyro check.
//			Fixed issue where changing anything in the general menu over-wrote the mixers.
//			Removed other font options from mugui_text.c. Adjusted F.Wing default sources.
//			Force factory reset for new eeprom structure changes.
//			Added back CamStab enable so that camera stability can be done without RC.
//			Added CamStab mixer preset. Restored full PID functionality.
//			Completely new idle screen. Status screen now has user settable timeout.
//			Greatly increased acc gain.
// Beta 3	Added anti-gyro noise gate into PID calculations.
//			Fixed bug where CamStab and failsafe would clash and lock the menu.
//			Completely reworked PID code to enable heading hold on all axis.
//			Autolevel menu now only handles accelerometers. Tweaked camstab mixer preset.
//			Added settable I-term limits in %. Added servo rate adjustment for CamStab mode.
//			In CamStab mode, switched off M5-M8 mixing and outputs to increase loop speed to about 400Hz.
//			Added separate P settings for Roll and Pitch accelerometers. Servo HIGH rate set to 300Hz.
//			Added preliminary IMU code. Removed small, ugly NiMh hack.
//			Changed autolevel setting to only do autolevel. No implied stability control.
// Beta 4	Fixed noob-level f*ck-up resulting in no I-term for Yaw in Beta3
//			Added menu setting for Yaw heading-hold auto-center "Yaw magic".
//			Corrected voltage scaling text. Fixed LiPo default voltage.
//			Menu now remembers last position. Added button acceleration.
//			Updated Aeroplane mixer preset. Removed 3-pos setting from General menu as not needed.
//			Added stack monitoring function. Reversed menu and value UP/DOWN sense. 
//			Hopefully fixed false throttle high alarm.
//			Completed and integrated basic IMU code (again).
//			Added General menu settings for Acc LPF and IMU CF factor.
// Beta 4.1 Trial fix for Autolevel switch-on issue (Successful).
//	
// V1.1		Based on OpenAero2 Beta 4.1 code
// Alpha 1	Stability and Autolevel switch setpoint adjustment	
//			RC mixers moved to main mixers. Fixed balance meter movement
//			Main mixers can now cross-mix up to four channels
//			Launch delay mode. Tweaked menu navigation and driver.
//			Added 120 degree swashplate preset. 
// Alpha 2	Removed PRESET channels and replaced with per-output trims.
//			Added HANDSFREE autolevel modes (autolevel at centered sticks)
//			Added working differential mode (finally)
// Alpha 3	Added IMU fixes for inverted flight. New inverted calibration mode
//			Code now 100.0% full. Had to remove one mixer preset.
// Beta 1	Bugfix for three-position function. Minor tweaks for Beta 1
// Beta 2	Bugfix - fixed vertical orientation mode.
// Beta 3	New I-term modes: Normal, Auto and 3D.
//			Reversed Yaw gyro setting in aeroplane mixer preset...
// Beta 4	Completely changed PID loop to take sticks into account.
//			Added offset calculation to exclude flaperon movement.
//			Totally changed mixer so that stability/autolevel has exclusive control of outputs (no RC).
//			Added Maximum turn angle setting in Autolevel mode.
//			Updated Z cal to make it easier to do and harder to screw up.
//			Restored missing ACC trims. Fixed I-term constrain calculation.
//			Added Normal/FlyByWire flight modes.
//			Added Dynamic gain setting. Many bug fixes.
// Beta 5	Fixed 3D mode mixing. Fixed autolevel in Vertical mode.
//			Increased 3D-mode RC input by 2x as per KeiTora request.
//			Added "Upside down" orientation.
// Beta 6	Fixed Flying Wing and Camstab mixer presets
//			Removed expo and differential settings and menu.
//			3D-mode RC input scaling now menu settable.
//			New failsafe menu and items. Set buzzer in failsafe.
//			Added extra gyro recal button.
//			Simplified buzzer code. Clear eeprom properly on reset.
//			Just squeezed Camstab preset mixer back in :)
// Beta 7	Changed Failsafe text from "Normal" to "Fixed".
//			Restore submenu position if going back into the same submenu.
//			Changed Flying Wing mixing to on-board only.
//			Reduce FBW RC input to 50%. Adjusted PID defaults.
//			Fixed failsafe throttle default.
// Beta 8	Add IMU mode switch (Normal/Advanced.)
//			Added primary channel reversing in RC setup menu.
//			Changed mixer order and added "Source mix" manual option.
//			No Fly-by-wire mode as for gyros always take stick input. Acc mixes in autolevel mode.
//			Fixed flap handling in stabilised modes. Normalised sensor polarity with RC.
//			Linked local G measurement with IMU calculation to solve negative G and (hopefully)
//			coordinated turn issues. Dynamic gain updated to suit new code. 
//			Flying wing mixer changed and tested in all modes.
//			Increased autolevel trim response x 4 
// Beta 9	Fixed Dynamic gain bug.
//			Split M1-M8 menus into three sections.	
//			Compacted several of the settings menus into one.	
//			New servo menus added, menus edited.
//			Added "aft" orientation. Fixed inv. cal. bug.
//			Adv. IMU defaults to ON. 
//			Added real-time servo updating for min/max/trim adjustment.
//			Compacted Wingdings fonts. Reduced RAM requirement in settings menu.
//			Added unused RAM usage meter in status screen.
//			Fixed min/max limits bug. Added basic aileron differential setting to RC menu
//			Added basic aileron differential setting to RC menu
//			Level meter responds properly in all orientations
//			Added stick calibration screen for idiots
//			Shrunk last bit of space out of fonts.
//			Added second aileron reversing independent of main aileron and added success confirmation.
//			Servos now return to center after trim, travel adjustments.
//			Button acceleration disabled when moving servos.
//			Battery alarm can be reset by cellcount = 0
// Beta 10	Fixed broken travel limits
//			Allowed trim amd travel adjustment of channels using THROTTLE as a source in PWM modes
//			Added "Sideways" orientation. Dropped default min cell voltage to 3.3V
//			Adjustable launch delay added. Launch resettable if throttle cut within the launch delay.
//			Differential around the right way now :)
//			Updated PWM output driver to have XPS-style staggered outputs
//			Mostly fixed interaction of flaperons and differential.
//			Servos center before trim/travel adjustment
// Beta 11	Allow real-time servo position setting of failsafe positions.	
//			Integrated ninja-level PWM code generation concept suggested by Jim Drew of Xtreme Power Systems
//			Now use -mcall-prologues in linker to save space but required mods to prevent stack overflow etc. 
//			Increased inter-PWM spacing to 128us and recalibrated all PWM routines.
// V1.2		Based on OpenAero2 V1.1 Beta 11 code
// Alpha 1	Added support for XPS Xtreme serial protocol
//			Added support for Spektrum serial protocol
//			Added support for Futaba S-bus serial protocol
//			Switching between all receiver modes now works without the need for a reboot.
//			Much easier to do a factory reset. Reduced RAM/ROM usage.
//			RC inputs display now shows all eight RC channels.
//			Removed logo, added new faster startup messages.
//			Added master/slave binding for Spektrum Satellite RXs at power-up
// Alpha 2	Three configurable flight modes with PID etc.
// Alpha 3	Rearranged menus and added axis-specific gyro modes
// Alpha 4	Rearranged menus and added auto-presets when changing gyro modes
//			Completely reworked/simplified PID gyro input code for I-terms 
//			Added flap deployment speed (0 = normal, 20 = super slow)
// Alpha 5	Reduced menu memory usage. Fixed axis lock bug.
// Alpha 6	Menus position-holding fixed. Added RC deadband adjustment.
//			Reset I-terms when changing between profiles.
//			Fixed order of EEPROM handling at start-up.
// Alpha 7	Sensors now have three states - OFF/ON/REV
// 			Added four phantom channels (PSU9 to PSU12) for mixing tricks
// Alpha 8	Fixed servo adjustment code.
// Beta 1	Fixed S-Bus channel order code. Tested and calibrated all serial protocols.
//			Fixed case where doing a factory reset also re-bound the Satellite RX.
//			Flap speed fixed.
// Beta 2	Stick rate changed to "Lock rate" and only affects Axis lock mode.
//			RC stick rate now always 2. Fixed bug where M8 could not go to 125%. 
//			Output code now covers -135% to 135%.
// Beta 3	Completely changed the value editing menu code so that servos are (almost)
//			continually updated.
// Beta 4	Servos are continually updated when no button pressed.
//			RC channel names changed to suit JR/Spektrum.
// Beta 5	Fixed potential bug with arrays trying to access Something[NONE].
//			Added switchable output mixers.
// Beta 6	Added intelligent RC sync speed algorithm.
//			For RC rates slower than 60Hz, PWM output is after each input pulse/packet
//			For RC rates higher than this, the PWM output is limited to 50Hz if "Servo rate" is LOW
//			RC rate now also tied to "Servo rate" seting so that if "Servo rate" is HIGH, 
//			higher rates can be used.
//			Tweaked Satellite bind timing to be exactly like the real thing.
//			Shrunk memory requirements a bit.
// Beta 7	Fixed bug where settings were lost before repowering after a reset
// Beta 8	Fixed bug where autolevel calibration wrong unless inverted calibration also done.
// Beta 9	Bugfix. Hands-free now takes its trigger from the RC deadband setting.
//
// V1.3		Based on OpenAero2 V1.2 Beta 8 code
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
// Beta 1.1 Added arming
//
//***********************************************************
//* To do
//***********************************************************
//
// Add arming/disarming
//
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
#include "..\inc\io_cfg.h"
#include "..\inc\rc.h"
#include "..\inc\servos.h"
#include "..\inc\vbat.h"
#include "..\inc\gyros.h"
#include "..\inc\init.h"
#include "..\inc\acc.h"
#include "..\inc\isr.h"
#include "..\inc\glcd_driver.h"
#include "..\inc\pid.h"
#include "..\inc\mixer.h"
#include "..\inc\glcd_buffer.h"
#include "..\inc\mugui.h"
#include "..\inc\glcd_menu.h"
#include "..\inc\menu_ext.h"
#include "..\inc\main.h"
#include "..\inc\imu.h"
#include "..\inc\eeprom.h"

#include <avr/interrupt.h> // debug

//***********************************************************
//* Fonts
//***********************************************************

#include "..\inc\Font_Verdana.h" 		// 8 (text) and 14 (titles) points
#include "..\inc\Font_WingdingsOE2.h"	// Cursor and markers

//***********************************************************
//* Defines
//***********************************************************

#define	SERVO_OVERDUE 9765			// Number of T2 cycles before servo will be overdue = 9765 * 1/19531 = 500ms
#define	SLOW_RC_RATE 41667			// Slowest RC rate tolerable for Plan A syncing = 2500000/60 = 41667
#define	SERVO_RATE_LOW 390			// Requested servo rate when in failsafe/Camstab LOW mode. 19531 / 50(Hz) = 390
#define	SERVO_RATE_HIGH 65			// Requested servo rate when in Camstab HIGH mode 300(Hz) = 65
#define LMA_TIMEOUT 1171860			// Number or T2 cycles before Lost Model alarm sounds (1 minute)
#define	PWM_DELAY 250				// Number of 8us blocks to wait between "Interrupted" and starting the PWM pulses 250 = 2ms
#define REFRESH_TIMEOUT 39060		// Amount of time to wait after last RX activity before refreshing LCD (2 seconds)
#define SECOND_TIMER 19531			// Unit of timing for seconds
#define ARM_TIMER_RESET 1000		// RC position to reset timer
#define TRANSITION_TIMER 1953		// Transition timer units (100ms * 16) (1 to 5 = 1.6s to 8s)
#define ARM_TIMER 97655				// Amount of time the sticks must be held to trigger arm/disarm. Currently five seconds.

//***********************************************************
//* Code and Data variables
//***********************************************************

// Flight variables
uint32_t ticker_32;					// Incrementing system ticker
int16_t	transition_value_16 = 0;
int16_t transition_counter = 0;

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
	uint32_t LostModel_timer = 0;
	uint32_t Arm_timer = 0;
	uint32_t RC_Rate_Timer = 0;

	// 16-bit timers
	uint16_t Status_timeout = 0;
	uint16_t UpdateStatus_timer = 0;
	uint16_t Ticker_Count = 0;
	uint16_t Servo_Timeout = 0;
	uint16_t Servo_Rate = 0;
	uint16_t Transition_timeout = 0;

	// Timer incrementers
	uint16_t LoopStartTCNT1 = 0;
	uint16_t RC_Rate_TCNT1 = 0;
	uint8_t Transition_TCNT2 = 0;
	uint8_t Status_TCNT2 = 0;
	uint8_t Refresh_TCNT2 = 0;
	uint8_t Lost_TCNT2 = 0;
	uint8_t Arm_TCNT2 = 0;
	uint8_t Ticker_TCNT2 = 0;
	uint8_t Servo_TCNT2 = 0;
	uint8_t ServoRate_TCNT2 = 0;

	// Locals
	uint8_t	LMA_minutes = 0;
	uint8_t Status_seconds = 0;
	uint8_t Menu_mode = STATUS_TIMEOUT;
	uint8_t i = 0;
	uint8_t	old_flight = 3;			// Current/old flight profile

	// Transition
	uint8_t Transition_state = TRANS_0;
	int16_t	temp_value_16_1 = 0;
	int16_t	temp_value_16_2 = 0;
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
			// If a button is pressed the mode changes to REQ_STATUS
			case IDLE:
				if((PINB & 0xf0) != 0xf0)
				{
					Menu_mode = REQ_STATUS;
					// Reset the status screen timeout
					Status_seconds = 0;
					menu_beep(1);
				}
				break;

			// Request the status be updated when safe
			case REQ_STATUS:
				// Reset safe to refresh flag
				Main_flags &= ~(1 << Refresh_safe);
				Menu_mode = WAITING_STATUS;
				break;

			// Waiting for status screen to be updated
			case WAITING_STATUS:
				// Next time Refresh_safe is set, switch to status screen
				if (Refresh_safe)
				{
					Menu_mode = STATUS;
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
			case WAITING_TIMEOUT:
				// In status screen, change back to idle after timing out
				if (Status_seconds >= Config.Status_timer)
				{
					Menu_mode = STATUS_TIMEOUT;
				}

				// Update status screen while waiting to time out
				else if (UpdateStatus_timer > SECOND_TIMER)
				{
					Menu_mode = REQ_STATUS;
				}

				// Jump to menu if button pressed
				else if(BUTTON1 == 0)
				{
					Menu_mode = MENU;
					menu_beep(1);
				}
				break;

			// In STATUS_TIMEOUT mode, the idle screen is displayed and the mode changed to IDLE
			case STATUS_TIMEOUT:
				// Pop up the Idle screen
				idle_screen();
				// Switch to IDLE mode
				Menu_mode = IDLE;
				break;

			// In MENU mode, 
			case MENU:
				LVA = 0;	// Make sure buzzer is off :)
				menu_main();
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
		//* ((Ticker_Count >> 8) &8) 	= 4.77Hz (LMA and LVA alarms)
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

		// Lost model alarm
		LostModel_timer += (uint8_t) (TCNT2 - Lost_TCNT2);
		Lost_TCNT2 = TCNT2;

		// Reset LMA count if any RX activity, LMA of, or CamStab (no RC used)
		if ((Flight_flags & (1 << RxActivity)) || (Config.LMA_enable == 0) || (Config.CamStab == ON))
		{														
			LostModel_timer = 0;
			Flight_flags &= ~(1 << Model_lost);
			LMA_minutes = 0;
			General_error &= ~(1 << LOST_MODEL); // Clear lost model bit		
		}
		
		if (LostModel_timer > LMA_TIMEOUT)
		{
			LMA_minutes++;
			LostModel_timer = 0;
		}

		// Trigger lost model alarm if enabled and due or failsafe
		if ((LMA_minutes >= Config.LMA_enable) && (Config.LMA_enable != 0))	
		{
			Flight_flags |= (1 << Model_lost);
			General_error |= (1 << LOST_MODEL); // Set lost model bit
		}

		// Beep buzzer if Vbat lower than trigger
		if (GetVbat() < Config.PowerTrigger)
		{
			Alarm_flags |= (1 << LVA_Alarm);	// Set LVA_Alarm flag
			General_error |= (1 << LOW_BATT); 	// Set low battery bit
		}
		else 
		{
			Alarm_flags &= ~(1 << LVA_Alarm);	// Clear LVA_Alarm flag
			General_error &= ~(1 << LOW_BATT); 	// Clear low battery bit
		}

		// Turn on buzzer if in alarm state (BUZZER_ON is oscillating)
		if	(((Alarm_flags & (1 << LVA_Alarm)) ||
			  (Flight_flags & (1 << Model_lost)) || 
			  (Alarm_flags & (1 << SIG_Alarm))) &&
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

		if (Config.ArmMode == ON)
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
				// If aileron is at max, arm the FC
				if (RCinputs[AILERON] > ARM_TIMER_RESET)
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

		// Zero RC when in Failsafe
		if (Flight_flags & (1 << Failsafe))
		{
			for (i = 0; i < MAX_RC_CHANNELS; i++)
			{
				RCinputs[i] = 0;
			}
		}

		// Clear Throttle High error once throtle reset
		if (RCinputs[THROTTLE] < 250)
		{
			General_error &= ~(1 << THROTTLE_HIGH);	
		}

		//************************************************************
		//* Flight mode selection
		//*
		//* Normally, there are three separate profiles, each with PID 
		//* values, however in Transition mode, there is only one 
		//* (Flight mode 2) which is a moving blend of Flight modes 0 to 1.
		//*
		//* The transition speed is controlled by the Config.TransitionSpeed 
		//* setting.
		//************************************************************

		// Manually only allow Flight mode 2 if not in transition mode
		if ((RxChannel[Config.FlightChan] > Config.Autotrigger3) && (Config.MixMode != TRANSITION))
		{
			Config.FlightSel = 2;			// Flight mode 2
		}	
		else if (RxChannel[Config.FlightChan] > Config.Autotrigger2)
		{
			Config.FlightSel = 1;			// Flight mode 1
		}
		else
		{
			Config.FlightSel = 0;			// Flight mode 0
		}

		// Reset update request each loop
		TransitionUpdated = false;

		// For the first startup, set up the right state for the current setup
		if (Config.MixMode == TRANSITION) 
		{
			// Check for initial startup
			if (old_flight == 3)
			{
				switch(Config.FlightSel)
				{
					case 0:
						Transition_state = TRANS_0;
						memcpy(&Config.FlightModeByte[2][1], &Config.FlightModeByte[0][1], (sizeof(flight_control_t) - 1));
						break;
					case 1:
					case 2:
						Transition_state = TRANS_0_to_1_start; // Debug - fix case where board is powered up in Flight mode 2
					//	Transition_state = TRANS_1;
					//	memcpy(&Config.FlightModeByte[2][1], &Config.FlightModeByte[1][1], (sizeof(flight_control_t) - 1));
						break;
					default:
						break;
				}		 
				old_flight = Config.FlightSel;
			}
		}

		// Update when changing flight modes in transition mode
		if (Config.FlightSel != old_flight)
		{
			// When in a timed transition mode
			if ((Config.MixMode == TRANSITION) && (Config.TransitionSpeed != 0))
			{
				// Flag that update is required if mode changed
				TransitionUpdated = true;
			}

			// Reset I-terms so that neutral is reset
			// but not in Transition mode
			if (Config.MixMode != TRANSITION)
			{
				IntegralGyro[ROLL] = 0;	
				IntegralGyro[PITCH] = 0;
				IntegralGyro[YAW] = 0;	
			}
		}

		// Check to see if the transition channel has changed when bound to an 
		// input channel to control transition (Config.TransitionSpeed = 0)
		// If so, set TransitionUpdated flag to trigger an update
		if ((Config.MixMode == TRANSITION) && (Config.TransitionSpeed == 0))
		{
			// 
			if (transition_value_16 != (RCinputs[Config.FlightChan] >> 7))
			{
				TransitionUpdated = true;
			}
		}

		//************************************************************
		//* Transition handling - Always reading from Profile 2
		//************************************************************
		
		if (Config.MixMode == TRANSITION)
		{
			Transition_timeout += (uint8_t) (TCNT2 - Transition_TCNT2);
			Transition_TCNT2 = TCNT2;

			// Update transition value. -1250 to 1250 --> -9 to 9 steps
			transition_value_16 = (RCinputs[Config.FlightChan] >> 7);

			// Update transition state change
			if (TransitionUpdated)
			{
				// Always in the TRANSITIONING state when Config.TransitionSpeed is 0
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
				// For direct entry to the transition state
				else if (Config.FlightSel == 2)
				{
					Transition_state = TRANS_1;
					old_flight = 2;
				}
			}

			// Update state, values and transition_counter every Config.TransitionSpeed if not zero. 1953 = 100ms
			if (((Config.TransitionSpeed != 0) && (Transition_timeout > (TRANSITION_TIMER * Config.TransitionSpeed))) ||
			// If bound to a channel update once
				  TransitionUpdated)
			{
				Transition_timeout = 0;
				TransitionUpdated = false;

				switch(Transition_state)
				{
					case TRANS_0:
						memcpy(&Config.FlightModeByte[2][1], &Config.FlightModeByte[0][1], (sizeof(flight_control_t) - 1));
						break;

					case TRANS_1:
						memcpy(&Config.FlightModeByte[2][1], &Config.FlightModeByte[1][1], (sizeof(flight_control_t) - 1));
						break;

					case TRANS_0_to_1_start:
					case TRANS_1_to_0_start:
						// Set start and end profiles
						if (Transition_state == TRANS_0_to_1_start)
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
						// Transition value update loop. Config.Flightcontrol has sizeof(flight_control_t) bytes
						// but we don't wish to merge the trigger value in byte 0.
						for (i = 1; i < (sizeof(flight_control_t) - 1) ; i++)
						{
							// Get start/end values
							temp_value_16_1 = Config.FlightModeByte[0][i];	// Promote to 16 bits
							temp_value_16_1 = temp_value_16_1 << 8;			// Multiply by 256
							temp_value_16_2 = Config.FlightModeByte[1][i];	// Promote to 16 bits
							temp_value_16_2 = temp_value_16_2 << 8;
							temp_value_16_2 = (temp_value_16_2 - temp_value_16_1) >> 4;	// Divide difference into 16ths

							// Process the transition values. temp_value_16_1 is either from the channel value
							// (transition_value_16) or driven by the transition_counter.
							// RC input is nominally +/- 1250. /128 = +/- 9.77 (span of 19.53). Limit this to 0 to 15.
							// This corresponds to values from -1280 to 768 (988us to 1.8ms) (0 extends to 1.04ms)
							if (Config.TransitionSpeed == 0)
							{
								temp_value_16_1 = transition_value_16;
								temp_value_16_1 = temp_value_16_1 + 7;
							}
							else 
							{
								temp_value_16_1 = transition_counter;
							}
							
							// Limit extent of transition value
							if (temp_value_16_1 < 0) temp_value_16_1 = 0;
							if (temp_value_16_1 > 16) temp_value_16_1 = 16;
							temp_value_16_2 = (temp_value_16_2 * temp_value_16_1); // 0 to 16 * difference

							// Get the start value and scale
							temp_value_16_1 = Config.FlightModeByte[0][i] << 8;

							// Add multiplied difference
							temp_value_16_1 += temp_value_16_2;

							// Scale back to 8-bit value and re-cast
							temp_value_16_1 = (temp_value_16_1 >> 8);

							// Update Profile 3 with merged value
							Config.FlightModeByte[2][i] = (int8_t) temp_value_16_1;

						} // Transition value update loop

						// Update travel limits and triggers
						UpdateLimits();

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
						// Profile 0 to 1, so counter increments to 16
						else
						{
							transition_counter++;
							if (transition_counter >= 16)
							{
								transition_counter = 16;
								Transition_state = TRANS_1;
							}
						}
						break;

					default:
						break;

				} // switch(Transition_state)

			} // Increment transition_counter

		} // if (Config.MixMode == TRANSITION)

		//************************************************************
		//* Set flight mode based on mixmode for transition
		//************************************************************

		if (Config.MixMode == TRANSITION)
		{
			Config.Flight = 2;
		}
		else
		{
			Config.Flight = Config.FlightSel;
		}

		old_flight = Config.FlightSel;

		//************************************************************
		//* Autolevel mode selection
		//************************************************************
		//* Primary override:
		//*		Autolevel always OFF if Config.AutoMode = OFF (default)
		//*		Autolevel disabled if Launch_Block = true
		//*		Autolevel ON if in Advanced failsafe condition
		//************************************************************

		switch(Config.FlightMode[Config.Flight].AutoMode)
		{
			case DISABLED:
				Flight_flags &= ~(1 << AutoLevel);	// De-activate autolevel mode
				break;

			case HANDSFREE:
				if (Flight_flags & (1 << HandsFree))	// If hands free
				{
					Flight_flags |= (1 << AutoLevel);// Activate autolevel mode
				}	
				else
				{
					Flight_flags &= ~(1 << AutoLevel); // De-activate autolevel mode
				}
				break;

			case ALWAYSON:
				Flight_flags |= (1 << AutoLevel);// Activate autolevel mode
				break;

			default:							// Disable by default
				break;
		}

		// Check for advanced Failsafe
		if ((Config.FailsafeType == 1) && (Flight_flags & (1 << Failsafe)) && (Config.CamStab == OFF))
		{
			Flight_flags |= (1 << AutoLevel);
		}

		//************************************************************
		//* Stability mode selection
		//************************************************************
		//* Primary override:
		//*		Stability enabled if Config.StabMode = ON
		//*		Stability always OFF if Config.StabMode = OFF (default)
		//************************************************************

		switch(Config.FlightMode[Config.Flight].StabMode)
		{
			case DISABLED:
				Flight_flags &= ~(1 << Stability);// De-activate autolevel mode
				break;
			case ALWAYSON:
				Flight_flags |= (1 << Stability);// Activate autolevel mode
				break;
			default:							// Disable by default
				break;
		}

		// Reset I-terms when stabilise is off
		if (!(Flight_flags & (1 << Stability)))
		{
			IntegralGyro[ROLL] = 0;	// Debug - may no longer be needed
			IntegralGyro[PITCH] = 0;
			IntegralGyro[YAW] = 0;
		}

		// Read gyros when required
		if ((Flight_flags & (1 << Stability)) || (Flight_flags & (1 << AutoLevel)))
		{
			ReadGyros();		// Read sensors
		}

		// Autolevel mode ON
		if (Flight_flags & (1 << AutoLevel))
		{
			ReadAcc();			// Only read Accs if in AutoLevel mode
			getEstimatedAttitude();
		}
        else
        {
            // Reset IMU each time autolevel restarted
            Main_flags |= (1 << FirstTimeIMU);
        }

		// Remove RC noise and detect when sticks centered
		RC_Deadband();

		// Calculate PID
		Calculate_PID();

		// Calculate mix
		ProcessMixer();

		// Transfer Config.Channel[i].value data to ServoOut[i] and check limits
		UpdateServos();

		//************************************************************
		//* Process servos, failsafe mode
		//************************************************************

		// Work out the current RC rate
		RC_Rate_Timer += (uint16_t) (TCNT1 - RC_Rate_TCNT1);
		RC_Rate_TCNT1 = TCNT1;

		// Assures even without synchronous RX, something will come out at SERVO_RATE (50Hz)
		// Servo_Rate increments at 19.531 kHz, in loop cycle chunks
		Servo_Rate += (uint8_t) (TCNT2 - ServoRate_TCNT2);
		ServoRate_TCNT2 = TCNT2;
		
		// Signal servo overdue after SERVO_OVERDUE time (500ms)
		// Servo_Timeout increments at 19.531 kHz, in loop cycle chunks
		Servo_Timeout += (uint8_t) (TCNT2 - Servo_TCNT2);
		Servo_TCNT2 = TCNT2;
		if (Servo_Timeout > SERVO_OVERDUE)
		{
			Overdue = true;
		}

		// Always clear overdue state is an input received.
		if (Interrupted)
		{
			Servo_Timeout = 0;				// Reset servo failsafe timeout
			Overdue = false;				// And no longer overdue...

			// Check to see if RC rate slower than 60Hz.
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


		// If in CabStab mode (no RC to synch) AND the Servo Rate is set to HIGH run at full speed
		if (Config.Servo_rate == HIGH)
		//if ((Config.CamStab == ON) && (Config.Servo_rate == HIGH))
		{
			if (Servo_Rate > SERVO_RATE_HIGH)
			{
				ServoTick = true;
				Servo_Rate = 0;
			}
		}

		// Otherwise, run at the appropriate rate as per measured RC rate
		else if (Servo_Rate > SERVO_RATE_LOW)
		{
			ServoTick = true;
			Servo_Rate = 0;

			// Force a sync to the next packet for fast RC unless the user has requested it faster
			if ((SlowRC == false) && (Config.Servo_rate == LOW))
			{
				Interrupted = false; 	// Plan B (fast RC)
			}
		}

		// If simply overdue, signal RX error message
		// If in independant camstab mode, don't bother
		if (Overdue && (Config.CamStab == OFF))
		{
			General_error |= (1 << NO_SIGNAL);	// Set NO_SIGNAL bit
			Alarm_flags |= (1 << SIG_Alarm);	// Set SIG_Alarm flag
		}
		else
		{
			General_error &= ~(1 << NO_SIGNAL);	// Clear NO_SIGNAL bit
			Alarm_flags &= ~(1 << SIG_Alarm);	// Clear SIG_Alarm flag
		}

		// Check for failsafe condition (Had RC lock but now overdue)
		if (Overdue && RC_Lock)
		{
			Flight_flags |= (1 << Failsafe);
			RC_Lock = false;
		}


		// Ensure that output_servo_ppm() is synchronised to the RC interrupts
		//if (Interrupted) 					// Plan A
		//if (Interrupted && ServoTick) 	// Plan B

		if ((Interrupted && SlowRC) || 					// Plan A (slow RC)
			(Interrupted && ServoTick && !SlowRC) ||	// Plan B (fast RC)
			(Interrupted && (Config.Servo_rate == HIGH))) // Plan C (any RC with servo rate = HIGH)
		{
			Interrupted = false;			// Reset interrupted flag
			ServoTick = false;
			Servo_Rate = 0;

			Main_flags |= (1 << Refresh_safe); // Safe to try and refresh status screen

			if (Config.RxMode != SBUS)		// SBUS can report its own failsafe
			{
				Flight_flags &= ~(1 << Failsafe);// Cancel failsafe unless failsafe source is receiver
			}

			output_servo_ppm();				// Output servo signal
		}

		// If in "no-RC" failsafe, or when doing independant camstab, just output unsynchronised
		// Note that if RC connected, Camstab will run at the RC rate
		else if ((Overdue && ServoTick) && ((Flight_flags & (1 << Failsafe)) || (Config.CamStab == ON)))
		{
			ServoTick = false;				// Reset servo update ticker
			Servo_Rate = 0;					// Reset servo rate timer
			Main_flags |= (1 << Refresh_safe); // Safe to try and refresh status screen
			output_servo_ppm();				// Output servo signal
		}

		if (Overdue && ServoTick)
		{
			Main_flags |= (1 << Refresh_safe); // Safe to try and refresh status screen
		}

		// Measure the current loop rate
		cycletime = TCNT1 - LoopStartTCNT1;	// Update cycle time
		LoopStartTCNT1 = TCNT1;				// Measure period of loop from here
		ticker_32 += cycletime;

	} // main loop
} // main()
