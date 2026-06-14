typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

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
#define VGA_ENTRY(CHAR, COLOR) ((u16)(CHAR) | (u16)((COLOR) << 8))

static u8 scancode_table[0x59] = {
    0,    0,                                             // null, escape
    '1',  '2', '3',  '4',  '5', '6', '7', '8', '9', '0', // nums
    '-',  '=', '\b', '\t',                               //
    'q',  'w', 'e',  'r',  't', 'y', 'u', 'i', 'o', 'p', '[',  ']', //
    '\n', 0, // enter, left control
    'a',  's', 'd',  'f',  'g', 'h', 'j', 'k', 'l', ';', '\'', '`', //
    0, // left shift
    '\\', 'z', 'x',  'c',  'v', 'b', 'n', 'm', ',', '.', '/',
    0,                                                 // right shift
    '*',                                               //
    0,                                                 //
    ' ',                                               //
    0,                                                 // caps lock
    0,    0,   0,    0,    0,   0,   0,   0,   0,   0, // F1-10
    0,    0,                                           // num lock, scroll lock
    '7',  '8', '9',  '-',  '4', '5', '6', '+', '1', '2', '3',  '0',
    '.',          // keypad
    0,    0,   0, // ?
    0,    0,      // F11, F12
};

typedef struct screen_s {
  int x;
  int y;
  u16 buffer[80 * 24];
} screen_t;

screen_t screens[9];

static u16 *fb = (u16 *)0xB8000;
static int screen_id = 0;
static u16 fg_color = COLOR_LIGHT_GREY;
static u16 bg_color = COLOR_BLACK;

void outb(u16 port, u8 value) {
  asm volatile("outb %b0, %w1" ::"a"(value), "Nd"(port) : "memory");
}

u16 inb(u16 port) {
  u8 value;
  asm volatile("inb %w1, %b0" : "=a"(value) : "Nd"(port) : "memory");
  return value;
}

void terminal_scroll() {
  for (int i = 80; i < 80 * 24; i++) {
    screens[screen_id].buffer[i - 80] = screens[screen_id].buffer[i];
  }
  for (int i = 0; i < 80; i++) {
    screens[screen_id].buffer[i + 23 * 80] =
        VGA_ENTRY(' ', VGA_COLOR(bg_color, fg_color));
  }
}

void terminal_printn(const char *s, int n) {
  int i = 0;
  while (i < n) {
    if (s[i] == '\n') {
      screens[screen_id].x = 0;
      if (++screens[screen_id].y >= 24) {
        terminal_scroll();
        screens[screen_id].y = 23;
        screens[screen_id].x = 0;
      }
    } else {
      screens[screen_id]
          .buffer[screens[screen_id].x + screens[screen_id].y * 80] =
          VGA_ENTRY(s[i], VGA_COLOR(bg_color, fg_color));

      if (++screens[screen_id].x >= 80) {
        screens[screen_id].x = 0;
        if (++screens[screen_id].y >= 24) {
          terminal_scroll();
          screens[screen_id].y = 23;
          screens[screen_id].x = 0;
        }
      }
    }
    i++;
  }
}

void terminal_print(const char *s) {
  int n = 0;
  while (s[n])
    n++;
  terminal_printn(s, n);
}

void terminal_enable_cursor(u8 cursor_start, u8 cursor_end) {
  outb(0x3D4, 0x0A);
  outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

  outb(0x3D4, 0x0B);
  outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void terminal_set_cursor(int x, int y) {
  u16 pos = y * 80 + x;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (u8)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

static int write_int(long i, int x) {
  if (i < 10) {
    fb[x + 24 * 80] = VGA_ENTRY('0', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
    fb[1 + x + 24 * 80] =
        VGA_ENTRY('0' + i, VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  } else {
    fb[x + 24 * 80] =
        VGA_ENTRY('0' + (i / 10), VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
    fb[1 + x + 24 * 80] =
        VGA_ENTRY('0' + (i % 10), VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  }
  return 2;
}

void terminal_flush(int min_x, int max_x, int min_y, int max_y) {
  for (int x = min_x; x < max_x; x++)
    for (int y = min_y; y <= max_y; y++)
      fb[x + y * 80] = screens[screen_id].buffer[x + y * 80];

  int c = 0;
  for (int i = 0; i < 9; i++) {
    fb[c + 24 * 80] = VGA_ENTRY(' ', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
    fb[c + 1 + 24 * 80] = VGA_ENTRY(
        '1' + i, VGA_COLOR(screen_id == i ? COLOR_LIGHT_BLUE : COLOR_LIGHT_GREY,
                           screen_id == i ? COLOR_WHITE : COLOR_BLACK));
    c += 2;
  }
  for (; c < 80; c++) {
    fb[c + 24 * 80] = VGA_ENTRY(' ', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  }

  int color = 0;
  for (c = 20; c < 20 + 15 * 2; c += 2) {
    fb[c + 24 * 80] = VGA_ENTRY(' ', VGA_COLOR(color, COLOR_BLACK));
    fb[c + 1 + 24 * 80] = VGA_ENTRY(' ', VGA_COLOR(color, COLOR_BLACK));
    color += 1;
  }

  c += 4;

  fb[(c++) + 24 * 80] =
      VGA_ENTRY('R', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  fb[(c++) + 24 * 80] =
      VGA_ENTRY('O', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  fb[(c++) + 24 * 80] =
      VGA_ENTRY('W', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  c++;

  c += write_int(screens[screen_id].y, c);
  c++;

  fb[(c++) + 24 * 80] =
      VGA_ENTRY('C', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  fb[(c++) + 24 * 80] =
      VGA_ENTRY('O', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  fb[(c++) + 24 * 80] =
      VGA_ENTRY('L', VGA_COLOR(COLOR_LIGHT_GREY, COLOR_BLACK));
  c++;

  c += write_int(screens[screen_id].x, c);

  terminal_set_cursor(screens[screen_id].x, screens[screen_id].y + 1);
}

void terminal_full_flush() { terminal_flush(0, 80, 0, 24); }

extern void ft_printf(const char *fmt, ...);

typedef struct idt_desc_s {
  u16 offset_1;
  u16 selector;
  u8 zero;
  u8 type_attributes;
  u16 offset_2;
} idt_desc_t;

typedef struct __attribute__((packed)) idtp_s {
  u16 limit;
  u32 base;
} idtp_t;

extern void int_handler();

extern void irq_ignore();
extern void irq_keyboard();

static void send_eoi() { outb(0x20, 0x20); }

void exception_handler() {
  terminal_print("exception caught !");
  terminal_full_flush();
}

void keyboard_irq() {
  int scancode = inb(0x60);

  if (scancode >= 0x58) {
    send_eoi();
    return;
  }

  if (scancode >= 0x3B && scancode <= 0x43) {
    screen_id = scancode - 0x3B;
  } else {
    u8 ch = scancode_table[scancode];
    if (ch != 0) {
      terminal_printn((char *)&ch, 1);
    }
  }

  terminal_full_flush();
  send_eoi();
}

idt_desc_t ints[128];

static void ints_init() {
  idtp_t idtp;
  idtp.limit = sizeof(ints) - 1;
  idtp.base = (u32)&ints;

  for (int i = 0; i < 31; i++) {
    ints[i].offset_1 = ((u32)&int_handler) & 0xffff;
    ints[i].offset_2 = (((u32)&int_handler) >> 16) & 0xffff;
    ints[i].selector = 0x10;
    ints[i].type_attributes = 0xF | (1 << 7);
    ints[i].zero = 0;
  }

  // remap the pic
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // unmask all interrupts
  outb(0x21, 0x0);
  outb(0xA1, 0x0);

  for (int i = 32; i < 128; i++) {
    ints[i].offset_1 = ((u32)irq_ignore) & 0xffff;
    ints[i].offset_2 = (((u32)irq_ignore) >> 16) & 0xffff;
    ints[i].selector = 0x10;
    ints[i].type_attributes = 0xE | (1 << 7);
    ints[i].zero = 0;
  }

  ints[32 + 1].offset_1 = ((u32)irq_keyboard) & 0xffff;
  ints[32 + 1].offset_2 = (((u32)irq_keyboard) >> 16) & 0xffff;

  asm volatile("lidt %0" : : "m"(idtp));
}

void kernel_main() {
  ints_init();

  // clear the terminal
  terminal_enable_cursor(0, 2);
  terminal_full_flush();

  // enable interrupts
  asm volatile("sti");

  for (;;) {
    asm("hlt");
  }
}

