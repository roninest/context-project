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

#include "pcheader.hpp"

#include "GorillaOverlay.hpp"
#include "Application.hpp"

namespace Context {
GorillaOverlay GorillaOverlay::GorillaOverlaySingleton;
//----------------------------------------------------------------------------------------------------------------------
GorillaOverlay *GorillaOverlay::GetSingletonPtr() {
  return &GorillaOverlaySingleton;
}
//----------------------------------------------------------------------------------------------------------------------
GorillaOverlay &GorillaOverlay::GetSingleton() {
  return GorillaOverlaySingleton;
}
//----------------------------------------------------------------------------------------------------------------------
bool GorillaOverlay::frameRenderingQueued(const Ogre::FrameEvent &evt) {
  caption->text(std::to_string(Application::GetSingleton().GetFpsFrames()));
  return true;
}
//----------------------------------------------------------------------------------------------------------------------
void GorillaOverlay::preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) {
}
//----------------------------------------------------------------------------------------------------------------------
void GorillaOverlay::postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt) {
}
//----------------------------------------------------------------------------------------------------------------------
void GorillaOverlay::Setup() {
  mSilverback = new Gorilla::Silverback();
  mSilverback->loadAtlas("dejavu");
  mScreen = mSilverback->createScreen(ogre_viewport_, "dejavu");
  layer = mScreen->createLayer(0);
  Ogre::Real vpW = mScreen->getWidth(), vpH = mScreen->getHeight();

  // Create our drawing layer
  layer = mScreen->createLayer(0);
  rect = layer->createRectangle(0, 0, vpW, vpH);
  //rect->background_gradient(Gorilla::Gradient_Diagonal, Gorilla::rgb(98,0,63), Gorilla::rgb(255,180,174));
  rect->background_colour(Gorilla::rgb(0, 0, 0, 0));

  markup = layer->createMarkupText(9, 5, 5,
                                   "%@24%Overlay system\n%@14%Text here%@9%\nHello Fuckers");

//  caption = layer->createCaption(9, vpW - 55, 5, "9");
//  caption->width(50);
//  caption->align(Gorilla::TextAlign_Right);
//
//  caption = layer->createCaption(14, vpW - 55, 18, "14");
//  caption->width(50);
//  caption->align(Gorilla::TextAlign_Right);

  caption = layer->createCaption(24, vpW - 55, 66, "24");
  caption->width(0);
  caption->align(Gorilla::TextAlign_Right);
  caption->text("1488");
}
//----------------------------------------------------------------------------------------------------------------------
void GorillaOverlay::Reset() {
  mSilverback->destroyScreen(mScreen);
//  mScreen = mSilverback->createScreen(ogre_viewport_, "dejavu");
//  layer = mScreen->createLayer(0);
}
}