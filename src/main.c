#include <gb/gb.h>
#include <gbdk/font.h>
#include <gbdk/console.h>
#include <stdio.h>

uint8_t rtc_s, rtc_m, rtc_h, rtc_dl, rtc_dh;

void read_rtc(void) {
    ENABLE_RAM;

    // Latch RTC
    *(volatile uint8_t *)0x6000 = 0x00;
    *(volatile uint8_t *)0x6000 = 0x01;

    // Read RTC registers
    SWITCH_RAM(0x08);
    rtc_s = *(volatile uint8_t *)0xA000;
    SWITCH_RAM(0x09);
    rtc_m = *(volatile uint8_t *)0xA000;
    SWITCH_RAM(0x0A);
    rtc_h = *(volatile uint8_t *)0xA000;
    SWITCH_RAM(0x0B);
    rtc_dl = *(volatile uint8_t *)0xA000;
    SWITCH_RAM(0x0C);
    rtc_dh = *(volatile uint8_t *)0xA000;

    DISABLE_RAM;
}

void rtc_stop(void) {
    uint8_t dh;

    ENABLE_RAM;

    SWITCH_RAM(0x0C);
    dh = *(volatile uint8_t *)0xA000;
    *(volatile uint8_t *)0xA000 = dh | 0x40; // set halt

    DISABLE_RAM;
}

void rtc_start(void) {
    uint8_t dh;

    ENABLE_RAM;

    SWITCH_RAM(0x0C);
    dh = *(volatile uint8_t *)0xA000;
    *(volatile uint8_t *)0xA000 = dh & ~0x40; // clear halt

    DISABLE_RAM;
}

// Decode day counter (Day Low + bit 0 of Day High) into 0-511
uint16_t rtc_get_day(void) {
    return ((uint16_t)(rtc_dh & 0x01) << 8) | rtc_dl;
}

// Day Counter Carry flag (Day High bit 7)
uint8_t rtc_get_carry(void) {
    return (rtc_dh & 0x80) ? 1 : 0;
}

// Halt flag (Day High bit 6)
uint8_t rtc_is_halted(void) {
    return (rtc_dh & 0x40) ? 1 : 0;
}

// Write s/m/h/day; preserves Halt/Carry flags in Day High
void write_rtc(uint8_t s, uint8_t m, uint8_t h, uint16_t day) {
    uint8_t dh;

    day &= 0x01FF; // 9-bit counter

    ENABLE_RAM;

    SWITCH_RAM(0x08);
    *(volatile uint8_t *)0xA000 = s;
    SWITCH_RAM(0x09);
    *(volatile uint8_t *)0xA000 = m;
    SWITCH_RAM(0x0A);
    *(volatile uint8_t *)0xA000 = h;

    SWITCH_RAM(0x0B);
    *(volatile uint8_t *)0xA000 = (uint8_t)(day & 0xFF);

    SWITCH_RAM(0x0C);
    dh = *(volatile uint8_t *)0xA000;
    dh = (dh & ~0x01) | (uint8_t)(day >> 8);
    *(volatile uint8_t *)0xA000 = dh;

    DISABLE_RAM;
}

void main(void){
    uint8_t input, prevInput = 0;

    // Loop forever
    while (1) {
        input = joypad();

        // Set demo time 12:34:56, day 0
        if ((input & J_A) && !(prevInput & J_A)) {
            write_rtc(56, 34, 12, 0);
        }
        if ((input & J_B) && !(prevInput & J_B)) {
            write_rtc(rtc_s, rtc_m, rtc_h, 100);
        }
        if ((input & J_SELECT) && !(prevInput & J_SELECT)) {
            rtc_stop();
        }
        if ((input & J_START) && !(prevInput & J_START)) {
            rtc_start();
        }

        prevInput = input;

        read_rtc();

        gotoxy(0, 0);
        printf("Seconds: %hu  \n", (uint8_t)rtc_s);
        printf("Minutes: %hu  \n", (uint8_t)rtc_m);
        printf("Hours: %hu  \n", (uint8_t)rtc_h);
        printf("Day: %u    \n", rtc_get_day());
        printf("Carry: %hu\n", (uint8_t)rtc_get_carry());
        printf("Status: %s\n", rtc_is_halted() ? "Halted " : "Running");
        printf("\nA:set  B:day100\n");
        printf("SELECT:stop\n");
        printf("START:run\n");

        wait_vbl_done();
    }
}
