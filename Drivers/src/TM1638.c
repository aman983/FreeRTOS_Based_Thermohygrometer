#include <avr/io.h>
#include <util/delay.h>
#include <TM1638.h>

uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
static uint8_t display_buf[9] = {0};

void Shift_out(uint8_t data)
{
    for(uint8_t bit=0; bit<8; bit++)
    {
        GPIO_SET(TM_CLK, 0);
        
        if(data & (1 << bit))
        {
            GPIO_SET(TM_DATA, 1);
        }else{
            GPIO_SET(TM_DATA, 0);
        }

        GPIO_SET(TM_CLK, 1);
        
    }
}


uint8_t Shift_in()
{
    uint8_t data = 0;
    

    for(uint8_t bit=0; bit<8; bit++)
    {
        GPIO_SET(TM_CLK, 0);
        

        if(PIND & (1 << TM_DATA))
        {
            data |= (1 << bit);
        }
        GPIO_SET(TM_CLK, 1);
    }

    
    return data;
}


void TM_Send_Command(uint8_t cmd)
{
    GPIO_SET(TM_STROBE, 0);
    Shift_out(cmd);
    GPIO_SET(TM_STROBE, 1);
}

void TM_Reset()
{
    TM_Send_Command(0x40);
    GPIO_SET(TM_STROBE, 0);
    Shift_out(0xC0);
    for(uint8_t i=0; i<16; i++)
    {
        Shift_out(0x00);
    }
    GPIO_SET(TM_STROBE, 1);
}

void TM_Setup(void)
{
    DDRD |= (1 << TM_STROBE) | (1 << TM_CLK) | (1 << TM_DATA);
    GPIO_SET(TM_STROBE, 1);
    GPIO_SET(TM_CLK, 1);
    GPIO_SET(TM_DATA, 1);
    TM_Reset();
    TM_Display_led(0);
}

void TM_Display_Buffer()
{

    // re-send full display
    TM_Send_Command(0x40);      // auto-increment mode
    GPIO_SET(TM_STROBE, 0);
    Shift_out(0x00);            // start address

    for (uint8_t i = 0; i < 8; i++)
    {
        Shift_out(display_buf[7-i]); // digit pattern
        Shift_out((display_buf[8] >> (7-i)) & 0x01);           // LEDs off
    }

    GPIO_SET(TM_STROBE, 1);
}
void TM_Display_digits(uint8_t number, uint8_t display_number)
{
    if (display_number >= 8 || number > 9) return;  // safety check

    // update buffer
    display_buf[display_number] = digits[number];

    TM_Display_Buffer();
}




void TM_Display_led(uint8_t led)
{
    display_buf[8] = led;

    TM_Display_Buffer();
}

uint16_t TM_Button_Read()
{
    uint16_t button_state = 0;

    GPIO_SET(TM_STROBE, 0);
    Shift_out(0x42);
    DDRD &= ~(1 << TM_DATA);
    for(uint8_t i=0; i<4; i++)
    {
        button_state |= Shift_in() << i;
    }

    GPIO_SET(TM_STROBE, 1);
    DDRD |= (1 << TM_DATA);
    return button_state;
}

void TM_Set_Brightness(uint8_t brightness, uint8_t on)
{
    if (brightness > 7) brightness = 7;   // clamp 0–7
    uint8_t cmd = 0x88 | (on ? 0x08 : 0x00) | brightness;
    TM_Send_Command(cmd);
}

void TM_Display_Number(uint16_t number, uint8_t display_number)
{
    uint8_t counter = display_number * 4;
    if (number == 0) {
        display_buf[counter++] = digits[0];  // put "0" in first place
    } else {
        while (number > 0 && counter < 8)    // 8 digits max
        {
            uint8_t digit = number % 10;     
            display_buf[counter++ ] = digits[digit];
            number /= 10;
        }
    }

    // Clear remaining digits
    if(display_number == 0)
    {
        while (counter < 4) {
        display_buf[counter++] = 0x00;  // blank
    }
    }else{
        while (counter < 8) {
        display_buf[counter++] = 0x00;  // blank
    }
    }
    
    TM_Display_Buffer();
}


void TM_Display_Float(float Number, uint8_t display_number)
{
    display_number = (display_number) * 4;
    Number *= 100;
    int16_t int_digit = Number;

    int16_t temp = int_digit;
    uint8_t counter = display_number;
    while (temp>0)
    {
        display_buf[counter++] = digits[temp % 10];
        temp /= 10;
    }
    display_buf[counter - 2] |= 0x80;
    

    TM_Display_Buffer();
}
