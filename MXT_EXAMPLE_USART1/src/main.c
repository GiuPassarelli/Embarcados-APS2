// fast.png: fast by Galaktionoff from the Noun Project
// daily.png: time hours by achmad mulyana from the Noun Project
// weight.png: Weight by Justin Blake from the Noun Project
// rinse.png: rinse by parsiall from the Noun Project
// centrifuge.png: centrifuge by Anton from the Noun Project
// custom.png: custom by BomSymbols from the Noun Project

#include "asf.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ioport.h"

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

static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_LIGHTBLUE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	ili9488_draw_pixmap(45, 55, fast.width, fast.height, fast.data);
	ili9488_draw_pixmap(185, 55, daily.width, daily.height, daily.data);
	ili9488_draw_pixmap(45, 195, heavy.width, heavy.height, heavy.data);
	ili9488_draw_pixmap(185, 195, rinse.width, rinse.height, rinse.data);
	ili9488_draw_pixmap(45, 335, centrifuge.width, centrifuge.height, centrifuge.data);
	ili9488_draw_pixmap(185, 335, custom.width, custom.height, custom.data);
	
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
	// array para escrita no LCD
	//uint8_t stingLCD[256];
	
	/* Initialize the board. */
	sysclk_init();
	board_init();
	ioport_init();

    /* Inicializa e configura o LCD */
	configure_lcd();

    /* Escreve na tela Computacao Embarcada 2018 */
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	//ili9488_draw_filled_rectangle(0, 300, ILI9488_LCD_WIDTH-1, 315);
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	
	//sprintf(stingLCD, "Computacao Embarcada %d", 2019);
	//ili9488_draw_string(10, 300, stingLCD);

	while (1) {
	
	}
	return 0;
}
