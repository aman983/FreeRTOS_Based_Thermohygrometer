#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <TM1638.h>
#include <Display.h>

/* Input Queues */
xQueueHandle Display_Group1_Queue;
xQueueHandle Display_Group2_Queue;
xQueueHandle Display_Led_Queue;

/* Output Queues */
xQueueHandle Display_Button_Queue;


/*  Display Group 1,2 & led   Button_State
        |                           ^
        |                           |
        V                           |  */
static void Display_task(void *pvParmas)
{
	TM_Setup();
	TM_Set_Brightness(2, 1);
	float display_group1 = 0, display_group2 = 0;
    uint16_t display_button = 0;
    uint8_t display_led = 0;  
	
	while (1)
	{
		if(xQueueReceive(Display_Group1_Queue, &display_group1, 10) == pdTRUE)
		{
			
			TM_Display_Float(display_group1, 0);
		}

        if(xQueueReceive(Display_Group2_Queue, &display_group2, 10) == pdTRUE)
		{
			TM_Display_Float(display_group2, 1);
		}


		if(xQueueReceive(Display_Led_Queue, &display_led, 10) == pdTRUE)
		{
			TM_Display_led(display_led);
		}
        /* Over Write the queue to send the latest value*/
        display_button = TM_Button_Read();
        xQueueOverwrite(Display_Button_Queue, &display_button);
		vTaskDelay(100/portTICK_RATE_MS);	
	}
}

void Display_init()
{
    Display_Button_Queue = xQueueCreate(1, sizeof(uint16_t));

    Display_Led_Queue = xQueueCreate(5, sizeof(uint8_t));
    Display_Group1_Queue = xQueueCreate(5, sizeof(float));
    Display_Group2_Queue = xQueueCreate(5, sizeof(float));

    xTaskCreate(Display_task, "Display Task", 200, NULL, 6, NULL);
}

