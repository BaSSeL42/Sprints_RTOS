#include "Botton.h"

#define Botton_1_Pin	PIN0							// Botton Pin 
#define Botton_1_Port PORT_0						// Botton Port

#define Botton_2_Pin	PIN1							// Botton Pin 
#define Botton_2_Port PORT_0						// Botton Port

#define Botton_3_Pin	PIN2							// Botton Pin 
#define Botton_3_Port PORT_0						// Botton Port

// Function to read botton status
pinState_t botton_read(unsigned char btn_num)
{
	pinState_t state;
	if(btn_num == 1)
		state = GPIO_read(Botton_1_Port,Botton_1_Pin);				// Read Botton status
	else if(btn_num == 2)
		state = GPIO_read(Botton_2_Port,Botton_2_Pin);				// Read Botton status
	else if(btn_num == 3)
		state = GPIO_read(Botton_3_Port,Botton_3_Pin);				// Read Botton status
	else{
		// do nothing
	}
	return state;
}

