#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdarg.h>
#include <ctype.h>


/** PINS **/

#define CLOCK_IN_pin PB4
#define CLOCK_IN_init DDRB &= ~(1<<CLOCK_IN_pin)
#define CLOCK_IN (PINB & (1<<CLOCK_IN_pin))

/*
#define CLOCK_OUT_pin PB4
#define CLOCK_OUT_init DDRB |=(1<<CLOCK_OUT_pin)
#define CLOCK_OUT_PORT PORTB
*/

#define DEBUG_pin PB0
#define DEBUG_init DDRB|=(1<<DEBUG_pin)
#define DEBUGFLIP PORTB ^= (1<<DEBUG_pin)

#define OUT_PORT PORTB
#define OUT_DDR DDRB
#define OUT_MASK 0b00001111
#define OUT_init OUT_DDR |= OUT_MASK

#define ALLOFF(p,x) p &= ~(x)
#define ALLON(p,x) p |= (x)
#define OFF(p,x) p &= ~(1<<(x))
#define TOGGLE(p,x) p ^= (1<<(x))
#define ON(p,x) p |= (1<<(x))

/** TIMER **/
#define TMROFFSET 0

volatile long tmr;

SIGNAL (SIG_OVERFLOW0){
	tmr+=1;
	TCNT0=TMROFFSET; // Re-init timer
	//DEBUGFLIP;

}

long gettmr(void){
	long result;
	cli();
	result = tmr;
	sei();
	return result;
}

void inittimer(void){

	TCCR0A=0;
	//TCCR0B=(1<<CS00) | (1<<CS01) | (0<<CS02); //prescale/64 = 125kHz (8us)
	TCCR0B=(0<<CS00) | (1<<CS01) | (0<<CS02); //prescale/8 = 1MHz (1us)
	//TCCR0B=(0<<CS00) | (0<<CS01) | (1<<CS02); //prescale/256 = 31.25kHz (32us)
	//each tmr is 256uS

	TCNT0=TMROFFSET;
	TIMSK=(1<<TOIE0); 					// Enable timer overflow interrupt

	tmr=0;

	sei();
}



/** MAIN **/


int main(void){

	char clock_up=0;
	char very_first_clock_pulse=1;
	unsigned long period;
	unsigned int div0=1024,div1=1024,div2=1024,div3=1024;
	unsigned int o0=0,o1=0,o2=0,o3=0;

	CLOCK_IN_init; 
	//CLOCK_OUT_init;

	OUT_init;

	DEBUG_init;

	inittimer();

	while(1){

		if (CLOCK_IN){
			if (!clock_up){
				clock_up=1;//rising edge only						
				if (very_first_clock_pulse) period=10000;
				else
				period=gettmr();

				cli();tmr=0;sei();

				if (period==0) period=1;
				//if (period>256) period=256;

				div3=256/period;	
				div3+=4;

				div2=32;
				div1=64;
				div0=0;

				if (++o0>div0){ o0=0;TOGGLE(OUT_PORT,PB0);}
				if (++o1>div1){ o1=0;TOGGLE(OUT_PORT,PB1);}
				if (++o2>div2){ o2=0;TOGGLE(OUT_PORT,PB2);}
				if (++o3>div3){ o3=0;TOGGLE(OUT_PORT,PB3);}

			}
			very_first_clock_pulse=0;
		}else{
			clock_up=0;

		}
	
	}	//endless loop


	return(1);
}
