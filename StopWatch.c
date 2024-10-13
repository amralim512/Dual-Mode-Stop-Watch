/* Dual Mode Stop Watch
 * Author: Amr Adel
 * */


#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

#define debounce 30
#define printer_delay 2

unsigned char toggle_flag=0,toggle=0,min=0,hr=0,sec=0,flag0=0,flag1=0,flag2=0;
unsigned char flag0_0=0,flag1_1=0,flag2_2=0;

void count_up(void)
{
	// enable the mode of increment and the led to indicate that mode and close buzzer led
//	PORTD|=(1<<PD4);
//	PORTD&=~(1<<PD5);
	PORTD&=~(1<<PD0);
	sec++;
	if(sec==60){sec=0;min++;}
	if(min==60){sec=0;min=0;hr++;}
}

void count_down(void)
{
	// change the led of the mode and enable the buzzer led
	if(toggle==1){
//		PORTD|=(1<<PD5);
//		PORTD&=~(1<<PD4);
		if(sec==0)
		{
			if(min==0)
			{
				if(hr==0)
				{
					PORTD|=(1<<PD0); // enable the buzzer led
				}
				else
				{
					hr--;
					min=59;
					sec=59;

				}
			}
			else
			{
				min--;
				sec=59;
			}
		}
		else
		{
			sec--;
		}
	}
}

void int0()
{//falling edge of reset button
	DDRD  &= (~(1<<PD2));
	PORTD|=(1<<PD2);
	MCUCR |= (1<<ISC01);
	GICR  |= (1<<INT0);
}
void int1(){//rising edge of pause button
	DDRD  &= (~(1<<PD3));
	MCUCR |= (1<<ISC11)|(1<<ISC10);
	GICR  |= (1<<INT1);
}
void int2(){//falling edge of resume button
	DDRD  &= (~(1<<PB3));
	MCUCSR &=~ (1<<ISC2);
	GICR  |= (1<<INT2);
}
void HR_UP(){
	if(!(PINB&(1<<PB1)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB1))){

			if(flag0==0)
			{
				hr++; // increment the hr
			}
			flag0=1;
		}

	}
	else
	{
		flag0=0;
	}
}
void HR_DOWN(){
	if(!(PINB&(1<<PB0)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB0))){

			if(flag0_0==0)
			{
				if(!hr==0)
				{
					hr--; // decrement the hr
				}

				flag0_0=1;
			}
		}
	}
	else
	{
		flag0_0=0;
	}
}
void MIN_UP(){
	if(!(PINB&(1<<PB4)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB4))){

			if(flag1==0)
			{
				min++; // increment the minutes
				flag1=1;
				if(min==60){
					min=0;
					hr++; // in case minutes become 59 increase the hours and reset the minutes
				}

			}
		}
		else
		{
			flag1=0;
		}
	}
}
void MIN_DOWN(){
	if(!(PINB&(1<<PB3)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB3))){

			if(flag1_1==0)
			{
				if(!min==0){
					min--; // decrement the minutes
				}

				flag1_1=1;
			}
		}else
		{
			flag1_1=0;
		}
	}
}
void SEC_UP(){
	if(!(PINB&(1<<PB6)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB6))){

			if(flag2==0)
			{
				sec++; // increment the seconds
				flag2=1;
				if(sec==60){
					sec=0;
					min++; // increment from minutes in case seconds=59
					flag2=1;
				}
			}
		}else
		{
			flag2=0;
		}
	}
}
void SEC_DOWN(){
	if(!(PINB&(1<<PB5)))
	{
		_delay_ms(debounce);
		if(!(PINB&(1<<PB5))){
			if(flag2_2==0)
			{
				if(!sec==0){
					sec--; // decrement from the seconds
				}
				flag2_2=1;
			}
		}else
		{
			flag2_2=0;
		}
	}
}
void mode(void){
	// function to toggle the flag of the mode to determine in case we will count up or down
	if(!(PINB&(1<<PB7))){
		_delay_ms(debounce);
		if(!(PINB&(1<<PB7))){
			if(toggle_flag==0){
				PORTD^=(1<<PD5);
				PORTD^=(1<<PD4);
				toggle=!toggle;
				toggle_flag=1;

			}
		}
		else{
			toggle_flag=0;
		}
	}
}
void timer1_enable(void){
	/*enable or timer in mode 12 where freq in ICR
	  we need interrupt each 1s so F_timer=1 and F_clk=16MHz and N=1024
      from the rule top=15624 placed in ICR1 to make T_timer=1s	*/
	TCNT1=0;
	ICR1=15624;
	TIMSK=(1<<OCIE1A);
	TCCR1A=(1<<FOC1A);
	TCCR1B=(1<<WGM13)|(1<<WGM12)|(1<<CS12)|(1<<CS10);
}
void display(void){
	/* we need to keep printing each time so we enable 1 by 1 of port A
	    so we enable each bit by bit of port A to display all the numbers
	    and keep a small delay between each segment just to keep all enabled*/
	PORTA=0x01;
	PORTC = (PORTC & 0xF0) | (hr/10 & 0x0F); // to assign in the tens of the hour
	_delay_ms(printer_delay);
	PORTA=0x02;
	PORTC = (PORTC & 0xF0) | (hr%10 & 0x0F); // to assign in the units of the hour
	_delay_ms(printer_delay);
	PORTA=0x04;
	PORTC = (PORTC & 0xF0) | (min/10 & 0x0F); // to assign in the tens of the minutes
	_delay_ms(printer_delay);
	PORTA=0x08;
	PORTC = (PORTC & 0xF0) | (min%10 & 0x0F); // to assign in the units of the minutes
	_delay_ms(printer_delay);
	PORTA=0x10;
	PORTC = (PORTC & 0xF0) | (sec/10 & 0x0F); // to assign in the tens of the seconds
	_delay_ms(printer_delay);
	PORTA=0X20;
	PORTC = (PORTC & 0xF0) | (sec%10 & 0x0F); // to assign in the units of the seconds
	_delay_ms(printer_delay);
}
ISR(INT0_vect){
	// rest all values of timer and counters
	TCNT1=0;
	sec=0;
	min=0;
	hr=0;
}
ISR(INT1_vect){
	// pause our Clock to stop at specific time by stopping prescalar
	TCCR1B&=~(1<<CS12);
	TCCR1B&=~(1<<CS10);
	TCCR1B&=~(1<<CS10);
}
ISR(INT2_vect){
	// resume the Clock by resuming the prescalar
	TCCR1B|=(1<<CS12)|(1<<CS10);
}
ISR(TIMER1_COMPA_vect){
	// interrupt each 1 sec sent by timer so that we increment or decrement based on mode we are in
	if(toggle==0){
		count_up();
	}
	else {
		count_down();
	}
}

int main(void){
	DDRD|=0x31; //for all port D buttons and LEDs
	PORTD|=0x10; // enable internal pull up resistance
	DDRB|=0x00; //Push Buttons of port B
	PORTB|=0xFF; // enable all port B internal pull up resistance
	DDRC|=0x0F;// output for port C 7-Segment
	PORTC|=0xF0; // we only write in 1st 4 bits
	DDRA|=0x2F; // To determine which 7-Segment
	PORTA|=0x3F; // to enable only 1st 6 bits of port A
	SREG|= (1<<7); // to enable the general register
	timer1_enable();
	int0();
	int1();
	int2();
	while(1){
		display();
		HR_UP();
		HR_DOWN();
		MIN_UP();
		MIN_DOWN();
		SEC_UP();
		SEC_DOWN();
		mode();
	}
}
