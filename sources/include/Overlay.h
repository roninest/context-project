//MIT License
//
//Copyright (c) 2020 Andrey Vasiliev
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

#include "Component.h"
#include "Singleton.h"
#include "Gorilla.h"
#include <memory>

namespace Ogre {
class RenderTarget;
class Texture;
class SceneNode;
}

namespace xio {
class Overlay final : public Component, public Singleton<Overlay> {
 public:
  Overlay();
  virtual ~Overlay();

  void Create() final;
  void Reset() final;
  void Clean() final {}
  void Loop(float time) final;
  void Text(const std::string &str);

 private:
  Gorilla::Silverback *atlas_ = nullptr;
  Gorilla::Screen *screen_ = nullptr;
  Gorilla::Layer *layer_ = nullptr;
  Gorilla::Caption *caption_ = nullptr;
  Gorilla::Rectangle *rect_ = nullptr;
  std::unique_ptr<Gorilla::OgreConsole> console_;

 public:
  Gorilla::OgreConsole* GetConsole() {
    return console_.get();
  }
};
}