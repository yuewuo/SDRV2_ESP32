#include "autoinclude.h"
#include "emdev.h"
#include "string.h"
//#include "tim.h"

static uart_pcb_t pcb;

static void uartInit(void)
{
	memset(&pcb, 0, sizeof(uart_pcb_t));
	pcb.pending_tx_data = 0;
	pcb.rxfifo.len = 0;
}

static int uartSend(uint8_t *buffer, uint32_t length, int timeout, int uart_num)
{
	/*uint8_t *tmp = buffer;
	while(length != 0)
	{
		USART_SendData(USART1, *tmp);
//    USART_SendData(USART3, *tmp);
		tmp++;
		length--;*/
		/* Loop until the end of transmission */
		/*while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{}
//    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
//		{}
}*/
	UARTGrp.Send(uart_num, (char*)buffer, length);
	return 0;
}

//static void uartSendByte(uint8_t ch, int uart_num)
//{
  //USART_SendData(USART1, (uint8_t) ch);
  /* Loop until the end of transmission */
  /*while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}*//*
  UARTGrp.Send(uart_num, (char*)(&ch), 1);

	return;
}*/

/*uint8_t uartRxBufferRead(void)
{
	uint8_t data;
	data = pcb.rxfifo.buf[pcb.rxfifo.rd_ptr];
	pcb.rxfifo.rd_ptr = (pcb.rxfifo.rd_ptr + 1) % CFG_UART_BUFSIZE;
	pcb.rxfifo.len--;*/

//  USART_SendData(USART3, data);
  /* Loop until the end of transmission */
//  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
//  {}

/*	return data;
}*/

int uartRxBufferReadArray(uint8_t *rx, uint32_t len, int timeout, int uart_num)
{
  /*uint32_t ticks = xTaskGetTickCount(); // TimerDrvGetTicks();
  uint8_t *ptr = rx;
  while(len)
  {
    if(pcb.rxfifo.len > 0)
    {
       len--;
      (*ptr++) = uartRxBufferRead();
    }

    if(TimerDrvGetDly(ticks, timeout))
    {
      printf("\r\n receive drv timeout\r\n");
      return -1;
    }
}*/
	int rxBytes = UARTGrp.Read(uart_num, rx, len, timeout);
	if (rxBytes < len) {
		printf("\r\n receive drv timeout\r\n");
        return -1;
	}
    return 0;
}

/*void uartRxBufferWrite(uint8_t data)
{
	pcb.rxfifo.buf[pcb.rxfifo.wr_ptr] = data;
	pcb.rxfifo.wr_ptr = (pcb.rxfifo.wr_ptr + 1) % CFG_UART_BUFSIZE;
	pcb.rxfifo.len++;
}

void uartRxBufferClearFIFO()
{
	pcb.rxfifo.rd_ptr = 0;
	pcb.rxfifo.wr_ptr = 0;
	pcb.rxfifo.len = 0;
}*/

/*static uint8_t uartRxBufferDataPending(void)
{
	if(pcb.rxfifo.len != 0)
	{
		return 1;
	}
	return 0;
}

static uart_pcb_t *uartGetPCB(void)
{
    return &pcb;
}*/


void mopen(void)
{
  uartInit();
}

/*int mreceive(uint8_t *pbtRx, const size_t szRx, int timeout, int uart_num)
{
  return uartRxBufferReadArray(pbtRx, szRx, timeout, uart_num);
}
//const
int msend(const uint8_t *pbtTx, const size_t szTx, int timeout, int uart_num)
{
  return uartSend((uint8_t *)pbtTx, szTx, timeout, uart_num);
}*/

void mclose(void)
{
  printf("bye bye\r\n");
}

static int mreceive1(uint8_t *pbtRx, const size_t szRx, int timeout) {
  return uartRxBufferReadArray(pbtRx, szRx, timeout, 1);
}
static int mreceive2(uint8_t *pbtRx, const size_t szRx, int timeout) {
  return uartRxBufferReadArray(pbtRx, szRx, timeout, 2);
}

static int msend1(const uint8_t *pbtTx, const size_t szTx, int timeout) {
  return uartSend((uint8_t *)pbtTx, szTx, timeout, 1);
}
static int msend2(const uint8_t *pbtTx, const size_t szTx, int timeout) {
  return uartSend((uint8_t *)pbtTx, szTx, timeout, 2);
}

static const char TAG[] = "emdev";

nfc_emdev * emdevlistforuart[2] = {NULL, NULL};

void emdev_init(nfc_emdev *emdev)
{
  if (emdev == emdevlistforuart[0]) {
	  emdev->receive = mreceive1;
	  emdev->send = msend1;
  } else if (emdev == emdevlistforuart[1]) {
	  emdev->receive = mreceive2;
	  emdev->send = msend2;
  } else {
	  ESP_LOGE(TAG, "This library has been port to ESP32, and usage is different");
	  ESP_LOGE(TAG, "use \"getUartGrpforNFCemdev(emdev, uart_num)\" to get uart resources");
	  return;
  }
  emdev->open = mopen;
  emdev->close = mclose;
}

void getUartGrpforNFCemdev(nfc_emdev *emdev, int uart_num) {
	if (uart_num == 1 || uart_num == 2) {
		UARTGrp_config_t* configp = UARTGrp.getConfig(uart_num);
		if (configp == NULL) {
			ESP_LOGE(TAG, "uart num %d has been occupied!", uart_num);
			return;
		}
		UARTGrp.init(uart_num);
		emdevlistforuart[uart_num - 1] = emdev;
	} else {
		ESP_LOGE(TAG, "error uart num %d", uart_num);
	}
}
