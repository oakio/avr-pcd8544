/*
 * pcd8544.c
 *
 * Created: 26.05.2014 22:28:49
 *  Author: oakio
 */

#include "pcd8544.h"
#include "font5x7.h"
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#define PCD8544_PORT    PORTD
#define PCD8544_DDR     DDRD
#define PCD8544_SCLK    PD7
#define PCD8544_MOSI    PD6
#define PCD8544_DC      PD5
#define PCD8544_CS      PD4
#define PCD8544_RESET   PD3

// bit ops
#define sbit(port, pin) port |= (1 << pin);
#define cbit(port, pin) port &= ~(1 << pin);

#define swap(a, b) { int16_t t = a; a = b; b = t; }

void pcd8544_begin(pcd8544_t* this)
{
    pcd8544_clear(this);
    
    // setup pin mode as output
    sbit(PCD8544_DDR, PCD8544_SCLK);
    sbit(PCD8544_DDR, PCD8544_MOSI);
    sbit(PCD8544_DDR, PCD8544_DC);
    sbit(PCD8544_DDR, PCD8544_CS);
    sbit(PCD8544_DDR, PCD8544_RESET);    

    // reset LCD registers
    cbit(PCD8544_PORT, PCD8544_RESET);
    _delay_ms(100);
    sbit(PCD8544_PORT, PCD8544_RESET);
    
    // setup LCD
    pcd8544_command(PCD8544_FUNCTIONSET | PCD8544_EXTENDED_INSTRUCTION_SET);
    pcd8544_command(PCD8544_SETBIAS | 0x4);
    pcd8544_command(PCD8544_SETVOP | 50); // contrast
    pcd8544_command(PCD8544_FUNCTIONSET); // implicit horizontal addressing
    pcd8544_command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}

void pcd8544_invalidate(pcd8544_t* this, uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax)
{
    if(xMin < this->xInvalidateMin) this->xInvalidateMin = xMin;
    if(yMin < this->yInvalidateMin) this->yInvalidateMin = yMin;
    if(xMax > this->xInvalidateMax) this->xInvalidateMax = xMax;
    if(yMax > this->yInvalidateMax) this->yInvalidateMax = yMax;
}

void pcd8544_clear(pcd8544_t* this)
{
    memset(this->buffer, 0, PCD8544_BUFFER_SIZE);
    pcd8544_invalidate(this, 0, 0, PCD8544_WIDTH-1, PCD8544_HEIGHT-1);
}

void pcd8544_setPixel(pcd8544_t* this, uint8_t x, uint8_t y, uint8_t color)
{
    if((x < 0) || (x >= PCD8544_WIDTH) || (y < 0) || (y >= PCD8544_HEIGHT))
        return; // invalid
    
    uint8_t bankNo = y / 8;
    uint8_t pixel = 1 << (y % 8);
    uint16_t index = x + bankNo * PCD8544_WIDTH;
    
    if(color > 0)
    {
        this->buffer[index] |= pixel;
    }
    else
    {
        this->buffer[index] &= ~pixel;
    }
    
    pcd8544_invalidate(this, x, y, x, y);
}



void pcd8544_line(pcd8544_t* this, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int8_t steep = (abs(y1-y0) > abs(x1-x0)) ? 1 : 0;
    if(steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }
    
    if(x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    
    int dErr = abs(y1-y0);
    int yStep = y0 > y1 ? -1 : 1;
    int dx = x1 - x0;
    
    int err = dx/2;
    int y = y0;
    
    for (uint8_t x = x0; x <= x1; x++)
    {
        if(steep) 
            pcd8544_setPixel(this, y, x, 255);
        else
            pcd8544_setPixel(this, x, y,255);
        err -= dErr;
        if(err<0)
        {
            y += yStep;
            err += dx;
        }
    }
}

void pcd8544_display(pcd8544_t* this)
{
    for(uint8_t bankNo = (this->yInvalidateMin/8); bankNo <= (this->yInvalidateMax/8); bankNo++)
    {
        int offset = bankNo * PCD8544_WIDTH;
        
        pcd8544_command(PCD8544_SETXADDR | this->xInvalidateMin);
        pcd8544_command(PCD8544_SETYADDR | bankNo);
        
        //digitalWrite(_dcPin, HIGH); // data mode on
        sbit(PCD8544_PORT, PCD8544_DC);
        //digitalWrite(_csPin, LOW);
        cbit(PCD8544_PORT, PCD8544_CS);
        for(uint8_t x = this->xInvalidateMin; x <= this->xInvalidateMax; x++)
        {
            char data = this->buffer[offset + x];
            pcd8544_spi_write(data);
        }
        //digitalWrite(_csPin, HIGH);
        sbit(PCD8544_PORT, PCD8544_CS);
    }
    
    // validate all
    this->xInvalidateMin = PCD8544_WIDTH-1;
    this->yInvalidateMin = PCD8544_HEIGHT-1;
    this->xInvalidateMax = 0;
    this->yInvalidateMax = 0;
}

void pcd8544_putChar(pcd8544_t* this, uint8_t row, uint8_t col, char symbol)
{			
    if((col < 0) || (col > 14) || (row < 0) || (row > 5))
	    return; // invalid
		
    for(uint8_t i=0; i<5; i++) // use memset?
	{
        int index = col * 6 + i + row * PCD8544_WIDTH;
        uint8_t data = pgm_read_byte(&(Font5x7[symbol - 0x20][i]));
	    this->buffer[index] = data;
    }

    pcd8544_invalidate(this, col*6, row*8, (col+1)*6-1, (row+1)*8-1);
}

void pcd8544_command(uint8_t cmd)
{
    //digitalWrite(_dcPin, LOW); // command mode on
    cbit(PCD8544_PORT, PCD8544_DC);
    //digitalWrite(_csPin, LOW);
    cbit(PCD8544_PORT, PCD8544_CS);
    pcd8544_spi_write(cmd);
    //digitalWrite(_csPin, HIGH);
    sbit(PCD8544_PORT, PCD8544_CS);
}

inline void pcd8544_spi_write(uint8_t data)
{
    for(uint8_t bit = 0x80; bit; bit = bit>>1) 
    {
        cbit(PCD8544_PORT, PCD8544_SCLK);
        if(data & bit)
        { 
            sbit(PCD8544_PORT, PCD8544_MOSI);
        }
        else 
        {
            cbit(PCD8544_PORT, PCD8544_MOSI);
        }
        sbit(PCD8544_PORT, PCD8544_SCLK);
    }
}