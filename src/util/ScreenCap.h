#ifndef __SCREEN_CAP_H__
#define __SCREEN_CAP_H__

#include "Image.h"

static int savedImages = 0;

int tgaSaveSeries(char *filename, short int width, short int height, unsigned char pixelDepth, unsigned char* imageData)
{
   char *newFilename;
   int status;

   newFilename = (char *)malloc(sizeof(char) * strlen(filename)+8);
   sprintf(newFilename, "%s%d.tga", filename, savedImages);

   status = Image::SaveImage(newFilename, width, height, imageData);

   if (status)
      savedImages++;
   free(newFilename);
   return 1;
}


int tgaGrabScreenSeries(char *filename, int xmin, int ymin, int xmax, int ymax)
{
   int w, h;
   unsigned char * imageData;

   w = xmax - xmin;
   h = ymax - ymin;

   imageData = (unsigned char *)malloc(sizeof(unsigned char) * w * h * 3);

   glReadPixels(xmin, ymin, xmax, ymax, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)imageData);

   int retVal = tgaSaveSeries(filename, w, h, 24, imageData);

   free(imageData);
   return retVal;
}


#endif