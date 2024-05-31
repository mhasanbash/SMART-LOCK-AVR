#include <avr/io.h>
#include <util/delay.h>
#include "lcd/lcd.h"
#define F_CPU 8000000UL

char keypad[16] = {'7', '8', '9', '/',
				   '4', '5', '6', '*',
				   '1', '2', '3', '-',
				   ' ', '0', '=', '+'};

int fault_count;
int pass_index, checkpass;
char password[4] = {'1','2','3','4'};
char input_pass[4] = {' ', ' ', ' ', ' '};
	
unsigned char k, t , col;
int key_code, key_col, key_row, i;

void init(){
	lcd_clrscr();
	lcd_gotoxy(0,0);
	lcd_puts("enter password:");
	lcd_gotoxy(0,1);
	pass_index = 0;
}


void check_password(){
	checkpass = 1;
	for (i=0; i<4; i++)
	{
		if (password[i] != input_pass[i])
		{
			checkpass = 0;
			break;
		}
	}
}


void read_keypad(){
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
}


void menu(){
	while(1){
		
		lcd_clrscr();
		lcd_gotoxy(0,0);
		lcd_puts("1-change password");
		lcd_gotoxy(0,1);
		lcd_puts("2-exit");
		_delay_ms(500);
		read_keypad();
		key_code = 4*(key_row)+key_col;
		
		if (keypad[key_code] == '1'){
			break;
		}
		else if(keypad[key_code] == '2'){
			break;
		}
	}
}


void athentication(){
	read_keypad();
	
	key_code = 4*(key_row)+key_col;
	lcd_putc(keypad[key_code]);
	input_pass[pass_index++] =  keypad[key_code];
	
	if (pass_index == 4){
		check_password();
		if (checkpass)
		{
			init();
			lcd_puts("password correct");
			_delay_ms(10000);
			menu();
			init();
		}
		else{
			init();
			lcd_puts("password wrong!");
			_delay_ms(10000);
			init();
			fault_count++;
		}
	}
	
	if (key_code == 12){
		init();
	}
	_delay_ms(500);
}


void danger(){
	for (i=1; i<10; i++)
	{
		PORTC = 0x01;
		_delay_ms(5000);
		PORTC = 0x00;
		_delay_ms(5000);
	}
}

int main(void)
{
	
	lcd_init(LCD_DISP_ON);
	init();
	DDRD = 0x0F;
	DDRC = 0x0F;
	fault_count = 0;
	
	while (1)
	{
		athentication();
		if(fault_count == 3){
			danger();
			fault_count = 0;
		}
	}
	return 0;
}

