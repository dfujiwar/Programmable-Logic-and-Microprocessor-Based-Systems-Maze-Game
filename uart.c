/*
 * uart.c
 *
 * Danica Fujiwara & Spencer Shaw
 *
 * CPE 329-17/18 Spring 2019
 */
#include <stdint.h>

#include "delay.h"
#include "led.h"
#include "msp.h"
#include "my_msp.h"
#include "maze_term.h"
#include "uart.h"

static unsigned char str_uart_welcome[] = "UART initialized";

inline void uart_init() {
    has_new = FALSE;
    new_char = 0;

    // Configure UART pins
    P1->SEL0 |= UART_PINS;  // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST;  // Put eUSCI in reset
    EUSCI_A0->CTLW0 =
        EUSCI_A_CTLW0_SWRST |       // Remain eUSCI in reset
        EUSCI_B_CTLW0_SSEL__SMCLK;  // Configure eUSCI clock source for SMCLK

    // Baud Rate calculation
    // 3000000/(115200) = 26.041667
    // Fractional portion = 0.041667
    // User's Guide Table 21-4: UCBRSx = 0x00
    // UCBRx = int (26.041667 / 16) = 1
    // UCBRFx = int (((26.041667/16)-1)*16) = 10

    EUSCI_A0->BRW = 1;  // Using baud rate calculator
    EUSCI_A0->MCTLW = (10 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16;
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;  // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;      // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;          // Enable USCI_A0 RX interrupt

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn)&31);

    // Enable global interrupt
    __enable_irq();

    term_clear_screen();
    uart_write_str(str_uart_welcome, sizeof(str_uart_welcome));
    uart_write_nl();
}

inline void uart_write(unsigned char c) {
    while (!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG)) {
    }
    EUSCI_A0->TXBUF = c;
    while (!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG)) {
    }
}

void uart_write_str(unsigned char c[], unsigned int size) {
    int i;
    for (i = 0; i < size; i++) {
        uart_write(c[i]);
    }
}

void uart_write_int(int acc) {
    if (acc < 0) {
        uart_write('-');
        acc = 0 - acc;
    }
    if (acc >= 10) {
        uart_write_int(acc / 10);
    }
    uart_write((uint8_t)((acc % 10) + '0'));
}

void uart_write_nl() {
    uart_write('\n');
    uart_write(0xD);
}

unsigned int uart_get_int() {
    int accumulator = 0;
    new_char = 255;
    while (new_char != ESCAPE_VAL) {
        if (has_new) {
            if (new_char == ESCAPE_VAL) {
                break;
            } else if (new_char >= '0' && new_char <= '9') {
                accumulator = (10 * accumulator) + (new_char - '0');
            }
            has_new = FALSE;
        }
    }
    has_new = FALSE;
    new_char = 0;
    return accumulator;
}

void uart_write_volts(unsigned int val_mv) {
    uart_write_int(val_mv / 100);
    uart_write('.');
    uart_write_int(val_mv % 100 / 10);
    uart_write_int(val_mv % 10);
    uart_write('V');
}
