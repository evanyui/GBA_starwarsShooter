#include "header.h"

unsigned short *videoBuffer = VIDEOBUFFERADDRESS;

void setPixel(int r, int c, u16 color)
{
	videoBuffer[OFFSET(r, c, 240)] = color;
}

void drawRect(int row, int col, int h, int w, u16 color)
{
	int r;
	for(r=0; r<h; r++)
	{
		// for(c=0; c<w; c++)
		// {
		// 	setPixel(row+r, col+c, color);
		// }
		DMA[3].src = &color;
		DMA[3].dst = videoBuffer +OFFSET(row+r, col, 240);
		DMA[3].cnt = w | DMA_ON | DMA_SOURCE_FIXED;
	}
}

//not used
void drawHollowRect(int row, int col, int h, int w, u16 color) 
{
	int c, r;
	for(c=0; c<w; c++)
	{
		setPixel(row, col+c, color);
		setPixel(row+h, col+c, color);
	}
	for(r=0; r<h; r++)
	{
		setPixel(row+r, col, color);
		setPixel(row+r, col+w, color);
	}
}

void drawImage3(int r, int c, int width, int height, const u16* image)
{
	for(int i = 0; i < width; ++i) {
		for(int j = 0; j < height; ++j) {
			videoBuffer[(r+i) + 240*(j+c)] = image[i + width*j];
		}
	}
}