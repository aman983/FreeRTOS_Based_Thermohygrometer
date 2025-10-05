
#ifndef TM168_H
#define TM168_H
#include <avr/io.h>
#include <util/delay.h>


#define TM_PORT     PORTD
#define TM_STROBE   PD4      
#define TM_CLK      PD3
#define TM_DATA     PD2

#define GPIO_SET(GPIO, State)   \
    do {                        \
        if (State)              \
            TM_PORT |= (1 << (GPIO)); \
        else                    \
            TM_PORT &= ~(1 << (GPIO)); \
    } while (0)



#define BUTTON_NO_PRESS 1
#define BUTTON1   3
#define BUTTON2   5
#define BUTTON3   9
#define BUTTON4   17
#define BUTTON5   33
#define BUTTON6   65
#define BUTTON7   129
#define BUTTON8   257

#define SEG_A   0x01
#define SEG_B   0x02
#define SEG_C   0x04
#define SEG_D   0x08
#define SEG_E   0x10
#define SEG_F   0x20
#define SEG_G   0x40
#define SEG_DP  0x80


void Shift_out(uint8_t data);
uint8_t Shift_in();

void TM_Send_Command(uint8_t cmd);

void TM_Reset();

void TM_Setup(void);
void TM_Display_Buffer();
void TM_Display_digits(uint8_t number, uint8_t display_number);
void TM_Display_led(uint8_t led);
uint16_t TM_Button_Read();

void TM_Set_Brightness(uint8_t brightness, uint8_t on);
void TM_Display_Number(uint16_t number, uint8_t display_number);
void TM_Display_Float(float Number, uint8_t display_number);
#endif