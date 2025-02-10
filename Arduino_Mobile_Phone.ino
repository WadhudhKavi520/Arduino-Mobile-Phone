/*
 * Project: Arduino-based Mobile Phone using GSM Module and TFT Display
 * Author: Wadhudh Kavi
 * Date: June 2024
 *
 * Description:
 * This project implements a basic mobile phone using an Arduino board, a SIM900A GSM module for cellular communication, 
 * and a 2.4-inch TFT touchscreen display for the user interface. The system allows users to dial numbers, make calls, 
 * and interact with a graphical keypad.
 *
 * Hardware Components:
 * 1. **Microcontroller**: Arduino Uno (or compatible)
 * 2. **GSM Module**: SIM800L (for sending AT commands and making calls)
 * 3. **TFT Display**: 2.4-inch TFT LCD with resistive touch (ILI9341 driver)
 * 4. **Touchscreen Controller**: Uses XPT2046 for detecting user input
 * 5. **Power Supply**: External power source recommended for stable GSM operation
 *
 * Libraries Used:
 * - LCDWIKI_KBV.h (For TFT display control)
 * - LCDWIKI_GUI.h (For graphical interface support)
 * - TouchScreen.h (For touchscreen input processing)
 * - SoftwareSerial.h (For serial communication with SIM800L).
 *
 * Pin Configuration:
 * - **GSM Module (SIM800L)**:
 *   - TX -> Arduino Pin 12 (Software Serial RX)
 *   - RX -> Arduino Pin 11 (Software Serial TX)
 *   - VCC -> 4.2V (Use a regulator if necessary)
 *   - GND -> Arduino GND
 * - **TFT Display**:
 *   - CS  -> A3
 *   - CD  -> A2
 *   - WR  -> A1
 *   - RD  -> A0
 *   - RESET -> A4
 * - **Touchscreen (Resistive)**:
 *   - XP -> Pin 6
 *   - XM -> A2
 *   - YP -> A1
 *   - YM -> Pin 7
 *
 * Communication Protocols:
 * - The GSM module communicates using AT commands via SoftwareSerial.
 * - The TFT display uses a parallel interface controlled via LCDWIKI_KBV.
 * - Touchscreen input is processed using analog readings and a resistive grid.
 *
 * Features Implemented:
 * - Graphical keypad interface for dialing numbers.
 * - Calling functionality via GSM module using AT commands.
 * - Call end functionality.
 * - Delete button to remove typed numbers.
 *
 * Notes:
 * - Ensure the GSM module is powered with a stable 4.2V power supply.
 * - The touchscreen requires calibration; adjust TS_MINX, TS_MAXX, TS_MINY, and TS_MAXY accordingly.
 * - Modify the number in the ATD command for testing actual calls.
 *
 */


#include <LCDWIKI_KBV.h> // Hardware-specific library
#include <LCDWIKI_GUI.h> // Core graphics library
#include <TouchScreen.h> // Touch library
#include <SoftwareSerial.h>

// Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(12, 11);

// LCD initialization
LCDWIKI_KBV my_lcd(240, 320, A3, A2, A1, A0, A4); // width, height, cs, cd, wr, rd, reset

// Color definitions
#define BLACK        0x0000
#define BLUE         0x001F
#define RED          0xF800
#define GREEN        0x07E0
#define CYAN         0x07FF
#define MAGENTA      0xF81F
#define YELLOW       0xFFE0
#define WHITE        0xFFFF
#define NAVY         0x000F
#define DARKGREEN    0x03E0
#define DARKCYAN     0x03EF
#define MAROON       0x7800
#define PURPLE       0x780F
#define OLIVE        0x7BE0
#define LIGHTGREY    0xC618
#define DARKGREY     0x7BEF
#define ORANGE       0xFD20
#define GREENYELLOW  0xAFE5
#define PINK         0xF81F

// UI details
#define BUTTON_R 25
#define BUTTON_SPACING_X 25
#define BUTTON_SPACING_Y 5
#define EDG_Y 5
#define EDG_X 20

// Touchscreen pins
#define YP A1
#define XM A2
#define YM 7
#define XP 6

// Touchscreen calibration
#define TS_MINX 910
#define TS_MAXX 180
#define TS_MINY 760
#define TS_MAXY 135

// Touchscreen pressure thresholds
#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Button structure
typedef struct _button_info {
    uint8_t button_name[10];
    uint8_t button_name_size;
    uint16_t button_name_colour;
    uint16_t button_colour;
    uint16_t button_x;
    uint16_t button_y;
} button_info;

// Button definitions
button_info phone_button[15] = {
    {"1", 3, BLACK, CYAN, EDG_X + BUTTON_R - 1, my_lcd.Get_Display_Height() - EDG_Y - 4 * BUTTON_SPACING_Y - 9 * BUTTON_R - 1},
    {"2", 3, BLACK, CYAN, EDG_X + 3 * BUTTON_R + BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 4 * BUTTON_SPACING_Y - 9 * BUTTON_R - 1},
    {"3", 3, BLACK, CYAN, EDG_X + 5 * BUTTON_R + 2 * BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 4 * BUTTON_SPACING_Y - 9 * BUTTON_R - 1},
    {"4", 3, BLACK, CYAN, EDG_X + BUTTON_R - 1, my_lcd.Get_Display_Height() - EDG_Y - 3 * BUTTON_SPACING_Y - 7 * BUTTON_R - 1},
    {"5", 3, BLACK, CYAN, EDG_X + 3 * BUTTON_R + BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 3 * BUTTON_SPACING_Y - 7 * BUTTON_R - 1},
    {"6", 3, BLACK, CYAN, EDG_X + 5 * BUTTON_R + 2 * BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 3 * BUTTON_SPACING_Y - 7 * BUTTON_R - 1},
    {"7", 3, BLACK, CYAN, EDG_X + BUTTON_R - 1, my_lcd.Get_Display_Height() - EDG_Y - 2 * BUTTON_SPACING_Y - 5 * BUTTON_R - 1},
    {"8", 3, BLACK, CYAN, EDG_X + 3 * BUTTON_R + BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 2 * BUTTON_SPACING_Y - 5 * BUTTON_R - 1},
    {"9", 3, BLACK, CYAN, EDG_X + 5 * BUTTON_R + 2 * BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - 2 * BUTTON_SPACING_Y - 5 * BUTTON_R - 1},
    {"#", 3, BLACK, PINK, EDG_X + BUTTON_R - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_SPACING_Y - 3 * BUTTON_R - 1},
    {"0", 3, BLACK, CYAN, EDG_X + 3 * BUTTON_R + BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_SPACING_Y - 3 * BUTTON_R - 1},
    {"*", 3, BLACK, PINK, EDG_X + 5 * BUTTON_R + 2 * BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_SPACING_Y - 3 * BUTTON_R - 1},
    {"end", 2, BLACK, RED, EDG_X + BUTTON_R - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_R - 1},
    {"call", 2, BLACK, GREEN, EDG_X + 3 * BUTTON_R + BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_R - 1},
    {"dele", 2, BLACK, LIGHTGREY, EDG_X + 5 * BUTTON_R + 2 * BUTTON_SPACING_X - 1, my_lcd.Get_Display_Height() - EDG_Y - BUTTON_R - 1},
};

// Display string on LCD
void show_string(uint8_t *str, int16_t x, int16_t y, uint8_t csize, uint16_t fc, uint16_t bc, boolean mode) {
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str, x, y);
}

// Check if a button is pressed
boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py) {
    return (px > x1 && px < x2) && (py > y1 && py < y2);
}

// Display the main menu
void show_menu() {
    for (uint16_t i = 0; i < sizeof(phone_button) / sizeof(button_info); i++) {
        my_lcd.Set_Draw_color(phone_button[i].button_colour);
        my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
        show_string(phone_button[i].button_name, phone_button[i].button_x - strlen(phone_button[i].button_name) * phone_button[i].button_name_size * 6 / 2 + 1, phone_button[i].button_y - phone_button[i].button_name_size * 8 / 2 + 1, phone_button[i].button_name_size, phone_button[i].button_name_colour, BLACK, 1);
    }
    my_lcd.Set_Draw_color(BLACK);
    my_lcd.Fill_Rectangle(1, 1, my_lcd.Get_Display_Width() - 2, 3);
    my_lcd.Fill_Rectangle(1, 29, my_lcd.Get_Display_Width() - 2, 31);
    my_lcd.Fill_Rectangle(1, 1, 3, 31);
    my_lcd.Fill_Rectangle(my_lcd.Get_Display_Width() - 4, 1, my_lcd.Get_Display_Width() - 2, 31);
}

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    my_lcd.Init_LCD();
    my_lcd.Fill_Screen(BLUE);
    show_menu();
}

uint16_t text_x = 10, text_y = 6;
uint16_t text_x_add = 6 * phone_button[0].button_name_size;
uint16_t text_y_add = 8 * phone_button[0].button_name_size;
uint16_t n = 0;

void loop() {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        p.x = map(p.x, TS_MINX, TS_MAXX, 0, my_lcd.Get_Display_Width());
        p.y = map(p.y, TS_MINY, TS_MAXY, 0, my_lcd.Get_Display_Height());

        for (uint16_t i = 0; i < sizeof(phone_button) / sizeof(button_info); i++) {
            if (is_pressed(phone_button[i].button_x - BUTTON_R, phone_button[i].button_y - BUTTON_R, phone_button[i].button_x + BUTTON_R, phone_button[i].button_y + BUTTON_R, p.x, p.y)) {
                my_lcd.Set_Draw_color(DARKGREY);
                my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
                show_string(phone_button[i].button_name, phone_button[i].button_x - strlen(phone_button[i].button_name) * phone_button[i].button_name_size * 6 / 2 + 1, phone_button[i].button_y - phone_button[i].button_name_size * 8 / 2 + 1, phone_button[i].button_name_size, WHITE, BLACK, 1);
                delay(100);
                my_lcd.Set_Draw_color(phone_button[i].button_colour);
                my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
                show_string(phone_button[i].button_name, phone_button[i].button_x - strlen(phone_button[i].button_name) * phone_button[i].button_name_size * 6 / 2 + 1, phone_button[i].button_y - phone_button[i].button_name_size * 8 / 2 + 1, phone_button[i].button_name_size, phone_button[i].button_name_colour, BLACK, 1);

                if (i < 12) { // Digits 0-9, #, *
                    if (n < 13) {
                        show_string(phone_button[i].button_name, text_x, text_y, phone_button[i].button_name_size, GREENYELLOW, BLACK, 1);
                        text_x += text_x_add - 1;
                        n++;
                    }
                } else if (i == 12) { // End call
                    my_lcd.Set_Draw_color(BLUE);
                    my_lcd.Fill_Rectangle(0, 33, my_lcd.Get_Display_Width() - 1, 42);
                    show_string("Calling ended", CENTER, 33, 1, OLIVE, BLACK, 1);
                } else if (i == 13) { // Call
                    my_lcd.Set_Draw_color(BLUE);
                    my_lcd.Fill_Rectangle(0, 33, my_lcd.Get_Display_Width() - 1, 42);
                    show_string("Calling...", CENTER, 33, 1, OLIVE, BLACK, 1);

                    // Dial the number
                    mySerial.println("ATD+916374354166;"); // Replace with the number to dial
                    updateSerial();
                    delay(20000); // Wait for 20 seconds
                    mySerial.println("ATH"); // Hang up
                    updateSerial();
                } else if (i == 14) { // Delete
                    if (n > 0) {
                        my_lcd.Set_Draw_color(BLUE);
                        text_x -= (text_x_add - 1);
                        my_lcd.Fill_Rectangle(text_x, text_y, text_x + text_x_add - 1, text_y + text_y_add - 2);
                        n--;
                    }
                }
            }
        }
    }
}

// Relay data between hardware serial and software serial
void updateSerial() {
    delay(500);
    while (Serial.available()) {
        mySerial.write(Serial.read());
    }
    while (mySerial.available()) {
        Serial.write(mySerial.read());
    }
}