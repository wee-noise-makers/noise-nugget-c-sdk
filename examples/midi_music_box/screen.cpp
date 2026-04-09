#include "screen.hpp"
#include "pgb1.h"

void setPixel(int x, int y, bool set)
{
    screen_set_pixel(127 - x, 63 - y, set);
}

void drawLine(int x0, int y0, int x1, int y1, bool set)
{
    int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;){  /* loop */
        setPixel (x0, y0, set);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }

}

void drawRect(Rectangle<int> rect, bool set) {
    drawLine(rect.getLeft(), rect.getTop(), rect.getLeft(), rect.getBottom(), set);
    drawLine(rect.getRight(), rect.getTop(), rect.getRight(), rect.getBottom(), set);
    drawLine(rect.getLeft(), rect.getTop(), rect.getRight(), rect.getTop(), set);
    drawLine(rect.getLeft(), rect.getBottom(), rect.getRight(), rect.getBottom(), set);
}

void fillRect(Rectangle<int> rect, bool set) {
    for (int y = rect.getTop(); y <= rect.getBottom(); y++) {
        drawLine(rect.getLeft(), y, rect.getRight(), y, set);
    }
}

const uint8_t font5x7[] =
{187, 214, 205, 231, 125, 253, 255, 255, 255, 141, 59, 130, 11, 38, 136, 241, 255, 251, 123, 140, 17, 70, 12, 64, 116,
 113, 56, 239, 92, 132, 17, 70, 224, 156, 115, 14, 196, 30, 247, 207, 223, 255, 247, 239, 247, 251, 246, 252, 255, 255,
 255, 255, 255, 255, 255, 255, 207, 157, 255, 174, 53, 176, 118, 239, 222, 251, 255, 127, 93, 118, 119, 250, 254, 156, 203,
 121, 255, 237, 156, 115, 206, 122, 239, 220, 190, 214, 19, 231, 156, 115, 110, 59, 231, 156, 123, 189, 223, 250, 251, 247,
 255, 253, 245, 253, 255, 125, 255, 255, 255, 255, 127, 255, 255, 255, 255, 125, 223, 191, 43, 208, 183, 222, 125, 213, 254,
 255, 111, 179, 223, 174, 208, 95, 231, 114, 238, 224, 190, 231, 156, 119, 222, 123, 183, 175, 246, 138, 57, 231, 156, 223,
 206, 185, 90, 111, 223, 215, 253, 29, 101, 76, 113, 7, 153, 111, 219, 84, 70, 24, 74, 136, 206, 57, 230, 64, 223,
 183, 239, 95, 227, 222, 127, 223, 8, 62, 248, 91, 237, 123, 237, 131, 59, 134, 255, 253, 127, 183, 1, 232, 29, 132,
 2, 236, 203, 189, 82, 14, 58, 24, 183, 115, 106, 239, 221, 247, 253, 255, 63, 230, 141, 139, 142, 237, 91, 183, 98,
 206, 185, 236, 183, 115, 110, 182, 235, 247, 83, 251, 131, 213, 171, 223, 87, 237, 252, 255, 102, 123, 63, 240, 220, 118,
 47, 231, 14, 238, 86, 206, 121, 231, 189, 115, 251, 106, 239, 140, 243, 74, 191, 237, 156, 170, 187, 247, 125, 255, 255,
 193, 121, 7, 118, 112, 251, 230, 173, 156, 131, 161, 199, 237, 156, 218, 115, 247, 125, 239, 255, 21, 58, 246, 239, 222,
 123, 255, 220, 221, 238, 238, 58, 183, 221, 205, 123, 255, 253, 149, 115, 206, 122, 239, 220, 182, 214, 59, 231, 188, 181,
 110, 187, 74, 220, 246, 253, 222, 255, 191, 115, 206, 249, 253, 220, 182, 117, 43, 231, 252, 235, 111, 153, 170, 242, 238,
 125, 223, 191, 127, 221, 79, 250, 215, 255, 239, 63, 247, 24, 65, 188, 49, 238, 152, 127, 191, 191, 239, 232, 96, 196,
 192, 7, 23, 179, 3, 206, 69, 159, 92, 220, 113, 59, 183, 65, 188, 241, 131, 31, 98, 12, 113, 71, 23, 115, 139,
 202, 69, 255, 58, 188, 105, 87, 195, 193, 220, 249, 251};

 static uint8_t get_bit(const uint8_t *bitmap, int bit_index) {
    int byte_index = bit_index / 8;
    int bit_position = bit_index % 8;

    return (bitmap[byte_index] >> bit_position) & 0x01;
}

int printH(int x, int y, char c, bool set)
{
    const int index = c - '!';

    if (index < 0 || index > 93) {
        return x;
    }

    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            const int bit_index = index * 5 + dx + dy * 470;
            if ((font5x7, bit_index) == 0) {
                setPixel(x + dx, y + dy, set);
            }
        }
    }
    return x + 6;
}

int printH(int x, int y, const std::string& str, bool set)
{
    for (const char *c = str.c_str(); *c != 0; c++) {
        x = printH(x, y, *c, set);
    }

    return x;
}

int printV(int x, int y, char c, bool set)
{
    const int index = c - '!';

    if (index < 0 || index > 93) {
        return y;
    }

    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            const int bit_index = index * 5 + dx + dy * 470;
            if (get_bit(font5x7, bit_index) == 0) {
                setPixel(x + dy, y - dx, set);
            }
        }
    }

    return y - 6;
}

int printV(int x, int y, const std::string& str, bool set)
{
    for (const char *c = str.c_str(); *c != 0; c++) {
        y = printV(x, y, *c, set);
    }
    return y;
}

int printMidiKeyV(int x, int y, uint8_t key, bool set) {

    char c = ' ';
    bool sharp = false;
    switch(key % 12)
    {
        case 0:  c = 'C'; break;
        case 1:  c = 'C'; sharp = true; break;
        case 2:  c = 'D'; break;
        case 3:  c = 'D'; sharp = true; break;
        case 4:  c = 'E'; break;
        case 5:  c = 'F'; break;
        case 6:  c = 'F'; sharp = true; break;
        case 7:  c = 'G'; break;
        case 8:  c = 'G'; sharp = true; break;
        case 9:  c = 'A'; break;
        case 10: c = 'A'; sharp = true; break;
        case 11: c = 'B'; break;
    }

    y = printV(x, y, c, set);
    if (sharp) {
        setPixel(x, y, set);
        setPixel(x, y - 2, set);
        setPixel(x + 2, y, set);
        setPixel(x + 2, y - 2, set);
    }
    y -= 3;

    uint8_t oct = (key / 12) -1;
    y = printV(x, y, std::to_string(oct), set);
    return y;
}