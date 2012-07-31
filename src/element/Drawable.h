#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

#include <stdio.h>
#include <string>


/**
 * Represents a drawable object.
 *
 * Currently just an interface for a draw method :)
 */
class Drawable
{
public:

   std::string id;

   virtual void draw()
   {
      printf("ERROR: Overwrite my function bitch!\n");
   }
};
#endif