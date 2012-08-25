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

#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <vector>
#include <string>
#include <tr1/memory>

#include <png.h>
#include <GL/gl.h>

/// \namespace Wavefront
/// \brief Classes allowing for the support of animated 3D models
///
/// A group of related classes which allow for the displaying of Wavefront models and
/// the addition of animations to models
namespace Wavefront
{

/// \class WavefrontException
/// \brief Allows problem specific messages to be thrown by all components of the library
class WavefrontException : public std::exception
{
private:
  std::string message; ///< The message text to be reported

public:
  /// \brief Constructor
  /// \param message The text to be reported
  WavefrontException(std::string message)
  {
    this->message = message;
  }

  /// \brief Destructor
  virtual ~WavefrontException() throw()
  {

  }

  /// \brief Obtains the reported text from the exception
  /// \return The reported text
  virtual const char* what() const throw()
  {
    return message.c_str();
  }

};

/// \class Vector3
/// \brief A simple vector class to hold the 3 axis values
class Vector3
{
public:
  Vector3();
  Vector3(const Vector3& source);
  Vector3(float x, float y, float z);

  void setX(float x);
  void setY(float y);
  void setZ(float z);

  float getX();
  float getY();
  float getZ();

private:
  float x; ///< The value of the x axis
  float y; ///< The value of the y axis
  float z; ///< The value of the z axis

};

/// \class Util
/// \brief A general utility class contining a few useful functions
///
/// This class contains functions not specific to any other class but
/// still required
class Util
{
public:
  static void splitLineWhitespace(std::string line, std::vector<std::string>* splitLine);
  static void split(std::string input, char splitter, std::vector<std::string>* output);
  static void calcNormal(float v[3][3], float out[3]);
  static void reduceToUnit(float vector[3]);
  static Vector3 calcNormal(Vector3 a, Vector3 b, Vector3 c);

};

/// \class Texture
/// \brief Handles the loading and binding of PNG images
///
/// Stores a loaded image in the OpenGL format on the graphics card
/// ready for later use.
class Texture
{
private:
  static void freePngStruct(png_structp png_ptr);
  static void freeInfoStruct(png_infop info_ptr);
  static void freeRows(png_bytep* rows);
  static void freeData(unsigned char* data);
  static void freeTexture(GLuint* texture);

  GLuint texture; std::tr1::shared_ptr<GLuint> _texture; ///< A reference to the texture on the graphics card

public:
  Texture(std::string path);
  ~Texture();

  void bind();
  static void unbind();

};

/// \class Material
/// \brief Stores all information about a given material
class Material
{
public:
  Material();

  void setDiffuse(Vector3 diffuse);
  Vector3 getDiffuse();
  std::string getName();
  void setName(std::string name);
  Texture* getTexture();
  void setTexture(Texture* texture);

private:
  std::string name; ///< The name of the material from the .mtl file
  Vector3 ambient; ///< The RGB ambient value for the lighting
  Vector3 diffuse; ///< The RGB diffuse value for the color
  Vector3 specular; ///< The value for the specular property
  float transparency; ///< The transparency amount for the Material
  std::tr1::shared_ptr<Texture> texture; ///< The texture this material uses

};

class CollisionShape;

/// \class Face
/// \brief Represents a triangular face made up of 3 vectors
///
/// Provides information on a single face including data such as
/// the texture coordinates, maximum and minimum bounds.
/// Also provides intersect tests between itself or collision shapes.
class Face
{
public:
  Face();
  Face(Vector3 a, Vector3 b, Vector3 c);
  ~Face();

  void setA(Vector3 a);
  void setB(Vector3 b);
  void setC(Vector3 c);

  void setTa(Vector3 ta);
  void setTb(Vector3 tb);
  void setTc(Vector3 tc);

  Vector3* getA();
  Vector3* getB();
  Vector3* getC();

  Vector3 getTa();
  Vector3 getTb();
  Vector3 getTc();

  float getMaxX();
  float getMaxY();
  float getMaxZ();

  float getMinX();
  float getMinY();
  float getMinZ();

private:
  Vector3 a; ///< A coordinate making the first point on the face
  Vector3 b; ///< A coordinate making the second point on the face
  Vector3 c; ///< A coordinate making the third point on the face

  Vector3 ta; ///< The first texture coordinate of the face
  Vector3 tb; ///< The second texture coordinate of the face
  Vector3 tc; ///< The third texture coordinate of the face

};

/// \class MaterialGroup
/// \brief A group of faces within a part with the same material
///
/// Because VBOs are used, it is impossible to switch textures between
/// individual faces, for this reason the faces are sorted into groups
/// prior to being uploaded to the graphics card.
class MaterialGroup
{
private:
  Material* material; ///< The reference to the material all the faces in this group use
  std::vector<std::tr1::shared_ptr<Face> > faces; ///< The collection of faces with the same material
  std::tr1::shared_ptr<GLuint> _vertexBuffer; GLuint vertexBuffer; ///< The location of the buffer containing vertex positions on the graphics card
  std::tr1::shared_ptr<GLuint> _normalBuffer; GLuint normalBuffer; ///< The location of the buffer containing normals on the graphics card
  std::tr1::shared_ptr<GLuint> _colorBuffer; GLuint colorBuffer; ///< The location of the buffer containing colors on the graphics card
  std::tr1::shared_ptr<GLuint> _coordBuffer; GLuint coordBuffer; ///< The location of the buffer containing texture coordinates on the graphics card

public:
  static void deleteBuffer(GLuint* buffer);

  MaterialGroup();

  void setMaterial(Material* material);
  void addFace(std::tr1::shared_ptr<Face> face);
  void upload();
  void draw();
  std::vector<std::tr1::shared_ptr<Face> >* getFaces();

};

/// \class Part
/// \brief A single movable part from the whole model
///
/// Contains a collection of all the material groups ready to be drawn at certain
/// translations and/or rotations giving the appearance of an animating model.
class Part
{
private:
  std::vector<std::tr1::shared_ptr<MaterialGroup> > materialGroups; ///< The material groups making up the part
  std::string name; ///< The name of the part as specified in the .obj file
  Vector3 center; ///< The center of the part (required for rotations to pivot around part rather than the origin).

public:
  Part();

  void addMaterialGroup(std::tr1::shared_ptr<MaterialGroup> materialGroup);
  std::vector<std::tr1::shared_ptr<MaterialGroup> >* getMaterialGroups();
  void setName(std::string name);
  std::string getName();
  void upload();
  void draw();
  Vector3* getCenter();

};

/// \class Model
/// \brief Represents the model loaded from the file
///
/// Consists of the parts hierarchy with a store of materials used by the different parts.
/// This can then be drawn using the current OpenGL matrix transformation.
class Model
{
private:
  std::vector<std::tr1::shared_ptr<Material> > materials; ///< A list of materials used by the model
  std::vector<std::tr1::shared_ptr<Part> > parts; ///< A list of parts contained within the model

public:
  Model(std::string path);
  ~Model();
  void _loadMtl(std::string prefix, std::string fileName);

  void draw();
  std::vector<std::tr1::shared_ptr<Part> >* getParts();

};

/// \class Frame
/// \brief A single frame of animation
///
/// A class to contain the translation and rotation of the specified parts during
/// a frame.
class Frame
{
  friend class Animation;

private:
  std::vector<std::string> partNames; ///< The names of the parts to be modified
  std::vector<Vector3> translations; ///< The amounts to translate the parts by
  std::vector<Vector3> rotations; ///< The amounts to rotate the parts by

public:
  void add(std::string partName, Vector3 translation, Vector3 rotation);
  int getIndexOfPart(std::string partName);
  Vector3 getTranslation(int index);
  Vector3 getRotation(int index);

};

/// \class Animation
/// \brief An animation loaded from a .anm file
///
/// Contains all the specified translations, rotations and model references contained
/// in the animation file.
class Animation
{
private:
  std::vector<std::tr1::shared_ptr<Frame> > frames; ///< The frames in which to manipulate the model

public:
  Animation(std::string path);

  void performTransformation(std::string partName, int frame, bool undo);
  int getFrameCount();
  void interpolate(int passes, bool join);

};

/// \class AnimatedModel
/// \brief Binds a model with a group of animations
///
/// Allows seperate instances of animations and frame positions whilst
/// still utilizing the same model. Allows the addition and removal of
/// animations to the animated model.
class AnimatedModel
{
private:
  Model* model; ///< The model to be used to animate
  std::vector<Animation*> animations; ///< The list of attached animations
  std::vector<double> framePositions; ///< The frame position of the animations

public:
  AnimatedModel(Model* model);
  ~AnimatedModel();

  void addAnimation(Animation* animation);
  void removeAnimation(Animation* animation);
  bool animationExists(Animation* animation);

  void draw();
  void update(double timeDelta);

};

}

#endif

