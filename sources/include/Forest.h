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
#include "SubComponent.h"
#include "ComponentLocator.h"
#include <functional>
#include <memory>

namespace Forests {
class PagedGeometry;
class PageLoader;
class GeometryPage;
}

namespace xio {
class Forest final : public ComponentLocator, public SubComponent {
 public:
  Forest();
  virtual ~Forest();
  void ProcessForest();
  void Update(float time) final;

 private:
  inline static std::function<float(float, float)> heigh_func_;
  std::vector<Forests::PagedGeometry *> pgeometry_;
  std::vector<Forests::PageLoader *> ploaders_;
  std::vector<Forests::GeometryPage *> gpages_;

 public:
  static void SetHeighFunc(const std::function<float(float, float)> &heigh_func) {
    heigh_func_ = heigh_func;
  }
};
}