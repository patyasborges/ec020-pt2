#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

#define extern

#include "easyweb.h"
#include "acc.h"

#include "ethmac.h"

#include "tcpip.h"
#include "LPC17xx.h"

#include "webside.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_i2c.h"
#include "oled.h"

#include "basic_io.h"

#define mainSENDER_1		1
#define mainSENDER_2		2

// CodeRed - added for use in dynamic side of web page
unsigned int aaPagecounter = 0;
unsigned int adcValue = 0;

static void init_ssp(void) {
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

}

static void init_i2c(void) {
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

/* The tasks to be created.  Two instances are created of the sender task while
 only a single instance is created of the receiver task. */
static void vSenderTask(void *pvParameters);
static void vReceiverTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* Declare a variable of type xQueueHandle.  This is used to store the queue
 that is accessed by all three tasks. */
xQueueHandle xQueue;

/* Define the structure type that will be passed on the queue. */
typedef struct {
	unsigned char ucValue;
	unsigned char ucSource;
} xData;

typedef struct {
	int32_t xoff;
	int32_t yoff;
	int32_t zoff;
} xData1;

xData1 total;

/* Declare two variables of type xData that will be passed on the queue. */
static const xData xStructsToSend[2] = { { 100, mainSENDER_1 }, /* Used by Sender1. */
{ 200, mainSENDER_2 } /* Used by Sender2. */
};

void initAll() {
	init_ssp();
	init_i2c();
	acc_init();
	oled_init();
	oled_clearScreen(OLED_COLOR_WHITE);

	TCPLowLevelInit();

	HTTPStatus = 0;                         // clear HTTP-server's flag register

	TCPLocalPort = TCP_PORT_HTTP;               // set port we want to listen to
}

xData1 readAcc() {

	xData1 total1;

	int8_t x = 0;
	int8_t y = 0;
	int8_t z = 0;

	acc_read(&x, &y, &z);

	total1.xoff = 0 - x;
	total1.yoff = 0 - y;
	total1.zoff = 64 - z;

	return total1;
}

void printOled(xData1 t) {
	char NewKey[6];

	sprintf(NewKey, "%04d", t.xoff); // insert pseudo-ADconverter value
	oled_putString(1, 25, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	sprintf(NewKey, "%04d", t.yoff); // insert pseudo-ADconverter value
	oled_putString(1, 33, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	sprintf(NewKey, "%04d", t.zoff); // insert pseudo-ADconverter value
	oled_putString(1, 41, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

void printHttp() {
	if (!(SocketStatus & SOCK_ACTIVE)) {
		TCPPassiveOpen();
	}
	DoNetworkStuff();
	HTTPServer();
}

const char *pcTextForTask1 = "Task 1 is running\n";
const char *pcTextForTask2 = "Task 2 is running\n";

#define mainDELAY_LOOP_COUNT1		( 0xffff )
#define mainDELAY_LOOP_COUNT2		( 0xff )

void vTaskFunction(void *pvParameters) {
	char *pcTaskName;
	volatile unsigned long ul;

	/* The string to print out is passed in via the parameter.  Cast this to a
	 character pointer. */
	pcTaskName = (char *) pvParameters;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;) {
		/* Print out the name of this task. */
		vPrintString(pcTaskName);

		readAcc();

		//printOled();

		//printHttp();

		/* Delay for a period. */
		for (ul = 0; ul < mainDELAY_LOOP_COUNT1; ul++) {
			/* This loop is just a very crude delay implementation.  There is
			 nothing to do in here.  Later exercises will replace this crude
			 loop with a proper delay/sleep function. */
		}
	}
}

void vTaskFunction1(void *pvParameters) {
	char *pcTaskName;
	volatile unsigned long ul;

	/* The string to print out is passed in via the parameter.  Cast this to a
	 character pointer. */
	pcTaskName = (char *) pvParameters;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;) {
		/* Print out the name of this task. */
		vPrintString(pcTaskName);

		//readAcc();

		//printOled();

		//printHttp();

		/* Delay for a period. */
		for (ul = 0; ul < mainDELAY_LOOP_COUNT1; ul++) {
			/* This loop is just a very crude delay implementation.  There is
			 nothing to do in here.  Later exercises will replace this crude
			 loop with a proper delay/sleep function. */
		}
	}
}

int main(void) {

	vPrintString("1\n");

	initAll();

	/* The queue is created to hold a maximum of 5 long values. */
	xQueue = xQueueCreate(3, sizeof( xData1 ));

	if (xQueue != NULL ) {
		/* Create two instances of the task that will write to the queue.  The
		 parameter is used to pass the value that the task should write to the queue,
		 so one task will continuously write 100 to the queue while the other task
		 will continuously write 200 to the queue.  Both tasks are created at
		 priority 1. */
		//xTaskCreate( vSenderTask, "Sender1", 240, ( void * ) 100, 1, NULL );
		xTaskCreate(vSenderTask, "Sender2", 240, (void * ) 200, 1, NULL);

		/* Create the task that will read from the queue.  The task is created with
		 priority 2, so above the priority of the sender tasks. */
		xTaskCreate(vReceiverTask, "Receiver", 240, NULL, 2, NULL);

		/* Start the scheduler so the created tasks start executing. */
		vTaskStartScheduler();
	} else {
		/* The queue could not be created. */
	}

	/* If all is well we will never reach here as the scheduler will now be
	 running the tasks.  If we do reach here then it is likely that there was
	 insufficient heap memory available for a resource to be created. */
	for (;;)
		;
	return 0;

	//xQueue = xQueueCreate(3, sizeof(xData));

	//while (1) {

	//readAcc();

	//printOled();

	//printHttp();
//	vPrintString("3\n");
//	//}
//
//	xTaskCreate(vTaskFunction, "Lendo sensor", 240, (void* )pcTextForTask1, 1, NULL);
//	xTaskCreate(vTaskFunction1, "Escrevendo...", 240, (void* )pcTextForTask2, 1, NULL);
//
//	vTaskStartScheduler();

	//vPrintString("2\n");

	/*if (xQueue != NULL ) {
	 xTaskCreate(vSenderTask, "Sender1", 240, (void * ) &(xStructsToSend[0]),
	 2, NULL);
	 xTaskCreate(vSenderTask, "Sender2", 240, (void * ) &(xStructsToSend[1]),
	 2, NULL);

	 xTaskCreate(vReceiverTask, "Receiver", 240, NULL, 1, NULL);

	 vTaskStartScheduler();
	 } else {
	 The queue
	 could not
	 be created
	 }*/

//	for (;;)
//		;
//	return 0;
}

void HTTPServer(void) {
	if (SocketStatus & SOCK_CONNECTED) // check if somebody has connected to our TCP
	{
		if (SocketStatus & SOCK_DATA_AVAILABLE) // check if remote TCP sent data
			TCPReleaseRxBuffer();                      // and throw it away

		if (SocketStatus & SOCK_TX_BUF_RELEASED) // check if buffer is free for TX
		{
			if (!(HTTPStatus & HTTP_SEND_PAGE)) // init byte-counter and pointer to webside
			{                                          // if called the 1st time
				HTTPBytesToSend = sizeof(WebSide) - 1; // get HTML length, ignore trailing zero
				PWebSide = (unsigned char *) WebSide;    // pointer to HTML-code
			}

			if (HTTPBytesToSend > MAX_TCP_TX_DATA_SIZE) // transmit a segment of MAX_SIZE
			{
				if (!(HTTPStatus & HTTP_SEND_PAGE)) // 1st time, include HTTP-header
				{
					memcpy(TCP_TX_BUF, GetResponse, sizeof(GetResponse) - 1);
					memcpy(TCP_TX_BUF + sizeof(GetResponse) - 1, PWebSide,
							MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1);
					HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE
							- sizeof(GetResponse) + 1;
					PWebSide += MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1;
				} else {
					memcpy(TCP_TX_BUF, PWebSide, MAX_TCP_TX_DATA_SIZE);
					HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE;
					PWebSide += MAX_TCP_TX_DATA_SIZE;
				}

				TCPTxDataCount = MAX_TCP_TX_DATA_SIZE;   // bytes to xfer
				InsertDynamicValues();               // exchange some strings...
				TCPTransmitTxBuffer();                   // xfer buffer
			} else if (HTTPBytesToSend)               // transmit leftover bytes
			{
				memcpy(TCP_TX_BUF, PWebSide, HTTPBytesToSend);
				TCPTxDataCount = HTTPBytesToSend;        // bytes to xfer
				InsertDynamicValues();        // exchange some strings...

				TCPTransmitTxBuffer();                   // send last segment
				TCPClose();                              // and close connection
				HTTPBytesToSend = 0;                     // all data sent
			}

			HTTPStatus |= HTTP_SEND_PAGE;              // ok, 1st loop executed
		}
	} else
		HTTPStatus &= ~HTTP_SEND_PAGE;       // reset help-flag if not connected
}

volatile unsigned int aaScrollbar = 400;

unsigned int GetAD7Val(void) {
	aaScrollbar = (aaScrollbar + 16) % 1024;
	adcValue = (aaScrollbar / 10) * 1000 / 1024;
	return aaScrollbar;
}

void InsertDynamicValues(void) {
	unsigned char *Key;
	char NewKey[6];
	unsigned int i;

	if (TCPTxDataCount < 4)
		return;                     // there can't be any special string

	Key = TCP_TX_BUF;

	for (i = 0; i < (TCPTxDataCount - 3); i++) {
		if (*Key == 'A')
			if (*(Key + 1) == 'D')
				if (*(Key + 3) == '%')
					switch (*(Key + 2)) {
					case '8':                                 // "AD8%"?
					{
						sprintf(NewKey, "%04d", total.xoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					case '7':                                 // "AD7%"?
					{
						sprintf(NewKey, "%04d", total.yoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					case '1':                                 // "AD1%"?
					{
						sprintf(NewKey, "%04d", total.zoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					}
		Key++;
	}
}

/*int main( void )
 {
 xQueue = xQueueCreate( 3, sizeof( xData ) );

 if( xQueue != NULL )
 {
 xTaskCreate( vSenderTask, "Sender1", 240, ( void * ) &( xStructsToSend[ 0 ] ), 2, NULL );
 xTaskCreate( vSenderTask, "Sender2", 240, ( void * ) &( xStructsToSend[ 1 ] ), 2, NULL );

 xTaskCreate( vReceiverTask, "Receiver", 240, NULL, 1, NULL );

 vTaskStartScheduler();
 }
 else
 {
 The queue could not be created.
 }

 for( ;; );
 return 0;
 }*/
/*-----------------------------------------------------------*/

static void vSenderTask(void *pvParameters) {
	xData1 lValueToSend;
	portBASE_TYPE xStatus;

	//lValueToSend = ( long ) pvParameters;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for (;;) {

		lValueToSend = readAcc();

		xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );

		if (xStatus != pdPASS) {
			/* We could not write to the queue because it was full � this must
			 be an error as the queue should never contain more than one item! */
			vPrintString("Could not send to the queue.\r\n");
		}

		taskYIELD();
	}
}
/*-----------------------------------------------------------*/

static void vReceiverTask(void *pvParameters) {
	/* Declare the variable that will hold the values received from the queue. */
	xData1 lReceivedValue;
	portBASE_TYPE xStatus;
	const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	/* This task is also defined within an infinite loop. */
	for (;;) {
		/* As this task unblocks immediately that data is written to the queue this
		 call should always find the queue empty. */
		if (uxQueueMessagesWaiting(xQueue) != 0) {
			vPrintString("Queue should have been empty!\r\n");
		}

		xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

		//printHttp();

		if (xStatus == pdPASS) {
			/* Data was successfully received from the queue, print out the received
			 value. */
			vPrintStringAndNumber("Received = ", lReceivedValue.xoff);
			printOled(lReceivedValue);
		} else {
			/* We did not receive anything from the queue even after waiting for 100ms.
			 This must be an error as the sending tasks are free running and will be
			 continuously writing to the queue. */
			vPrintString("Could not receive from the queue.\r\n");
		}
	}
}

void vApplicationMallocFailedHook(void) {
	/* This function will only be called if an API call to create a task, queue
	 or semaphore fails because there is too little heap RAM remaining. */
	for (;;)
		;
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void) {
	/* This example does not use the tick hook to perform any processing. */
}
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	/* This function will only be called if a task overflows its stack.  Note
	 that stack overflow checking does slow down the context switch
	 implementation. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

