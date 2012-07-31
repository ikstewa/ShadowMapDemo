#ifndef _X3DPARSER_H_
#define _X3DPARSER_H_

#include "../element/Scene.h"
#include <string>


class X3dParser
{

	public:
      Scene* parseFile(std::string filename);

};

#endif
