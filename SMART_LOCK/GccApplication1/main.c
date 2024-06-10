#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include "lcd/lcd.h"
#define F_CPU 8000000UL

#define PASSWORD_LENGTH 4
// ?????? ????? ???????
#define MAX_PASSWORDS 3
// ???? ???? ??????? ?? EEPROM
#define EEPROM_START_ADDRESS 0x00
#define EEPROM_COUNT_ADDRESS (EEPROM_START_ADDRESS + (MAX_PASSWORDS * (PASSWORD_LENGTH + 1)))
#define EEPROM_SIZE 1024


char keypad[16] = {'7', '8', '9', '/',
				   '4', '5', '6', '*',
				   '1', '2', '3', '-',
				   ' ', '0', '=', '+'};

int fault_count;
int pass_index, checkpass;
char saved_password[5] = {'1','2','3','4','\0'};
char main_password[4][5] = {{' ', ' ', ' ', ' ', '\0'}, {' ', ' ', ' ', ' ', '\0'}, {' ', ' ', ' ', ' ', '\0'}, {' ', ' ', ' ', ' ', '\0'}} ;

char input_pass[4] = {' ', ' ', ' ', ' '};
char new_pass[4] = {' ', ' ', ' ', ' '};
	
unsigned char k, t , col;
int key_code, key_col, key_row, i;



void init_auth(){
	lcd_clrscr();
	lcd_gotoxy(0,0);
	lcd_puts("enter password:");
	lcd_gotoxy(0,1);
	pass_index = 0;
}


void init(){
	lcd_clrscr();
	lcd_gotoxy(0,0);
}


void save_password_to_eeprom(uint8_t index, const char* password) {
	if (index >= MAX_PASSWORDS) return; 
	uint16_t address = EEPROM_START_ADDRESS + (index * (PASSWORD_LENGTH + 1));
	uint8_t length = strlen(password);
	eeprom_write_byte((uint8_t*)address, length);
	for (uint8_t i = 0; i < length; i++) {
		eeprom_write_byte((uint8_t*)(address + 1 + i), password[i]);
	}
}

void read_password_from_eeprom(uint8_t index, char* password) {
	if (index >= MAX_PASSWORDS) return; 
	uint16_t address = EEPROM_START_ADDRESS + (index * (PASSWORD_LENGTH + 1));
	uint8_t length = eeprom_read_byte((uint8_t*)address); 
	for (uint8_t i = 0; i < length; i++) {
		password[i] = eeprom_read_byte((uint8_t*)(address + 1 + i));
	}
	password[length] = '\0';
}


int add_pass(const char* new_password) {
	uint8_t count = eeprom_read_byte((uint8_t*)EEPROM_COUNT_ADDRESS);
	if (count >= MAX_PASSWORDS) {
		init();
		lcd_puts("memory is full");
		_delay_ms(1000);
		init();
		return -1; 
	}
	save_password_to_eeprom(count, new_password);
	eeprom_write_byte((uint8_t*)EEPROM_COUNT_ADDRESS, count + 1);
	return 0;
}

void clear_eeprom(void) {
	for (uint16_t i = 0; i < EEPROM_SIZE; i++) {
		eeprom_write_byte((uint8_t*)i, 0xFF);
	}
}

int replace_pass(uint8_t index, const char* new_password) {
	if (index >= MAX_PASSWORDS) {
		return -1; 
	}
	save_password_to_eeprom(index, new_password);
	return 0; 
}




void check_password(){
	checkpass = 1;
	for (int j = 0; j<4; j++)
	{
		for (i=0; i<4; i++)
		{
			if (main_password[j][i] != input_pass[i])
			{
				checkpass = 0;
				break;
			}
			checkpass = 1;
		}
		if (checkpass == 1)
		{
			checkpass = 1;
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


void read_new_password(){
	read_keypad();
	new_pass[0] = 4*(key_row)+key_col;
	lcd_putc('*');
	_delay_ms(500);
	read_keypad();
	new_pass[1] = 4*(key_row)+key_col;
	lcd_putc('*');
	_delay_ms(500);
	read_keypad();
	new_pass[2] = 4*(key_row)+key_col;
	lcd_putc('*');
	_delay_ms(500);
	read_keypad();
	new_pass[3] = 4*(key_row)+key_col;
	lcd_putc('*');
	_delay_ms(500);
	init();
}


void menu(){
	while(1){
		
		init();
		lcd_puts("1-manage password");
		lcd_gotoxy(0,1);
		lcd_puts("2-exit");
		_delay_ms(500);
		read_keypad();
		key_code = 4*(key_row)+key_col;
		
		if (keypad[key_code] == '1'){
			init();
			lcd_puts("1-add password");
			lcd_gotoxy(0,1);
			lcd_puts("2-change password");
			_delay_ms(500);
			read_keypad();
			key_code = 4*(key_row)+key_col;
			
			if (keypad[key_code] == '1'){
				init();
				
				lcd_puts("enter new password");
				_delay_ms(500);
				read_new_password();
				_delay_ms(500);
				add_pass(new_pass);
			}
			else if(keypad[key_code] == '2'){
				init();
				
				lcd_puts("which index?");
				read_keypad();
				key_code = 4*(key_row)+key_col;
				_delay_ms(500);
				
				lcd_puts("enter new password");
				read_new_password();
				_delay_ms(500);
				replace_pass(key_code, new_pass);
			}
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
			init_auth();
			lcd_puts("password correct");
			_delay_ms(10000);
			menu();
			init_auth();
		}
		else{
			init_auth();
			lcd_puts("password wrong!");
			_delay_ms(10000);
			init_auth();
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
	
	add_pass(saved_password);
	
	read_password_from_eeprom(0, main_password[0]);
	lcd_puts(main_password[1]);
	
	_delay_ms(1000);
	init_auth();
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

