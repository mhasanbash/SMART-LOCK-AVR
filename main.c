#include <avr/io.h>
#include <util/delay.h>
#include "lcd/lcd.h"
#define F_CPU 8000000UL
#include <stdio.h>

char keypad[16] = {'7', '8', '9', '/',
				   '4', '5', '6', '*',
				   '1', '2', '3', '-',
				   ' ', '0', '=', '+'};

int pass_index, checkpass;
char password[4] = {'1','2','3','4'};
char input_pass[4] = {' ', ' ', ' ', ' '};

void init(){
	lcd_clrscr();
	lcd_gotoxy(0,0);
	lcd_puts("enter password:");
	lcd_gotoxy(0,1);
}

void read_keypad(){
	
}

int main(void)
{
    /* Replace with your application code */
	
	lcd_init(LCD_DISP_ON);
	unsigned char k, t , col;
	int key_code, key_col, key_row, i;
	init();
	
	DDRD = 0x0F;
	while (1)
	{
		PORTD = 0xF0;
		col = PIND  | 0x0F;
		
		//wait for all keys is free
		while (col != 0xFF)
		{
			col = PIND | 0x0F;
		}
		
		//wait for one key is clicked
		while (col == 0xFF)
		{
			col = PIND | 0x0F;
		}
		
		_delay_ms(500);
		k = 0x10;
		
		//detect column
		for (i = 0; i < 4; i++)
		{
			t = k | col;
			if (t == 0xFF){
				key_col = i;
				break;
			}
			else
				k = k << 1;
		}
		
		k = 0x01;
		
		//detect row
		for (i = 0; i < 4; i++)
		{
			PORTD = ~(k);
			_delay_us(5); // Small delay for the signal to settle
			col = PIND | 0x0F;
			if (col != 0xFF)
			{
				key_row = i;
				break;
			}
			else
				k = k << 1;
		}
		
		key_code = 4*(key_row)+key_col;
		lcd_putc(keypad[key_code]);
		if (key_code == 12){
			init();
		}
		_delay_ms(500);
	}
	return 0;
}

