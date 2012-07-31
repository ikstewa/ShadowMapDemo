#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Drawable.h"
#include "Material.h"
#include "../util/vec3.h"

using namespace::std;

/**
 * Object to represent a drawable shape.
 */
class Shape : public Drawable
{
   public:

      Material *m_material;

      vector<Vec3f> m_coordinates;
      vector<Vec3i> m_coordIndex;
      vector<Vec3f> m_textureCoordinates;

      Vec3f m_position;
      Vec3f m_rotation; // rotation along x,y,z axis applied in that order?

      // default initializer
      Shape() :   m_material(NULL),
                  vertices(NULL),
                  indices(NULL),
                  texCoords(NULL),
                  m_position(Vec3f()),
                  m_rotation(Vec3f())
      {}
      // draws the shape
      void draw();

      // finishes the coordinate and index vector copying to a static array
      // MUST BE CALLED AFTER LOADING BEFORE RENDERING!
      void finalizeData();

protected:

      GLfloat *vertices;
      GLuint *indices;
      GLfloat *texCoords;
      GLfloat *normals;


};


#endif
