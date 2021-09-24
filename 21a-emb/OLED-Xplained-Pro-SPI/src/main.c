#include <asf.h>

#include "oled/gfx_mono_ug_2832hsweg04.h"
#include "oled/gfx_mono_text.h"
#include "oled/sysfont.h"

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOC
#define BUT_2_PIO_ID ID_PIOC
#define BUT_2_IDX 31
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOA
#define BUT_3_PIO_ID ID_PIOA
#define BUT_3_IDX 19
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)

volatile char but_1 = 0;
volatile char but_2 = 0;
volatile char but_3 = 0;

volatile char f_rtt_alarm = false;

volatile Bool enable = true;

volatile Bool erase = false;

volatile int senha[6] = {BUT_1_PIO, BUT_1_PIO, BUT_2_PIO, BUT_2_PIO, BUT_3_PIO, BUT_1_PIO};

void but1_callback(void) {
	but_1 = 1;
}

void but2_callback(void) {
	but_2 = 1;
}

void but3_callback(void) {
	but_3 = 1;
}

void io_init(void) {
  pmc_enable_periph_clk(LED_1_PIO_ID);
  pmc_enable_periph_clk(LED_2_PIO_ID);
  pmc_enable_periph_clk(LED_3_PIO_ID);
  pmc_enable_periph_clk(BUT_1_PIO_ID);
  pmc_enable_periph_clk(BUT_2_PIO_ID);
  pmc_enable_periph_clk(BUT_3_PIO_ID);

  pio_configure(LED_1_PIO, PIO_OUTPUT_0, LED_1_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_2_PIO, PIO_OUTPUT_0, LED_2_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_3_PIO, PIO_OUTPUT_0, LED_3_IDX_MASK, PIO_DEFAULT);

  pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

  pio_handler_set(BUT_1_PIO, BUT_1_PIO_ID, BUT_1_IDX_MASK, PIO_IT_FALL_EDGE,
  but1_callback);
  pio_handler_set(BUT_2_PIO, BUT_2_PIO_ID, BUT_2_IDX_MASK, PIO_IT_FALL_EDGE,
  but2_callback);
  pio_handler_set(BUT_3_PIO, BUT_3_PIO_ID, BUT_3_IDX_MASK, PIO_IT_FALL_EDGE,
  but3_callback);

  pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
  pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
  pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);

  pio_get_interrupt_status(BUT_1_PIO);
  pio_get_interrupt_status(BUT_2_PIO);
  pio_get_interrupt_status(BUT_3_PIO);

  NVIC_EnableIRQ(BUT_1_PIO_ID);
  NVIC_SetPriority(BUT_1_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_2_PIO_ID);
  NVIC_SetPriority(BUT_2_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_3_PIO_ID);
  NVIC_SetPriority(BUT_3_PIO_ID, 4);
}


void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		//f_rtt_alarme = false;
		//pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		
	}

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		erase = true;
		
	}
}


static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
}


int main(void) {
  board_init();
  sysclk_init();
  delay_init();
  io_init();
  gfx_mono_ssd1306_init();
  
  int i = 0;
  int password[6];
  
  but_1 = 0;
  but_2 = 0;
  but_3 = 0;
  
  int aberto = 0;
  
  gfx_mono_draw_string("Cofre fechado", 10,0, &sysfont);
  
  //f_rtt_alarm = true;
  
  while (1) {
	  
	  if(erase){
		  pio_set_output(LED_1_PIO, LED_1_IDX_MASK, 1, 0, 0);
		  pio_set_output(LED_2_PIO, LED_2_IDX_MASK, 1, 0, 0);
		  pio_set_output(LED_3_PIO, LED_3_IDX_MASK, 1, 0, 0);
		  enable = true;
		  f_rtt_alarm = true;                  // flag RTT alarme
		  but_1 = 0;
		  but_2 = 0;
		  but_3 = 0;
		  gfx_mono_draw_string("            ", 10,0, &sysfont);
		  erase = false;
	  }
	  
	  if(enable){
		  
		  if(aberto && but_1){
			  gfx_mono_draw_string("                ", 10,0, &sysfont);
			  gfx_mono_draw_string("Cofre fechado", 10,0, &sysfont);
			  but_1 = 0;
			  aberto = 0;
		  }
		  
		  if(but_1){
			  password[i] = BUT_1_PIO;
			  i++;
			  //gfx_mono_draw_string("*", 20,16, &sysfont);
			  but_1 = 0;
		  }
		  if(i == 1){
			  gfx_mono_draw_string("*", 20,18, &sysfont);
		  }
		  
		  if(but_2){
			  password[i] = BUT_2_PIO;
			  i++;
			  //gfx_mono_draw_string("*"*i, 20,16, &sysfont);
			  but_2 = 0;
		  }
		  
		  if(i == 2){
			  gfx_mono_draw_string("**", 20,18, &sysfont);
		  }
		  
		  
		  if(but_3){
			  password[i] = BUT_3_PIO;
			  i++;
			  //gfx_mono_draw_string("*", 20,16, &sysfont);
			  but_3 = 0;
		  }
		  
		  if(i == 3){
			  gfx_mono_draw_string("***", 20,18, &sysfont);
		  }
		  if(i == 4){
			  gfx_mono_draw_string("****", 20,18, &sysfont);
		  }
		  if(i == 5){
			  gfx_mono_draw_string("*****", 20,18, &sysfont);
		  }
		  
		  if(i == 6){
			  gfx_mono_draw_string("******", 20,18, &sysfont);
			  if(password[0] == senha[0] && password[1] == senha[1] && password[2] == senha[2] && password[3] == senha[3] && password[4] == senha[4] && password[5] == senha[5]){
				  gfx_mono_draw_string("                ", 10,0, &sysfont);
				  gfx_mono_draw_string("Cofre aberto", 10,0, &sysfont);
				  pio_set_output(LED_1_PIO, LED_1_IDX_MASK, 1, 0, 0);
				  pio_set_output(LED_2_PIO, LED_2_IDX_MASK, 1, 0, 0);
				  pio_set_output(LED_3_PIO, LED_3_IDX_MASK, 1, 0, 0);
				  delay_ms(500);
				  gfx_mono_draw_string("          ", 20,18, &sysfont);
				  aberto = 1;
			  }
			  else{
				  gfx_mono_draw_string("Senha errada", 10,0, &sysfont);
				  gfx_mono_draw_string("          ", 20,18, &sysfont);
				  pio_set_output(LED_1_PIO, LED_1_IDX_MASK, 0, 0, 0);
				  pio_set_output(LED_2_PIO, LED_2_IDX_MASK, 0, 0, 0);
				  pio_set_output(LED_3_PIO, LED_3_IDX_MASK, 0, 0, 0);
				  delay_ms(100);
				  f_rtt_alarm = true;
				  enable = false;
				  i = 0;
				  int password[4];
				  
			  }
			  int password[4];
			  i = 0;
			  but_1 = 0;
			  but_2 = 0;
			  but_3 = 0;
			  
		  }
	  }
	  
	  if(f_rtt_alarm){
		  
		  
		  
		  uint16_t pllPreScale = (int) (((float) 32768) / 4.0);
		  uint32_t irqRTTvalue = 16;
		  
		  // reinicia RTT para gerar um novo IRQ
		  RTT_init(pllPreScale, irqRTTvalue);
		  
		  f_rtt_alarm = false;
	  }	  
	  
	  
  }
}
