#ifndef _SCENE_H_
#define _SCENE_H_

#include <gl/glew.h>
#include "Drawable.h"
#include "Shape.h"
#include "Transform.h"
#include "Light.h"
#include "../shadelib/shader.h"
#include <vector>

using namespace::std;

class Scene : public Drawable
{
protected:
   vector<Drawable*> m_dynamicObjects;
   vector<Drawable*> m_staticObjects;

   vector<Light*> m_lights;

   // camera transform?
   Vec3f m_cameraLoc;

   

public:

   Transform* m_transform;

   Shader* activeShader;


   Scene() : m_transform(NULL),
             m_cameraLoc(100.0f,300.0f,300.0f)
   {}
   ~Scene();

   void setupCamera();
   void setupLights();
   void renderScene();

   void addShape(Shape* shape, bool dynamic);
   void addLight(Light* l);
   void draw();
   void printElements();


};

#endif