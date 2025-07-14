/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO
static uint8_t s_led_state = 0;
#ifdef CONFIG_BLINK_LED_STRIP
static led_strip_handle_t led_strip;

char inputString[64] = {'s', 's', 's', 'd', 'd', 'd', 'w', 'w', 'w', 'd', 'a', 's', 'd', 'd', 'a', 'w', 's', 'a', 's', 'd', 's', 'w', 's', 'd', 'w', 'd', 'w', 'd', 'w', 'w', 'a', 'w', 'a', 's', 'd', 'a', 's', 'd', 'd', 'a', 's', 'a', 's', 'd', 's', 'w', 's', 'a', 's', 'd', 's', 'w', 's', 'd', 'w', 'd', 'w', 'd', 'w', 'w', 'a', 'w', 'a', '\0'};

int lastIndex = 34;
float brightness = 0.5;
int output = -1;

// init head coords
int horizontal = 4;
int vertical = 4;
int headInit = 36;
int tail = 35;

int snake[65] = {-1}; // full snake populated entirely except final slot, used to keep track of snake size

// colorine
int fruitR = 229;
int fruitG = 57;
int fruitB = 57;

int snakeR = 71;
int snakeG = 12;
int snakeB = 90;

// void spawnFruit(void ){
//     int value=-1;
//     int upper_bound = 64;
//     int lower_bound = 0;
//     for (int i = 0; i < 10; i++) {
//         value = rand() % (upper_bound - lower_bound + 1)
//                     + lower_bound;
//         if(snake[value] == -1) break;
//     }
// }

int codeIntoCoordinates(int input)
{
    if (input < 0 && input > 63)
        return -1; // wrong input
    horizontal = input / 8;
    vertical = input % 8;

    return 0;
}

int decodeCoordenates(int x, int y)
{
    if (x < 0)
        x = 7;
    if (x > 7)
        x = 0;

    if (y < 0)
        y = 7;
    if (y > 7)
        y = 0;
    return (x * 8) + y;
    printf("x %d y %d\n", x, y);
};

void updateSnakeCoordinates(int newCoordinate, _Bool grow)
{
    int aux = snake[0];
    int aux2 = snake[1];
    snake[0] = newCoordinate;

    for (int i = 1; i < snake[64]; i++)
    {
        snake[i] = aux;
        aux = aux2;
        aux2 = snake[i + 1];
        if (grow && i == snake[64] - 1)
        {
            snake[i + 1] = aux;
            snake[64]++;
            break;
        }
    }
}

void headMovementUpdate(char input, int x, int y)
{
    int output = -1;
    int oldPos = decodeCoordenates(x, y);
    switch (input)
    {
    case 'a':
        output = decodeCoordenates(x, y - 1);
        updateSnakeCoordinates(output, false);
        codeIntoCoordinates(output); // updates new head coordinates automaticaly
        printf("Move left, led number:%d. Coordinates: (%d,%d)\n", output, horizontal, vertical);
        break;
    case 's':
        output = decodeCoordenates(x + 1, y);
        updateSnakeCoordinates(output, false);
        codeIntoCoordinates(output);
        printf("Move down, led number:%d. Coordinates: (%d,%d)\n", output, horizontal, vertical);
        break;
    case 'w':
        output = decodeCoordenates(x - 1, y);
        updateSnakeCoordinates(output, false);
        codeIntoCoordinates(output);
        printf("Move up, led number:%d. Coordinates: (%d,%d)\n", output, horizontal, vertical);
        break;
    case 'd':
        output = decodeCoordenates(x, y + 1);
        updateSnakeCoordinates(output, false);
        codeIntoCoordinates(output);
        printf("Move right, led number:%d. Coordinates: (%d,%d)\n", output, horizontal, vertical);
        break;

    default:
        break;
    }
    // return output;
}

static void updateLed(void)
{
    for (int i = 0; i < snake[64]; i++)
    {
        led_strip_set_pixel(led_strip, snake[i], snakeR / 2, snakeG / 2, snakeB / 2);
    }
    led_strip_refresh(led_strip);
}

static void updateLedOff(void)
{
    printf("Position that will be erased %d\n", snake[snake[64] - 1]);
    led_strip_set_pixel(led_strip, snake[snake[64] - 1], 0, 0, 0);
    led_strip_refresh(led_strip);
}

// static void updateLedWithIndex(int indexCoord, bool erase)
// {
//     if (erase)
//     {
//         led_strip_set_pixel(led_strip, indexCoord, 0, 0, 0);
//         led_strip_refresh(led_strip);
//     }
//     else
//     {
//         led_strip_set_pixel(led_strip, indexCoord, snakeR, snakeG, snakeB);
//         led_strip_refresh(led_strip);
//     }
// }
// static void updateLed(char userInput)
// {
//     // reset last
//     led_strip_set_pixel(led_strip, lastIndex, 0, 0, 0);
//     led_strip_refresh(led_strip);
//     // move head
//     int index = headMovement(userInput, horizontal, vertical);
//     lastIndex = tail;
//     tail = index;
//     led_strip_set_pixel(led_strip, tail, snakeR, snakeG, snakeB);
//     led_strip_set_pixel(led_strip, index, snakeR, snakeG, snakeB);

//     led_strip_refresh(led_strip);
// }

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 64, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

void app_main(void)
{
    snake[64] = 4; // initial snake size
    snake[0] = 36;
    snake[1] = 35;
    snake[2] = 34;
    snake[3] = 33;

    /* Configure the peripheral according to the LED type */
    configure_led();

    // led_strip_set_pixel(led_strip, tail, snakeR, snakeG, snakeB); // cola inicial
    // led_strip_set_pixel(led_strip, 36, snakeR, snakeG, snakeB);   // cabeza inicial

    led_strip_refresh(led_strip);

    for (int j = 0; j < sizeof(inputString) - 1; j++)
    {
        headMovementUpdate(inputString[j], horizontal, vertical);
        updateLed();
        updateLedOff();
        vTaskDelay(50);
    }

    // clear all leds
    led_strip_clear(led_strip);
}
