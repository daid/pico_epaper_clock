#pragma once

#include <cstdint>

extern uint8_t epaper_buffer[16*250];

void epaper_init();
void epaper_full_refresh();
void epaper_prepare_partial();
void epaper_partial_refresh();
void epaper_sleep();