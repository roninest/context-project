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

#include "Compositors.h"
#include "Exception.h"
#include "ConfiguratorJson.h"
#include "DotSceneLoaderB.h"

namespace Context {

class GBufferSchemeHandler : public Ogre::MaterialManager::Listener {
 public:
  GBufferSchemeHandler() {
    mGBufRefMat = Ogre::MaterialManager::getSingleton().getByName("Context/gbuffer");
    mGBufRefMat->load();

    mGBufRefMat2 = Ogre::MaterialManager::getSingleton().getByName("Context/gbuffer_disable");
    mGBufRefMat2->load();
  }

  Ogre::Technique *handleSchemeNotFound(unsigned short schemeIndex,
                                        const Ogre::String &schemeName,
                                        Ogre::Material *originalMaterial,
                                        unsigned short lodIndex,
                                        const Ogre::Renderable *rend) final {
    auto *pass = originalMaterial->getTechnique(0)->getPass(0);
    int alpha_rejection = static_cast<int>(pass->getAlphaRejectValue());
    Ogre::Technique *gBufferTech = originalMaterial->createTechnique();

    if (pass->getNumTextureUnitStates() > 0 && alpha_rejection > 0) {
      gBufferTech->setSchemeName(schemeName);
      Ogre::Pass *gbufPass = gBufferTech->createPass();
      *gbufPass = *mGBufRefMat2->getTechnique(0)->getPass(0);
    } else {
      gBufferTech->setSchemeName(schemeName);
      Ogre::Pass *gbufPass = gBufferTech->createPass();
      *gbufPass = *mGBufRefMat->getTechnique(0)->getPass(0);
    }

    return gBufferTech;
  }

 private:
  Ogre::MaterialPtr mGBufRefMat;
  Ogre::MaterialPtr mGBufRefMat2;
};

//----------------------------------------------------------------------------------------------------------------------
class DepthSchemeHandler : public Ogre::MaterialManager::Listener {
 public:
  DepthSchemeHandler() {
    mGBufRefMat = Ogre::MaterialManager::getSingleton().getByName("Context/depth_alpha");
    mGBufRefMat->load();

    mGBufRefMat2 = Ogre::MaterialManager::getSingleton().getByName("Context/depth");
    mGBufRefMat2->load();
  }

  Ogre::Technique *handleSchemeNotFound(unsigned short schemeIndex,
                                        const Ogre::String &schemeName,
                                        Ogre::Material *originalMaterial,
                                        unsigned short lodIndex,
                                        const Ogre::Renderable *rend) final {

    auto *pass = originalMaterial->getTechnique(0)->getPass(0);
    int alpha_rejection = static_cast<int>(pass->getAlphaRejectValue());

    if (pass->getNumTextureUnitStates() > 0 && alpha_rejection > 0) {
      Ogre::Technique *gBufferTech = originalMaterial->createTechnique();
      gBufferTech->setSchemeName(schemeName);
      Ogre::Pass *gbufPass = gBufferTech->createPass();
      *gbufPass = *mGBufRefMat->getTechnique(0)->getPass(0);

      auto *texPtr2 = gbufPass->getTextureUnitState("BaseColor");
      texPtr2->setContentType(Ogre::TextureUnitState::CONTENT_NAMED);
      texPtr2->setTextureFiltering(Ogre::TFO_NONE);

      auto texture_albedo = originalMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("Albedo");

      if (texture_albedo) {
        auto texture_name = texture_albedo->getTextureName();
        texPtr2->setTextureName(texture_name);
      }

      return gBufferTech;
    } else {
      Ogre::Technique *gBufferTech2 = originalMaterial->createTechnique();
      gBufferTech2->setSchemeName(schemeName);
      Ogre::Pass *gbufPass2 = gBufferTech2->createPass();
      *gbufPass2 = *mGBufRefMat2->getTechnique(0)->getPass(0);

      return gBufferTech2;
    }
  }

 private:
  Ogre::MaterialPtr mGBufRefMat;
  Ogre::MaterialPtr mGBufRefMat2;
};

Compositors Compositors::CompositorManagerSingleton;
//----------------------------------------------------------------------------------------------------------------------
Compositors *Compositors::GetSingletonPtr() {
  return &CompositorManagerSingleton;
}
//----------------------------------------------------------------------------------------------------------------------
Compositors &Compositors::GetSingleton() {
  return CompositorManagerSingleton;
}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) {

}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) {

}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::CreateMotionBlurCompositor() {
  /// Motion blur effect
  Ogre::CompositorPtr comp3 = Ogre::CompositorManager::getSingleton().create(
      "MotionBlur", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
  );
  Ogre::CompositionTechnique *t = comp3->createTechnique();
  {
    Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
    def->width = 0;
    def->height = 0;
    def->formatList.push_back(Ogre::PF_R8G8B8);
  }
  {
    Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
    def->width = 0;
    def->height = 0;
    def->formatList.push_back(Ogre::PF_R8G8B8);
  }
  {
    Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
    def->width = 0;
    def->height = 0;
    def->formatList.push_back(Ogre::PF_R8G8B8);
  }
  /// Render scene
  {
    Ogre::CompositionTargetPass *tp = t->createTargetPass();
    tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
    tp->setOutputName("scene");
  }
  /// Initialisation pass for sum texture
  {
    Ogre::CompositionTargetPass *tp = t->createTargetPass();
    tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
    tp->setOutputName("sum");
    tp->setOnlyInitial(true);
  }
  /// Do the motion blur
  {
    Ogre::CompositionTargetPass *tp = t->createTargetPass();
    tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
    tp->setOutputName("temp");
    {
      Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
      pass->setMaterialName("Ogre/Compositor/Combine");
      pass->setInput(0, "scene");
      pass->setInput(1, "sum");
    }
  }
  /// Copy back sum texture
  {
    Ogre::CompositionTargetPass *tp = t->createTargetPass();
    tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
    tp->setOutputName("sum");
    {
      Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
      pass->setMaterialName("Ogre/Compositor/Copyback");
      pass->setInput(0, "temp");
    }
  }
  /// Display result
  {
    Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
    tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
    {
      Ogre::CompositionPass *pass = tp->createPass(Ogre::CompositionPass::PT_RENDERQUAD);
      pass->setMaterialName("Ogre/Compositor/MotionBlur");
      pass->setInput(0, "sum");
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::Setup() {
  ConfiguratorJson::GetSingleton().Assign(graphics_shadows_enable_, "graphics_shadows_enable");
  ConfiguratorJson::GetSingleton().Assign(compositor_use_bloom_, "compositor_use_bloom");
  ConfiguratorJson::GetSingleton().Assign(compositor_use_ssao_, "compositor_use_ssao");
  ConfiguratorJson::GetSingleton().Assign(compositor_use_blur_, "compositor_use_blur");
  ConfiguratorJson::GetSingleton().Assign(compositor_use_hdr_, "compositor_use_hdr");
  ConfiguratorJson::GetSingleton().Assign(compositor_use_moution_blur_, "compositor_use_moution_blur");

  if (compositor_use_ssao_) {
    ssaog_buffer_scheme_handler_ = std::make_unique<GBufferSchemeHandler>();
    Ogre::MaterialManager::getSingleton().addListener(ssaog_buffer_scheme_handler_.get(), "GBuffer");
  }

  if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/Main"))
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/Main", true);
  else
    Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Context/Main compositor\n");

  if (compositor_use_ssao_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/GBuffer"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/GBuffer", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add ShadowReceiver compositor\n");
  }

  if (compositor_use_bloom_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/Bloom"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/Bloom", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");
  }

  if (compositor_use_ssao_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/SSAO"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/SSAO", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");

    for (int i = 0; i < 1; i++) {
      if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/FilterY"))
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/FilterY", true);
      else
        Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");

      if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/FilterX"))
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/FilterX", true);
      else
        Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Filter compositor\n");
    }
  }

  if (compositor_use_moution_blur_) {
    throw Exception("Motion blur not implemented!");
    CreateMotionBlurCompositor();

    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "MotionBlur"))
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "MotionBlur", true);
    else
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Modulate compositor\n");
  }

  std::string modulate_compositor = "Context/Modulate";

  modulate_compositor += compositor_use_bloom_ ? "/Bloom" : "";
  modulate_compositor += compositor_use_ssao_ ? "/SSAO" : "";
  modulate_compositor += compositor_use_hdr_ ? "/HDR" : "";

  if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, modulate_compositor))
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, modulate_compositor, true);
  else
    Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Modulate compositor\n");

  if (compositor_use_hdr_) {
    if (Ogre::CompositorManager::getSingleton().addCompositor(viewport_, "Context/HDR")) {
      Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, "Context/HDR", true);
    } else {
      Ogre::LogManager::getSingleton().logMessage("Context core:: Failed to add Modulate compositor\n");
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::Reset() {

}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::SetCompositor(const std::string &compositor_) {
  if (compositor_ != "none") {
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, current_post_, false);
    current_compositor_ = compositor_;
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, current_post_, true);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Compositors::SetPost(const std::string &post_) {
  if (post_ != "none") {
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, current_post_, false);
    current_post_ = post_;
    Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport_, current_post_, true);
  }
}

} //namespace Context