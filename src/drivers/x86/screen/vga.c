/**
 *  VGA Text Driver Implementation
 *
 *  Writes directly to VGA memory buffer.
 *  Uses hardware cursor registers (0x3D4/0x3D5).
 *
 *  Scrolling is implemented by shifting rows upward.
 *
 *  Notes:
 *      - Single global context
 *      - Text mode only
 *  Author: Sreyas A (TrickyNinja)
 */

#include "vga.h"
#include "memory_utils.h"
#include "arch/i686/io.h"

static VGA_Context g_currentContext = {
    (uint8_t *)0xB8000,
    25,
    80,
    VGA_DEFAULT_COLOR,
    0,
    0,
};


void vga_set_cursor(uint8_t csrX, uint8_t csrY)
{
  g_currentContext.csrX = csrX;
  g_currentContext.csrY = csrY;
  if (g_currentContext.csrX >= g_currentContext.maxCols)
  {
    g_currentContext.csrX = 0;
    g_currentContext.csrY++;
  }
  if (g_currentContext.csrY >= g_currentContext.maxRows)
    vga_scroll(1);

  uint32_t offset = csr_to_offset(g_currentContext.csrX, g_currentContext.csrY);
  offset /= 2;
  write_portb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
  write_portb(VGA_DATA_REGISTER, (uint8_t)(offset >> 8));
  write_portb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
  write_portb(VGA_DATA_REGISTER, (uint8_t)(offset & 0xff));
}

uint32_t vga_get_cursor_offset()
{
  write_portb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
  uint32_t offset = read_portb(VGA_DATA_REGISTER) << 8;
  write_portb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
  offset += read_portb(VGA_DATA_REGISTER);
  return offset * 2;
}

void vga_clear()
{
  uint16_t blank = 0x20 | (g_currentContext.color << 8);

  memsetw((uint16_t *)g_currentContext.videoAddress, blank, g_currentContext.maxCols * g_currentContext.maxRows);
  vga_set_cursor(0, 0);
}

void vga_print_char_at(char character, int col, int row, uint8_t color)
{
  vga_set_cursor(col, row);
  vga_print_char(character, color);
}

void vga_print_char(char character, uint8_t color)
{
  if (!color)
  {
    color = g_currentContext.color;
  }

  if (character == '\n')
  {
    g_currentContext.csrX = 0;
    g_currentContext.csrY++;
  }

  else if (character == '\r')
  {
    g_currentContext.csrX = 0;
  }
  else if (character == '\t')
  {
    g_currentContext.csrX = (g_currentContext.csrX + 8) & ~(8 - 1);
  }
  else if (character == '\b')
  {
   if (g_currentContext.csrX != 0) g_currentContext.csrX--;
  
   uint32_t offset = csr_to_offset(g_currentContext.csrX, g_currentContext.csrY);
   memset(g_currentContext.videoAddress + offset, VGA_BLANK, 1);
   memset(g_currentContext.videoAddress + offset + 1, color, 1);
  }

  /* Any character greater than or equal to space is printable */
  else if (character >= ' ')
  {
    uint32_t offset = csr_to_offset(g_currentContext.csrX, g_currentContext.csrY);
    memset(g_currentContext.videoAddress + offset, character, 1);
    memset(g_currentContext.videoAddress + offset + 1, color, 1);
    g_currentContext.csrX++;
  }
  vga_set_cursor(g_currentContext.csrX, g_currentContext.csrY);
}

uint32_t vga_scroll(uint8_t amt)
{
  uint32_t offset = (amt * g_currentContext.maxCols) * 2; // the memory offset where x=0, y=amt
  uint32_t count = (g_currentContext.maxRows - amt) * g_currentContext.maxCols; // the number of words(2bytes) from x=0, y=amt to max address
  memcpy(g_currentContext.videoAddress, g_currentContext.videoAddress + offset, count * 2); // copy every byte from x=0, y=amt to max addres amt*max_col offset back 
  vga_set_cursor(g_currentContext.csrX, g_currentContext.csrY - amt);                  //  Move csrY back amt times so that its relatively in the same position it originally was
  memsetw((uint16_t *)(g_currentContext.videoAddress + count * 2), VGA_BLANK, amt * 80); // sets all rows from x=79, y=max_col-amt to the end of vga array as the blank character
  return vga_get_cursor_offset();
}

void vga_enable_cursor_blinking()
{
  write_portb(VGA_CTRL_REGISTER, VGA_CURSOR_START_REG);
  uint8_t cursorStart = read_portb(VGA_DATA_REGISTER);
  cursorStart = (cursorStart & 0xC0) | (14 & 0x1F);
  write_portb(VGA_CTRL_REGISTER, VGA_CURSOR_START_REG);
  write_portb(VGA_DATA_REGISTER, cursorStart);

  write_portb(VGA_CTRL_REGISTER, VGA_CURSOR_END_REG);
  write_portb(VGA_DATA_REGISTER, 15 & 0x1F);
}

void vga_set_color(uint8_t color)
{
  g_currentContext.color = color;
}

// Helper functions
uint32_t csr_to_offset(uint8_t col, uint8_t row)
{
  return 2 * (row * g_currentContext.maxCols + col);
}
