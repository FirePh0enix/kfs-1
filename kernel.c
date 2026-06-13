enum vga_color {
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

#define VGA_COLOR(BG, FG) ((FG) | ((BG) << 4))
#define VGA_ENTRY(CHAR, COLOR)                                                 \
  ((unsigned short)(CHAR) | (unsigned short)((COLOR) << 8))

static unsigned short *fb = (unsigned short *)0xB8000;
static int x = 0;
static int y = 0;
static unsigned short fg_color = COLOR_LIGHT_GREY;
static unsigned short bg_color = COLOR_BLACK;

static void terminal_print(const char *s) {
  while (*s) {
    fb[x + y * 80] = VGA_ENTRY(*s, VGA_COLOR(bg_color, fg_color));
    if (++x >= 80) {
      x = 0;
      if (++y >= 25) {
        // TODO: Scroll the text
      }
    }
    s++;
  }
}

void kernel_main() {
  // hello world!
  terminal_print("42");
}

