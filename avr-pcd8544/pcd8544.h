/*
 * pcd8544.h
 *
 * Created: 26.05.2014 22:59:27
 *  Author: oakio
 */ 

#ifndef PCD8544_H_
#define PCD8544_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define PCD8544_HEIGHT 48
#define PCD8544_WIDTH 84
#define PCD8544_BUFFER_SIZE (PCD8544_WIDTH * PCD8544_HEIGHT / 8)

#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_EXTENDED_INSTRUCTION_SET 0x1

#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

#define PCD8544_DISPLAYCONTROL 0x8
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

#define PCD8544_BLACK 255
#define PCD8544_WHITE 0

struct pcd8544
{
    // LCD video memory
    uint8_t buffer[PCD8544_BUFFER_SIZE];
    
    // invalidate rectangle
    uint8_t xInvalidateMin;
    uint8_t yInvalidateMin;
    uint8_t xInvalidateMax;
    uint8_t yInvalidateMax;
};
typedef struct pcd8544 pcd8544_t;

void pcd8544_begin        (pcd8544_t* this);
void pcd8544_invalidate   (pcd8544_t* this, uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax);
void pcd8544_clear        (pcd8544_t* this);
void pcd8544_setPixel     (pcd8544_t* this, uint8_t x, uint8_t y, uint8_t color);
void pcd8544_line         (pcd8544_t* this, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void pcd8544_display      (pcd8544_t* this);

void pcd8544_putChar      (pcd8544_t* this, uint8_t row, uint8_t col, char symbol);

void pcd8544_command      (uint8_t cmd);
void pcd8544_spi_write    (uint8_t data);

#endif /* PCD8544_H_ */