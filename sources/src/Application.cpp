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

#include "pcheader.h"

#include "Application.h"
#include "Exception.h"
#include "Overlay.h"
#include "HwCheck.h"
#include "DesktopIcon.h"
#include "ComponentLocator.h"
#include "DesktopIcon.h"

#ifdef _WIN32
extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace xio {
Application::Application() {
  try {
    Init_();
  }
  catch (Exception &e) {
    Message_("Exception occurred", e.getDescription());
    throw e;
  }
  catch (Ogre::Exception &e) {
    Message_("Exception occurred (OGRE)", e.getFullDescription());
    throw e;
  }
  catch (std::exception &e) {
    Message_("Exception occurred (std::exception)", e.what());
    throw e;
  }
};
Application::~Application() = default;
//----------------------------------------------------------------------------------------------------------------------
void Application::Init_() {
  DesktopIcon icon;
  icon.Init();
  icon.Save("XioDemo");

  conf_ = std::make_unique<YamlConfigurator>("config.yaml");
  Renderer::SetConfigurator(conf_.get());
  int window_width = conf_->Get<int>("window_width");
  int window_high = conf_->Get<int>("window_high");
  bool window_fullscreen = conf_->Get<bool>("window_fullscreen");
  renderer_ = std::make_unique<Renderer>(window_width, window_high, window_fullscreen);

  // Shadows param
  bool shadow_enable = conf_->Get<bool>("graphics_shadows_enable");
  float shadow_far = conf_->Get<float>("graphics_shadows_far_distance");
  int16_t tex_size = conf_->Get<int16_t>("graphics_shadows_texture_resolution");
  int tex_format = conf_->Get<int>("graphics_shadows_texture_format");
  renderer_->GetShadowSettings().UpdateParams(shadow_enable, shadow_far, tex_size, tex_format);

  io_ = std::make_unique<InputSequencer>();
  physics_ = std::make_unique<Physics>();
  sounds_ = std::make_unique<Sound>();
  overlay_ = std::make_unique<Overlay>();
  loader_ = std::make_unique<DotSceneLoaderB>();

  components_.push_back(sounds_.get());
  components_.push_back(loader_.get());
  components_.push_back(physics_.get());
  components_.push_back(renderer_.get());
  components_.push_back(overlay_.get());

  ComponentLocator::LocateComponents(conf_.get(), io_.get(), renderer_.get(), physics_.get(), sounds_.get(), overlay_.get(), loader_.get());

  for (auto &it : components_) {
    it->Create();
  }

  io_->RegObserver(overlay_->GetConsole());
  renderer_->Refresh();

  // Texture filtering
  std::string graphics_filtration = conf_->Get<std::string>("graphics_filtration");
  Ogre::TextureFilterOptions tfo = Ogre::TFO_BILINEAR;
  if (graphics_filtration == "anisotropic")
    tfo = Ogre::TFO_ANISOTROPIC;
  else if (graphics_filtration == "bilinear")
    tfo = Ogre::TFO_BILINEAR;
  else if (graphics_filtration == "trilinear")
    tfo = Ogre::TFO_TRILINEAR;
  else if (graphics_filtration == "none")
    tfo = Ogre::TFO_NONE;

  renderer_->UpdateParams(tfo, conf_->Get<int>("graphics_anisotropy_level"));

  loader_->LocateComponents(conf_.get(), io_.get(), renderer_.get(), physics_.get(), sounds_.get(), overlay_.get());
  verbose_ = conf_->Get<bool>("global_verbose_enable");
  lock_fps_ = conf_->Get<bool>("global_lock_fps");
  target_fps_ = conf_->Get<int>("global_target_fps");
  io_->RegWinObserver(this);
  renderer_->Resize(conf_->Get<int>("window_width"),
                    conf_->Get<int>("window_high"),
                    conf_->Get<bool>("window_fullscreen"));
  renderer_->GetWindow().SetCaption(conf_->Get<std::string>("window_caption"));

  if (!verbose_) {
    auto *logger = new Ogre::LogManager();
    std::string log_name = "Ogre.log";
    logger->createLog(log_name, true, false, true);
    Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
    Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_BOREME);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Clear_() {
  io_->Clear();
  io_->UnregWinObserver(this);

  for (auto &it : components_)
    it->Clean();

  for (auto &it : components_)
    it->Reset();

  renderer_.reset();
  io_.reset();
  physics_.reset();
  sounds_ .reset();
  overlay_.reset();
  loader_ .reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
//----------------------------------------------------------------------------------------------------------------------
void Application::InitState_(std::unique_ptr<AppState> &&next_state) {
  if (cur_state_) {
    cur_state_->Clear();
    renderer_->Refresh();
    Ogre::Root::getSingleton().removeFrameListener(cur_state_.get());
    io_->UnregObserver(cur_state_.get());
  }

  cur_state_ = move(next_state);
  io_->RegObserver(cur_state_.get());
  Ogre::Root::getSingleton().addFrameListener(cur_state_.get());

  cur_state_->LocateComponents(conf_.get(),
                               io_.get(),
                               renderer_.get(),
                               physics_.get(),
                               sounds_.get(),
                               overlay_.get(),
                               loader_.get());
  cur_state_->Create();
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Quit() {
  quit_ = false;
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Event(const SDL_Event &evt) {
  if (evt.type == SDL_WINDOWEVENT) {
    static bool fullscreen = renderer_->GetWindow().GetFullscreen();

    if (!fullscreen) {
      if (evt.window.event == SDL_WINDOWEVENT_LEAVE || evt.window.event == SDL_WINDOWEVENT_MINIMIZED) {
        suspend_ = true;
        renderer_->GetWindow().SetCursorStatus(true, false, false);
      } else if (evt.window.event == SDL_WINDOWEVENT_TAKE_FOCUS || evt.window.event == SDL_WINDOWEVENT_RESTORED
          || evt.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
        suspend_ = false;
        renderer_->GetWindow().SetCursorStatus(false, true, true);
      }
    } else {
      if (evt.window.event == SDL_WINDOWEVENT_MINIMIZED) {
        suspend_ = true;
        renderer_->GetWindow().SetCursorStatus(true, false, false);
      } else if (evt.window.event == SDL_WINDOWEVENT_TAKE_FOCUS || evt.window.event == SDL_WINDOWEVENT_RESTORED
          || evt.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
        suspend_ = false;
        renderer_->GetWindow().SetCursorStatus(false, true, true);
      }
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Other(uint8_t type, int32_t code, void *data1, void *data2) {}
//----------------------------------------------------------------------------------------------------------------------
void Application::Loop_() {
  while (quit_) {
    if (cur_state_) {
      auto duration_before_frame = std::chrono::system_clock::now().time_since_epoch();
      long millis_before_frame = std::chrono::duration_cast<std::chrono::microseconds>(duration_before_frame).count();

      int fps_frames_ = 0;
      long delta_time_ = 0;

      if (delta_time_ > 1000000) {
        current_fps_ = fps_frames_;
        delta_time_ = 0;
        fps_frames_ = 0;
      }

      io_->Capture();

      if (!suspend_) {
        cur_state_->Loop();
        if (cur_state_->IsDirty()) {
          for (auto &it : components_)
            it->Clean();

          InitState_(move(cur_state_->GetNextState()));
        }

        auto duration_before_update = std::chrono::system_clock::now().time_since_epoch();
        long millis_before_update =
            std::chrono::duration_cast<std::chrono::microseconds>(duration_before_update).count();
        float frame_time = static_cast<float>(millis_before_update - time_of_last_frame_) / 1000000;
        time_of_last_frame_ = millis_before_update;

        for (auto *it : components_)
          it->Loop(frame_time);

        renderer_->RenderOneFrame();
      }

#ifdef DEBUG
      if (verbose_)
        std::cout << std::flush;
#endif

      auto duration_after_render = std::chrono::system_clock::now().time_since_epoch();
      long millis_after_render = std::chrono::duration_cast<std::chrono::microseconds>(duration_after_render).count();
      long render_time = millis_after_render - millis_before_frame;

      if (lock_fps_) {
        long delay = static_cast<long> ((1000000 / target_fps_) - render_time);
        if (delay > 0)
          std::this_thread::sleep_for(std::chrono::microseconds(delay));
      }

      auto duration_after_loop = std::chrono::system_clock::now().time_since_epoch();
      long millis_after_loop = std::chrono::duration_cast<std::chrono::microseconds>(duration_after_loop).count();

      long time_since_last_frame_ = millis_after_loop - millis_before_frame;
      delta_time_ += time_since_last_frame_;

      fps_frames_++;
    } else {
      quit_ = true;
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Go_() {
  if (cur_state_) {
    InitState_(move(cur_state_));
    quit_ = true;
    auto duration_before_update = std::chrono::system_clock::now().time_since_epoch();
    time_of_last_frame_ = std::chrono::duration_cast<std::chrono::microseconds>(duration_before_update).count();
    Loop_();
    Clear_();
  }
}
//----------------------------------------------------------------------------------------------------------------------
int Application::Message_(const std::string &caption, const std::string &message) {
  std::cerr << caption << '\n';
  std::cerr << message << '\n';
#ifdef _WIN32
  MessageBox(nullptr, caption.c_str(), message.c_str(), MB_ICONERROR);
#endif
  return 1;
}
//----------------------------------------------------------------------------------------------------------------------
int Application::Main(std::unique_ptr<AppState> &&scene_ptr) {
  try {
#ifndef DEBUG
    std::ios_base::sync_with_stdio(false);
#endif
    SetInitialState(move(scene_ptr));
    Go_();
  }
  catch (Exception &e) {
    return Message_("Exception occurred", e.getDescription());
  }
  catch (Ogre::Exception &e) {
    return Message_("Exception occurred (OGRE)", e.getFullDescription());
  }
  catch (std::exception &e) {
    return Message_("Exception occurred (std::exception)", e.what());
  }

  return 0;
}
}