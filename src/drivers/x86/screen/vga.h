/**
 *  Module: VGA Text Driver
 *  Purpose: 80x25 text mode output using VGA memory (0xB8000)
 *
 *  Provides:
 *      - Character printing
 *      - Cursor control
 *      - Screen clearing and scrolling
 *
 *  Used as the primary kernel console.
 *
 *  Author: Sreyas A (TrickyNinja)
 */

#pragma once

#include <stdint.h>

// VGA I/O port addresses for cursor control
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5

// Cursor register indices
#define VGA_CURSOR_START_REG 0x0A
#define VGA_CURSOR_END_REG   0x0B

// Cursor position offset registers
#define VGA_OFFSET_LOW   0x0F
#define VGA_OFFSET_HIGH  0x0E

// Default color attributes (foreground | background << 4)
#define VGA_DEFAULT_COLOR 0x07
#define VGA_LIGHT_COLOR   0xF0  // Bit 7 may enable blinking on real hardware
#define VGA_HACKER_COLOR  0x02

// Blank character with current color
#define VGA_BLANK (0x20 | (g_currentContext.color << 8))

/**
 *  VGA text mode context structure.
 *
 *  Holds display state information including:
 *      - Base video memory address
 *      - Screen dimensions
 *      - Current color attribute
 *      - Cursor position
 */
typedef struct
{
  uint8_t *videoAddress;  // Base address of VGA text buffer (0xB8000)
  const uint8_t maxRows;  // Total number of rows (typically 25)
  const uint8_t maxCols;  // Total number of columns (typically 80)

  uint8_t color;          // Current foreground/background color
  uint8_t csrX;           // Cursor column
  uint8_t csrY;           // Cursor row

} VGA_Context;

static VGA_Context g_currentContext = {
    (uint8_t *)0xB8000,
    25,
    80,
    VGA_DEFAULT_COLOR,
    0,
    0,
};


/**
 *  Sets the hardware cursor position.
 *
 *  Updates both the VGA hardware cursor and
 *  the internal context cursor position.
 *
 *  @param csrX Column position (0 to maxCols-1)
 *  @param csrY Row position (0 to maxRows-1)
 */
void vga_set_cursor(uint8_t csrX, uint8_t csrY);

/**
 *  Returns the current cursor offset in the VGA buffer.
 *
 *  Offset is measured in character cells.
 *
 *  @return Cursor offset
 */
uint32_t vga_get_cursor_offset();

/** 
*  Prints a charcater at a specific row and column of the screen and moves the cursor there
*
*  @param character Ascii character to be printed
*  @param col Column where the character needs to be printed
*  @param row Row where the character needs to be printed
*  @param color Background and foreground color information
*/
void vga_print_char_at(char character, int col, int row, uint8_t color);

/**
 *  Prints a character at the current cursor position.
 *
 *  Advances the cursor automatically.
 *
 *  @param character ASCII character to print
 *  @param color Background and foreground color information
 */
void vga_print_char(char character, uint8_t color);

/**
 *  Clears the entire screen.
 *
 *  Fills the VGA buffer with blank characters
 *  and resets the cursor to (0,0).
 */
void vga_clear();

/**
 *  Scrolls the screen upward by the specified number of rows.
 *
 *  Moves existing text up and clears newly freed rows.
 *
 *  @param amt Number of rows to scroll
 *  @return New cursor offset after scrolling
 */
uint32_t vga_scroll(uint8_t amt);

/**
 *  Enables hardware cursor blinking.
 *
 *  Configures VGA cursor registers to allow blinking.
 */
void vga_enable_cursor_blinking();

/**
 *  Sets the current text color attribute.
 *
 *  @param color Foreground and background color value
 */
void vga_set_color(uint8_t color);

// Helper functions
uint32_t csr_to_offset(uint8_t col, uint8_t row);