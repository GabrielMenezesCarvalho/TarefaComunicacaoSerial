#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "TarefaComunicacaoSerial.pio.h"

// Definições de pinos e constantes
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_DISPLAY 0x3C

#define BOTAO_VERDE 5
#define BOTAO_AZUL 6
#define LED_VERDE_PIN 11
#define LED_AZUL_PIN 12

#define IS_RGBW false
#define NUM_PIXELS 25
#define PINO_MATRIZ 7

// Variáveis globais para controle de LEDs e display
uint8_t led_r = 1, led_g = 1, led_b = 1; // Intensidades RGB
static volatile uint32_t last_time = 0;  // Controle de debounce
bool cor = true;
ssd1306_t ssd;

// Matrizes de LED para números 0-9
bool matrixLED[10][NUM_PIXELS] = {
    {// 0
     1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {// 1
     0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0},
    {// 2
     1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {// 3
     1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {// 4
     0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0},
    {// 5
     1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {// 6
     1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {// 7
     0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},
    {// 8
     1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {// 9
     1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}
};

// Função de debounce para os botões
bool debounce(uint gpio)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 300000)
    { // 300ms debounce
        last_time = current_time;
        return true;
    }
    return false;
}

// Função de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (!debounce(gpio))
        return;

    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);

    if (gpio == BOTAO_VERDE)
    {
        gpio_put(LED_VERDE_PIN, !gpio_get(LED_VERDE_PIN));
        ssd1306_draw_string(&ssd, "LED RGB VERDE", 11, 17);
        ssd1306_draw_string(&ssd, gpio_get(LED_VERDE_PIN) ? "LIGADO" : "DESLIGADO", 40, 37);
        printf("LED RGB verde %s\n", gpio_get(LED_VERDE_PIN) ? "ligado" : "desligado");
    }
    else if (gpio == BOTAO_AZUL)
    {
        gpio_put(LED_AZUL_PIN, !gpio_get(LED_AZUL_PIN));
        ssd1306_draw_string(&ssd, "LED RGB AZUL", 12, 17);
        ssd1306_draw_string(&ssd, gpio_get(LED_AZUL_PIN) ? "LIGADO" : "DESLIGADO", 40, 37);
        printf("LED RGB azul %s\n", gpio_get(LED_AZUL_PIN) ? "ligado" : "desligado");
    }

    ssd1306_send_data(&ssd);
}

// Inicialização do display SSD1306
void init_display()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, 128, 64, false, ENDERECO_DISPLAY, I2C_PORT);
    ssd1306_fill(&ssd, false); // Limpa o display preenchendo com pixels apagados
    ssd1306_send_data(&ssd);   // Atualiza o display para refletir a limpeza
}

// Inicialização da matriz de LEDs WS2812
void init_led_matrix()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &TarefaComunicacaoSerial_program);
    TarefaComunicacaoSerial_program_init(pio, sm, offset, PINO_MATRIZ, 800000, IS_RGBW);
}

// Funções auxiliares para controle da matriz de LEDs
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_number_led(uint8_t r, uint8_t g, uint8_t b, int num)
{
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        put_pixel(matrixLED[num][i] ? color : 0);
    }
}

int main()
{
    stdio_init_all();

    // Inicializa periféricos
    init_display();
    init_led_matrix();

    // Configura LEDs RGB
    gpio_init(LED_VERDE_PIN);
    gpio_set_dir(LED_VERDE_PIN, GPIO_OUT);
    gpio_init(LED_AZUL_PIN);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);

    // Configura botões com interrupção
    gpio_init(BOTAO_VERDE);
    gpio_set_dir(BOTAO_VERDE, GPIO_IN);
    gpio_pull_up(BOTAO_VERDE);
    gpio_set_irq_enabled_with_callback(BOTAO_VERDE, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BOTAO_AZUL);
    gpio_set_dir(BOTAO_AZUL, GPIO_IN);
    gpio_pull_up(BOTAO_AZUL);
    gpio_set_irq_enabled_with_callback(BOTAO_AZUL, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        if (stdio_usb_connected())
        {
            char c;
            if (scanf(" %c", &c) == 1)
            {
                ssd1306_fill(&ssd, false);
                ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);

                if (c == ';')
                {
                    ssd1306_draw_string(&ssd, "a b c d e f g", 8, 10);
                    ssd1306_draw_string(&ssd, "h i j k l m n", 8, 22);
                    ssd1306_draw_string(&ssd, "o p q u v x y", 8, 34);
                    ssd1306_draw_string(&ssd, "z", 60, 48);
                }
                else if (c >= '0' && c <= '9')
                {
                    ssd1306_draw_char(&ssd, c, 60, 30);           // Exibe o número no centro
                    set_number_led(led_r, led_g, led_b, c - '0'); // Exibe o número na matriz de LEDs
                }
                else
                {
                    ssd1306_draw_char(&ssd, c, 60, 30);
                }

                ssd1306_send_data(&ssd);
            }
        }
    }
    return 0;
}
