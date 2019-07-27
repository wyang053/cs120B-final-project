/*
 * GccApplication1.c
 * Ping Pong game. inputs: 4 buttons. output:8x8 led matrix.
 * Complexities: shift registers, 2 players, game logic.
 *
 *The classic pong game consists of two paddles on opposite 
 *sides of the screen. The user is controlling one of the 
 *paddles while the computer controls the other paddle. 
 *There is a ball that bounces in between the paddles. 
 *The ball speeds up and changes angle (as well as direction) 
 *when it hits the corner of either paddle. If the ball hits 
 *the center of the paddle it continues at the same angle, 
 *but reverses direction.
 *
 * Created: 7/23/2019 11:38:09 AM
 * Author : Weiyi Yang
 */
#define 	F_CPU   8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#define  DATA_HIGH()	(PORTB |= (1<<PORTB0))
#define  DATA_LOW()		(PORTB &= ~ (1<<PORTB0))
#define  STORE_HIGH()	(PORTB |= (1<<PORTB1))
#define  STORE_LOW()	(PORTB &= ~ (1<<PORTB1))
#define  SHIFT_HIGH()	(PORTB |= (1<<PORTB2))
#define  SHIFT_LOW()	(PORTB &= ~ (1<<PORTB2))
#define  LED1_HIGH()	(PORTB |= (1<<PORTB3))
#define  LED1_LOW()		(PORTB &= ~ (1<<PORTB3))
#define  LED2_HIGH()	(PORTB |= (1<<PORTB4))
#define  LED2_LOW()		(PORTB &= ~ (1<<PORTB4))
#define  up1			((PINA & (1<<PINA0)))
#define  down1			((PINA & (1<<PINA1)))
#define  up2			((PINA & (1<<PINA2)))
#define  down2			((PINA & (1<<PINA3)))
#define  reset			((PINA & (1<<PINA4)))
//#define	 _BV( bit )		( 1<<(bit) )
void store_SM();
enum store_states{store_init, store_high, store_low}store_state;
enum paddle1SM_states{paddle1SM_wait, up1_press, up1_release, 
					  down1_press, down1_release}paddle1SM_state;
enum paddle2SM_states{paddle2SM_wait, up2_press, up2_release,
					  down2_press, down2_release}paddle2SM_state;					  	
enum ballLogicSM_states{ball_init, ball_wait, ball_left, ball_right, 
						ball_upleft, ball_downleft, ball_upright, 
						ball_downright, player1win, player2win
						}ballLogicSM_state;
void shiftOut(uint8_t val);
volatile unsigned char TimerFlag=0;
unsigned long _avr_timer_M=1;
unsigned long _avr_timer_cntcurr=0;
unsigned char store_SM_elapsedTime;
unsigned short ballLogicSM_elapsedTime;
unsigned char paddle1POS;
unsigned char paddle2POS;
unsigned char xPOS;
unsigned char yPOS;// y position is in binary

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1=0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
	
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;

}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr== 0)
	{
		TimerISR();
		_avr_timer_cntcurr=_avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M=M;
	_avr_timer_cntcurr = _avr_timer_M;
}
 uint8_t pattern[8] = { //initial matrix
			0b00111000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			};

void softRest(){
	if (reset)
	{
		paddle1SM_state = paddle1SM_wait;
		paddle2SM_state = paddle2SM_wait;
		ballLogicSM_state = ball_init;
		LED1_LOW();
		LED2_LOW();
		paddle1POS=16;
		paddle2POS=16;
		xPOS = 3;
		yPOS = 16;
		ballPOS_update();
		pattern[0]=0b00111000;
		pattern[7]=0b00111000;
	}
}			
	
	
void ballPOS_update(){
	/*
	char i;
	while (i>0 && i<7)
	{
	pattern[i]=0;	
	i++;
	}
	*/
	
	pattern [xPOS]=(yPOS)<<0;
	if (ballLogicSM_state == ball_left || ballLogicSM_state == ball_upleft 
	|| ballLogicSM_state == ball_downleft)
	{
		pattern[xPOS+1]=0;
	}else if (ballLogicSM_state == ball_right 
	|| ballLogicSM_state == ball_upright || ballLogicSM_state == ball_downright)
	{
		pattern[xPOS-1]=0;
	}
	
}	
 
void ballLogicSM(){
	switch (ballLogicSM_state){
		case ball_init:
		ballLogicSM_state = ball_wait;
		break;
		case  ball_wait:
		ballLogicSM_state = ball_left;
		break;
		case ball_left:
		if (xPOS>1)
		{
			ballLogicSM_state = ball_left;
		}else {
			if (paddle1POS == yPOS)
			{
				ballLogicSM_state = ball_right;
			}else if (paddle1POS == yPOS<<1) //hit paddle1 bottom 
			{
				ballLogicSM_state = ball_upright;
			}else if (paddle1POS == yPOS>>1)
			{
				ballLogicSM_state = ball_downright;
			}else
			{
				ballLogicSM_state = player2win;
			}
			
		}
		break;
		case ball_right:
		if (xPOS!=6)
		{
			ballLogicSM_state = ball_right;
			}else {
			if (paddle2POS == yPOS ) //test middle hit
			{
				ballLogicSM_state = ball_left;
			}else if (paddle2POS == yPOS<<1) 
			{
				ballLogicSM_state = ball_upleft;
			}else if (paddle2POS == yPOS>>1)
			{
				ballLogicSM_state = ball_downleft;
			}
			
		}
		break;
		
		case ball_upright:
		if (yPOS<128 && xPOS>1 )
		{
			ballLogicSM_state = ball_upright;
		} else {
			if (xPOS!=6)
			{
				ballLogicSM_state = ball_downright;
			}else if (paddle2POS==yPOS>>2) // top hit
				{
					ballLogicSM_state = ball_downleft;
				}else if (paddle2POS==yPOS>>1) // middle hit
				{
					ballLogicSM_state = ball_downleft;
				}else if (paddle2POS==yPOS) // bottom hit
				{
					ballLogicSM_state = ball_upleft;
				}
			
		}
		break;
		
		case ball_downright:
		if (yPOS>1 && xPOS<6)
		{
			ballLogicSM_state = ball_downright;
			} else {
				if (xPOS!=6)
				{
					ballLogicSM_state = ball_upright;
				}else if (paddle2POS==yPOS>>2) // top hit
				{
					ballLogicSM_state = ball_downleft;
				}else if (paddle2POS==yPOS>>1) // middle hit
				{
					ballLogicSM_state = ball_upleft;
				}else if (paddle2POS==yPOS) // bottom hit
				{
					ballLogicSM_state = ball_upleft;
				}
			
		}
		break;
		case  ball_downleft:
		if (yPOS>1 && xPOS>1)
		{
			ballLogicSM_state = ball_downleft;
			
		}else 
		{
			if(xPOS!=1){
				ballLogicSM_state = ball_upleft;
			} else if (paddle1POS==yPOS>>2) // top hit
			{
				ballLogicSM_state = ball_downright;
			}else if (paddle1POS==yPOS>>1) // middle hit
			{
				ballLogicSM_state = ball_upright;
			}else if (paddle1POS==yPOS) // bottom hit
			{
				ballLogicSM_state = ball_upright;
			}
			
		}
		break;
		
		case ball_upleft:
		if (yPOS<128 && xPOS>1)
		{
			ballLogicSM_state = ball_upleft;
		}else {
			if (xPOS!=1)
			{
				ballLogicSM_state = ball_downleft;
			}else if (paddle1POS==yPOS<<2) // bottom hit
				{
					ballLogicSM_state = ball_upright;
				}else if (paddle1POS==yPOS<<1) // middle hit
				{
					ballLogicSM_state = ball_downright;
				}else if (paddle1POS==yPOS) // top hit
				{
					ballLogicSM_state = ball_upright;
				}
			
		}
		break;
		case player1win:
		ballLogicSM_state = player1win;
		break;
		case player2win:
		ballLogicSM_state = player2win;
		break;
	}
		
	
	switch (ballLogicSM_state){
		case ball_init:
		xPOS = 3;
		yPOS = 16;
		ballPOS_update();
		break;
		case  ball_wait:
		break;
		case ball_left:
		xPOS --;
		yPOS = 16;
		ballPOS_update();
		break;
		case ball_right:
		xPOS ++;
		yPOS = 16;
		ballPOS_update();
		break;
		case ball_upright:
		xPOS ++;
		yPOS =yPOS*2;
		ballPOS_update();
		break;
		case ball_downright:
		xPOS ++;
		yPOS =yPOS/2;
		ballPOS_update();
		break;
		case ball_downleft:
		xPOS --;
		yPOS =yPOS/2;
		ballPOS_update();
		break;
		case ball_upleft:
		xPOS --;
		yPOS =yPOS*2;
		ballPOS_update();
		break;
		case player1win:
		LED1_HIGH();
		break;
		case player2win:
		LED2_HIGH();
		break;
	}
	/*  not working switch
	switch (ballLogicSM_state){
		case ball_init:
		ballLogicSM_state = ball_wait;
		break;
		case ball_wait:
		ballLogicSM_state = ball_left;
		break;
		
		case  ball_left:
		
		if (!(xPOS==1))
		{
			ballLogicSM_state = ball_left;
		}else if (xPOS==1)
		{
			ballLogicSM_state = ball_right;
		}
		
		break;
		case ball_right:
		break;
	}
	
	switch (ballLogicSM_state){
		case ball_init:
		xPOS=3;yPOS=16;
		ballPOS_update();
		break;
		case  ball_wait:
		break;
		
		case  ball_left:
		xPOS=2;yPOS=16;
		ballPOS_update();
		break;
		case  ball_right:
		break;
		
	}
	*/
	
}

void ioSetup() {
	 DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4); //PORTB as output
	
	 DDRA |= (1<<DDA0) | (1<<DDA1) | (1<<DDA2) | (1<<DDA3) | (1<<DDA4); 
	 //PORTA as input
	
}
void paddle1SM(){
	switch (paddle1SM_state){
		case paddle1SM_wait:
		if (!up1 && !down1)
		{
			paddle1SM_state = paddle1SM_wait;
		}else if (up1 && !down1)
		{
			paddle1SM_state = up1_press;
		
		}else if (down1 && !up1)
		{
			paddle1SM_state = down1_press;
		}
		break;
		
		case  up1_press:
		if (up1 && !down1)
		{
			paddle1SM_state = up1_press;
		}else if (!up1 && !down1)
		{
			paddle1SM_state = up1_release;
		}
		break;
		
		case up1_release:
			paddle1SM_state = paddle1SM_wait;
			break;
		
		case down1_press:
		if (down1 && !up1)
		{
			paddle1SM_state = down1_press;
		}else if (!down1 && !up1)
		{
			paddle1SM_state = down1_release;
		}
		break;
		
		case  down1_release:
		paddle1SM_state = paddle1SM_wait;
		break;
		
		
	}
switch (paddle1SM_state){ //actions
		case paddle1SM_wait:
		
		break;
		
		case  up1_press:
		break;
	
		case up1_release://upshift
		if (pattern[0]!=224){
			pattern[0]=pattern[0]<<1;
			paddle1POS=paddle1POS<<1;
		}
	
		break;
	
		case down1_press:
		break;
	
		case  down1_release://downshift
		if (pattern[0]>7){
			pattern[0]=pattern[0]>>1;
			paddle1POS=paddle1POS>>1;
		}

		break;
	
	
	}
}

void paddle2SM(){
	switch (paddle2SM_state){
		case paddle2SM_wait:
		if (!up2 && !down2)
		{
			paddle2SM_state = paddle2SM_wait;
		}else if (up2 && !down2)
		{
			paddle2SM_state = up2_press;
			
		}else if (down2 && !up2)
		{
			paddle2SM_state = down2_press;
		}
		break;
		
		case  up2_press:
		if (up2 && !down2)
		{
			paddle2SM_state = up2_press;
		}else if (!up2 && !down2)
		{
			paddle2SM_state = up2_release;
		}
		break;
		
		case up2_release:
		paddle2SM_state = paddle2SM_wait;
		break;
		
		case down2_press:
		if (down2 && !up2)
		{
			paddle2SM_state = down2_press;
		}else if (!down2 && !up2)
		{
			paddle2SM_state = down2_release;
		}
		break;
		
		case  down2_release:
		paddle2SM_state = paddle2SM_wait;
		break;
		
		
	}
	switch (paddle2SM_state){ //actions
		case paddle2SM_wait:
		break;
		
		case  up2_press:
		break;
		
		case up2_release://upshift
		if (pattern[7]!=224){
			pattern[7]=pattern[7]<<1;
			paddle2POS=paddle2POS<<1;
		}
		
		break;
		
		case down2_press:
		break;
		
		case  down2_release://downshift
		if (pattern[7]!=7){
			pattern[7]=pattern[7]>>1;
			paddle2POS=paddle2POS>>1;
		}

		break;
		
		
	}
}

int main() {
	ioSetup();
	
	const unsigned long timerPeriod = 1;
	TimerSet(timerPeriod); TimerOn();
	store_state = store_init;
	paddle1SM_state = paddle1SM_wait;
	paddle2SM_state = paddle2SM_wait;
	ballLogicSM_state = ball_init;
	LED1_LOW();
	LED2_LOW();
	paddle1POS=16;
	paddle2POS=16;
	xPOS = 3;
	yPOS = 16;
	ballPOS_update();
	
	
	while (1){
		softRest();
		paddle1SM();
		paddle2SM();
		
		for (int i=0; i<8; i++) {
			
			shiftOut( ~pattern[i]);
			shiftOut( 128 >> i);
			while(store_SM_elapsedTime>=1){
				store_SM();
				store_SM();
				store_SM_elapsedTime=0;
			}
			store_SM_elapsedTime += timerPeriod;
		}
		
		while (ballLogicSM_elapsedTime>=300){
			ballLogicSM();
			
			ballLogicSM_elapsedTime = 0;
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
		ballLogicSM_elapsedTime += timerPeriod;
	}
	return 0;
}

void store_SM() {
	switch (store_state){
		case store_init:
		store_state = store_high;
		break;
		case store_high:
		store_state = store_low;
		break;
		case  store_low:
		store_state = store_high;
		break;
		
	}
	switch (store_state){
		case  store_init:
		break;
		case store_high:
		STORE_HIGH();
		break;
		case  store_low:
		STORE_LOW();
		break;
	}
	
//	_delay_ms(1);
	
//	_delay_ms(1);
}
 
void shiftOut(uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		 if(!!(val & (1 << i))){
			 DATA_HIGH();
			 }else{
			 DATA_LOW();
		 }
		SHIFT_HIGH();
		SHIFT_LOW();
	}
}