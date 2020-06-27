/*
 * main.c
 *
 * Danica Fujiwara & Spencer Shaw
 *
 * CPE 329-17/18 Spring 2019
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "msp.h"

//#include "button.h"
#include "bno055.h"
#include "delay.h"
#include "i2c.h"
#include "led.h"
#include "maze_term.h"
#include "my_msp.h"
#include "uart.h"

#define FREQ FREQ_48_MHZ
#define UPDATE_DELAY (75)

unsigned char str_enter_reg[] = "Enter register address: ";
unsigned char str_reg[] = "Reg address: ";
unsigned char str_enter_val[] = "Enter value: ";
unsigned char str_val[] = "Value: ";
unsigned char str_writing[] = "Writing value ";
unsigned char str_to_reg[] = " to reg ";
unsigned char str_from_reg[] = " from reg ";
unsigned char str_done[] = "Done!";
unsigned char str_reading[] = "Reading vals...";
unsigned char str_vals_start[] = "X: ";
unsigned char str_vals_mid[] = "\tY: ";

volatile unsigned char char_data = '7';
volatile uint8_t got_fresh_char = FALSE;

int main(void) {
    int16_t x_val = 1;
    int16_t y_val = 1;

    init(FREQ);

    bno_write(BNO_REG_MODE, BNO_MODE_NDOF);

    delay_ms(250, FREQ_48_MHZ);

    while (1) {
        // uart_write_nl();
        // uart_write_str(str_vals_start, sizeof(str_vals_start));
        // uart_write_int(x_val);
        // uart_write_str(str_vals_mid, sizeof(str_vals_mid));
        // uart_write_int(y_val);

        paint_terminal();
        delay_ms(1000, FREQ);
        level_animation(1);
        maze1();
        while (!check_win(WIN_M1_X, WIN_M1_Y)) {
            bno_read_angles(&x_val, &y_val);

            // update_ball(x_val, y_val);
            set_ball_vels(x_val, y_val);
            check_maze1();

            delay_ms(UPDATE_DELAY, FREQ);
        }
        win_animation();
        level_animation(2);
        maze2();
        while (!check_win(WIN_M2_X, WIN_M2_Y)) {
            bno_read_angles(&x_val, &y_val);

            // update_ball(x_val, y_val);
            set_ball_vels(x_val, y_val);
            check_maze2();

            delay_ms(UPDATE_DELAY, FREQ);
        }
        win_animation();
        level_animation(3);
        maze3();
        while (!check_win(WIN_M3_X, WIN_M3_Y)) {
            bno_read_angles(&x_val, &y_val);

            // update_ball(x_val, y_val);
            set_ball_vels(x_val, y_val);
            check_maze3();

            delay_ms(UPDATE_DELAY, FREQ);
        }
        win_animation();
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void) {
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) {
        if (has_new) {
            return;
        }
        // Check if the TX buffer is empty first
        while (!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG)) {
        }

        new_char = EUSCI_A0->RXBUF;
        has_new = TRUE;
        // Echo the received character back
        EUSCI_A0->TXBUF = new_char;
        // delay_ms(10, FREQ);
    }
}
