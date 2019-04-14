/** 
 * Computação Embarcada
 * APS 2 
 * Rafael . Corsi @ insper . edu . br
 */ 
#include <stdio.h>
#include "maquina1.h"

typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
	uint8_t dataSize;
} tImage;

#include "h_icons/centrifuge.h"

/**
 * Inicializa ordem do menu
 * retorna o primeiro ciclo que
 * deve ser exibido.
 */
t_ciclo *initMenuOrder(){
  c_rapido.previous = &c_enxague;
  c_rapido.next = &c_diario;

  c_diario.previous = &c_rapido;
  c_diario.next = &c_pesado;

  c_pesado.previous = &c_diario;
  c_pesado.next = &c_enxague;

  c_enxague.previous = &c_pesado;
  c_enxague.next = &c_centrifuga;

  c_centrifuga.previous = &c_enxague;
  c_centrifuga.next = &c_rapido;

  return(&c_diario);
}

void main(void){
  //t_ciclo *p_primeiro = initMenuOrder();
  //printf("%s", p_primeiro->next->next->nome);
  
  //ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  //sprintf(stingLCD, "Computacao Embarcada %d", 2018);
  //ili9488_draw_string(0, 50, stingLCD);
  
  // desenha imagem lavagem na posicao X=80 e Y=150
  ili9488_draw_pixmap(80, 150, image_data_centrifuge.width, image_data_centrifuge.height, image_data_centrifuge.data);
  
  while(1){
	  
  }
}
