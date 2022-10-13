#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/timer.h"

#define PIN_BUSY 28
#define PIN_CS   1
#define PIN_SCK  2
#define PIN_MOSI 3
#define PIN_DC   4
#define PIN_RST  5

#include "img.h"

const unsigned char LUT_DATA[] = {
    0x80,
    0x60,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00, //LUT0: BB:     VS 0 ~7
    0x10,
    0x60,
    0x20,
    0x00,
    0x00,
    0x00,
    0x00, //LUT1: BW:     VS 0 ~7
    0x80,
    0x60,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00, //LUT2: WB:     VS 0 ~7
    0x10,
    0x60,
    0x20,
    0x00,
    0x00,
    0x00,
    0x00, //LUT3: WW:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT4: VCOM:   VS 0 ~7

    0x03,
    0x03,
    0x00,
    0x00,
    0x02, // TP0 A~D RP0
    0x09,
    0x09,
    0x00,
    0x00,
    0x02, // TP1 A~D RP1
    0x03,
    0x03,
    0x00,
    0x00,
    0x02, // TP2 A~D RP2
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP3 A~D RP3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP4 A~D RP4
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP5 A~D RP5
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP6 A~D RP6

    0x15,
    0x41,
    0xA8,
    0x32,
    0x30,
    0x0A,
};
const unsigned char LUT_DATA_part[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT0: BB:     VS 0 ~7
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT1: BW:     VS 0 ~7
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT2: WB:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT3: WW:     VS 0 ~7
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, //LUT4: VCOM:   VS 0 ~7

    0x0A,
    0x00,
    0x00,
    0x00,
    0x00, // TP0 A~D RP0
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP1 A~D RP1
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP2 A~D RP2
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP3 A~D RP3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP4 A~D RP4
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP5 A~D RP5
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP6 A~D RP6

    0x15,
    0x41,
    0xA8,
    0x32,
    0x30,
    0x0A,
};

void readBusy() {
    while(gpio_get(PIN_BUSY) == 1) {
        tight_loop_contents();
    }
}

void sendCmd(uint8_t cmd) {
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

void sendData(uint8_t cmd) {
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

void selectLUT(uint8_t *wave_data)
{
    sendCmd(0x32);
    for(int count = 0; count < 70; count++)
        sendData(wave_data[count]);
}

void initFull()
{
    readBusy();
    sendCmd(0x12); // soft reset
    readBusy();

    sendCmd(0x74); //set analog block control
    sendData(0x54);
    sendCmd(0x7E); //set digital block control
    sendData(0x3B);

    sendCmd(0x01); //Driver output control
    sendData(0xF9);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x11); //data entry mode
    sendData(0x01);

    sendCmd(0x44); //set Ram-X address start/end position
    sendData(0x00);
    sendData(0x0F); //0x0C-->(15+1)*8=128

    sendCmd(0x45);  //set Ram-Y address start/end position
    sendData(0xF9); //0xF9-->(249+1)=250
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x3C); //BorderWavefrom
    sendData(0x03);

    sendCmd(0x2C);  //VCOM Voltage
    sendData(0x55); //

    sendCmd(0x03); //
    sendData(LUT_DATA[70]);

    sendCmd(0x04); //
    sendData(LUT_DATA[71]);
    sendData(LUT_DATA[72]);
    sendData(LUT_DATA[73]);

    sendCmd(0x3A); //Dummy Line
    sendData(LUT_DATA[74]);
    sendCmd(0x3B); //Gate time
    sendData(LUT_DATA[75]);

    selectLUT((unsigned char *)LUT_DATA); //LUT

    sendCmd(0x4E); // set RAM x address count to 0;
    sendData(0x00);
    sendCmd(0x4F); // set RAM y address count to 0X127;
    sendData(0xF9);
    sendData(0x00);
    readBusy();
}

void initPartial()
{
    sendCmd(0x2C); //VCOM Voltage
    sendData(0x26);

    readBusy();
    selectLUT((unsigned char *)LUT_DATA_part);
    sendCmd(0x37);
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);
    sendData(0x40);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x22);
    sendData(0xC0);
    sendCmd(0x20);
    readBusy();

    sendCmd(0x3C); //BorderWavefrom
    sendData(0x01);
}

int main() {
    stdio_init_all();
    while(!stdio_usb_connected()) tight_loop_contents();
    sleep_ms(500);

    spi_init(spi0, 4000000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_init(PIN_BUSY);
    gpio_set_dir(PIN_BUSY, GPIO_IN);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 1);
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 1);
    
    sleep_ms(1);
    gpio_put(PIN_RST, 0);
    sleep_ms(25);
    gpio_put(PIN_RST, 1);

    //gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(PIN_CS, GPIO_FUNC_SPI);

    initFull();

sendCmd(0x24); //write RAM for black(0)/white (1)
for (int i = 0; i < 250 * 16; i++)
{
    sendData(0xFF);
}
sendCmd(0x22);
sendData(0xC7);
sendCmd(0x20);
readBusy();

    initPartial();

int zones_mask = 0;
while(true) {
zones_mask += 1;
sendCmd(0x24); //write RAM for black(0)/white (1)
for (int i = 0; i < 250 * 16; i++)
{
    uint8_t data = 0xFF;
    for (int n=0; n<28; n++)
        if (zones_mask & (1 << n))
            data &= ZONES[n][i];
    sendData(data);
}
sendCmd(0x22);
sendData(0x0C);
sendCmd(0x20);
readBusy();
}
    return 0;
}
