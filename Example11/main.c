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

int32_t xoff = 0;
int32_t yoff = 0;
int32_t zoff = 0;
int8_t x = 0;

int8_t y = 0;
int8_t z = 0;

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
static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Declare a variable of type xQueueHandle.  This is used to store the queue
that is accessed by all three tasks. */
xQueueHandle xQueue;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
	unsigned char ucValue;
	unsigned char ucSource;
} xData;

/* Declare two variables of type xData that will be passed on the queue. */
static const xData xStructsToSend[ 2 ] =
{
	{ 100, mainSENDER_1 }, /* Used by Sender1. */
	{ 200, mainSENDER_2 }  /* Used by Sender2. */
};

int main(void) {

	char NewKey[6];
	init_ssp();

	init_i2c();
	acc_init();

	oled_init();
	oled_clearScreen(OLED_COLOR_WHITE);

	TCPLowLevelInit();

	HTTPStatus = 0;                         // clear HTTP-server's flag register

	TCPLocalPort = TCP_PORT_HTTP;               // set port we want to listen to

	while (1) {


			acc_read(&x, &y, &z);
			xoff = 0 - x;
			yoff = 0 - y;
			zoff = 64 - z;

			sprintf(NewKey, "%04d", xoff); // insert pseudo-ADconverter value
			oled_putString(1, 25, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
			sprintf(NewKey, "%04d", yoff); // insert pseudo-ADconverter value
			oled_putString(1, 33, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
			sprintf(NewKey, "%04d", zoff); // insert pseudo-ADconverter value
			oled_putString(1, 41, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

			if (!(SocketStatus & SOCK_ACTIVE))
				TCPPassiveOpen();   // listen for incoming TCP-connection
			DoNetworkStuff();                // handle network and easyWEB-stack
											 // events
			HTTPServer();


	}
}

// This function implements a very simple dynamic HTTP-server.
// It waits until connected, then sends a HTTP-header and the
// HTML-code stored in memory. Before sending, it replaces
// some special strings with dynamic values.
// NOTE: For strings crossing page boundaries, replacing will
// not work. In this case, simply add some extra lines
// (e.g. CR and LFs) to the HTML-code.

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
						sprintf(NewKey, "%04d", xoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					case '7':                                 // "AD7%"?
					{
						sprintf(NewKey, "%04d", yoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					case '1':                                 // "AD1%"?
					{
						sprintf(NewKey, "%04d", zoff); // insert pseudo-ADconverter value
						memcpy(Key, NewKey, 4);
						break;
					}
					}
		Key++;
	}
}

/*int main( void )
{
     The queue is created to hold a maximum of 3 structures of type xData.
    xQueue = xQueueCreate( 3, sizeof( xData ) );

	if( xQueue != NULL )
	{
		 Create two instances of the task that will write to the queue.  The
		parameter is used to pass the structure that the task should write to the 
		queue, so one task will continuously send xStructsToSend[ 0 ] to the queue
		while the other task will continuously send xStructsToSend[ 1 ].  Both 
		tasks are created at priority 2 which is above the priority of the receiver.
		xTaskCreate( vSenderTask, "Sender1", 240, ( void * ) &( xStructsToSend[ 0 ] ), 2, NULL );
		xTaskCreate( vSenderTask, "Sender2", 240, ( void * ) &( xStructsToSend[ 1 ] ), 2, NULL );

		 Create the task that will read from the queue.  The task is created with
		priority 1, so below the priority of the sender tasks.
		xTaskCreate( vReceiverTask, "Receiver", 240, NULL, 1, NULL );

		 Start the scheduler so the created tasks start executing.
		vTaskStartScheduler();
	}
	else
	{
		 The queue could not be created.
	}
		
     If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was 
    insufficient heap memory available for a resource to be created.
	for( ;; );
	return 0;
}*/
/*-----------------------------------------------------------*/

static void vSenderTask( void *pvParameters )
{
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* The first parameter is the queue to which data is being sent.  The 
		queue was created before the scheduler was started, so before this task
		started to execute.

		The second parameter is the address of the structure being sent.  The
		address is passed in as the task parameter. 

		The third parameter is the Block time - the time the task should be kept
		in the Blocked state to wait for space to become available on the queue
		should the queue already be full.  A block time is specified as the queue
		will become full.  Items will only be removed from the queue when both
		sending tasks are in the Blocked state.. */
		xStatus = xQueueSendToBack( xQueue, pvParameters, xTicksToWait );

		if( xStatus != pdPASS )
		{
			/* We could not write to the queue because it was full - this must
			be an error as the receiving task should make space in the queue
			as soon as both sending tasks are in the Blocked state. */
			vPrintString( "Could not send to the queue.\n" );
		}

		/* Allow the other sender task to execute. */
		taskYIELD();
	}
}
/*-----------------------------------------------------------*/

static void vReceiverTask( void *pvParameters )
{
/* Declare the structure that will hold the values received from the queue. */
xData xReceivedStructure;
portBASE_TYPE xStatus;

	/* This task is also defined within an infinite loop. */
	for( ;; )
	{
		/* As this task only runs when the sending tasks are in the Blocked state, 
		and the sending tasks only block when the queue is full, this task should
		always find the queue to be full.  3 is the queue length. */
		if( uxQueueMessagesWaiting( xQueue ) != 3 )
		{
			vPrintString( "Queue should have been full!\n" );
		}

		/* The first parameter is the queue from which data is to be received.  The
		queue is created before the scheduler is started, and therefore before this
		task runs for the first time.

		The second parameter is the buffer into which the received data will be
		placed.  In this case the buffer is simply the address of a variable that
		has the required size to hold the received structure. 

		The last parameter is the block time - the maximum amount of time that the
		task should remain in the Blocked state to wait for data to be available 
		should the queue already be empty.  A block time is not necessary as this
		task will only run when the queue is full so data will always be available. */
		xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 );

		if( xStatus == pdPASS )
		{
			/* Data was successfully received from the queue, print out the received
			value and the source of the value. */
			if( xReceivedStructure.ucSource == mainSENDER_1 )
			{
				vPrintStringAndNumber( "From Sender 1 = ", xReceivedStructure.ucValue );
			}
			else
			{
				vPrintStringAndNumber( "From Sender 2 = ", xReceivedStructure.ucValue );
			}
		}
		else
		{
			/* We did not receive anything from the queue.  This must be an error 
			as this task should only run when the queue is full. */
			vPrintString( "Could not receive from the queue.\n" );
		}
	}
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}






