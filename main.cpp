#include "epaper.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "hardware/rtc.h"

#include "img.h"

#define UP 0x04
#define UL 0x01
#define UR 0x20
#define MD 0x08
#define DL 0x02
#define DR 0x40
#define DN 0x10
uint8_t dec_to_segments[10] = {
    UP | UL | UR | DL | DR | DN,
    UR | DR,
    UP | UR | MD | DL | DN,
    UP | UR | MD | DR | DN,
    UL | UR | MD | DR,
    UP | UL | MD | DR | DN,
    UP | UL | MD | DL | DR | DN,
    UP | UR | DR,
    UP | UL | UR | MD | DL | DR | DN,
    UP | UL | UR | MD | DR | DN,
};


void draw7seg(int zone_offset, int value, float to_next_fraction)
{
    for(int n=0; n<7; n++) {
        auto& zone = ZONES[n + zone_offset];
        if (dec_to_segments[value] & (1 << n)) {
            int offset = zone.offset;
            for(int x=0; x<zone.width; x++)
                for(int y=0; y<zone.height; y++)
                    epaper_buffer[offset + x * 16 + y] &= zone.data[x*zone.height+y];

            if (!(dec_to_segments[(value + 1) % 10] & (1 << n))) {
                //Next segment step will be off, so hide this line.
                for(int tmp=0; tmp<zone.line_length * to_next_fraction; tmp++) {
                    auto p = zone.line[tmp];
                    epaper_buffer[p.x * 16 + p.y / 8] |= (0x80 >> (p.y % 8));
                }
            }
        } else {
            if ((dec_to_segments[(value + 1) % 10] & (1 << n))) {
                //Next segment step will be on, so show this line.
                for(int tmp=0; tmp<zone.line_length * to_next_fraction; tmp++) {
                    auto p = zone.line[tmp];
                    epaper_buffer[p.x * 16 + p.y / 8] &=~(0x80 >> (p.y % 8));
                }
            }
        }
    }
}


int main() {
    stdio_init_all();
    //while(!stdio_usb_connected()) tight_loop_contents();
    sleep_ms(500);

    rtc_init();

    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };
    rtc_set_datetime(&t);

    epaper_init();

    for(int n=0; n<28; n++)
    {
        int offset = ZONES[n].offset;
        for(int x=0; x<ZONES[n].width; x++)
            for(int y=0; y<ZONES[n].height; y++)
                epaper_buffer[offset + x * 16 + y] &= ZONES[n].data[x*ZONES[n].height+y];
    }
    epaper_full_refresh();
    epaper_prepare_partial();

    auto last_sec = t.sec;
    while(true)
    {
        while(t.sec == last_sec) {
            rtc_get_datetime(&t);
            sleep_ms(100);
        }
        last_sec = t.sec;

        memset(epaper_buffer, 0xFF, sizeof(epaper_buffer));
        draw7seg(21, t.sec % 10, 0.5f);
        draw7seg(14, t.sec / 10, float(t.sec % 10) / 10.0f);
        draw7seg( 7, t.min % 10, float(t.sec) / 60.0f);
        draw7seg( 0, t.min / 10, float(t.sec + (t.min % 10) * 60) / 600.0f);

        epaper_partial_refresh();
    }
    epaper_sleep();

    reset_usb_boot(25, 0);
    return 0;
}
