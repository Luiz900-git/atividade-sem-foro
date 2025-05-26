#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/bootrom.h"
#include "stdio.h"

#include "pico/time.h"  // Necessário para time_us_64()

// Tempo mínimo entre cliques (em microssegundos)
#define DEBOUNCE_TIME_US 200000  // 200 ms


uint64_t ultimoEventoA = 0;
uint64_t ultimoEventoB = 0;

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define LEDR 13
#define LEDG 11
#define LEDB 12

#define SW 22

#define BOTAO_A 5
#define BOTAO_B 6

#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Configuração do pino do buzzer
#define BUZZER_PIN 21

// Configuração da frequência do buzzer (em Hz)
#define BUZZER_FREQUENCY 4000


SemaphoreHandle_t xButtonSem;
ssd1306_t ssd;
SemaphoreHandle_t xContadorSem;
SemaphoreHandle_t xDisplayMutex;  // Mutex para proteger o display
uint16_t eventosProcessados = 0;

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}


// ISR do botão
void gpio_callback_two(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  //Nenhum contexto de tarefa foi despertado
    xSemaphoreGiveFromISR(xButtonSem, &xHigherPriorityTaskWoken);    //Libera o semáforo
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Troca o contexto da tarefa
}

void vButtonTask(void *paramns){

    // Inicializa o pino do botão do joystick
  gpio_init(SW);             // Inicializa o pino do botão
  gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
  gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações

   gpio_init(LEDR);
   gpio_init(LEDG);
   gpio_init(LEDB);

    gpio_set_dir(LEDR, GPIO_OUT);
    gpio_set_dir(LEDG, GPIO_OUT);
    gpio_set_dir(LEDB, GPIO_OUT);

    char buffer[32];


  while (true){
    if (xSemaphoreTake(xButtonSem, portMAX_DELAY) == pdTRUE){

            gpio_put(LEDB, true);
            gpio_put(LEDG, false);
            gpio_put(LEDR, false);

            beep(BUZZER_PIN, 200); // Bipe de 500ms
           // beep(BUZZER_PIN, 200); // Bipe de 500ms

            eventosProcessados = 0;

            /*ssd1306_fill(&ssd, 0);
            sprintf(buffer, "Quant: %d", eventosProcessados);
            ssd1306_draw_string(&ssd, "Elementos ", 5, 10);
            ssd1306_draw_string(&ssd, "Armazenados:", 5, 19);
            ssd1306_draw_string(&ssd, buffer, 5, 44);
            ssd1306_send_data(&ssd);*/

             
            
            vTaskDelay(pdMS_TO_TICKS(200));  


    }



  }



}

/*// ISR para BOOTSEL e botão de evento
void gpio_irq_handler_two(uint gpio, uint32_t events) {
    if (gpio == SW) {
         gpio_callback_two(gpio, events);
    } 
}*/

/*
// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}
*/

/*ssd1306_t ssd;
SemaphoreHandle_t xContadorSem;
SemaphoreHandle_t xDisplayMutex;  // Mutex para proteger o display
uint16_t eventosProcessados = 0;
*/
void gpio_callback(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xContadorSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vTaskLed(void *params){
    gpio_init(LEDR);
    gpio_init(LEDG);
    gpio_init(LEDB);

    gpio_set_dir(LEDR, GPIO_OUT);
    gpio_set_dir(LEDG, GPIO_OUT);
    gpio_set_dir(LEDB, GPIO_OUT);

    // Inicializar o PWM no pino do buzzer
    pwm_init_buzzer(BUZZER_PIN);

    while(true){
        if (xSemaphoreTake(xContadorSem, portMAX_DELAY) == pdTRUE)
        {
            if((eventosProcessados) == 0 ){
                gpio_put(LEDB, true);
                gpio_put(LEDG, false);
                gpio_put(LEDR, false);
                //eventosProcessados = 0;
            } else if((eventosProcessados) >= 1 && (eventosProcessados) <= 8){
                gpio_put(LEDB, false);
                gpio_put(LEDG, true);
                gpio_put(LEDR, false);
            } else if ((eventosProcessados) == 9){
                gpio_put(LEDB, false);
                gpio_put(LEDG, true);
                gpio_put(LEDR, true);
            } else if ((eventosProcessados) > 10 && (eventosProcessados) <= 50 ){
                gpio_put(LEDB, false);
                gpio_put(LEDG, false);
                gpio_put(LEDR, true);
                //beep(BUZZER_PIN, 200); // Bipe de 500ms
                eventosProcessados = 10;
                //beep(BUZZER_PIN, 200); // Bipe de 500ms
            } else if ((eventosProcessados) == 10){
                gpio_put(LEDB, false);
                gpio_put(LEDG, false);
                gpio_put(LEDR, true);
                beep(BUZZER_PIN, 200); // Bipe de 500ms

            }
            
            else {
                gpio_put(LEDB, true);
                gpio_put(LEDG, false);
                gpio_put(LEDR, false);
                eventosProcessados = 0;


            }
        }
                vTaskDelay(pdMS_TO_TICKS(400));

    }
}


// Tarefa 1: escreve uma mensagem no display
void vContadorTask(void *params) {

        char buffer[32];

    while (true) {
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY)==pdTRUE) {
            ssd1306_fill(&ssd, 0);
            sprintf(buffer, "Quant: %d", eventosProcessados);
            ssd1306_draw_string(&ssd, "Elementos ", 5, 10);
            ssd1306_draw_string(&ssd, "Armazenados:", 5, 19);
            ssd1306_draw_string(&ssd, buffer, 5, 44);
            ssd1306_send_data(&ssd);
            xSemaphoreGive(xDisplayMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}








/*
void vContadorTask(void *params)
{
    char buffer[32];

    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
        ssd1306_fill(&ssd, 0);
        ssd1306_draw_string(&ssd, "Aguardando ", 5, 25);
        ssd1306_draw_string(&ssd, "  evento...", 5, 34);
        ssd1306_send_data(&ssd);
        xSemaphoreGive(xDisplayMutex);
    }

    while (true)
    {
        if (xSemaphoreTake(xContadorSem, portMAX_DELAY) == pdTRUE)
        {
            //eventosProcessados++;

           /*if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                ssd1306_fill(&ssd, 0);
                sprintf(buffer, "Quant: %d", eventosProcessados);
                ssd1306_draw_string(&ssd, "Elemento ", 5, 10);
                ssd1306_draw_string(&ssd, "adicionado!", 5, 19);
                ssd1306_draw_string(&ssd, buffer, 5, 44);
                ssd1306_send_data(&ssd);
                xSemaphoreGive(xDisplayMutex);
            }

            //vTaskDelay(pdMS_TO_TICKS(1500));

            if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
                ssd1306_fill(&ssd, 0);
                ssd1306_draw_string(&ssd, "Aguardando ", 5, 25);
                ssd1306_draw_string(&ssd, "  evento...", 5, 34);
                ssd1306_send_data(&ssd);
                xSemaphoreGive(xDisplayMutex);
            }
        }
              //  vTaskDelay(pdMS_TO_TICKS(100));

    }
}*/

// ISR para BOOTSEL e botão de evento
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_B)
    {   
        eventosProcessados--;
        gpio_callback(gpio, events);

        //reset_usb_boot(0, 0);
    }
    else if (gpio == BOTAO_A)
    {   
        eventosProcessados++;
        gpio_callback(gpio, events);
    }
    else if (gpio == SW)
    {
        eventosProcessados = 0;
        gpio_callback_two(gpio, events);
    }
}

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    xContadorSem = xSemaphoreCreateCounting(10, 0);
    xDisplayMutex = xSemaphoreCreateMutex();  // Cria o mutex
     xButtonSem = xSemaphoreCreateBinary();

    xTaskCreate(vContadorTask, "ContadorTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskLed, "ControlaLeds", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
     xTaskCreate(vButtonTask, "BotaoTask", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
