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

#include "Application.h"
#include "Storage.h"
#include "Exception.h"
#include "ConfiguratorJson.h"

#ifdef _WIN32
#include <windows.h>

extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Context {
Application::Application() {
  Init_();
};
Application::~Application() = default;

//----------------------------------------------------------------------------------------------------------------------
void Application::Init_() {
  renderer_ = std::make_unique<Renderer>();
  conf_ = std::make_unique<ConfiguratorJson>();
  physics_ = std::make_unique<Physic>();
  sounds_ = std::make_unique<Sound>();
  overlay_ = std::make_unique<Overlay>();
  loader_ = std::make_unique<DotSceneLoaderB>();
  overlay_->Init();
  renderer_->UpdateParams();
  loader_->SetWorld(physics_.get());
  conf_->Assign(verbose_, "global_verbose_enable");
  conf_->Assign(target_fps_, "global_target_fps");
  conf_->Assign(lock_fps_, "global_lock_fps");

  io::InputSequencer::Instance().RegEventListener(this);

  if (!verbose_) {
    auto *logger = new Ogre::LogManager();
    std::string log_name = "Ogre.log";

    logger->createLog(log_name, true, false, true);
    Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
    Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_BOREME);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Render_() {
  renderer_->Render();
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Reset_() {
  auto *root = Ogre::Root::getSingleton().getSceneManager("Default");
  root->destroyAllEntities();
  root->destroyAllLights();
  root->destroyAllParticleSystems();
  root->destroyAllAnimations();
  root->destroyAllAnimationStates();
  root->destroyAllStaticGeometry();
  root->destroyAllManualObjects();
  root->destroyAllInstanceManagers();
  root->destroyAllBillboardChains();
  root->destroyAllBillboardSets();
  root->destroyAllMovableObjects();
  root->destroyAllCameras();
  root->getRootSceneNode()->removeAndDestroyAllChildren();
  Ogre::ResourceGroupManager::getSingleton().unloadResourceGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}
//----------------------------------------------------------------------------------------------------------------------
void Application::InitCurrState_() {
  cur_state_->Init();
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Quit() {
  quit_ = false;
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Event(const SDL_Event &evt) {
  const int fps_inactive = 30;

  if (evt.type == SDL_WINDOWEVENT) {
    static bool fullscreen = renderer_->GetWindow().GetFullscreen();

    if (!fullscreen) {
      if (evt.window.event == SDL_WINDOWEVENT_LEAVE || evt.window.event == SDL_WINDOWEVENT_MINIMIZED) {
        suspend_ = true;
        target_fps_ = fps_inactive;
        lock_fps_ = true;
      } else if (evt.window.event == SDL_WINDOWEVENT_TAKE_FOCUS || evt.window.event == SDL_WINDOWEVENT_RESTORED
          || evt.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
        suspend_ = false;
        conf_->Assign(target_fps_, "global_target_fps");
        conf_->Assign(lock_fps_, "global_lock_fps");
      }
    } else {
      if (evt.window.event == SDL_WINDOWEVENT_MINIMIZED) {
        suspend_ = true;
        target_fps_ = fps_inactive;
        lock_fps_ = true;
      } else if (evt.window.event == SDL_WINDOWEVENT_TAKE_FOCUS || evt.window.event == SDL_WINDOWEVENT_RESTORED
          || evt.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
        suspend_ = false;
        conf_->Assign(target_fps_, "global_target_fps");
        conf_->Assign(lock_fps_, "global_lock_fps");
      }
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Other(Uint8 type, int32_t code, void *data1, void *data2) {}
//----------------------------------------------------------------------------------------------------------------------
void Application::Loop_() {
  while (quit_) {
    if (GetCurState()) {
      auto *root = Ogre::Root::getSingleton().getSceneManager("Default");
      auto duration_before_frame = std::chrono::system_clock::now().time_since_epoch();
      long millis_before_frame = std::chrono::duration_cast<std::chrono::microseconds>(duration_before_frame).count();

      int fps_frames_;
      long delta_time_;

      if (delta_time_ > 1000000) {
        current_fps_ = fps_frames_;
        delta_time_ = 0;
        fps_frames_ = 0;
      }

      io::InputSequencer::Instance().Capture();

      if (!suspend_) {
        if (waiting_) {
          loader_->Clear();
          physics_->Clear();

          root->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
          root->destroyAllEntities();
          root->destroyAllLights();
          root->destroyAllParticleSystems();
          root->destroyAllAnimations();
          root->destroyAllAnimationStates();
          root->destroyAllStaticGeometry();
          root->destroyAllRibbonTrails();
          root->destroyAllManualObjects();
          root->destroyAllInstanceManagers();
          root->destroyAllBillboardChains();
          root->destroyAllBillboardSets();
          root->destroyAllMovableObjects();
          root->getRootSceneNode()->removeAndDestroyAllChildren();

          renderer_->UpdateParams();
          InitCurrState_();
          physics_->Start();
          waiting_ = false;
        }

        Render_();
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
      Quit();
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::Go_() {
  if (GetCurState()) {
    GetCurState()->Init();
    quit_ = true;
    Loop_();
    io::InputSequencer::Instance().Reset();
    Reset_();
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Application::SetCurState_(std::unique_ptr<AppState> &&scene_ptr) {
  SetInitialState(move(scene_ptr));
}
//----------------------------------------------------------------------------------------------------------------------
int Application::Main(std::unique_ptr<AppState> &&scene_ptr) {
#ifdef _MSC_VER
  SDL_SetMainReady();
#endif
  int return_value = 0;
  std::ios_base::sync_with_stdio(false);

  try {
    SetCurState_(move(scene_ptr));
    Go_();
  }
  catch (Exception &e) {
    const std::string caption = "Exception occurred (Context core)";
    std::cerr << caption << std::endl;
    std::stringstream message;
    message << e.getDescription() << std::endl;
    std::cerr << message.str();

#ifdef _WIN32
    MessageBox(nullptr, message.str().c_str(), caption.c_str(), MB_ICONERROR);
#endif

    return_value = 1;
  }
  catch (Ogre::Exception &e) {
    const std::string caption = "Exception occurred (OGRE)";
    std::cerr << caption << std::endl;
    std::stringstream message;
    message << e.getFullDescription() << std::endl;
    std::cerr << message.str();

#ifdef _WIN32
    MessageBox(nullptr, message.str().c_str(), caption.c_str(), MB_ICONERROR);
#endif

    return_value = 1;
  }
  catch (std::exception &e) {
    const std::string caption = "Exception occurred (std::exception)";
    std::cerr << caption << std::endl;
    std::cerr << e.what() << std::endl;
#ifdef _WIN32
    MessageBox(nullptr, e.what(), caption.c_str(), MB_ICONERROR);
#endif

    return_value = 1;
  }
  catch (...) {
    const std::string caption = "Unhandled exception occurred\n";
    std::cerr << caption << std::endl;
#ifdef _WIN32
    MessageBox(nullptr, caption.c_str(), caption.c_str(), MB_ICONERROR);
#endif

    return_value = 1;
  }

#if defined _WIN32 && defined DEBUG
  if (config_igManager::Instance().GetBool("application_ask_before_quit")) {
    std::cout << std::endl;
    std::cout << "Press any key to end program..." << std::endl;
    getchar();
  }
#endif

  return return_value;
} //class Application
} //namespace Context