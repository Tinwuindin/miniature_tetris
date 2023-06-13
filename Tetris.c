/*
 * File:   Tetris.c
 * Author: Emmanuel Esqueda Rodríguez
 *
 * Created on 2 de junio de 2023, 04:11 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 4000000

//Where is conected RS/RW/E
#define Bus_control PORTC
#define Bus_control_conf TRISC
//Where data is send
#define Bus_port PORTD
#define Bus_port_conf TRISD

#include <xc.h>
#include "Screen_16x2_driver.h"

void Start_window();
void Figure_spawn();
void Game_over();
void Score();
void Rotate(unsigned char direction);

unsigned char i, j, k, con, game_over = 1, fast_fall, and_result, xor_result, or_result, movement = 0;
unsigned char score_ptr[3];
unsigned int score;

void __interrupt() screen_frames(void){
    //No interrupts while updating the screen
    INTCON = 0;
    Nframe();
    TMR0 = 0;
    //Interrups on
    INTCON = 0b10100000;
    return;
}

void main() {
    //PORTB as input and activate the weak pull ups
    PORTB = 0xFF;
	WPUB = 0xFF;
	ANSELH = 0;
	TRISB = 0xFF;
    Screen_Init();
    Start_window();
    while(1){
        //Check if a game is not running
        if(game_over == 1){
            if(PORTBbits.RB0 == 0){
                //Clean display before game starts
                game_over = 0;
                score = 0;
                __delay_ms(10);
                for(i = 1; i <= 19; i++){
                    inferior[i] = 0;
                    superior[i] = 0x41;
                }
                superior[1] = 0xFF;
                superior[19] = 0xFF;
                Screen_write(0x45,"           `");     
            }
        }else{
            while(movement != 0){
                movement = 0;
                for(i = 0;i != 20; i++){
                    //Get where exist 2 pixels set
                    and_result = inferior[i] & inferior[(i + 1)];
                    //Pass the upper data to 1 level down
                    inferior[i] |= inferior[(i + 1)];
                    //If there 2 pixels together keep them, else delete with an and mask
                    if(inferior[(i + 1)] != (inferior[(i + 1)] & and_result)){
                        inferior[(i + 1)] &= and_result;
                        movement ++;
                    }
                }
                if((PORTBbits.RB2 == 0) && (fast_fall == 0))
                    Rotate(0);
                if((PORTBbits.RB4 == 0) && (fast_fall == 0))
                    Rotate(1);
                if(PORTBbits.RB6 == 0)
                    fast_fall = 1;
                //Look for complete lines
                for(i = 0;i != 20; i++){
                    if(inferior[i] == 0xFF){
                        inferior[i] = 0;
                        Score();
                        movement ++;
                    }
                }
                if(fast_fall == 0)
                    __delay_ms(100);
                else
                    __delay_ms(20);
            }
            Screen_write(0x05,"SCORE:      `");
            Figure_spawn();
        }		
    }
    return;
}

void Score(){
    //Separate the value of points
    score ++;
    score_ptr[0] = score / 100;
    score_ptr[1] = ((score % 100) / 10);
    score_ptr[2] = ((score % 100) % 10);
    k = 0;
    for(j = 0x45; j != 0x48; j++){
        if(score_ptr[k] == 0)
            Screen_write(j,"0`");
        else if(score_ptr[k] == 1)
            Screen_write(j,"1`");
        else if(score_ptr[k] == 2)
            Screen_write(j,"2`");
        else if(score_ptr[k] == 3)
            Screen_write(j,"3`");
        else if(score_ptr[k] == 4)
            Screen_write(j,"4`");
        else if(score_ptr[k] == 5)
            Screen_write(j,"5`");
        else if(score_ptr[k] == 6)
            Screen_write(j,"6`");
        else if(score_ptr[k] == 7)
            Screen_write(j,"7`");
        else if(score_ptr[k] == 8)
            Screen_write(j,"8`");
        else if(score_ptr[k] == 9)
            Screen_write(j,"9`");
        k++;
    }
    
    return;
}

void Rotate(unsigned char direction){
    __delay_ms(10);
    con = 0;
    for(i = 1;i != 20;i++){
        //Get where exist 2 pixels set
        and_result = inferior[i] & inferior[(i - (1 + con))];
        //Do a xor between original and where 2 pixels are set
        xor_result = inferior[i] ^ and_result;
        //Check if there bits above
        con = 0;
        if((xor_result & inferior[i + 1]) != 0){
            con = 1;
        }
        //Move 1 bit
        if(direction == 1){
            if((xor_result & 0x0F) == 0x0F){
                xor_result >>= 1;
                xor_result |= 0x0F;
            }else if((xor_result & 0x07) == 0x07){
                xor_result >>= 1;
                xor_result |= 0x07;
            }
            else if((xor_result & 0x03) == 0x03){
                xor_result >>= 1;
                xor_result |= 0x03;
            }else if((xor_result & 0x01) == 0x01){
                xor_result >>= 1;
                xor_result |= 0x01;
            }else
                xor_result >>= 1;
        }
        else
            if((xor_result & 0xF0) == 0xF0){
                xor_result <<= 1;
                xor_result |= 0xF0;
            }else if((xor_result & 0xE0) == 0xE0){
                xor_result <<= 1;
                xor_result |= 0xE0;
            }
            else if((xor_result & 0xC0) == 0xC0){
                xor_result <<= 1;
                xor_result |= 0xC0;
            }else if((xor_result & 0x80) == 0x80){
                xor_result <<= 1;
                xor_result |= 0x80;
            }else
                xor_result <<= 1;
        //Make a or with 2 pixels set (and_result) and movement result
        or_result = and_result | xor_result;
        //Now, restore if 1 displaced is on 1 than alredy exist
        //Original & displaced bits
        and_result = and_result & xor_result;
        //Move back
        if(direction == 1)
            and_result <<= 1;
        else
            and_result >>= 1;
        //Added mounted bits to previous results and save
        inferior[i] = or_result | and_result;
    }
    return;
}

void Figure_spawn(){
    unsigned char figure_spawn;
    //Spawn a "random" figure
    figure_spawn = (TMR0 >> 6);
    fast_fall = 0;
    
    /* 0 will be a 2*2 square
     * 1 will be a 1*4 line
     * 2 will be a L 
     * 3 later, i have no idea what more add     
     */
    
    if(figure_spawn == 0){
        //Using an AND mask to check if exist space enought    
        if(((inferior[18] & 0x18) != 0) || ((inferior[19] & 0x18) != 0))
            Game_over();
        if(game_over == 0){
            //Creation of the figure
            inferior[18] |= 0x18;
            inferior[19] |= 0x18;
            //movement is the condition to start falling
            movement = 1;
            //Delay to give 2 frames before start falling
            __delay_ms(20);
        }        
    }else if(figure_spawn == 1){
        //Same as the firts, just adjusted to the size of the another figure
        if((inferior[19] & 0x3C) != 0)
            Game_over();
        if(game_over == 0){
            inferior[19] |= 0x3C;
            movement = 1;
            __delay_ms(20);
        }
    }else if(figure_spawn == 2){
        if(((inferior[18] & 0x04) != 0) || ((inferior[19] & 0x1C) != 0))
            Game_over();
        if(game_over == 0){
            inferior[19] |= 0x04;
            inferior[18] |= 0x1C;
            movement = 1;
            __delay_ms(20);
        }
    }
    return;   
}

void Game_over(){
    game_over = 1;
    Screen_write(0x05,"GAME OVER`");
    for(j = 0; j <= 5; j++){
        for(i = 0; i <= 19; i++)
            inferior[i] = ~inferior[i];
        __delay_ms(250);
    }
    return;
}

void Start_window(){
    Screen_write(0x05,"TETRIS `");
    Screen_write(0x45,"PRESS START `");
}