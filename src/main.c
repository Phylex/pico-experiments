#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"

#include "ssd1306.h"
#include "image.h"
#include "acme_5_outlines_font.h"
#include "bubblesstandard_font.h"
#include "crackers_font.h"
#include "BMSPA_font.h"

const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[4]= {acme_font, bubblesstandard_font, crackers_font, BMSPA_font};
const char *words[]= {"Hello", "World", "DRIVER"};

#define I2C_ID i2c1
#define I2C_BAUD 100000
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

#define UART_ID uart0
#define UART_BAUD 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define DISPLAY_SIZE_X 128
#define DISPLAY_SIZE_Y 64
#define FONT_SIZE_X 5
#define FONT_SIZE_Y 8
#define DCHAR_BUF_SIZE 8 * 25
#define SLEEPTIME 500

void setup_gpios(void);
void animation(void);

typedef struct {
    uint8_t x; 		/**< width of display */
    uint8_t y;		/**< height of display */
    char c;		/**< stores pages of display (calculated on initialization*/
} display_char;

void draw_text_screen(ssd1306_t *screen, display_char *dcharbuf, size_t chars_to_display) {
    for (size_t i = 0; i < chars_to_display; i++) {
        ssd1306_draw_char(screen, dcharbuf[i].x, dcharbuf[i].y, 1, dcharbuf[i].c);
    }
}

void print_dchar_buf_to_uart(uart_inst_t *uart, display_char *dcbuf, size_t chars_to_print) {
    char buf[128];
    for (size_t i = 0; i < chars_to_print; i++) {
        sprintf(buf, "c: %c, x: %d, y: %d\r\n", dcbuf[i].c, dcbuf[i].x, dcbuf[i].y);
        uart_puts(uart, buf);
    }


}

int main() {
    /* initialize the chip */
    stdio_init_all();
    i2c_init(I2C_ID, I2C_BAUD);
    uart_init(UART_ID, UART_BAUD); 
    
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_pull_up(I2C_SCL_PIN);
    gpio_pull_up(I2C_SDA_PIN);
    bi_decl(bi_2pins_with_func(I2C_SDA_PIN, I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_2pins_with_func(UART_RX_PIN, UART_TX_PIN, GPIO_FUNC_I2C));

    /* initialize the display */ 
    ssd1306_t disp;
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_ID);
    ssd1306_clear(&disp);
    ssd1306_show(&disp);
    uint8_t x_cursor_pos = 0;
    uint8_t y_cursor_pos = 0;
    display_char text_buffer[DCHAR_BUF_SIZE];
    uint8_t next_free_buf_pos = 0;
    size_t available_chars = 0;
    char *out_str = (char *)malloc(128*sizeof(char));
    while (1) {
        if(uart_is_readable(UART_ID)){
            char c = uart_getc(UART_ID);
            display_char dc;
            text_buffer[next_free_buf_pos].x = x_cursor_pos;
            text_buffer[next_free_buf_pos].y = y_cursor_pos;
            text_buffer[next_free_buf_pos].c = c;
            next_free_buf_pos++;
            if (next_free_buf_pos > DCHAR_BUF_SIZE) {
                next_free_buf_pos = 0;
            }
            sprintf(out_str, "nfbp: %d\r\n", next_free_buf_pos);
            uart_puts(UART_ID, out_str);
            available_chars++;
            x_cursor_pos += FONT_SIZE_X;
            if (available_chars > DCHAR_BUF_SIZE) {
                available_chars = DCHAR_BUF_SIZE;
            }
            if (x_cursor_pos  + FONT_SIZE_X > DISPLAY_SIZE_X) {
                x_cursor_pos = 0;
                y_cursor_pos += FONT_SIZE_Y;
            }
            if (y_cursor_pos + FONT_SIZE_Y > DISPLAY_SIZE_Y) {
                y_cursor_pos = 0;
            }
            ssd1306_clear(&disp);
            draw_text_screen(&disp, text_buffer, available_chars);
            ssd1306_show(&disp);
        }
    }

    return 0;
}
