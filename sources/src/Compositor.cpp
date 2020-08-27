/*
MIT License

Copyright (c) 2020 Andrey Vasiliev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "pcheader.h"

#include "Compositor.h"
#include "Exception.h"
#include "CompositorHelper.h"

namespace Context {
Compositor::~Compositor() noexcept {}
//----------------------------------------------------------------------------------------------------------------------
Compositor::Compositor() {  bool graphics_shadows_enable_ = true;
  bool compositor_use_bloom_ = false;
  bool compositor_use_ssao_ = false;
  bool compositor_use_hdr_ = false;

  if (compositor_use_ssao_) {
    ssaog_buffer_scheme_handler_ = new GBufferSchemeHandler();
    Ogre::MaterialManager::getSingleton().addListener(ssaog_buffer_scheme_handler_, "GBuffer");
  }
    auto *scene = Ogre::Root::getSingleton().getSceneManager("Default");
    auto *camera = scene->getCamera("Default");
    auto *viewport = camera->getViewport();

  if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/Main"))
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/Main", true);
  else
    Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Context/Main compositor\n");

  if (compositor_use_ssao_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/GBuffer"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/GBuffer", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add ShadowReceiver compositor\n");
  }

  if (compositor_use_bloom_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/Bloom"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/Bloom", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");
  }

  if (compositor_use_ssao_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/SSAO"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/SSAO", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");

    for (int i = 0; i < 1; i++) {
      if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/FilterY"))
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/FilterY", true);
      else
        Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");

      if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/FilterX"))
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/FilterX", true);
      else
        Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");
    }
  }

  std::string modulate_compositor = "Context/Modulate";

  modulate_compositor += compositor_use_bloom_ ? "/Bloom" : "";
  modulate_compositor += compositor_use_ssao_ ? "/SSAO" : "";
  modulate_compositor += compositor_use_hdr_ ? "/HDR" : "";

  if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, modulate_compositor))
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, modulate_compositor, true);
  else
    Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Modulate compositor\n");

  if (compositor_use_hdr_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Context/HDR")) {
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Context/HDR", true);
    } else {
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Modulate compositor\n");
    }
  }
}
}
