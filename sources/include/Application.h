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
#include <OgreFrameListener.h>
#include <OgreRenderTargetListener.h>
#include <OgreLog.h>
#include "Engine.h"
#include "StateManager.h"
#include "ComponentLocator.h"
#include "view_ptr.h"

namespace xio {

class Application final : public WindowObserver, public Ogre::LogListener, public ComponentLocator {
 public:
  explicit Application(char** argv = {});
  virtual ~Application();
  int Main(std::unique_ptr<AppState> &&scene_ptr);
  int GetCurrentFps() const;

 private:
  void Init_(char** argv = {});
  void Loop_();
  void Go_();
  int Message_(const std::string &caption, const std::string &message);

  void Event(const SDL_Event &evt) final;
  void Other(uint8_t type, int32_t code, void *data1, void *data2) final;
  void Quit() final;

  void messageLogged(const std::string &message, Ogre::LogMessageLevel lml, \
        bool maskDebug, const std::string &logName, bool &skipThisMessage) final;

  void WriteLogToFile_(const std::string &file_name);
  void PrintLogToConsole_();

  StateManager state_manager_;
  std::unique_ptr<Engine> engine_;

  bool running_ = true;
  bool suspend_ = false;
  int64_t time_of_last_frame_ = 0;
  int64_t cumulated_time_ = 0;
  int64_t fps_counter_ = 0;
  int current_fps_ = 0;
  int target_fps_ = 60;
#ifdef DEBUG
  bool verbose_ = true;
#else
  bool verbose_ = false;
#endif
  bool lock_fps_ = true;
  std::string log_;
};

} //namespace
