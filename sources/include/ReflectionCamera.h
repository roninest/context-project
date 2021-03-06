//MIT License
//
//Copyright (c) 2021 Andrei Vasilev
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once
#include <Ogre.h>
#include <vector>

namespace xio {
class ReflectionCamera final : public Ogre::RenderTargetListener {
 public:
  ReflectionCamera(Ogre::Plane plane, unsigned int tex_size);

  virtual ~ReflectionCamera();

 private:
  void preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) override;
  void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) override;
  void Clear_();
  void Init_(unsigned int tex_size);

  std::shared_ptr<Ogre::Texture> reflection_tex_;
  std::shared_ptr<Ogre::Texture> refraction_tex_;
  Ogre::Plane plane_;
  Ogre::Camera *rcamera_ = nullptr;
  Ogre::SceneManager *scene_ = nullptr;
  Ogre::ShadowTechnique technique_ = Ogre::SHADOWTYPE_NONE;
  bool pssm_shadows_ = false;
  float far_clip_distance = 0.0;


 public:
  const uint32_t SUBMERGED_MASK = 0x0F0;
  const uint32_t SURFACE_MASK = 0x00F;
  const uint32_t WATER_MASK = 0xF00;

//----------------------------------------------------------------------------------------------------------------------
  void SetPlane(Ogre::Plane plane);

  std::shared_ptr<Ogre::Texture> GetReflectionTex() const noexcept;

  std::shared_ptr<Ogre::Texture> GetRefractionTex() const noexcept;
};
}