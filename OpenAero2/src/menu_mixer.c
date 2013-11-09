//***********************************************************
//* menu_mixer.c
//***********************************************************

//***********************************************************
//* Includes
//***********************************************************

#include <avr/pgmspace.h> 
#include <avr/io.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "..\inc\io_cfg.h"
#include "..\inc\init.h"
#include "..\inc\mugui.h"
#include "..\inc\glcd_menu.h"
#include "..\inc\menu_ext.h"
#include "..\inc\glcd_driver.h"
#include "..\inc\main.h"
#include "..\inc\eeprom.h"
#include "..\inc\mixer.h"

//************************************************************
// Prototypes
//************************************************************

// Menu items
void menu_mixer(uint8_t i);

//************************************************************
// Defines
//************************************************************

#define INPUTITEMS 7
#define OUTPUTITEMS 7

#define MIXERSTART 211 	// Start of Menu text items
#define MIXOFFSET  80	// Value offsets

//************************************************************
// RC menu items
//************************************************************
	 
const uint8_t MixerMenuText[2][INPUTITEMS+1] PROGMEM = 
{
	{230,105,0,143,143,143,143,143},
	{230,0,230,0,230,0,230,0}
};

const menu_range_t mixer_menu_ranges[2][INPUTITEMS+1] PROGMEM = 
{
	{
		// Input mixer ranges (8)
		{CH1,CH12,1,1,CH1},				// Ch. number
		{THROTTLE,NOCHAN,1,1,NOCHAN},	// Source
		{-125,125,5,0,100},				// Source volume (%)
		{OFF, REV,1,1,OFF},				// roll_gyro
		{OFF, REV,1,1,OFF},				// pitch_gyro
		{OFF, REV,1,1,OFF},				// yaw_gyro
		{OFF, REV,1,1,OFF},				// roll_acc
		{OFF, REV,1,1,OFF},				// pitch_acc
	},
	{
		// Output mixer ranges (8)
		{CH1,CH12,1,1,CH1},				// Ch. number
		{-125,125,5,0,0},				// Offset (%)
		{SRC1,NOMIX,1,1,NOMIX},			// Output B
		{-125,125,5,0,0},				// Output B volume
		{SRC1,NOMIX,1,1,NOMIX},			// Output C
		{-125,125,5,0,0},				// Output C volume
		{SRC1,NOMIX,1,1,NOMIX},			// Output D
		{-125,125,5,0,0},				// Output D volume
	}
};

//************************************************************
// Main menu-specific setup
//************************************************************

void menu_mixer(uint8_t section)
{
	static uint8_t mix_top = MIXERSTART;
	static	uint8_t old_section;
	int8_t *value_ptr;

	int8_t values[INPUTITEMS+1];
	menu_range_t range;
	uint8_t text_link = 0;
	uint8_t offset;			// Index into channel structure
	uint8_t	items;			// Items in group

	// If submenu item has changed, reset submenu positions
	if (section != old_section)
	{
		mix_top = MIXERSTART;
		old_section = section;
	}

	// Get mixer menu offsets
	// 1 = input mixer data, 2 = output mixer data
	switch(section)
	{
		case 1:
			items = INPUTITEMS+1;
			offset = 0;
			value_ptr = &values[0];
			break;
		case 2:
			items = OUTPUTITEMS+1;
			offset = INPUTITEMS+1;
			value_ptr = &values[0];
			break;
		default:
			items = INPUTITEMS+1;
			offset = 0;
			value_ptr = &values[0];
			break;
	}

	while(button != BACK)
	{
		// Load values from eeprom and insert channel number at the top of each - messy
		values[0] = Config.MenuChannel;
		if (section == 1)
		{
			memcpy(&values[1],&Config.Channel[Config.MenuChannel].source_a,(sizeof(int8_t) * INPUTITEMS));
		}
		else
		{
			memcpy(&values[1],&Config.Channel[Config.MenuChannel].offset,(sizeof(int8_t) * OUTPUTITEMS));
		}

		// Print menu
		print_menu_items(mix_top + offset, MIXERSTART + offset, value_ptr, 1, (prog_uchar*)mixer_menu_ranges[section - 1], 0, MIXOFFSET, (prog_uchar*)MixerMenuText[section - 1], cursor);

		// Handle menu changes
		update_menu(items, MIXERSTART, offset, button, &cursor, &mix_top, &menu_temp);
		range = get_menu_range ((prog_uchar*)mixer_menu_ranges[section - 1], menu_temp - MIXERSTART - offset);

		if (button == ENTER)
		{
			text_link = pgm_read_byte(&MixerMenuText[section - 1][menu_temp - MIXERSTART - offset]);
			do_menu_item(menu_temp, value_ptr + (menu_temp - MIXERSTART - offset), 1, range, 0, text_link, false, 0);
		}

		// Save modified data back to Config
		// Copy channel number back to global
		switch(section)
		{
			case 1:
				memcpy(&Config.Channel[Config.MenuChannel].source_a,&values[1],(sizeof(int8_t) * INPUTITEMS));
				Config.MenuChannel = values[0];
				break;
			case 2:
				memcpy(&Config.Channel[Config.MenuChannel].offset,&values[1],(sizeof(int8_t) * OUTPUTITEMS));
				Config.MenuChannel = values[0];
				break;
			default:
				break;
		}

		// Save and exit
		if (button == ENTER)
		{
			UpdateLimits();			 // Update travel limits based on percentages
			Save_Config_to_EEPROM(); // Save value and return
		}

	}
	_delay_ms(200);
}



