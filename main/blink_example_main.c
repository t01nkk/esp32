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
// static uint8_t s_led_state = 0;
#ifdef CONFIG_BLINK_LED_STRIP
static led_strip_handle_t led_strip;

char inputString[64] = {'s', 's', 's', 's', 'd', 'd', 'd', 'd', 'w', 'w', 'w', 'w', 'w', 'w', 'w', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 's', 's', 's', 's', 's', 's', 's', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'w', 'w', 'w', 'w', 'w', 'w', 'w', '\0'};

int lastIndex;
bool updated = false;
float brightness = 0.5;
int output = -1;

int horizontal = 4;
int vertical = 4;
int headInit = 36;
int tail = 35;

int snake[65] = {-1};

// colorine
int fruitR = 10;
int fruitG = 10;
int fruitB = 10;

int snakeR = 4;
int snakeG = 10;
int snakeB = 4;

int codeIntoCoordinates(int input)
{
    if (input < 0 && input > 63)
        return -1; // wrong input
    horizontal = input / 8;
    vertical = input % 8;

    return 0;
}

int decodeCoordinates(int x, int y)
{
    printf("x %d y %d\n", x, y);
    if (x < 0)
        x = 0;
    if (x > 7)
        x = 7;

    if (y < 0)
        y = 0;
    if (y > 7)
        y = 7;
    return (x * 8) + y;
};

bool updateSnakeCoordinates(int newCoordinate, _Bool grow)
{
    if (newCoordinate == snake[1])
    {
        printf(" Ilegal moove NC %d = BH %d .. cant go backwards m'dude\n", newCoordinate, snake[1]);
        return false;
    }
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
    return true;
}

void headMovementUpdate(char input, int x, int y)
{

    int output = -1;
    switch (input)
    {
    case 'a':
        output = decodeCoordinates(x, y - 1);
        updated = updateSnakeCoordinates(output, false);
        printf("Move left, success %s, led number: %d. Coordinates: (%d,%d)\n", updated ? "true" : "false", output, horizontal, vertical);
        if (updated)
            codeIntoCoordinates(output);
        break;
    case 's':
        output = decodeCoordinates(x + 1, y);
        updated = updateSnakeCoordinates(output, false);
        printf("Move down, success %s, led number: %d. Coordinates: (%d,%d)\n", updated ? "true" : "false", output, horizontal, vertical);
        if (updated)
            codeIntoCoordinates(output);
        break;
    case 'w':
        output = decodeCoordinates(x - 1, y);
        updated = updateSnakeCoordinates(output, false);
        printf("Move up, success %s, led number: %d. Coordinates: (%d,%d)\n", updated ? "true" : "false", output, horizontal, vertical);
        if (updated)
            codeIntoCoordinates(output);
        break;
    case 'd':
        output = decodeCoordinates(x, y + 1);
        updated = updateSnakeCoordinates(output, false);
        printf("Move right, success %s, led number: %d. Coordinates: (%d,%d)\n", updated ? "true" : "false", output, horizontal, vertical);
        if (updated)
            codeIntoCoordinates(output);
        break;

    default:
        break;
    }
}

static void updateLed(void)
{
    for (int i = 0; i < snake[64]; i++)
    {
        led_strip_set_pixel(led_strip, snake[i], snakeR, snakeG, snakeB);
    }
    lastIndex = snake[snake[64] - 1];
    led_strip_refresh(led_strip);
}

static void updateLedOff(void)
{
    printf("Clear led position %d\n", lastIndex);
    led_strip_set_pixel(led_strip, lastIndex, 0, 0, 0);
    led_strip_refresh(led_strip);
}

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
    snake[64] = 5;
    snake[0] = 36;
    snake[1] = 35;
    snake[2] = 34;
    snake[3] = 33;
    snake[4] = 32;
    lastIndex = snake[64];
    /* Configure the peripheral according to the LED type */
    configure_led();

    led_strip_refresh(led_strip);

    for (int j = 0; j < sizeof(inputString) - 1; j++)
    {
        headMovementUpdate(inputString[j], horizontal, vertical);
        updateLed();
        if (updated)
            updateLedOff();
        vTaskDelay(30);
        updated = false;
    }

    // clear all leds
    led_strip_clear(led_strip);
}
