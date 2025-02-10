#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "inc/config_matriz.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/irq.h"

uint static volatile ultimoTime = 0;
volatile bool estado_led_verd = false;
volatile bool estado_led_azul = false;
ssd1306_t ssd;

char estado_azul[10] = "AZUL: ";
char estado_verd[11] = "VERDE: ";

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define OUT_PIN 7

#define led_verd 11
#define led_azul 12

#define bot_A 5
#define bot_B 6

char str[10] = "ENTRADA: ";

void setup();
void display();

void setup(){
  stdio_init_all();   

  gpio_init(led_verd);
  gpio_set_dir(led_verd, GPIO_OUT);

  gpio_init(led_azul);
  gpio_set_dir(led_azul, GPIO_OUT);

  gpio_init(bot_A);
  gpio_set_dir(bot_A, GPIO_IN);
  gpio_pull_up(bot_A);

  gpio_init(bot_B);
  gpio_set_dir(bot_B, GPIO_IN);
  gpio_pull_up(bot_B);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);

  // Envia os dados para o display
  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);
}

//rotina da interrupção
  void callback_button(uint botao, uint32_t events)
{
  uint timeAtual = to_us_since_boot(get_absolute_time()); // Tempo em milissegundos 
  //250 ms para evitar ação dupla do botão
  if(timeAtual - ultimoTime > 250000){
    ultimoTime = timeAtual;

   bool estadoBotao = gpio_get(botao);

    if(!estadoBotao){
      if(botao == bot_A){
        estado_led_verd = !estado_led_verd;   // Inverte o estado
        gpio_put(led_verd, estado_led_verd); 
        strncpy(&estado_verd[7], estado_led_verd ? "on " : "off", 4);
        printf("O botao A foi precionado!! O led verde foi alterado.\n");
        display();
      }
      else if(botao == bot_B){
        estado_led_azul = !estado_led_azul;   // Inverte o estado
        gpio_put(led_azul, estado_led_azul); 
        strncpy(&estado_azul[6], estado_led_azul ? "on " : "off", 4);
        printf("O botao B foi precionado!! O led azul foi alterado.\n");
        display();
      }
    }
  }
}

void display(){
      bool cor = true;
      cor = !cor;
 
      ssd1306_draw_string(&ssd, str, 8, 10); // Desenha uma string 
      ssd1306_draw_string(&ssd, estado_azul, 8, 30); // Desenha uma string 
      ssd1306_draw_string(&ssd, estado_verd, 8, 45); // Desenha uma string    
      ssd1306_send_data(&ssd); // Atualiza o display
}

int main()
{ // Inicializa comunicação USB CDC para monitor serial
  setup();
  PIO pio = pio_config();

  gpio_set_irq_enabled_with_callback(bot_A, GPIO_IRQ_EDGE_FALL, true, &callback_button);
  gpio_set_irq_enabled_with_callback(bot_B, GPIO_IRQ_EDGE_FALL, true, &callback_button);
    
    while (true)
    { 
      char c;
      scanf("%c", &c);

      if(c >= '0' && c <= '9'){
        define_numero(c, pio, 0);
      }
      str[9] = c;
      display();
      sleep_ms(1000); 
    }
  }
   
