/*
 * listen-udp.c - Illustrate simple TCP connection
 * It opens a blocking socket and
 * listens for messages in a for loop.  It takes the name of the machine
 * that it will be listening on as argument.
 */

/*
sudo gcc  -lm ./sk9822led.c ./column.c -o test_column
*/

#include <sys/types.h>

#include "sk9822led.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *device = "/dev/spidev0.0";
static const int message_size = 3000 * 3;
static const int lines_num = 22; // 22
static const int lines_leds = 90; // 90
static const int leds = 1980; // 90*22
static int continue_looping;

#define SERVER_PORT 3000

void stop_program(int sig);
void HSVtoRGB(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b);

main(int argc, char *argv[])
{
    char message[message_size];

    int bytes;

    // -------- sk9822 ----------
    sk9822_buffer buf;
    int fd;
    int return_value;
    sk9822_color *p;
    int i;
    float h;
    uint8_t r, g, b;

    /* Open the device file using Low-Level IO */
    fd = open(device, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        exit(1);
    }

    /* Initialize the SPI bus for Total Control Lighting */
    return_value = spi_init(fd);
    if (return_value == -1) {
        fprintf(stderr, "SPI initialization error %d: %s\n", errno, strerror(errno));
        exit(1);
    }

    /* Initialize pixel buffer */
    if (sk9822_init(&buf, leds) < 0) {
        fprintf(stderr, "Pixel buffer initialization error: Not enough memory.\n");
        exit(1);
    }

    /* Set the gamma correction factors for each color */
    set_gamma(2.2, 2.2, 2.2);

    /* Blank the pixels */
    for (i = 0; i < leds; i++) {
        write_color(&buf.pixels[i], 0x00, 0x00, 0x00);
    }

    send_buffer(fd, &buf, leds);
    signal(SIGINT, stop_program);

    continue_looping = 1;
    int isGroving = 1;
    int led, line;
    uint8_t value = 0;
    while (continue_looping) {

        // message[bytes] = '\0';
        // printf("recv: %s\n", message);
        HSVtoRGB((uint8_t)h, 13, 17, &r, &g, &b);
        for (led = 0; led < leds; ++led) {
            // printf("led: %i, line: %i offset: %i\n", led, line, offset);
            // if (led > 0)
            //  write_color(&buf.pixels[offset-1], 0x00, 0x00, 0x00);
            // else
            //  write_color(&buf.pixels[offset+lines_leds-1], 0x00, 0x00, 0x00);
            // printf("%d  %d  %d\n",message[i*3+0], message[i*3+1], message[i*3+2]);
            // printf("%d  %d  %d\n",r,g,b);
            write_color(&buf.pixels[led], (uint8_t)r, (uint8_t)g, (uint8_t)b);
            // write_color(&buf.pixels[led], value, value, value);
        }
        h += 1;
        if (h > 255)
          h=0;
        int cntr = 0;
        for (; cntr < 5; cntr++) {
            send_buffer(fd, &buf, leds);
            // printf("%d\n",value);

            // value+= isGroving ? 1 :-1;
            // if (value > 20)
            //  isGroving = 0;
            // else if (value <= 0) {
            //  isGroving = 1;
            //   h += 1;
            // }

            usleep(500);
        }
    }

    for (i = 0; i < leds; i++) {
        write_color(&buf.pixels[i], 0x00, 0x00, 0x00);
    }
    send_buffer(fd, &buf, leds);

    return 0;
}

// for (led = 0; led < lines_leds; led++) {
//     // message[bytes] = '\0';
//     // printf("recv: %s\n", message);
//     for (line = 0; line < lines_num; line++) {
//         int offset = (line * lines_leds + led);
//         // printf("led: %i, line: %i offset: %i\n", led, line, offset);
//         if (led > 0)
//          write_color(&buf.pixels[offset-1], 0x00, 0x00, 0x00);
//         else
//          write_color(&buf.pixels[offset+lines_leds-1], 0x00, 0x00, 0x00);

//         write_color(&buf.pixels[offset], 0xff, 0xff, 0xff);
//     }
//     send_buffer(fd, &buf, leds);

//     usleep(50000);
// }

void stop_program(int sig)
{
    /* Ignore the signal */
    signal(sig, SIG_IGN);
    /* stop the looping */
    continue_looping = 0;
    /* Put the ctrl-c to default action in case something goes wrong */
    signal(sig, SIG_DFL);
}

uint8_t max_whiteness = 15;
uint8_t max_value = 17;
uint8_t sixth_hue = 16;
uint8_t third_hue = 16 * 2;
uint8_t half_hue = 16 * 3;
uint8_t two_thirds_hue = 16 * 4;
uint8_t five_sixths_hue = 16 * 5;
uint8_t full_hue = 16 * 6;

/* Convert hsv values (0<=h<360, 0<=s<=1, 0<=v<=1) to rgb values (0<=r<=1, etc) */
// void HSVtoRGB(double h, double s, double v, double *r, double *g, double *b) {
void HSVtoRGB(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // if (hsv.v == 0) return black;

    uint8_t high = v * max_whiteness; // channel with max value
    // if (hsv.s == 0) return rgb(high, high, high);

    uint8_t W = max_whiteness - s;
    uint8_t low = v * W; // channel with min value
    uint8_t rising = low;
    uint8_t falling = high;

    uint8_t h_after_sixth = h % sixth_hue;
    if (h_after_sixth > 0) { // not at primary color? ok, h_after_sixth = 1..sixth_hue - 1
        uint8_t z = s * (uint8_t)(v * h_after_sixth) / sixth_hue;
        rising += z;
        falling -= z + 1; // it's never 255, so ok
    }

    uint8_t H = h;
    while (H >= full_hue)
        H -= full_hue;

    if (H < sixth_hue) {
        *r = high;
        *g = rising;
        *b = low;
    }
    else if (H < third_hue) {
        *r = falling;
        *g = high;
        *b = low;
    }
    else if (H < half_hue) {
        *r = low;
        *g = high;
        *b = rising;
    }
    else if (H < two_thirds_hue) {
        *r = low;
        *g = falling;
        *b = high;
    }
    else if (H < five_sixths_hue) {
        *r = rising;
        *g = low;
        *b = high;
    }
    else {
        *r = high;
        *g = low;
        *b = falling;
    }
    return;
}