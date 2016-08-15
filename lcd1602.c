#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stddef.h>
#include "lcd1602.h"

// --------------------------------------------------------------------------------

#define LCD_ROWS 2
#define LCD_COLUMNS 16

#define PIN_HIGH(p, pin) p |= (1 << (pin))
#define PIN_LOW(p, pin) p &= ~(1 << (pin))

// --------------------------------------------------------------------------------

// LCD commands
#define LCD_CLEARDISPLAY (1 << 0)
#define LCD_RETURNHOME (1 << 1)
#define LCD_ENTRYMODESET (1 << 2)
#define LCD_DISPLAYCONTROL (1 << 3)
#define LCD_CURSORSHIFT (1 << 4)
#define LCD_FUNCTIONSET (1 << 5)
#define LCD_SETCGRAMADDR (1 << 6)
#define LCD_SETDDRAMADDR (1 << 7)

// Option flags for LCD_ENTRYMODESET (set cursor movement)
#define LCD_ENTRYSHIFTDECREMENT 0
#define LCD_ENTRYSHIFTINCREMENT (1 << 0)
#define LCD_ENTRYRIGHT 0
#define LCD_ENTRYLEFT (1 << 1)

// Option flags for LCD_DISPLAYCONTROL
#define LCD_BLINKOFF 0
#define LCD_BLINKON (1 << 0)
#define LCD_CURSOROFF 0
#define LCD_CURSORON (1 << 1)
#define LCD_DISPLAYOFF 0
#define LCD_DISPLAYON (1 << 2)

// Option flags for LCD_CURSORSHIFT
#define LCD_MOVELEFT 0
#define LCD_MOVERIGHT (1 << 2)
#define LCD_CURSORMOVE 0
#define LCD_DISPLAYMOVE (1 << 3)

// Option flags for LCD_FUNCTIONSET
#define LCD_5X8DOTS 0
#define LCD_5X10DOTS (1 << 2)
#define LCD_1LINE 0
#define LCD_2LINES (1 << 3)
#define LCD_4BITMODE 0
#define LCD_8BITMODE (1 << 4)

// --------------------------------------------------------------------------------

static void lcd_delay(uint8_t ms);
static void lcd_pulse_enable(struct lcd_t *lcd);
static void lcd_write_4_bits(struct lcd_t *lcd, uint8_t data);
static void lcd_command(struct lcd_t *lcd, uint8_t value);
static void lcd_write(struct lcd_t *lcd, uint8_t value);
static void lcd_send(struct lcd_t *lcd, uint8_t value, bool mode);

// --------------------------------------------------------------------------------

struct lcd_t lcd_initialize(uint8_t reg, uint8_t enable, uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7)
{
	struct lcd_t lcd;

	lcd.register_pin = reg;
	lcd.enable_pin = enable;
	lcd.data4_pin = data4;
	lcd.data5_pin = data5;
	lcd.data6_pin = data6;
	lcd.data7_pin = data7;

	// Enable the LCD pins for writing.
	LCD_DDR |= (1 << lcd.register_pin);
	LCD_DDR |= (1 << lcd.enable_pin);

	LCD_DDR |= (1 << lcd.data4_pin);
	LCD_DDR |= (1 << lcd.data5_pin);
	LCD_DDR |= (1 << lcd.data6_pin);
	LCD_DDR |= (1 << lcd.data7_pin);

	// We'll have to wait for the voltage to rise above 2.7V before executing commands.
	// Delaying the startup by 50ms here should do the trick.
	lcd_delay(50);

	// Enter a mode for commands.
	PIN_LOW(LCD_PORT, lcd.register_pin);
	PIN_LOW(LCD_PORT, lcd.enable_pin);

	// Set 4 bit mode.
	lcd_write_4_bits(&lcd, 0x03);
	lcd_delay(5);

	lcd_write_4_bits(&lcd, 0x03);
	lcd_delay(5);

	lcd_write_4_bits(&lcd, 0x03);
	_delay_ms(1);

	lcd_write_4_bits(&lcd, 0x02);

	// Initialize the display.
	lcd_command(&lcd, LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINES | LCD_5X8DOTS);
	lcd_command(&lcd, LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);

	// Clear the display.
	lcd_clear(&lcd);

	// Set display mode.
	lcd_command(&lcd, LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);

	// All done!
	return lcd;
}

void lcd_set_cursor(struct lcd_t *lcd, uint8_t row, uint8_t column)
{
	if (row >= LCD_ROWS) {
		row = LCD_ROWS - 1;
	}

	if (column >= LCD_COLUMNS) {
		column = LCD_COLUMNS - 1;
	}

	lcd_command(lcd, LCD_SETDDRAMADDR | (column + (row == 0 ? 0x00 : 0x40)));
}

void lcd_print(struct lcd_t *lcd, const char *text)
{
	while (*text) {
		lcd_write(lcd, *text);
		++text;
	}
}

void lcd_print_glyph(struct lcd_t *lcd, uint8_t glyph)
{
	lcd_write(lcd, glyph);
}

void lcd_clear(struct lcd_t *lcd)
{
	// CLear and wait for a bit.
	lcd_command(lcd, LCD_CLEARDISPLAY);
	lcd_delay(2);

	// Reset cursor back to beginning on clear.
	lcd_command(lcd, LCD_SETDDRAMADDR);
}

void lcd_create_glyph(struct lcd_t *lcd, uint8_t index, uint8_t *bitmap)
{
	// Only indices 0-7 can be written to.
	index &= 0x7;

	lcd_command(lcd, LCD_SETCGRAMADDR | (index << 3));

	for (int i = 0; i < 8; i++) {
	    lcd_write(lcd, bitmap[i]);
	}

	// Go back to writing to DDRAM. This also resets the cursor position to 0, 0.
	lcd_command(lcd, LCD_SETDDRAMADDR);
}

static void lcd_delay(uint8_t ms)
{
	while (--ms > 0) {
		_delay_ms(1);
	}
}

static void lcd_pulse_enable(struct lcd_t *lcd)
{
	PIN_LOW(LCD_PORT, lcd->enable_pin);
	_delay_us(1);

	PIN_HIGH(LCD_PORT, lcd->enable_pin);
	_delay_us(1);

	PIN_LOW(LCD_PORT, lcd->enable_pin);
	_delay_ms(1);
}

static void lcd_write_4_bits(struct lcd_t *lcd, uint8_t data)
{
	if ((data >> 0) & 0x1) {
		PIN_HIGH(LCD_PORT, lcd->data4_pin);
	}
	else {
		PIN_LOW(LCD_PORT, lcd->data4_pin);
	}

	if ((data >> 1) & 0x1) {
		PIN_HIGH(LCD_PORT, lcd->data5_pin);
	}
	else {
		PIN_LOW(LCD_PORT, lcd->data5_pin);
	}

	if ((data >> 2) & 0x1) {
		PIN_HIGH(LCD_PORT, lcd->data6_pin);
	}
	else {
		PIN_LOW(LCD_PORT, lcd->data6_pin);
	}

	if ((data >> 3) & 0x1) {
		PIN_HIGH(LCD_PORT, lcd->data7_pin);
	}
	else {
		PIN_LOW(LCD_PORT, lcd->data7_pin);
	}

	lcd_pulse_enable(lcd);
}

static void lcd_command(struct lcd_t *lcd, uint8_t value)
{
	lcd_send(lcd, value, false);
}

static void lcd_write(struct lcd_t *lcd, uint8_t value)
{
	lcd_send(lcd, value, true);
}

static void lcd_send(struct lcd_t *lcd, uint8_t value, bool mode)
{
	if (mode) {
		PIN_HIGH(LCD_PORT, lcd->register_pin);
	}
	else {
		PIN_LOW(LCD_PORT, lcd->register_pin);
	}

	lcd_write_4_bits(lcd, value >> 4);
	lcd_write_4_bits(lcd, value);
}
