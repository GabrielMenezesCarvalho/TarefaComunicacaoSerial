#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "TarefaComunicacaoSerial.pio.h"

// Definições de pinos e constantes para I2C e LEDs
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
ssd1306_t ssd;  // Estrutura para controle do display SSD1306

// Matrizes de LED para representar números de 0 a 9 na matriz WS2812
bool matrixLED[10][NUM_PIXELS] = {
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 0
    {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0}, // 1
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 2
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 3
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0}, // 4
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // 5
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1}, // 6
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1}, // 7
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}  // 9
};

// Função de debounce para evitar leituras errôneas dos botões devido a ruídos
bool debounce(uint gpio) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 300000) { // 300ms debounce
        last_time = current_time;
        return true;
    }
    return false;
}

// Função de interrupção para os botões VERDE e AZUL
void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!debounce(gpio)) return;

    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false); // Desenha uma borda no display

    if (gpio == BOTAO_VERDE) {
        gpio_put(LED_VERDE_PIN, !gpio_get(LED_VERDE_PIN)); // Alterna o estado do LED verde
        ssd1306_draw_string(&ssd, "LED RGB VERDE", 11, 17);
        ssd1306_draw_string(&ssd, gpio_get(LED_VERDE_PIN) ? "LIGADO" : "DESLIGADO", 40, 37);
        printf("LED RGB verde %s\n", gpio_get(LED_VERDE_PIN) ? "ligado" : "desligado");
    } else if (gpio == BOTAO_AZUL) {
        gpio_put(LED_AZUL_PIN, !gpio_get(LED_AZUL_PIN)); // Alterna o estado do LED azul
        ssd1306_draw_string(&ssd, "LED RGB AZUL", 12, 17);
        ssd1306_draw_string(&ssd, gpio_get(LED_AZUL_PIN) ? "LIGADO" : "DESLIGADO", 40, 37);
        printf("LED RGB azul %s\n", gpio_get(LED_AZUL_PIN) ? "ligado" : "desligado");
    }

    ssd1306_send_data(&ssd); // Atualiza o display com as novas informações
}

// Inicialização do display OLED SSD1306 via I2C
void init_display() {
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C na frequência de 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, 128, 64, false, ENDERECO_DISPLAY, I2C_PORT); // Inicializa o display
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd);   // Atualiza o display
}

// Inicialização da matriz de LEDs WS2812
void init_led_matrix() {
    PIO pio = pio0; // Usa o primeiro bloco de PIO
    int sm = 0; // Usa a primeira state machine
    uint offset = pio_add_program(pio, &TarefaComunicacaoSerial_program); // Adiciona o programa PIO
    TarefaComunicacaoSerial_program_init(pio, sm, offset, PINO_MATRIZ, 800000, IS_RGBW); // Inicializa a matriz de LEDs
}

// Função para enviar um pixel RGB para a matriz de LEDs
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para converter valores RGB em um valor de 32 bits para o LED
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Define a cor dos LEDs para representar um número
void set_number_led(uint8_t r, uint8_t g, uint8_t b, int num) {
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(matrixLED[num][i] ? color : 0); // Acende o LED correspondente ao número
    }
}

int main() {
    stdio_init_all(); // Inicializa o USB para comunicação serial

    init_display();  // Inicializa o display OLED
    init_led_matrix(); // Inicializa a matriz de LEDs WS2812

    // Configura LEDs RGB como saída
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

    // Loop principal para ler caracteres via USB e exibir no display e matriz de LEDs
    while (true) {
        if (stdio_usb_connected()) {
            char c;
            if (scanf(" %c", &c) == 1) {
                ssd1306_fill(&ssd, false); // Limpa o display antes de exibir novo caractere
                ssd1306_rect(&ssd, 3, 3, 122, 58, true, false); // Desenha uma borda no display

                if (c == ';') {
                    // Exibe o alfabeto quando o caractere ';' é recebido
                    ssd1306_draw_string(&ssd, "a b c d e f g", 8, 10);
                    ssd1306_draw_string(&ssd, "h i j k l m n", 8, 22);
                    ssd1306_draw_string(&ssd, "o p q u v x y", 8, 34);
                    ssd1306_draw_string(&ssd, "z", 60, 48);
                } else if (c >= '0' && c <= '9') {
                    // Exibe números tanto no display quanto na matriz de LEDs
                    ssd1306_draw_char(&ssd, c, 60, 30);
                    set_number_led(led_r, led_g, led_b, c - '0');
                } else {
                    // Exibe outros caracteres recebidos via USB
                    ssd1306_draw_char(&ssd, c, 60, 30);
                }

                ssd1306_send_data(&ssd); // Atualiza o display com o conteúdo
            }
        }
    }
    return 0;
}
