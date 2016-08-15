/*
 * AVR library to interface with an LCD1602 type liquid crystal display.
 * To keep the code (and circuits) as simple as possible, the display
 * can currently only be controlled in four bit mode.
 */

#ifndef __AVR_LCD1602_H
#define __AVR_LCD1602_H

#include <stdint.h>

 // --------------------------------------------------------------------------------

struct lcd_t {
	uint8_t register_pin;
	uint8_t enable_pin;
	uint8_t data4_pin;
	uint8_t data5_pin;
	uint8_t data6_pin;
	uint8_t data7_pin;
};

// --------------------------------------------------------------------------------

struct lcd_t lcd_initialize(uint8_t reg, uint8_t enable, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7);

void lcd_set_cursor(struct lcd_t *lcd, uint8_t row, uint8_t column);
void lcd_print(struct lcd_t *lcd, const char *text);
void lcd_print_glyph(struct lcd_t *lcd, uint8_t glyph);
void lcd_clear(struct lcd_t *lcd);
void lcd_create_glyph(struct lcd_t *lcd, uint8_t index, uint8_t *bitmap); // Index can be 0-7. Bitmap is a 5x8 representation of the glyph (8 rows)

#endif /* __AVR_LCD1602_H */
