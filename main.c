#include <avr/io.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <UART.h>
#include <stdio.h>
#include <Display.h>

#define mainLED_TASK_PRIORITY			(tskIDLE_PRIORITY)
#define mainLED_TASK_PRIORITY 			(tskIDLE_PRIORITY+1)

void vLEDFlashTask(void *pvParms)
{
	DDRB |= (1 << 5);

	for(;;) {
		PORTB ^= (1 << 5);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}


extern xQueueHandle Display_Group1_Queue;
extern xQueueHandle Display_Group2_Queue;
extern xQueueHandle Display_Led_Queue;
extern xQueueHandle Display_Button_Queue;


void Button_Task(void *pvParams)
{
	uint16_t button=0;
	int16_t data = 0;
	while (1)
	{
		if(xQueueReceive(Display_Button_Queue, &button, 10) == pdTRUE)
		{
			if(button == BUTTON1)
			{
				data++;
				xQueueSend(Display_Led_Queue, &data, 10);
			}	
		}
		vTaskDelay(100/portTICK_RATE_MS);
	}
}

#define DHT_IN PD5
#define DHT_DDR DDRD
#define DHT_PORT PORTD
#define DHT_PIN PIND

uint8_t get_dht(float *ret)
{
    uint8_t data[5] = {0};
    uint16_t timeout;

    // MCU start signal
    DHT_DDR |= (1 << DHT_IN);      // output
    DHT_PORT &= ~(1 << DHT_IN);    // low
    _delay_ms(18);
    DHT_PORT |= (1 << DHT_IN);     // high for 20-40us
    _delay_us(40);
    DHT_DDR &= ~(1 << DHT_IN);     // input

    // Wait for sensor response (low → high → low)
    timeout = 10000;
    while ((DHT_PIN & (1 << DHT_IN)) && --timeout);  // wait for LOW
    if (!timeout) return 0;

    timeout = 10000;
    while (!(DHT_PIN & (1 << DHT_IN)) && --timeout); // wait for HIGH
    if (!timeout) return 0;

    timeout = 10000;
    while ((DHT_PIN & (1 << DHT_IN)) && --timeout);  // wait for LOW
    if (!timeout) return 0;

    // Read 5 bytes = 40 bits
    for (uint8_t byte = 0; byte < 5; byte++)
    {
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            // Wait for high pulse
            timeout = 10000;
            while (!(DHT_PIN & (1 << DHT_IN)) && --timeout);
            if (!timeout) return 0;

            _delay_us(35); // check after ~35us (DHT11/22 threshold)

            data[byte] <<= 1;
            if (DHT_PIN & (1 << DHT_IN))
                data[byte] |= 1;

            // Wait for line to go low
            timeout = 10000;
            while ((DHT_PIN & (1 << DHT_IN)) && --timeout);
            if (!timeout) return 0;
        }
    }

	// after you have filled data[0..4] and passed checksum:
	uint16_t hum_raw = ((uint16_t)data[0] << 8) | data[1];   // 16-bit humidity
	int16_t  temp_raw = ((uint16_t)(data[2] & 0x7F) << 8) | data[3]; // 15-bit temperature
	if (data[2] & 0x80) temp_raw = -temp_raw; // sign bit

	ret[0] = hum_raw * 0.1f;   // humidity in %
	ret[1] = temp_raw * 0.1f;  // temperature in °C

	// range checks on the converted values
	if (ret[0] < 0.0f || ret[0] > 100.0f) return 0;
	if (ret[1] < -40.0f || ret[1] > 80.0f)  return 0;
	return 1;
}

void DHT_Task(void *pvParams)
{
	float data[2] = {0};
	while (1)
	{
		if(get_dht(data) == 1)
		{
			xQueueSend(Display_Group1_Queue, &data[1], 10);
			xQueueSend(Display_Group2_Queue, &data[0], 10);
		}else{
			printf("NA\n");
		}
		vTaskDelay(2000/portTICK_RATE_MS);
		
	}
}


void vApplicationIdleHook( void );

portSHORT main(void)
{
	UART_init();
	Display_init();

	xTaskCreate(vLEDFlashTask, "LED", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL);
	xTaskCreate(Button_Task, "button", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL);
	xTaskCreate(DHT_Task, "DHT", 250, NULL, mainLED_TASK_PRIORITY, NULL);
	vTaskStartScheduler();
	return 0;
}

void vApplicationIdleHook( void )
{
	//vCoRoutineSchedule();
}

