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

unsigned int aaPagecounter = 0;
unsigned int adcValue = 0;

static void init_ssp(void) {
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

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

	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	SSP_Cmd(LPC_SSP1, ENABLE);
}

static void init_i2c(void) {
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(LPC_I2C2, 100000);

	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void vSenderReadTask(void *pvParameters);
static void vReceiverWriteTask(void *pvParameters);
static void vWWWTask(void *pvParameters);

xQueueHandle xQueue;

typedef struct {
	int32_t xoff;
	int32_t yoff;
	int32_t zoff;
} xData1;

void initAll() {

	init_ssp();
	init_i2c();
	acc_init();
	oled_init();
	oled_clearScreen(OLED_COLOR_WHITE);

	TCPLowLevelInit();

	HTTPStatus = 0;

	TCPLocalPort = TCP_PORT_HTTP;
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

void printOled(xData1* t) {

	char NewKey[6];

	sprintf(NewKey, "%04d", t->xoff);
	oled_putString(1, 25, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	sprintf(NewKey, "%04d", t->yoff);
	oled_putString(1, 33, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	sprintf(NewKey, "%04d", t->zoff);
	oled_putString(1, 41, NewKey, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

void printHttp(xData1* t) {
	if (!(SocketStatus & SOCK_ACTIVE))
		TCPPassiveOpen();   // listen for incoming TCP-connection
	DoNetworkStuff();
	HTTPServer(&t);
}

int main(void) {

	vPrintString("Iniciando...\n");

	initAll();

	xQueue = xQueueCreate(10, sizeof(xData1));

	if (xQueue != NULL ) {
		xTaskCreate(vSenderReadTask, "READ", 240, NULL, 1, NULL);
		xTaskCreate(vReceiverWriteTask, "WRITE", 240, NULL, 2, NULL);
		//xTaskCreate(vWWWTask, "WWW", 240, NULL, 1, NULL);

		vTaskStartScheduler();
	} else {
		/* The queue could not be created. */
	}

	//vWWWTask(NULL);

	for (;;)
		;
	return 0;
}

void HTTPServer(xData1* t) {
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
				InsertDynamicValues(&t);              // exchange some strings...
				TCPTransmitTxBuffer();                   // xfer buffer
			} else if (HTTPBytesToSend)               // transmit leftover bytes
			{
				memcpy(TCP_TX_BUF, PWebSide, HTTPBytesToSend);
				TCPTxDataCount = HTTPBytesToSend;        // bytes to xfer
				InsertDynamicValues(&t);        // exchange some strings...

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

void InsertDynamicValues(xData1* t) {
	unsigned char *Key;
	char NewKey[6];
	unsigned int i;

	if (TCPTxDataCount < 4)
		return;

	Key = TCP_TX_BUF;

	for (i = 0; i < (TCPTxDataCount - 3); i++) {
		if (*Key == 'A')
			if (*(Key + 1) == 'D')
				if (*(Key + 3) == '%')
					switch (*(Key + 2)) {
					case '8':
					{
						sprintf(NewKey, "%04d", t->xoff);
						memcpy(Key, NewKey, 4);
						break;
					}
					case '7':
					{
						sprintf(NewKey, "%04d", t->yoff);
						memcpy(Key, NewKey, 4);
						break;
					}
					case '1':
					{
						sprintf(NewKey, "%04d", t->zoff);
						memcpy(Key, NewKey, 4);
						break;
					}
					}
		Key++;
	}
}

static void vSenderReadTask(void *pvParameters) {

	vPrintString("vSenderReadTask\r\n");

	xData1 lValueToSend;
	portBASE_TYPE xStatus;

	for (;;) {

		lValueToSend = readAcc();

		xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );
		vPrintStringAndNumber("vSenderReadTask = ", lValueToSend.xoff);

		if (xStatus != pdPASS) {
			vPrintString("Could not send to the queue.\r\n");
		}

		taskYIELD();
	}
}

static void vWWWTask(void *pvParameters) {

	vPrintString("vWWWTask\r\n");

	xData1 lValueToSend;
	portBASE_TYPE xStatus;

	while(1) {

		lValueToSend = readAcc();


		vPrintStringAndNumber("vWWWTask = ", lValueToSend.xoff);

		printHttp(&lValueToSend);
		printOled(&lValueToSend);

		//taskYIELD();
	}
}
/*-----------------------------------------------------------*/

static void vReceiverWriteTask(void *pvParameters) {

	xData1 lReceivedValue;
	portBASE_TYPE xStatus;
	const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	for (;;) {
		if (uxQueueMessagesWaiting(xQueue) != 0) {
			vPrintString("Queue should have been empty!\r\n");
		}

		xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

		if (xStatus == pdPASS) {
			vPrintStringAndNumber("vReceiverWriteTask = ", lReceivedValue.xoff);
			printOled(&lReceivedValue);
		} else {
			vPrintString("Could not receive from the queue.\r\n");
		}
	}
}

void vApplicationMallocFailedHook(void) {
	for (;;)
		;
}

void vApplicationIdleHook(void) {
}

void vApplicationTickHook(void) {
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	for (;;)
		;
}
