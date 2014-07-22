#include <inttypes.h>
#include <lcd.h>
#include <lcd_ssd1306.h>

#include <spark_wiring.h>
#include <spark_wiring_spi.h>

static void inline ssd1306_command(lcd_t *lcd, uint8_t c)
{
  digitalWrite(lcd->cs, LOW);
  digitalWrite(lcd->dc, LOW);
  SPI.transfer(c);
  digitalWrite(lcd->cs, HIGH);
}

static void inline ssd1306_data_start(lcd_t *lcd)
{
  digitalWrite(lcd->dc, HIGH);
  digitalWrite(lcd->cs, LOW);
}

static void inline ssd1306_data_end(lcd_t *lcd)
{
  digitalWrite(lcd->cs, HIGH);
}

static void inline ssd1306_data(lcd_t *lcd, uint8_t data)
{
  SPI.transfer(data);
}

void ssd1306_init(lcd_t *lcd, int reset_pin, int cs_pin, int dc_pin)
{
  lcd->reset = reset_pin;
  lcd->cs = cs_pin;
  lcd->dc = dc_pin;
  lcd->x = 0;
  lcd->y = 0;

  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(0);
  SPI.begin();

  pinMode(lcd->reset, OUTPUT);
  pinMode(lcd->cs, OUTPUT);
  pinMode(lcd->dc, OUTPUT);

  digitalWrite(lcd->cs, HIGH);
  digitalWrite(lcd->dc, HIGH);

  digitalWrite(lcd->reset, HIGH);
  Delay_Microsecond(1000);
  digitalWrite(lcd->reset, LOW);
  Delay_Microsecond(10000);
  digitalWrite(lcd->reset, HIGH);

  ssd1306_command(lcd, SSD1306_DISPLAYOFF);                    // 0xAE
  ssd1306_command(lcd, SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
  ssd1306_command(lcd, 0x80);                                  // the suggested ratio 0x80
  ssd1306_command(lcd, SSD1306_SETMULTIPLEX);                  // 0xA8
  ssd1306_command(lcd, 0x1F);
  ssd1306_command(lcd, SSD1306_SETDISPLAYOFFSET);              // 0xD3
  ssd1306_command(lcd, 0x0);                                   // no offset
  ssd1306_command(lcd, SSD1306_SETSTARTLINE | 0x0);            // line #0


  ssd1306_command(lcd, SSD1306_MEMORYMODE);                    // 0x20
  ssd1306_command(lcd, 0x00);                                  // 0x0 act like ks0108
  ssd1306_command(lcd, SSD1306_SEGREMAP | 0x1);

  // Set scan direction
  ssd1306_command(lcd, SSD1306_COMSCANDEC);

  ssd1306_command(lcd, SSD1306_SETCOMPINS);                    // 0xDA
  ssd1306_command(lcd, 0x02);

  ssd1306_command(lcd, SSD1306_SETCONTRAST);                   // 0x81
  ssd1306_command(lcd, 0x8F);

  ssd1306_command(lcd, SSD1306_SETPRECHARGE);                  // 0xd9

  ssd1306_command(lcd, 0xF1);

  ssd1306_command(lcd, SSD1306_SETVCOMDETECT);                 // 0xDB
  ssd1306_command(lcd, 0x40);
  ssd1306_command(lcd, SSD1306_DISPLAYALLON_RESUME);           // 0xA4
  ssd1306_command(lcd, SSD1306_NORMALDISPLAY);                 // 0xA6

  ssd1306_command(lcd, SSD1306_CHARGEPUMP);                    // 0x8D
  ssd1306_command(lcd, 0x14);

  ssd1306_command(lcd, SSD1306_DISPLAYON);
}

void ssd1306_clear(lcd_t *lcd)
{
  uint16_t i;
  ssd1306_command(lcd, SSD1306_MEMORYMODE);
  ssd1306_command(lcd, 0);

  // Set column start and end address
  ssd1306_command(lcd, SSD1306_COLUMNADDR);
  ssd1306_command(lcd, 0);
  ssd1306_command(lcd, 127);

  // Set page start and end address
  ssd1306_command(lcd, SSD1306_PAGEADDR);
  ssd1306_command(lcd, 0);
  ssd1306_command(lcd, 3);

  ssd1306_data_start(lcd);
  for(i=0;i<128*4;i++)
  {
    ssd1306_data(lcd, 0);
  } 
  ssd1306_data_end(lcd); 
}

void ssd1306_print(lcd_t *lcd, char *str)
{
  int len = strlen(str);
  int i;

  for(i=0;i<len;i++)
  {
    ssd1306_write(lcd, str[i]);
  }
}

void ssd1306_cursor(lcd_t *lcd, int x, int y)
{
  lcd->x = x;
  lcd->y = y;
}

/**
 * @brief Write a single character at the current cursor position
 *
 * Using vertical addressing mode, we can have the hardware
 * automatically move to the next column after writing 7 pixels
 * in each page (rows).
 */
void ssd1306_write(lcd_t *lcd, char c)
{
  uint8_t *font_bitmap = (uint8_t *)&font[(int)(c*5)];
  uint8_t i;

  uint8_t start = lcd->x * 6;

  // Set vertical addressing mode
  ssd1306_command(lcd, SSD1306_MEMORYMODE);
  ssd1306_command(lcd, 1);

  // Set column start and end address
  ssd1306_command(lcd, SSD1306_COLUMNADDR);
  ssd1306_command(lcd, start);
  ssd1306_command(lcd, start+5);

  // Set page start and end address
  ssd1306_command(lcd, SSD1306_PAGEADDR);
  ssd1306_command(lcd, lcd->y);
  ssd1306_command(lcd, lcd->y);

  ssd1306_data_start(lcd);
  for(i=0;i<5;i++)
  {
    ssd1306_data(lcd, font_bitmap[i]);
  }
  ssd1306_data_end(lcd);

  lcd->x++;
  if(lcd->x >= 21)
  {
    lcd->y++;
    lcd->x = 0;
  }
 
  if(lcd->y >= 4)
    lcd->y = 0; 
    
}
