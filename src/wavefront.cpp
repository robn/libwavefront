/*********************************************************************************
 *
 * Copyright (c) 2012, Sanguine Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <tr1/functional>

#include <GL/glew.h>

#include <wavefront.h>

namespace Wavefront
{

/// \brief Load the model from a file
/// \param path The path of the .obj model to load
Model::Model(std::string path)
{
  int fileNameStart = -1;
  std::string line;
  std::vector<std::string> splitLine;
  std::vector<std::string> splitParameter;
  std::ifstream file(path.c_str());

  std::vector<Vector3> vertices;
  std::vector<Vector3> vertexTextures;

  std::tr1::shared_ptr<Part> part;
  std::tr1::shared_ptr<MaterialGroup> materialGroup;
  std::tr1::shared_ptr<Face> face;

  glewInit();

  if(file.is_open() == false)
  {
    throw WavefrontException("Failed to open \"" + path + "\"");
  }

  materials.push_back(std::tr1::shared_ptr<Material>(new Material()));
  materials.at(0)->setName("Default");
  materials.at(0)->setDiffuse(Vector3(1, 1, 1));

  while(file.eof() == false)
  {
    getline(file, line);
    splitLine.clear();
    Util::splitLineWhitespace(line, &splitLine);

    if(splitLine.size() < 1)
    {
      continue;
    }

    if(splitLine.at(0) == "mtllib")
    {
      fileNameStart = -1;

      for(int i = path.length() - 1; i >= 0; i--)
      {
        if(path[i] == '\\' || path[i] == '/')
        {
          fileNameStart = i;
          break;
        }
      }

      if(fileNameStart == -1)
      {
        _loadMtl("", splitLine.at(1));
      }
      else
      {
        _loadMtl(path.substr(0, fileNameStart), splitLine.at(1));
      }
    }

    if(splitLine.at(0) == "v")
    {
      //std::cout << "Vertex" << " " << splitLine.at(1) << " " << splitLine.at(2) << " " << splitLine.at(3) << std::endl;
      vertices.push_back(Vector3());
      vertices.at(vertices.size() - 1).setX(atof(splitLine.at(1).c_str()));
      vertices.at(vertices.size() - 1).setY(atof(splitLine.at(2).c_str()));
      vertices.at(vertices.size() - 1).setZ(atof(splitLine.at(3).c_str()));
    }
    else if(splitLine.at(0) == "vt")
    {
      //std::cout << "VertexTexture" << " " << splitLine.at(1) << " " << splitLine.at(2) << " " << splitLine.at(3) << std::endl;
      vertexTextures.push_back(Vector3());
      vertexTextures.at(vertexTextures.size() - 1).setX(atof(splitLine.at(1).c_str()));
      vertexTextures.at(vertexTextures.size() - 1).setY(-atof(splitLine.at(2).c_str()));

      if(splitLine.size() > 3)
      {
        vertexTextures.at(vertexTextures.size() - 1).setZ(atof(splitLine.at(3).c_str()));
      }
    }
    else if(splitLine.at(0) == "g" || splitLine.at(0) == "o")
    {
      //std::cout << "New Part: " << splitLine.at(1) << std::endl;
      part.reset(new Part());
      part->setName(splitLine.at(1));
      parts.push_back(part);
    }
    else if(splitLine.at(0) == "usemtl")
    {
      //std::cout << "Material Group: " << splitLine.at(1) << std::endl;
      materialGroup.reset(new MaterialGroup());

      for(int i = 0; i < materials.size(); i++)
      {
        if(materials.at(i)->getName() == splitLine.at(1))
        {
          materialGroup->setMaterial(materials.at(i).get());
        }
      }

      part->addMaterialGroup(materialGroup);
    }
    else if(splitLine.at(0) == "f")
    {
      // HACK
      if(part.get() == NULL)
      {
        part = std::tr1::shared_ptr<Part>(new Part());
        parts.push_back(part);
      }

      if(materialGroup.get() == NULL)
      {
        materialGroup = std::tr1::shared_ptr<MaterialGroup>(new MaterialGroup);
        materialGroup->setMaterial(materials.at(0).get());
        part->addMaterialGroup(materialGroup);
      }
      // ENDHACK

      face.reset(new Face());
      splitParameter.clear(); Util::split(splitLine.at(1), '/', &splitParameter);
      face->setA(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

      if(splitParameter.size() > 1 && splitParameter.at(1) != "")
      {
        face->setTa(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
      }

      splitParameter.clear(); Util::split(splitLine.at(2), '/', &splitParameter);
      face->setB(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

      if(splitParameter.size() > 1 && splitParameter.at(1) != "")
      {
        face->setTb(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
      }

      splitParameter.clear(); Util::split(splitLine.at(3), '/', &splitParameter);
      face->setC(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

      if(splitParameter.size() > 1 && splitParameter.at(1) != "")
      {
        face->setTc(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
      }

      materialGroup->addFace(face);

      if(splitLine.size() > 4)
      {
        face.reset(new Face());
        splitParameter.clear(); Util::split(splitLine.at(3), '/', &splitParameter);
        face->setA(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

        if(splitParameter.size() > 1 && splitParameter.at(1) != "")
        {
          face->setTa(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
        }

        splitParameter.clear(); Util::split(splitLine.at(4), '/', &splitParameter);
        face->setB(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

        if(splitParameter.size() > 1 && splitParameter.at(1) != "")
        {
          face->setTb(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
        }

        splitParameter.clear(); Util::split(splitLine.at(1), '/', &splitParameter);
        face->setC(vertices.at(atoi(splitParameter.at(0).c_str()) - 1));

        if(splitParameter.size() > 1 && splitParameter.at(1) != "")
        {
          face->setTc(vertexTextures.at(atoi(splitParameter.at(1).c_str()) - 1));
        }

        materialGroup->addFace(face);
      }
    }
  }

  for(int i = 0; i < parts.size(); i++)
  {
    parts.at(i)->upload();
  }
}

/// \brief The model destructor
Model::~Model(){}

/// \brief Load the additional .mtl file from path
/// \param prefix The directory containing the model
/// \param fileName The name of the .mtl file
void Model::_loadMtl(std::string prefix, std::string fileName)
{
  std::string line;
  std::ifstream file(std::string(prefix + "/" + fileName).c_str());
  std::vector<std::string> splitLine;
  std::tr1::shared_ptr<Material> material;
  Vector3 vector3;

  if(file.is_open() == false)
  {
    throw WavefrontException("Failed to open \"" + fileName + "\"");
  }

  while(file.eof() == false)
  {
    getline(file, line);
    splitLine.clear();
    Util::splitLineWhitespace(line, &splitLine);

    if(splitLine.size() < 1)
    {
      continue;
    }

    if(splitLine.at(0) == "newmtl")
    {
      material.reset(new Material());
      material->setName(splitLine.at(1));
      materials.push_back(material);
    }

    if(splitLine.at(0) == "Kd")
    {
      vector3 = Vector3();
      vector3.setX(atof(splitLine.at(1).c_str()));
      vector3.setY(atof(splitLine.at(2).c_str()));
      vector3.setZ(atof(splitLine.at(3).c_str()));
      material->setDiffuse(vector3);
    }

    if(splitLine.at(0) == "map_Kd")
    {
      material->setTexture(new Texture(prefix + "/" + splitLine.at(1)));
    }
  }
}

/// \brief Obtain a list of parts making up the model
/// \return A vector of parts
std::vector<std::tr1::shared_ptr<Part> >* Model::getParts()
{
  return &parts;
}

/// \brief Iterate through the parts and draw the model
void Model::draw()
{
  //GLboolean texture2d = false;
  //GLboolean colorMaterial = false;
  //GLboolean depthTest = false;

  //glGetBooleanv(GL_TEXTURE_2D, &texture2d);
  //glEnable(GL_TEXTURE_2D);
  //glGetBooleanv(GL_COLOR_MATERIAL, &colorMaterial);
  //glEnable(GL_COLOR_MATERIAL);
  //glGetBooleanv(GL_DEPTH_TEST, &depthTest);
  //glEnable(GL_DEPTH_TEST);

  for(int i = 0; i < parts.size(); i++)
  {
    parts.at(i)->draw();
  }

  //if(texture2d == true) { glEnable(GL_TEXTURE_2D); }
  //else { glDisable(GL_TEXTURE_2D); }

  //if(colorMaterial == true) { glEnable(GL_COLOR_MATERIAL); }
  //else { glDisable(GL_COLOR_MATERIAL); }

  //if(depthTest == true) { glEnable(GL_DEPTH_TEST); }
  //else { glDisable(GL_DEPTH_TEST); }
}

/// \brief Default constructor
Vector3::Vector3()
{
  x = 0;
  y = 0;
  z = 0;
}

/// \brief Copy constructor
/// \param source Reference to the Vector3 to be copied
Vector3::Vector3(const Vector3& source)
{
  x = source.x;
  y = source.y;
  z = source.z;
}

/// \brief Constructor
/// \param x The x coordinate of the Vector3
/// \param y The y coordinate of the Vector3
/// \param z The z coordinate of the Vector3
Vector3::Vector3(float x, float y, float z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

/// \brief Set the x position
/// \param x The new x coordinate
void Vector3::setX(float x)
{
  this->x = x;
}

/// \brief Set the y position
/// \param y The new y coordinate
void Vector3::setY(float y)
{
  this->y = y;
}

/// \brief Set the z position
/// \param z The new z coordinate
void Vector3::setZ(float z)
{
  this->z = z;
}

/// \brief Obtain the x position
/// \return The x position
float Vector3::getX()
{
  return x;
}

/// \brief Obtain the y position
/// \return The y position
float Vector3::getY()
{
  return y;
}

/// \brief Obtain the z position
/// \return The z position
float Vector3::getZ()
{
  return z;
}

/// \brief Default constructor
Face::Face()
{

}

/// \brief Constructor
/// \param a The first point of the face
/// \param b The second point of the face
/// \param c The third point of the face
Face::Face(Vector3 a, Vector3 b, Vector3 c)
{
  this->a = a;
  this->b = b;
  this->c = c;
}

/// \brief Destructor
Face::~Face()
{

}

/// \brief Set the first point
/// \param a The first point
void Face::setA(Vector3 a)
{
  this->a = a;
}

/// \brief Set the second point
/// \param b The second point
void Face::setB(Vector3 b)
{
  this->b = b;
}

/// \brief Set the third point
/// \param c The third point
void Face::setC(Vector3 c)
{
  this->c = c;
}

/// \brief Set the first texture coordinate
/// \param ta The new first texture coordinate
void Face::setTa(Vector3 ta)
{
  this->ta = ta;
}

/// \brief Set the second texture coordinate
/// \param tb The new second texture coordinate
void Face::setTb(Vector3 tb)
{
  this->tb = tb;
}

/// \brief Set the third texture coordinate
/// \param tc The new third texture coordinate
void Face::setTc(Vector3 tc)
{
  this->tc = tc;
}

/// \brief Obtain the first texture coordinate
/// \return The first texture coordinate
Vector3 Face::getTa()
{
  return ta;
}

/// \brief Obtain the second texture coordinate
/// \return The second texture coordinate
Vector3 Face::getTb()
{
  return tb;
}

/// \brief Obtain the third texture coordinate
/// \return The third texture coordinate
Vector3 Face::getTc()
{
  return tc;
}

/// \brief Obtain a pointer to the first point
/// \return A pointer to the first point
Vector3* Face::getA()
{
  return &a;
}

/// \brief Obtain a pointer to the second point
/// \return A pointer to the second point
Vector3* Face::getB()
{
  return &b;
}

/// \brief Obtain a pointer to the third point
/// \return A pointer to the third point
Vector3* Face::getC()
{
  return &c;
}

/// \brief Obtain the highest x position in the face
/// \return The highest x position
float Face::getMaxX()
{
  float maxX = a.getX();

  if(b.getX() > maxX)
  {
    maxX = b.getX();
  }

  if(c.getX() > maxX)
  {
    maxX = c.getX();
  }

  return maxX;
}

/// \brief Obtain the highest y position in the face
/// \return The highest y position
float Face::getMaxY()
{
  float maxY = a.getY();

  if(b.getY() > maxY)
  {
    maxY = b.getY();
  }

  if(c.getY() > maxY)
  {
    maxY = c.getY();
  }

  return maxY;
}

/// \brief Obtain the highest z position in the face
/// \return The highest z position
float Face::getMaxZ()
{
  float maxZ = a.getZ();

  if(b.getZ() > maxZ)
  {
    maxZ = b.getZ();
  }

  if(c.getZ() > maxZ)
  {
    maxZ = c.getZ();
  }

  return maxZ;
}

/// \brief Obtain the lowest x position in the face
/// \return The lowest x position
float Face::getMinX()
{
  float minX = a.getX();

  if(b.getX() < minX)
  {
    minX = b.getX();
  }

  if(c.getX() < minX)
  {
    minX = c.getX();
  }

  return minX;
}

/// \brief Obtain the lowest y position in the face
/// \return The lowest y position
float Face::getMinY()
{
  float minY = a.getY();

  if(b.getY() < minY)
  {
    minY = b.getY();
  }

  if(c.getY() < minY)
  {
    minY = c.getY();
  }

  return minY;
}

/// \brief Obtain the lowest z position in the face
/// \return The lowest z position
float Face::getMinZ()
{
  float minZ = a.getZ();

  if(b.getZ() < minZ)
  {
    minZ = b.getZ();
  }

  if(c.getZ() < minZ)
  {
    minZ = c.getZ();
  }

  return minZ;
}

/// \brief Default constructor
Part::Part()
{

}

/// \brief Send the part data to the graphics card
///
/// Iterate through the contained MaterialGroups and call their individual
/// upload function. Also the center of the part is calculated at this stage.
void Part::upload()
{
  float minX = 999999;
  float maxX = -999999;
  float minY = 999999;
  float maxY = -999999;
  float minZ = 999999;
  float maxZ = -999999;
  std::vector<std::tr1::shared_ptr<Face> >* faces;

  for(int i = 0; i < materialGroups.size(); i++)
  {
    faces = materialGroups.at(i)->getFaces();

    for(int a = 0; a < faces->size(); a++)
    {
      // HACK: getMaxX called more than needed
      if(faces->at(a)->getMaxX() > maxX)
      {
        maxX = faces->at(a)->getMaxX();
      }
      if(faces->at(a)->getMaxY() > maxY)
      {
        maxY = faces->at(a)->getMaxY();
      }
      if(faces->at(a)->getMaxZ() > maxZ)
      {
        maxZ = faces->at(a)->getMaxZ();
      }

      if(faces->at(a)->getMinX() < minX)
      {
        minX = faces->at(a)->getMinX();
      }
      if(faces->at(a)->getMinY() < minY)
      {
        minY = faces->at(a)->getMinY();
      }
      if(faces->at(a)->getMinZ() < minZ)
      {
        minZ = faces->at(a)->getMinZ();
      }
    }
  }

  center = Vector3((minX + maxX) / 2,
                  (minY + maxY) / 2,
                  (minZ + maxZ) / 2);

  //std::cout << "Center: " << center.getX() << " " << center.getY() << " " << center.getZ() << std::endl;
  //std::cout << name << " " << minX << " " << maxX << " " << minY << " " << maxY << " " << minZ << " " << maxZ << std::endl;

  for(int i = 0; i < materialGroups.size(); i++)
  {
    materialGroups.at(i)->upload();
  }
}

/// \brief Draw the Part
///
/// Iterate through the MaterialGroups and call their draw function
void Part::draw()
{
  for(int i = 0; i < materialGroups.size(); i++)
  {
    materialGroups.at(i)->draw();
  }
}

/// \brief Obtain the collection of MaterialGroups
/// \return A pointer to the MaterialGroups
std::vector<std::tr1::shared_ptr<MaterialGroup> >* Part::getMaterialGroups()
{
  return &materialGroups;
}

/// \brief Obtain the center of the Part
/// \return A pointer to the center Vector3
Vector3* Part::getCenter()
{
  return &center;
}

/// \brief Specify the name of the Part
/// \param name The new name of the Part
void Part::setName(std::string name)
{
  this->name = name;
}

/// \brief Obtain the name of the Part
/// \return The name of the Part
std::string Part::getName()
{
  return name;
}

/// \brief Add a new MaterialGroup to the Part
/// \param materialGroup The MaterialGroup to add
void Part::addMaterialGroup(std::tr1::shared_ptr<MaterialGroup> materialGroup)
{
  materialGroups.push_back(materialGroup);
}

/// \brief Default constructor
MaterialGroup::MaterialGroup()
{
  material = NULL;
}

/// \brief Set the Material for the MaterialGroup
/// \param material The new Material for the group
void MaterialGroup::setMaterial(Material* material)
{
  this->material = material;
}

/// \brief Add a new Face to the MaterialGroup
/// \param face The new Face to add
void MaterialGroup::addFace(std::tr1::shared_ptr<Face> face)
{
  faces.push_back(face);
}

/// \brief Obtain a list of Faces contained within the group
/// \return A pointer to the list of faces
std::vector<std::tr1::shared_ptr<Face> >* MaterialGroup::getFaces()
{
  return &faces;
}

/// \brief Convenience function to delete an OpenGL buffer
/// \param buffer The OpenGL buffer to delete
///
/// On Windows platforms, it seems that glDeleteBufferARB is implemented as
/// a macro and thus does not work with std::tr1::shared_ptr
void MaterialGroup::deleteBuffer(GLuint* buffer)
{
  glDeleteBuffersARB(1, buffer);
}

/// \brief Upload the buffer data to the graphics card
///
/// The buffer data stored in memory needs to be uploaded to the graphics card
/// so it can be used very quickly.
void MaterialGroup::upload()
{
  std::vector<float> vertices;
  std::vector<float> colors;
  std::vector<float> normals;
  std::vector<float> coords;
  Vector3 normal;

  glGenBuffersARB(1, &vertexBuffer);
  _vertexBuffer.reset(&vertexBuffer, std::tr1::bind(MaterialGroup::deleteBuffer, &vertexBuffer));
  glGenBuffersARB(1, &colorBuffer);
  _colorBuffer.reset(&colorBuffer, std::tr1::bind(MaterialGroup::deleteBuffer, &colorBuffer));
  glGenBuffersARB(1, &normalBuffer);
  _normalBuffer.reset(&normalBuffer, std::tr1::bind(MaterialGroup::deleteBuffer, &normalBuffer));
  glGenBuffersARB(1, &coordBuffer);
  _coordBuffer.reset(&coordBuffer, std::tr1::bind(MaterialGroup::deleteBuffer, &coordBuffer));

  for(int i = 0; i < faces.size(); i++)
  {
    normal = Util::calcNormal(*faces.at(i)->getA(), *faces.at(i)->getB(), *faces.at(i)->getC());
    vertices.push_back(faces.at(i)->getA()->getX());
    vertices.push_back(faces.at(i)->getA()->getY());
    vertices.push_back(faces.at(i)->getA()->getZ());
    colors.push_back(material->getDiffuse().getX());
    colors.push_back(material->getDiffuse().getY());
    colors.push_back(material->getDiffuse().getZ());
    colors.push_back(1);
    normals.push_back(normal.getX());
    normals.push_back(normal.getY());
    normals.push_back(normal.getZ());
    coords.push_back(faces.at(i)->getTa().getX());
    coords.push_back(faces.at(i)->getTa().getY());
    //coords.push_back(faces.at(i)->getTa().getZ());

    /*
    std::cout << "verts.push_back(" << faces.at(i)->getA().getX() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getA().getY() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getA().getZ() << ");" << std::endl;

    std::cout << "verts.push_back(" << faces.at(i)->getB().getX() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getB().getY() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getB().getZ() << ");" << std::endl;

    std::cout << "verts.push_back(" << faces.at(i)->getC().getX() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getC().getY() << "); ";
    std::cout << "verts.push_back(" << faces.at(i)->getC().getZ() << ");" << std::endl;

    std::cout << "coords.push_back(" << faces.at(i)->getTa().getX() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTa().getY() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTa().getZ() << ");" << std::endl;

    std::cout << "coords.push_back(" << faces.at(i)->getTb().getX() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTb().getY() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTb().getZ() << ");" << std::endl;

    std::cout << "coords.push_back(" << faces.at(i)->getTc().getX() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTc().getY() << "); ";
    std::cout << "coords.push_back(" << faces.at(i)->getTc().getZ() << ");" << std::endl;
    */

    vertices.push_back(faces.at(i)->getB()->getX());
    vertices.push_back(faces.at(i)->getB()->getY());
    vertices.push_back(faces.at(i)->getB()->getZ());
    colors.push_back(material->getDiffuse().getX());
    colors.push_back(material->getDiffuse().getY());
    colors.push_back(material->getDiffuse().getZ());
    colors.push_back(1);
    normals.push_back(normal.getX());
    normals.push_back(normal.getY());
    normals.push_back(normal.getZ());
    coords.push_back(faces.at(i)->getTb().getX());
    coords.push_back(faces.at(i)->getTb().getY());
    //coords.push_back(faces.at(i)->getTb().getZ());

    vertices.push_back(faces.at(i)->getC()->getX());
    vertices.push_back(faces.at(i)->getC()->getY());
    vertices.push_back(faces.at(i)->getC()->getZ());
    colors.push_back(material->getDiffuse().getX());
    colors.push_back(material->getDiffuse().getY());
    colors.push_back(material->getDiffuse().getZ());
    colors.push_back(1);
    normals.push_back(normal.getX());
    normals.push_back(normal.getY());
    normals.push_back(normal.getZ());
    coords.push_back(faces.at(i)->getTc().getX());
    coords.push_back(faces.at(i)->getTc().getY());
    //coords.push_back(faces.at(i)->getTc().getZ());
  }

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, colorBuffer);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, colors.size()*sizeof(float), &colors[0], GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, normals.size()*sizeof(float), &normals[0], GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, coordBuffer);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, coords.size()*sizeof(float), &coords[0], GL_STATIC_DRAW_ARB);
}

/// \brief Draw the previously uploaded data on the graphics card
void MaterialGroup::draw()
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  if(material->getTexture() != NULL)
  {
    material->getTexture()->bind();
  }
  else
  {
    Texture::unbind();
  }

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, colorBuffer);
  glColorPointer(4, GL_FLOAT, 0, NULL);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
  glNormalPointer(GL_FLOAT, 0, NULL);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, coordBuffer);
  glTexCoordPointer(2, GL_FLOAT, 0, NULL);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
  glVertexPointer(3, GL_FLOAT, 0, NULL);

  glDrawArrays(GL_TRIANGLES, 0, faces.size() * 3);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  Texture::unbind();
}

/// \brief Default constructor
Material::Material()
{
  // HACK
  diffuse.setX(1);
  diffuse.setY(1);
  diffuse.setZ(1);
  // ENDHACK
}

/// \brief Obtain the stored diffuse color from the material
/// \return The diffuse color RGB values
Vector3 Material::getDiffuse()
{
  return diffuse;
}

/// \brief Store an RGB diffuse color in the Material
/// \param diffuse The RGB values to store
void Material::setDiffuse(Vector3 diffuse)
{
  this->diffuse = diffuse;
}

/// \brief Set the name of the Material
/// \param name The new name
void Material::setName(std::string name)
{
  this->name = name;
}

/// \brief Obtain the Texture referenced by the Material
/// \return A pointer to the stored texture
Texture* Material::getTexture()
{
  return texture.get();
}

/// \brief Set the texture referenced by the Material
/// \param texture The new texture to reference
void Material::setTexture(Texture* texture)
{
  this->texture.reset(texture);
}

/// \brief Obtain the name of the material
/// \return The name of the material
std::string Material::getName()
{
  return name;
}

/// \brief Split the specified string by the specified delimeter
/// \param input The string to split
/// \param splitter The character to split by
/// \param output The array to populate
void Util::split(std::string input, char splitter, std::vector<std::string>* output)
{
  std::string current;

  for(int i = 0; i < input.length(); i++)
  {
    if(input[i] == splitter)
    {
      output->push_back(current);
      current = "";
    }
    else
    {
      current += input[i];
    }
  }

  if(current.length() > 0)
  {
    output->push_back(current);
  }
}

/// \brief Split the specified string by any type of whitespace
/// \param line The line to split
/// \param splitLine The array to populate by the split string
///
/// The whitespace to split by includes r, n, t and ' '
void Util::splitLineWhitespace(std::string line, std::vector<std::string>* splitLine)
{
  bool lastWasWhiteSpace = false;
  std::string current;

  for(int i = 0; i < line.length(); i++)
  {
    if(line[i] == '\n' || line[i] == '\r')
    {
      continue;
    }

    if(line[i] == ' ')
    {
      if(lastWasWhiteSpace == false)
      {
        splitLine->push_back(current);
        current = "";
        lastWasWhiteSpace = true;
      }
    }
    else
    {
      current += line[i];
      lastWasWhiteSpace = false;
    }
  }

  if(current.length() > 0)
  {
    splitLine->push_back(current);
  }
}

/// \brief Normalize the specified Vector3
/// \param vector The Vector3 to normalize
void Util::reduceToUnit(float vector[3])
{
  float length = 0;

  length = (float)sqrt((vector[0]*vector[0]) + (vector[1]*vector[1]) + (vector[2]*vector[2]));

  if(length == 0.0f)
  {
    length = 1.0f;
  }

  vector[0] /= length;
  vector[1] /= length;
  vector[2] /= length;
}

/// \brief Calculate the normal from the specified triangle
/// \param v The vertices representing the triangle
/// \param out The array in which to populate
void Util::calcNormal(float v[3][3], float out[3])
{
  float v1[3],v2[3];
  static const int x = 0;
  static const int y = 1;
  static const int z = 2;

  v1[x] = v[0][x] - v[1][x];
  v1[y] = v[0][y] - v[1][y];
  v1[z] = v[0][z] - v[1][z];

  v2[x] = v[1][x] - v[2][x];
  v2[y] = v[1][y] - v[2][y];
  v2[z] = v[1][z] - v[2][z];

  out[x] = v1[y]*v2[z] - v1[z]*v2[y];
  out[y] = v1[z]*v2[x] - v1[x]*v2[z];
  out[z] = v1[x]*v2[y] - v1[y]*v2[x];

  reduceToUnit(out);
}

/// \brief Calculate the normal from the specified triangle
/// \param a The Vector3 making up the first point of the triangle
/// \param b The Vector3 making up the second point of the triangle
/// \param c The Vector3 making up the thrid point of the triangle
/// \return The face normal as a Vector3
Vector3 Util::calcNormal(Vector3 a, Vector3 b, Vector3 c)
{
  Vector3 result;
  float v[3][3] = { 0 };
  float o[3] = { 0 };

  v[0][0] = a.getX();
  v[0][1] = a.getY();
  v[0][2] = a.getZ();

  v[1][0] = b.getX();
  v[1][1] = b.getY();
  v[1][2] = b.getZ();

  v[2][0] = c.getX();
  v[2][1] = c.getY();
  v[2][2] = c.getZ();

  calcNormal(v, o);

  result.setX(o[0]);
  result.setY(o[1]);
  result.setZ(o[2]);

  return result;
}

/// \brief Clear the data from the character array
/// \param data The pointer to the character array to clear
///
/// Because using c-style arrays, shared_ptr had difficulties with the deleter function
void Texture::freeData(unsigned char* data)
{
  delete[] data;
}

/// \brief Delete the texture stored on the graphics card
/// \param texture Reference of the texture to free
///
/// Because Windows seems to implement this function as a typedef or macro it
/// gave shared_ptr issues
void Texture::freeTexture(GLuint* texture)
{
  glDeleteTextures(1, texture);
}

/// \brief Free the specified PNG struct (from the heap)
/// \param png_ptr The pointer to the struct
void Texture::freePngStruct(png_structp png_ptr)
{
  png_destroy_read_struct(&png_ptr, NULL, NULL);
}

/// \brief Free the specified PNG info struct
/// \param info_ptr The pointer to free
void Texture::freeInfoStruct(png_infop info_ptr)
{
  png_structp temp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_destroy_read_struct(&temp, &info_ptr, NULL);
}

/// \brief Free the specified PNG byte rows
/// \param rows The pointer to rows to free
void Texture::freeRows(png_bytep* rows)
{
  int y = 0;

  while(true)
  {
    if(rows[y] == NULL)
    {
      break;
    }

    free(rows[y]);
    y++;
  }

  free(rows);
}

/// \brief Constructor
/// \param path The path of the texture to load
Texture::Texture(std::string path)
{
  int ctype = 0;
  int width = 0;
  int height = 0;
  int counter = 0;
  int number_of_passes = 0;
  png_byte color_type = 0;
  png_byte bit_depth = 0;
  png_byte header[8] = { 0 };
  std::tr1::shared_ptr<unsigned char> data;

  ctype = 3;
  FILE* _fp = NULL; std::tr1::shared_ptr<FILE> fp;
  png_structp _png_ptr; std::tr1::shared_ptr<png_structp> png_ptr;
  png_infop _info_ptr; std::tr1::shared_ptr<png_infop> info_ptr;
  std::tr1::shared_ptr<png_bytep> row_pointers;

  _fp = fopen(path.c_str(), "rb");

  if(_fp == NULL)
  {
    throw WavefrontException("Failed to open file \"" + path + "\"");
  }

  fp.reset(_fp, std::tr1::bind(fclose, std::tr1::placeholders::_1));
  fread(header, 1, 8, fp.get());

  if(png_sig_cmp(header, 0, 8))
  {
    throw WavefrontException("File \"" + path + "\" is not a recognised PNG file");
  }

  _png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if(_png_ptr == NULL)
  {
    throw WavefrontException("Failed to create PNG read struct");
  }

  png_ptr.reset(&_png_ptr, std::tr1::bind(Texture::freePngStruct, _png_ptr));
  _info_ptr = png_create_info_struct(_png_ptr);

  if(_info_ptr == NULL)
  {
    throw WavefrontException("Failed to create PNG info struct");
  }

  info_ptr.reset(&_info_ptr, std::tr1::bind(Texture::freeInfoStruct, _info_ptr));

  if(setjmp(png_jmpbuf(*png_ptr.get())))
  {
    throw WavefrontException("Failed to initialize IO");
  }

  png_init_io(*png_ptr.get(), fp.get());
  png_set_sig_bytes(*png_ptr.get(), 8);
  png_read_info(*png_ptr.get(), *info_ptr.get());
  width = png_get_image_width(*png_ptr.get(), *info_ptr.get());
  height = png_get_image_height(*png_ptr.get(), *info_ptr.get());
  color_type = png_get_color_type(*png_ptr.get(), *info_ptr.get());

  if(color_type == PNG_COLOR_TYPE_RGB)
  {
    ctype = 3;
  }
  else if(color_type == PNG_COLOR_TYPE_RGBA)
  {
    ctype = 4;
  }
  else
  {
    throw WavefrontException("Invalid color type");
  }

  bit_depth = png_get_bit_depth(*png_ptr.get(), *info_ptr.get());
  number_of_passes = png_set_interlace_handling(*png_ptr.get());
  png_read_update_info(*png_ptr.get(), *info_ptr.get());

  //std::cout << width << " " << height << std::endl;

  row_pointers.reset((png_bytep*)calloc(height + 1, sizeof(png_bytep)), std::tr1::bind(freeRows, std::tr1::placeholders::_1));

  for(int y = 0; y < height; y++)
  {
    row_pointers.get()[y] = (png_byte*)malloc(png_get_rowbytes(*png_ptr.get(), *info_ptr.get()));
  }

  //std::cout << "Debug: " << png_get_rowbytes(*png_ptr.get(), *info_ptr.get()) << std::endl;
  //std::cout << "Debug: " << width * 4 << std::endl;

  png_read_image(*png_ptr.get(), row_pointers.get());
  //data.reset(new unsigned char[width*height*4], std::tr1::bind(freeData, std::tr1::placeholders::_1));
  data.reset(new unsigned char[width*height*ctype], std::tr1::bind(freeData, std::tr1::placeholders::_1));

  for(int y = 0; y < height; y++)
  {
    //for(int x = 0; x < width*4; x++)
    for(int x = 0; x < width*ctype; x++)
    {
      data.get()[counter] = (unsigned char)row_pointers.get()[y][x];
      //std::cout << (int)data.get()[counter] << " ";
      counter++;
    }

    //std::cout << std::endl;
  }

  glGenTextures(1, &texture);
  _texture.reset(&texture, std::tr1::bind(Texture::freeTexture, &texture));
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if(ctype == 3)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.get());
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
  }
}

/// \brief Destructor
Texture::~Texture(){}

/// \brief Bind the texture so that the shader's sampler can use it
void Texture::bind()
{
  glBindTexture(GL_TEXTURE_2D, texture);
}

/// \brief Unbind the texture so that subsequent draws do not use the texture
void Texture::unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

/// \brief Constructor
/// \param path The path of the animation to load
Animation::Animation(std::string path)
{
  Frame* frame = NULL;
  std::ifstream file;
  std::string line;
  std::vector<std::string> splitLine;

  file.open(path.c_str());

  if(file.is_open() == false)
  {
    throw WavefrontException("Failed to open '" + path + "'");
  }

  while(file.eof() == false)
  {
    getline(file, line);
    splitLine.clear();
    Util::splitLineWhitespace(line, &splitLine);

    if(splitLine.size() < 1)
    {
      continue;
    }

    if(splitLine.at(0) == "f")
    {
      frames.push_back(std::tr1::shared_ptr<Frame>(new Frame()));
      frame = frames.at(frames.size() - 1).get();
      //std::cout << "Added frame" << std::endl;
    }

    if(splitLine.at(0) == "t")
    {
      frame->add(splitLine.at(1),
                 Vector3(atof(splitLine.at(2).c_str()), atof(splitLine.at(3).c_str()), atof(splitLine.at(4).c_str())),
                 Vector3(atof(splitLine.at(5).c_str()), atof(splitLine.at(6).c_str()), atof(splitLine.at(7).c_str())));
      //std::cout << "Added transition" << std::endl;
    }
  }
}

/// \brief Smooths and slows down the animation by interpolating rotations and positions between frames.
/// \param passes The number of new interpolated frames added between each of the original frames
/// \param join Set to true to smooth the translation between last and first frame
void Animation::interpolate(int passes, bool join)
{
  Frame* frame = NULL;
  Frame* nextFrame = NULL;
  Frame* newFrame = NULL;
  Vector3* rotation = NULL;
  Vector3* translation = NULL;
  Vector3* nextRotation = NULL;
  Vector3* nextTranslation = NULL;

  for(int p = 0; p < passes; p++)
  {

  for(int i = 0; i < frames.size(); i++)
  {
    frame = frames.at(i).get();

    if(i == frames.size() - 1)
    {
      if(join == false)
      {
        break;
        //continue;
      }

      nextFrame = frames.at(0).get();
    }
    else
    {
      nextFrame = frames.at(i + 1).get();
    }

    newFrame = new Frame();

    for(int j = 0; j < frame->partNames.size(); j++)
    {
      rotation = &frame->rotations.at(j);
      translation = &frame->translations.at(j);
      nextRotation = &nextFrame->rotations.at(j);
      nextTranslation = &nextFrame->translations.at(j);
      Vector3 newRotation((rotation->getX() + nextRotation->getX()) / 2,
                          (rotation->getY() + nextRotation->getY()) / 2,
                          (rotation->getZ() + nextRotation->getZ()) / 2);

      Vector3 newTranslation((translation->getX() + nextTranslation->getX()) / 2,
                             (translation->getY() + nextTranslation->getY()) / 2,
                             (translation->getZ() + nextTranslation->getZ()) / 2);

      newFrame->partNames.push_back(frame->partNames.at(j));
      newFrame->rotations.push_back(newRotation);
      newFrame->translations.push_back(newTranslation);
    }

    frames.insert(frames.begin() + i + 1, std::tr1::shared_ptr<Frame>(newFrame));
    i++;
  }

  }
}

/// \brief Use the specified part name and perform the matching translations and rotations on it
/// \param partName The name of the part to translate / rotate
/// \param frame The frame index of the translations and rotations
/// \param undo Unused
void Animation::performTransformation(std::string partName, int frame, bool undo)
{
  int partIndex = -1;
  Vector3 translation;
  Vector3 rotation;

  partIndex = frames.at(frame)->getIndexOfPart(partName);

  if(partIndex == -1)
  {
    return;
  }

  translation = frames.at(frame)->getTranslation(partIndex);
  rotation = frames.at(frame)->getRotation(partIndex);

  glTranslatef(translation.getX(), translation.getY(), translation.getZ());
  glRotatef(rotation.getZ(), 0, 0, 1);
  glRotatef(rotation.getY(), 0, 1, 0);
  glRotatef(rotation.getX(), 1, 0, 0);
  //usleep(50000);
}

/// \brief Obtain the amount of frames this Animation contains
/// \return The number of frames
int Animation::getFrameCount()
{
  return frames.size();
}

/// \brief Constructor
/// \param model The model to use when drawing the animations
AnimatedModel::AnimatedModel(Model* model)
{
  this->model = model;
}

/// \brief Destructor
AnimatedModel::~AnimatedModel()
{

}

/// \brief Simply iterates the current frame positions (or sets it back to 0 when it goes over the max frames)
///
/// This function should ideally be called based on a frame delta so not to be tied to the frame rate.
void AnimatedModel::update(double timeDelta)
{
  for(int a = 0; a < animations.size(); a++)
  {
    framePositions.at(a)+=timeDelta;

    if(framePositions.at(a) >= animations.at(a)->getFrameCount())
    {
      framePositions[a] = 0;
    }
  }
}

/// \brief Draws the attached model but first performs translations and rotations depending on animation state
void AnimatedModel::draw()
{
  //GLboolean texture2d = false;
  //GLboolean colorMaterial = false;
  //GLboolean depthTest = false;

  //glGetBooleanv(GL_TEXTURE_2D, &texture2d);
  //glEnable(GL_TEXTURE_2D);
  //glGetBooleanv(GL_COLOR_MATERIAL, &colorMaterial);
  //glEnable(GL_COLOR_MATERIAL);
  //glGetBooleanv(GL_DEPTH_TEST, &depthTest);
  //glEnable(GL_DEPTH_TEST);

  for(int i = 0; i < model->getParts()->size(); i++)
  {
    glPushMatrix();
    glTranslatef(model->getParts()->at(i)->getCenter()->getX(),
                 model->getParts()->at(i)->getCenter()->getY(),
                 model->getParts()->at(i)->getCenter()->getZ());

    for(int a = 0; a < animations.size(); a++)
    {
      animations.at(a)->performTransformation(model->getParts()->at(i)->getName(),
                                              framePositions.at(a),
                                              false);
    }

    glTranslatef(-model->getParts()->at(i)->getCenter()->getX(),
                 -model->getParts()->at(i)->getCenter()->getY(),
                 -model->getParts()->at(i)->getCenter()->getZ());

    model->getParts()->at(i)->draw();
    glPopMatrix();
  }

  //if(texture2d == true) { glEnable(GL_TEXTURE_2D); }
  //else { glDisable(GL_TEXTURE_2D); }

  //if(colorMaterial == true) { glEnable(GL_COLOR_MATERIAL); }
  //else { glDisable(GL_COLOR_MATERIAL); }

  //if(depthTest == true) { glEnable(GL_DEPTH_TEST); }
  //else { glDisable(GL_DEPTH_TEST); }
}

/// \brief Check to see whether the specified animation has already been added to the AnimatedModel
/// \param animation The Animation to check
/// \return True if the animation already exists
bool AnimatedModel::animationExists(Animation* animation)
{
  for(int i = 0; i < animations.size(); i++)
  {
    if(animations.at(i) == animation)
    {
      return true;
    }
  }

  return false;
}

/// \brief Add animation to the AnimatedModel
/// \param animation The Animation to add
void AnimatedModel::addAnimation(Animation* animation)
{
  for(int i = 0; i < animations.size(); i++)
  {
    if(animations.at(i) == animation)
    {
      return;
    }
  }

  animations.push_back(animation);
  framePositions.push_back(0);
}

/// \brief Remove animation from the AnimatedModel
/// \param animation The Animation to remove
void AnimatedModel::removeAnimation(Animation* animation)
{
  for(int i = 0; i < animations.size(); i++)
  {
    if(animations.at(i) == animation)
    {
      animations.erase(animations.begin() + i);
      framePositions.erase(framePositions.begin() + i);
      return;
    }
  }
}

/// \brief Add a new translation and rotation to the Frame
/// \param partName The name of the part to manipulate
/// \param translation The amount to move the part by
/// \param rotation The amount to rotate the part by
void Frame::add(std::string partName, Vector3 translation, Vector3 rotation)
{
  partNames.push_back(partName);
  translations.push_back(translation);
  rotations.push_back(rotation);
}

/// \brief Obtain the index of the part within the parts array
/// \param partName The pertname to use to perform the search
/// \return -1 if no part was found with the specified name
int Frame::getIndexOfPart(std::string partName)
{
  for(int i = 0; i < partNames.size(); i++)
  {
    if(partNames.at(i) == partName)
    {
      return i;
    }
  }

  return -1;
}

/// \brief Obtain the frame's translation based on a specified index
/// \param index The index of the translation to obtain
/// \return A vector containing the translation
Vector3 Frame::getTranslation(int index)
{
  return translations.at(index);
}

/// \brief Obtain the frame's rotation based on a specified index
/// \param index The index of the rotation to obtain
/// \return A vector containing the rotation
Vector3 Frame::getRotation(int index)
{
  return rotations.at(index);
}

}

