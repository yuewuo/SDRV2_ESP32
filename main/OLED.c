#include "lib/lib.h"
#include "lib/oledfont.h"

static spi_device_handle_t spi;

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

//Send a command to the OLED. Uses spi_device_transmit, which waits until the transfer is complete.
void oled_cmd(/*spi_device_handle_t spi, */const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//Send data to the OLED. Uses spi_device_transmit, which waits until the transfer is complete.
void oled_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void oled_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	oled_cmd(0xb0+y);
	oled_cmd(((x&0xf0)>>4)|0x10);
	oled_cmd((x&0x0f)|0x01);
}
//Open OLED display
void OLED_Display_On(void)
{
	oled_cmd(0X8D); //SET DCDC Command
	oled_cmd(0X14); //DCDC ON
	oled_cmd(0XAF); //DISPLAY ON
}
//Open OLED display
void OLED_Display_Off(void)
{
	oled_cmd(0X8D);  //SET DCD Command
	oled_cmd(0X10);  //DCDC OFF
	oled_cmd(0XAE);  //DISPLAY OFF
}
//clear screen, black
const static unsigned char init128_oled[128] = {0};
void OLED_Clear(void)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		oled_cmd(0xb0+i);    //set page address(0~7)
		oled_cmd(0x00);      //low address
		oled_cmd(0x10);      //high address
		oled_data(spi, init128_oled, 128);
	} //refresh display
}


//display char at specific location, as well as part of it
//x:0~127
//y:0~63
//mode:0 means reverse display, and 1 means normal display
//size:select char size 16/12
void OLED_ShowChar(uint8_t x, uint8_t y, char chr)
{
	unsigned char c;
	c = chr - ' '; //bias
	if(x>Max_Column-1) {x=0;y=y+2;}
	if(SIZE == 16) {
		OLED_Set_Pos(x,y);
		oled_data(spi, F8X16+c*16, 8);
		OLED_Set_Pos(x,y+1);
		oled_data(spi, F8X16+c*16+8, 8);
	} else {
		OLED_Set_Pos(x,y+1);
		oled_data(spi, F6x8[c], 6);
	}
}
//m^n function
uint32_t oled_pow(uint8_t m, uint8_t n)
{
	uint32_t result=1;
	while(n--) result*=m;
	return result;
}
//display two number
//x,y :start axis
//len :length of number
//size:size of char
//mode:0,tianchong, 1,diejia
//num :number(0~4294967295)
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
	uint8_t t, temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++) {
		temp = (num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0) {
				OLED_ShowChar(x+(size/2)*t,y,' ');
				continue;
			} else enshow=1;
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0');
	}
}
//show string
void OLED_ShowString(uint8_t x, uint8_t y, char *chr)
{
	uint8_t j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j]);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//show Chinese
/*void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no)
{
	uint8_t t,adder=0;
	OLED_Set_Pos(x,y);
    oled_data(spi, Hzk[2*no], 16);
    adder+=16;
    OLED_Set_Pos(x,y+1);
    oled_data(spi, Hzk[2*no+1], 16);
    adder+=16;
}*/
//show BMP(128*64), start at(x(0~128), y(0~7))
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, unsigned char BMP[])
{
 unsigned int j=0;
 unsigned char x,y;

  if(y1%8==0) y=y1/8;
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
	    oled_data(spi, BMP+j, x1-x0);
	    j+=(x1-x0);
	}
}


//Init SSD1306
void OLED_Init(void)
{
	esp_err_t ret;
    //spi_device_handle_t spi; static global
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=10000000,               //Clock out at 10 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=oled_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);
    //Attach the OLED to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    assert(ret==ESP_OK);

    //int cmd=0;
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);


	oled_cmd(0xAE);//--turn off oled panel
	oled_cmd(0x00);//---set low column address
	oled_cmd(0x10);//---set high column address
	oled_cmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	oled_cmd(0x81);//--set contrast control register
	oled_cmd(0xCF); // Set SEG Output Current Brightness
	oled_cmd(0xA1);//--Set SEG/Column Mapping     0xa0×óÓÒ·ŽÖÃ 0xa1Õý³£
	oled_cmd(0xC8);//Set COM/Row Scan Direction   0xc0ÉÏÏÂ·ŽÖÃ 0xc8Õý³£
	oled_cmd(0xA6);//--set normal display
	oled_cmd(0xA8);//--set multiplex ratio(1 to 64)
	oled_cmd(0x3f);//--1/64 duty
	oled_cmd(0xD3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	oled_cmd(0x00);//-not offset
	oled_cmd(0xd5);//--set display clock divide ratio/oscillator frequency
	oled_cmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	oled_cmd(0xD9);//--set pre-charge period
	oled_cmd(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	oled_cmd(0xDA);//--set com pins hardware configuration
	oled_cmd(0x12);
	oled_cmd(0xDB);//--set vcomh
	oled_cmd(0x40);//Set VCOM Deselect Level
	oled_cmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	oled_cmd(0x02);//
	oled_cmd(0x8D);//--set Charge Pump enable/disable
	oled_cmd(0x14);//--set(0x10) disable
	oled_cmd(0xA4);// Disable Entire Display On (0xa4/0xa5)
	oled_cmd(0xA6);// Disable Inverse Display On (0xa6/a7)
	oled_cmd(0xAF);//--turn on oled panel

	oled_cmd(0xAF); /*display ON*/
	OLED_Clear();
	OLED_Set_Pos(0,0);
}
