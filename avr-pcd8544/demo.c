/*
 * demo.c
 *
 * Created: oakio
 */ 
#include "pcd8544.h"

unsigned char black = PCD8544_BLACK;
unsigned char white = PCD8544_WHITE;

const char* text = "Hello world!";

int main(void)
{
    pcd8544_t lcd;
    pcd8544_begin(&lcd);

    while(1)
    {
        pcd8544_clear(&lcd);
        for (int k=0;k<6;k++)
        {        
            int i=0;
            while(*(text+i))
            {
                char symbol = *(text+i);
                pcd8544_putChar(&lcd, k, 1+i, symbol);
                pcd8544_display(&lcd);
                _delay_ms(100);
                i++;
            }
        }

        for(uint8_t k=1;k<8;k++)
        {
            pcd8544_clear(&lcd);
            for(uint8_t x=0;x<PCD8544_WIDTH;x+=k)
                for(uint8_t y=0;y<PCD8544_HEIGHT;y+=k)
                    pcd8544_setPixel(&lcd, x, y, black);
            pcd8544_display(&lcd);
            _delay_ms(300);
        }
        
        pcd8544_clear(&lcd);
        for(uint8_t x=0; x<PCD8544_WIDTH; x++)
        {
            for(uint8_t y=0;y<PCD8544_HEIGHT;y++)
                pcd8544_setPixel(&lcd, x, y, black);
            pcd8544_display(&lcd);
            
            _delay_ms(30);

            // smart clear
            for(uint8_t y=0;y<PCD8544_HEIGHT;y++)
                pcd8544_setPixel(&lcd, x, y, white);
            pcd8544_display(&lcd);
        }
        
        pcd8544_clear(&lcd);
        for(uint8_t y=0;y<PCD8544_HEIGHT;y++)
        {
            for(uint8_t x=0;x<PCD8544_WIDTH;x++)
                pcd8544_setPixel(&lcd, x, y, black);
            pcd8544_display(&lcd);
            
            _delay_ms(30);
            
            // smart clear
            for(uint8_t x=0;x<PCD8544_WIDTH;x++)
                pcd8544_setPixel(&lcd, x, y, white);
            pcd8544_display(&lcd);
        }
    }
}