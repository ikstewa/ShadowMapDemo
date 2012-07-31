#include "X3dParser.h"
#include "tinyxml/tinyxml.h"
#include "../element/Transform.h"
#include "../texlib/TextureLibrary.h"

#include <map>

void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
   // skip delimiters at the beginning.
   string::size_type lastPos = str.find_first_not_of(delimiters, 0);
   // Find first "non-delimiter".
   string::size_type pos = str.find_first_of(delimiters, lastPos);

   while (string::npos != pos || string::npos != lastPos)
   {
      // Found a token, add it to the vector
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
   }
}

Vec3f vectorToVec3(vector<string>& tokens)
{
   return Vec3f(atof(tokens[0].c_str()), atof(tokens[1].c_str()), atof(tokens[2].c_str()));
}

/**
 * *XML FORMAT*
 *
 * X3D
 *    Scene
 *       Transform
 *          Shape
 *             Appearance
 *                Material
 *             IndexFaceSet
 *                Coordinate
 *          ...
 */
Scene* X3dParser::parseFile(string filename)
{
   // Model transform
   Transform *transform;
   // Scene
   Scene *scene;
   // map of materials
   map<string, Material*> materialMap;


   printf("PARSING FILE %s\n", filename.c_str());

   TiXmlDocument doc(filename.c_str());
   if (doc.LoadFile(TIXML_ENCODING_UTF8))
   {
      // Grab the X3D element
      TiXmlHandle hRoot(doc.FirstChild("X3D"));
      TiXmlElement *x3d = hRoot.Element();

      // Grab the Scene element
      TiXmlElement *pXmlScene = x3d->FirstChildElement();
      if (!pXmlScene)
      {
         fprintf(stderr, "ERROR: X3dParser: Scene element not found!\n");
         return NULL;
      }
      scene = new Scene();

      // Grab the Transform element
      TiXmlElement *pXmlTrans = pXmlScene->FirstChildElement();
      string elemTag(pXmlTrans->Value());
      if (elemTag.compare("Transform") != 0)
      {
         // try to find the WorldInfo tag first
         if (elemTag.compare("WorldInfo") != 0)
         {
            fprintf(stderr, "ERROR: X3dParser: Transform not found!\n");
            return NULL;
         }
         pXmlTrans = pXmlTrans->NextSiblingElement();
      }
      // save the transform
      vector<string> rotTokens;
      vector<string> transTokens;
      tokenize(pXmlTrans->Attribute("rotation"), rotTokens, " ");
      tokenize(pXmlTrans->Attribute("translation"), transTokens, " ");

      transform = new Transform(vectorToVec3(rotTokens), vectorToVec3(transTokens));

      // loop through all the shapes
      for ( TiXmlElement *pElem = pXmlTrans->FirstChildElement();
            pElem;
            pElem = pElem->NextSiblingElement() )
      {
         std::string type = pElem->Value();
         //printf("XML Element: %s %s\n", type.c_str(), pElem->Attribute("DEF"));

         if (!type.compare("Shape"))
         {
            Shape* shape = new Shape();
            shape->id = pElem->Attribute("DEF");

            // Appearance element
            // Attribute returns 0 when no attribute found. Fix for NPE with string creation.
            TiXmlElement *pXmlAppearance = pElem->FirstChildElement();
            const char* a = pXmlAppearance->Attribute("DEF");
            string materialName = a ? a : "";

            // contains a new material
            if (materialName.length() > 0)
            {
               string attribs;
               vector<string> attribTokens;

               // Grab the material element
               TiXmlElement *pXmlMaterial = pXmlAppearance->FirstChildElement();
               Material* material = new Material();
               material->id = materialName;

               // ambient intensity
               pXmlMaterial->QueryFloatAttribute("ambientIntensity", &(material->m_ambientIntensity));

               // diffuse color
               attribs = pXmlMaterial->Attribute("diffuseColor");
               attribTokens.clear();
               tokenize(attribs, attribTokens, " ");
               material->m_diffuseColor = vectorToVec3(attribTokens);

               // emissive color
               attribs = pXmlMaterial->Attribute("emissiveColor");
               attribTokens.clear();
               tokenize(attribs, attribTokens, " ");
               material->m_emissiveColor = vectorToVec3(attribTokens);

               // shininess
               pXmlMaterial->QueryFloatAttribute("shininess", &(material->m_shininess));

               // specular color
               attribs = pXmlMaterial->Attribute("specularColor");
               attribTokens.clear();
               tokenize(attribs, attribTokens, " ");
               material->m_specularColor = vectorToVec3(attribTokens);

               // transparency
               pXmlMaterial->QueryFloatAttribute("transparency", &(material->m_transparency));

               // Check for a texture
               pXmlMaterial = pXmlMaterial->NextSiblingElement();
               if (pXmlMaterial && string(pXmlMaterial->Value()).compare("ImageTexture") == 0)
               {
                  // load the texture
                  vector<string> tokens;
                  string texFile = pXmlMaterial->Attribute("url");
                  tokenize(texFile, tokens, "\\");
                  material->m_texID = TextureLibrary::getInstance()->Load(tokens[tokens.size()-1]);
               }


               // set the shape material
               shape->m_material = material;

               // add to the Material map
               materialMap[material->id] = material;
            }
            // using a previously defined material
            else
            {
               materialName = pXmlAppearance->Attribute("USE");

               shape->m_material = materialMap[materialName];
            }

            // IndexedFaceSet element
            TiXmlElement *pXmlFaceSet = pXmlAppearance->NextSiblingElement();
            vector<string> coordArray;
            string coord;
            // coord index
            coord = pXmlFaceSet->Attribute("coordIndex");
            tokenize(coord, coordArray, ", ");
            // loop through all the coord index. <tri1, tri2, tri3, -1>
            for (unsigned int i = 0; i < coordArray.size(); i += 4)
            {
               // ignore the 4th index
               shape->m_coordIndex.push_back(Vec3i(atoi(coordArray[i].c_str()),
                                                   atoi(coordArray[i+1].c_str()),
                                                   atoi(coordArray[i+2].c_str())));
            }
            // coordinates
            TiXmlElement *pXmlCoordinate = pXmlFaceSet->FirstChildElement();
            coord = pXmlCoordinate->Attribute("point");
            coordArray.clear();
            tokenize(coord, coordArray, ", ");
            for (unsigned int i = 0; i < coordArray.size(); i += 3)
            {
               shape->m_coordinates.push_back( Vec3f(atof(coordArray[i].c_str()),
                                                     atof(coordArray[i+1].c_str()),
                                                     atof(coordArray[i+2].c_str())) );
            }

            // Look for a texture coordinate
            pXmlCoordinate = pXmlCoordinate->NextSiblingElement();
            if (pXmlCoordinate && string(pXmlCoordinate->Value()).compare("TextureCoordinate") == 0)
            {
               printf("Texture coordinate found!\n");
               coord = pXmlCoordinate->Attribute("point");
               coordArray.clear();
               tokenize(coord, coordArray, ", ");
               for (unsigned int i = 0; i < coordArray.size(); i += 2)
               {
                  shape->m_textureCoordinates.push_back(
                     Vec3f(atof(coordArray[i].c_str()),
                           atof(coordArray[i+1].c_str()),
                           0.0 ));
               }
            }


            // finalize the shape.. no more changes
            shape->finalizeData();
            // add the shape to the scene
            scene->addShape(shape, true);
         }
         else
         {
            fprintf(stderr, "ERROR: X3dParser: Unexpected element type!\n");
            return NULL;
         }

      }

      // return the scene
      return scene;
   }
   else
   {
      fprintf(stderr, "ERROR: X3dParser: Could not open file!?\n");
   }

   return NULL;
}


