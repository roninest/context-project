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

#pragma once

#include <OgreFrameListener.h>
#include <OgreRenderTargetListener.h>
#include <OgreLog.h>

#include "IoListeners.h"
#include "DummyListener.h"
#include "Graphics.h"
#include "Sound.h"
#include "Physic.h"
#include "DotSceneLoaderB.h"
#include "Compositors.h"
#include "Singleton.h"
#include "Overlay.h"

namespace Context {
class AppState;
}

namespace Context {
class Application : public io::OtherEventListener, public Ogre::LogListener, public Singleton {
 public:
  Application();
  virtual ~Application();

  int Main(std::unique_ptr<AppState> &&scene_ptr);
  void SetCurState(std::unique_ptr<AppState> &&scene_ptr);

 private:
  void Init_();
  void Reset_();
  void Render_();
  void Loop_();
  void Go_();

  void Event(const SDL_Event &evt) override;
  void Other(Uint8 type, int32_t code, void *data1, void *data2) override;
  void Quit() override;

  void messageLogged(const std::string &message, Ogre::LogMessageLevel lml, \
        bool maskDebug, const std::string &logName, bool &skipThisMessage) final {
    switch (lml) {
      case Ogre::LML_WARNING: break;
      case Ogre::LML_NORMAL: break;
      case Ogre::LML_CRITICAL: break;
      case Ogre::LML_TRIVIAL: break;
    }
  }

  bool quit_ = false;
  bool suspend_ = false;
  int current_fps_ = 0;
  int target_fps_ = 60;
  bool verbose_ = false;
  bool lock_fps_ = true;
  Graphics graphics_;
  Physic physics_;
  Sound sounds_;
  Overlay overlay_;
  std::unique_ptr<Compositors> compositor_;
  DotSceneLoaderB loader_;
}; //class Application
} //namespace Context
