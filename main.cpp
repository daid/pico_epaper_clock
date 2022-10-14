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

#define UP 0x01
#define UL 0x02
#define UR 0x04
#define MD 0x08
#define DL 0x10
#define DR 0x20
#define DN 0x40
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
uint8_t segment_to_zone[] = {
    21 + 0, // UP
    21 + 1, // UL
    21 + 6, // UR
    21 + 4, // MD
    21 + 2, // DL
    21 + 5, // DR
    21 + 3, // DN

    14 + 3, // UP
    14 + 0, // UL
    14 + 5, // UR
    14 + 4, // MD
    14 + 1, // DL
    14 + 6, // DR
    14 + 2, // DN

    7 + 3, // UP
    7 + 0, // UL
    7 + 5, // UR
    7 + 4, // MD
    7 + 1, // DL
    7 + 6, // DR
    7 + 2, // DN

    3, // UP
    0, // UL
    5, // UR
    4, // MD
    1, // DL
    6, // DR
    2, // DN
};


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
    while(!stdio_usb_connected())
    {
        while(t.sec == last_sec) {
            rtc_get_datetime(&t);
            sleep_ms(100);
        }
        last_sec = t.sec;

        memset(epaper_buffer, 0xFF, sizeof(epaper_buffer));
        for(int n=0; n<7; n++) {
            if (dec_to_segments[t.sec % 10] & (1 << n)) {
                auto& zone = ZONES[segment_to_zone[n]];
                int offset = zone.offset;
                for(int x=0; x<zone.width; x++)
                    for(int y=0; y<zone.height; y++)
                        epaper_buffer[offset + x * 16 + y] &= zone.data[x*zone.height+y];
            }

            if (dec_to_segments[t.sec / 10] & (1 << n)) {
                auto& zone = ZONES[segment_to_zone[n + 7]];
                int offset = zone.offset;
                for(int x=0; x<zone.width; x++)
                    for(int y=0; y<zone.height; y++)
                        epaper_buffer[offset + x * 16 + y] &= zone.data[x*zone.height+y];
            }

            if (dec_to_segments[t.min % 10] & (1 << n)) {
                auto& zone = ZONES[segment_to_zone[n + 14]];
                int offset = zone.offset;
                for(int x=0; x<zone.width; x++)
                    for(int y=0; y<zone.height; y++)
                        epaper_buffer[offset + x * 16 + y] &= zone.data[x*zone.height+y];
            }

            if (dec_to_segments[t.min / 10] & (1 << n)) {
                auto& zone = ZONES[segment_to_zone[n + 21]];
                int offset = zone.offset;
                for(int x=0; x<zone.width; x++)
                    for(int y=0; y<zone.height; y++)
                        epaper_buffer[offset + x * 16 + y] &= zone.data[x*zone.height+y];
            }
        }
        epaper_partial_refresh();
    }
    epaper_sleep();

    reset_usb_boot(25, 0);
    return 0;
}
