#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>
#include <stdlib.h>

class Shader
{

public:
    Shader();

    virtual bool attach();

    bool addVert(const char*);
    bool addFrag(const char*);
    bool build();

    void printProgramInfoLog();
    void printShaderInfoLog(GLuint);


    GLuint shaderProg;


};

#endif