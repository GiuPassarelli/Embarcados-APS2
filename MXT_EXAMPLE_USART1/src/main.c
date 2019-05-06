 /** 
 * Example of usage of the maXTouch component with USART
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 */

/**
* fast.png: fast by Galaktionoff from the Noun Project
* daily.png: time hours by achmad mulyana from the Noun Project
* weight.png: Weight by Justin Blake from the Noun Project
* rinse.png: rinse by parsiall from the Noun Project
* centrifuge.png: centrifuge by Anton from the Noun Project
* custom.png: custom by BomSymbols from the Noun Project
* right_arrow.png: Right by Rose Alice Design from the Noun Project
* left_arrow.png: Left by Rose Alice Design from the Noun Project
* run.png: icon made by Freepik from Flaticon.com
*/

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ioport.h"
#include "conf_board.h"
#include "conf_example.h"
#include "conf_uart_serial.h"
#include "maquina1.h"

#define MAX_ENTRIES        3
#define STRING_LENGTH     70

#define HOUR        0
#define MINUTE      0
#define SECOND      0

#define USART_TX_MAX_LENGTH     0xff

// Botão que representará porta
// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

volatile int page_number = 0;
volatile int customize_open = 0;
volatile int lock_screen = 0;
volatile int security_block = 0;
volatile int value_selected = 0;
volatile int selection_addition = 0;
volatile int running = 0;
volatile int freeze_mode = 0;

void RTC_init(void);

struct ili9488_opt_t g_ili9488_display_opt;

typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
	uint8_t dataSize;
} tImage;

#include "icons/fast.h"
#include "icons/daily.h"
#include "icons/heavy.h"
#include "icons/rinse.h"
#include "icons/centrifuge.h"
#include "icons/custom.h"
#include "icons/right_arrow.h"
#include "icons/left_arrow.h"
#include "icons/run.h"
#include "icons/arrow-down.h"
#include "icons/arrow-up.h"
#include "icons/back-arrow.h"
#include "icons/circle-outline.h"
#include "icons/stop.h"
#include "icons/end.h"
#include "icons/snowflake.h"

/**
 * Inicializa ordem do menu
 * retorna o primeiro ciclo que
 * deve ser exibido.
 */
t_ciclo *initMenuOrder(){
  c_rapido.previous = &c_centrifuga;
  c_rapido.next = &c_custom;

  c_diario.previous = &c_custom;
  c_diario.next = &c_pesado;

  c_pesado.previous = &c_diario;
  c_pesado.next = &c_enxague;

  c_enxague.previous = &c_pesado;
  c_enxague.next = &c_centrifuga;

  c_centrifuga.previous = &c_enxague;
  c_centrifuga.next = &c_rapido;
  
  c_custom.previous = &c_rapido;
  c_custom.next = &c_diario;

  return(&c_diario);
}


static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);	
	
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
	//ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
}

/**
 * \brief Set maXTouch configuration
 *
 * This function writes a set of predefined, optimal maXTouch configuration data
 * to the maXTouch Xplained Pro.
 *
 * \param device Pointer to mxt_device struct
 */
static void mxt_init(struct mxt_device *device)
{
	enum status_code status;

	/* T8 configuration object data */
	uint8_t t8_object[] = {
		0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
		0x00, 0x32, 0x19
	};

	/* T9 configuration object data */
	uint8_t t9_object[] = {
		0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
		0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
		0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
		0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
		0x02
	};

	/* T46 configuration object data */
	uint8_t t46_object[] = {
		0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
		0x00, 0x00
	};
	
	/* T56 configuration object data */
	uint8_t t56_object[] = {
		0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};

	/* TWI configuration */
	twihs_master_options_t twi_opt = {
		.speed = MXT_TWI_SPEED,
		.chip  = MAXTOUCH_TWI_ADDRESS,
	};

	status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
	Assert(status == STATUS_OK);

	/* Initialize the maXTouch device */
	status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
			MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
	Assert(status == STATUS_OK);

	/* Issue soft reset of maXTouch device by writing a non-zero value to
	 * the reset register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

	/* Wait for the reset of the device to complete */
	delay_ms(MXT_RESET_TIME);

	/* Write data to configuration registers in T7 configuration object */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

	/* Write predefined configuration data to configuration objects */
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
			MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

	/* Issue recalibration command to maXTouch device by writing a non-zero
	 * value to the calibrate register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
			MXT_GEN_COMMANDPROCESSOR_T6, 0)
			+ MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}


void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_ALREN);

}

static void draw_struct(t_ciclo *selected_mode, int added_value_x, int added_value_y){
	uint8_t cicle_name[256];
	uint8_t enx_tempo[256];
	uint8_t enx_qnt[256];
	uint8_t rpm[256];
	uint8_t cent_tempo[256];
	uint8_t heavy_on[256];
	uint8_t bubbles[256];
	
	sprintf(cicle_name, selected_mode->nome);
	sprintf(enx_tempo, "Tempo de enxague: %d", selected_mode->enxagueTempo);
	sprintf(enx_qnt, "Quantidade: %d", selected_mode->enxagueQnt);
	sprintf(rpm, "RPM: %d", selected_mode->centrifugacaoRPM);
	sprintf(cent_tempo, "Tempo de centrifug: %d", selected_mode->centrifugacaoTempo);
	
	if(selected_mode->heavy == 0){
		sprintf(heavy_on, "Modo pesado: off");
	}
	if(selected_mode->heavy == 1){
		sprintf(heavy_on, "Modo pesado: on");
	}
	if(selected_mode->bubblesOn == 0){
		sprintf(bubbles, "Modo bolhas: off");
	}
	if(selected_mode->bubblesOn == 1){
		sprintf(bubbles, "Modo bolhas: on");
	}
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	ili9488_draw_string(100 + added_value_x, 10 + added_value_y, cicle_name);
	ili9488_draw_string(2, 40 + added_value_y, enx_tempo);
	ili9488_draw_string(2, 70 + added_value_y, enx_qnt);
	ili9488_draw_string(2, 100 + added_value_y, rpm);
	ili9488_draw_string(2, 130 + added_value_y, cent_tempo);
	ili9488_draw_string(2, 160 + added_value_y, heavy_on);
	ili9488_draw_string(2, 190 + added_value_y, bubbles);
}

static void select_screen(){
	t_ciclo *p_primeiro = initMenuOrder();
	t_ciclo *selected_mode;
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
	switch (page_number){
		case 0:
			ili9488_draw_pixmap(115, 60, daily.width, daily.height, daily.data);
			selected_mode = p_primeiro;
			break;
		case 1:
			ili9488_draw_pixmap(115, 60, heavy.width, heavy.height, heavy.data);
			selected_mode = p_primeiro->next;
			break;
		case 2:
			ili9488_draw_pixmap(115, 60, rinse.width, rinse.height, rinse.data);
			selected_mode = p_primeiro->next->next;
			break;
		case 3:
			ili9488_draw_pixmap(115, 60, centrifuge.width, centrifuge.height, centrifuge.data);
			selected_mode = p_primeiro->next->next->next;
			break;
		case 4:
			ili9488_draw_pixmap(115, 60, fast.width, fast.height, fast.data);
			selected_mode = p_primeiro->previous->previous;
			break;
		case 5:
			ili9488_draw_pixmap(115, 60, custom.width, custom.height, custom.data);
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
			ili9488_draw_string(100, 170, "Customizado");
			break;
		default:
			break;

	ili9488_draw_pixmap(250, 60, right_arrow.width, right_arrow.height, right_arrow.data);
	ili9488_draw_pixmap(5, 60, left_arrow.width, left_arrow.height, left_arrow.data);
	ili9488_draw_pixmap(130, 380, run.width, run.height, run.data);
	
	if(page_number != 5){
		draw_struct(selected_mode, 20, 160);
	}
}

static void open_customization(){
	customize_open = 1;
	t_ciclo *p_primeiro = initMenuOrder();
	t_ciclo *selected_mode = p_primeiro->previous;
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);

	draw_struct(selected_mode, 0, 20);
	
	ili9488_draw_pixmap(90, 240, arrowup.width, arrowup.height, arrowup.data);
	ili9488_draw_pixmap(90, 300, arrowdown.width, arrowdown.height, arrowdown.data);
	ili9488_draw_pixmap(170, 270, circleoutline.width, circleoutline.height, circleoutline.data);
	ili9488_draw_pixmap(280, 50, backarrow.width, backarrow.height, backarrow.data);
	ili9488_draw_pixmap(130, 380, run.width, run.height, run.data);
}

static void run_module(){
	if(security_block == 1){
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
		ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
		ili9488_draw_pixmap(110, 140, stop.width, stop.height, stop.data);
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
		ili9488_draw_string(75, 260, "Porta aberta!");
		delay_s(5);
		if(customize_open == 1){
			open_customization();
		}
		else{
			select_screen();
		}
		
	}
	else if(security_block == 0){
		
		uint32_t minute;
		uint32_t second;
		uint32_t hour;
		
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
		ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
		ili9488_draw_string(50, 240, "Lavagem em progresso");
		ili9488_draw_pixmap(230, 60, end.width, end.height, end.data);
		ili9488_draw_pixmap(5, 60, snowflake.width, snowflake.height, snowflake.data);
		
		hour = 0;
		minute = 0;
		second = 0;
		rtc_set_time(RTC, hour, minute, second);
		
		running = 1;
	}
	
}

static void change_value(int signal){
	t_ciclo *p_primeiro = initMenuOrder();
	t_ciclo *selected_mode = p_primeiro->previous;
	uint8_t new_value[256];
	int x_value;
	int y_value;
	int case_value = 0;
	
	switch (selection_addition){
		case 0:
			selected_mode->enxagueTempo += (signal * 1);
			if(selected_mode->enxagueTempo < 0){
				selected_mode->enxagueTempo = 0;
			}
			case_value = selected_mode->enxagueTempo;
			x_value = 215;
			y_value = 60;
			break;
		case 30:
			selected_mode->enxagueQnt += (signal * 1);
			if(selected_mode->enxagueQnt < 0){
				selected_mode->enxagueQnt = 0;
			}
			case_value = selected_mode->enxagueQnt;
			x_value = 145;
			y_value = 90;
			break;
		case 60:
			selected_mode->centrifugacaoRPM += (signal * 100);
			if(selected_mode->centrifugacaoRPM < 0){
				selected_mode->centrifugacaoRPM = 0;
			}
			case_value = selected_mode->centrifugacaoRPM;
			x_value = 60;
			y_value = 120;
			break;
		case 90:
			selected_mode->centrifugacaoTempo += (signal * 1);
			if(selected_mode->centrifugacaoTempo < 0){
				selected_mode->centrifugacaoTempo = 0;
			}
			case_value = selected_mode->centrifugacaoTempo;
			x_value = 240;
			y_value = 150;
			break;
		case 120:
			selected_mode->heavy = !selected_mode->heavy;
			case_value = selected_mode->heavy;
			x_value = 155;
			y_value = 180;
			break;
		case 150:
			selected_mode->bubblesOn = !selected_mode->bubblesOn;
			case_value = selected_mode->bubblesOn;
			x_value = 155;
			y_value = 210;
			break;
		default:
			break;
	}
	
	if(selection_addition == 120 || selection_addition == 150){
		if(case_value == 0){
			sprintf(new_value, "off");
		}
		if(case_value == 1){
			sprintf(new_value, "on");
		}
	}
	else{
		sprintf(new_value, "%d", case_value);
	}
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
	ili9488_draw_filled_rectangle(x_value, y_value, x_value + 40, y_value + 15);
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	ili9488_draw_string(x_value, y_value, new_value);
}

uint32_t convert_axis_system_x(uint32_t touch_y) {
	// entrada: 4096 - 0 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_WIDTH - ILI9488_LCD_WIDTH*touch_y/4096;
}

uint32_t convert_axis_system_y(uint32_t touch_x) {
	// entrada: 0 - 4096 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_HEIGHT*touch_x/4096;
}

void update_screen(uint32_t tx, uint32_t ty) {
	if(ty >= 60 && ty <= 120 && !customize_open) {
		if(tx >= 5 && tx <= 65) {
			if (!running){
				page_number -= 1;
				if (page_number < 0) {
					page_number += 6;
				}
				select_screen();
			}
			if (running){
				freeze_mode = 1;
			}
			
		} 
		else if(tx >= 250 && tx <= 310 && !freeze_mode) {
			page_number += 1;
			if (page_number >= 6) {
				page_number -= 6;
			}
			running = 0;
			select_screen();
		}
	}
	if(ty >= 380 && ty <= 450 && tx >= 130 && tx <= 200){
		if(page_number == 5  && !customize_open){
			open_customization();
		}
		else{
			run_module();
		}
	}
	if(customize_open){
		if(tx >= 90 && tx <= 150 && !value_selected){
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
			if(ty >= 240 && ty <= 300 && selection_addition > 0){          //UP-ARROW
				ili9488_draw_filled_rectangle(280, 50 + selection_addition, 310, 80 + selection_addition);
				selection_addition -= 30;
				ili9488_draw_pixmap(280, 50 + selection_addition, backarrow.width, backarrow.height, backarrow.data);
			}
			if(ty >= 300 && ty <= 360 && selection_addition < 150){          //DOWN-ARROW
				ili9488_draw_filled_rectangle(280, 50 + selection_addition, 310, 80 + selection_addition);
				selection_addition += 30;
				ili9488_draw_pixmap(280, 50 + selection_addition, backarrow.width, backarrow.height, backarrow.data);
			}
		}
		if(tx >= 90 && tx <= 150 && value_selected){
			if(ty >= 240 && ty <= 300){           //UP-ARROW
				change_value(1);
			}
			if(ty >= 300 && ty <= 360){          //DOWN-ARROW
				change_value(-1);
			}
		}
		if(tx >= 170 && tx <= 230 && ty >= 270 && ty <= 330){     //SELECT
			value_selected = !value_selected;
		}
	}
}


void mxt_handler(struct mxt_device *device)
{
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
	
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}
		
		 // eixos trocados (quando na vertical LCD)
		uint32_t conv_x = convert_axis_system_x(touch_event.y);
		uint32_t conv_y = convert_axis_system_y(touch_event.x);
		
		/* Format a new entry in the data string that will be sent over USART */
		sprintf(buf, "Nr: %1d, X:%4d, Y:%4d, Status:0x%2x conv X:%3d Y:%3d\n\r",
				touch_event.id, touch_event.x, touch_event.y,
				touch_event.status, conv_x, conv_y);
		update_screen(conv_x, conv_y);

		/* Add the new string to the string buffer */
		strcat(tx_buf, buf);
		i++;
		
		break;
		/* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

	/* If there is any entries in the buffer, send them over USART */
	if (i > 0) {
		usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
	}
}


void mxt_debounce(struct mxt_device *device)
{
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
	
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}
		
		 // eixos trocados (quando na vertical LCD)
		uint32_t conv_x = convert_axis_system_x(touch_event.y);
		uint32_t conv_y = convert_axis_system_y(touch_event.x);
		

		/* Add the new string to the string buffer */
		strcat(tx_buf, buf);
		i++;

		/* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

}

void check_door(void){
	security_block = !security_block;
}

void io_init(void)
{
	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE );

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_RISE_EDGE,
	check_door);

	// Ativa interrupção
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}


/**
 * \brief Main application function.
 *
 * Initialize system, UART console, network then start weather client.
 *
 * \return Program return value.
 */
int main(void)
{
	struct mxt_device device; /* Device data container */

	/* Initialize the USART configuration struct */
	const usart_serial_options_t usart_serial_options = {
		.baudrate     = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength   = USART_SERIAL_CHAR_LENGTH,
		.paritytype   = USART_SERIAL_PARITY,
		.stopbits     = USART_SERIAL_STOP_BIT
	};
	
	/* Initialize the board. */
	sysclk_init();
	board_init();
	ioport_init();
	RTC_init();

    /* Inicializa e configura o LCD */
	configure_lcd();
	
	
	/* Initialize the mXT touch device */
	mxt_init(&device);
	
	/* Initialize stdio on USART */
	stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);
	
	io_init();
	
	t_ciclo *selected_mode;
	
	uint32_t minute;
	uint32_t second;
	uint32_t hour;
	
	char bufferTime[32];
	
	int lastSecond = 0;
	
	printf("\n\rmaXTouch data USART transmitter\n\r");
	security_block = 0;
	while (1) {
		/* Check for any pending messages and run message handler if any
		 * message is found in the queue */
		if (mxt_is_message_pending(&device)) {
			mxt_handler(&device);
			delay_ms(800);
			mxt_debounce(&device);
		}
		if(running){
			selected_mode = &c_diario;
			for(int i = 0; i < page_number; i++) {
				selected_mode = selected_mode->next;
			}
			
			rtc_get_time(RTC, &hour, &minute, &second);
			
			int minutesToFinish = (selected_mode->enxagueTempo * selected_mode->enxagueQnt) + selected_mode->centrifugacaoTempo;
			
			if(minute != minutesToFinish){
				sprintf(bufferTime, "%02d:%02d", minute, second);
				
				if(lastSecond != second){
					ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
					ili9488_draw_filled_rectangle(100, 300, 300, 350);
					ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
					
					ili9488_draw_string(100, 300, bufferTime);
					lastSecond = second;
				}
				
				rtc_get_time(RTC, &hour, &minute, &second);
			}
			else{
				ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
				ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
				ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
				ili9488_draw_string(50, 240, "Lavagem concluida!");
				running = 0;
				freeze_mode = 0;
				delay_s(2);
				select_screen();
				lastSecond = 0;
			}
		}
		
	}
	return 0;
}
