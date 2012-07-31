#include "Shape.h"
#include <algorithm>
#include "../texlib/TextureLibrary.h"

void Shape::draw()
{
   //printf("DRAWING SHAPE! %s\n", id.c_str());

   glPushMatrix();

   // set the texture
   m_material->attach();

   glTranslatef(m_position.x, m_position.y, m_position.z);

   // activate the vertex array
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);

   // normals
   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, 0, normals);

   if(texCoords)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
   }

   // draw the shape
   glDrawElements(GL_TRIANGLES, m_coordIndex.size()*3, GL_UNSIGNED_INT, indices);

   // deactive vertex array
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);

   TextureLibrary::getInstance()->Activate(TEXTURE_DISABLE);

   glPopMatrix();
}

void Shape::finalizeData()
{
   //---------------------------------------------------
   // Transform the object from world to model coords
   // Note: uses the center of mass
   Vec3d offset = Vec3d();
   for (unsigned int i = 0; i < m_coordinates.size(); i++)
   {
      offset.x += m_coordinates[i].x;
      offset.y += m_coordinates[i].y;
      offset.z += m_coordinates[i].z;
   }

   // set the offset
   offset /= m_coordinates.size();
   m_position = Vec3f(offset.x, offset.y, offset.z);

   // why can't i have a lambda function??
   for (unsigned int i = 0; i < m_coordinates.size(); i++)
   {
      m_coordinates[i] -= m_position;
   }

   /*
   // verify test
   offset = Vec3d();
   for (unsigned int i = 0; i < m_coordinates.size(); i++)
   {
      offset.x += m_coordinates[i].x;
      offset.y += m_coordinates[i].y;
      offset.z += m_coordinates[i].z;
   }
   Vec3d avg = offset;// / m_coordinates.size();
   if (avg.x != 0.0 || avg.y != 0.0 || avg.z != 0.0)
   {
      printf("ERROR: did not translate correctly [%f,%f,%f]\n", avg.x, avg.y, avg.z);
   }
   */


   //--------------------------------------------------------
   // Convert the vectors to GLfloat arrays for vertex array
   vertices = new GLfloat[m_coordinates.size()*3];
   indices = new GLuint[m_coordIndex.size()*3];
   if (m_textureCoordinates.size() > 0)
   {
      texCoords = new GLfloat[m_textureCoordinates.size()*2];
   }

   // Vertices
   for (unsigned int i = 0; i < m_coordinates.size(); i++)
   {
      vertices[i*3] = m_coordinates[i].x;
      vertices[i*3+1] = m_coordinates[i].y;
      vertices[i*3+2] = m_coordinates[i].z;
   }

   // Index set
   for (unsigned int i = 0; i < m_coordIndex.size(); i++)
   {
      indices[i*3] = (m_coordIndex[i].x);
      indices[i*3+1] = (m_coordIndex[i].y);
      indices[i*3+2] = (m_coordIndex[i].z);
   }

   // Texture Coordinates
   for (unsigned int i = 0; i < m_textureCoordinates.size(); i++)
   {
      texCoords[i*2] = m_textureCoordinates[i].x;
      texCoords[i*2+1] = m_textureCoordinates[i].y;
      // z ignored
   }

   //--------------------------------------------------------------------------
   // generate the normals!
   normals = new GLfloat[m_coordinates.size()*3];
   memset(normals, 0, m_coordinates.size()*3*sizeof(GLfloat));
   for (unsigned int i = 0; i < m_coordIndex.size(); i++)
   {
      Vec3i idx = m_coordIndex[i];
      Vec3f n = m_coordinates[idx.x].getSurfNorm(m_coordinates[idx.y], m_coordinates[idx.z]);

      // three vertices
      normals[idx.x*3] = n.x;
      normals[idx.x*3+1] = n.y;
      normals[idx.x*3+2] = n.z;

      normals[idx.y*3] = n.x;
      normals[idx.y*3+1] = n.y;
      normals[idx.y*3+2] = n.z;

      normals[idx.z*3] = n.x;
      normals[idx.z*3+1] = n.y;
      normals[idx.z*3+2] = n.z;
   }
}

