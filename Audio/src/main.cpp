#include <iostream>

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "FMOD/fmod.hpp"
#include "FMOD/fmod_errors.h"

#include <chrono>
#include <cassert>

#include "TestEnvironment/TestEnvironment.h"

int main()
{
	// init glfw
	if (!glfwInit())
	{
		std::cerr << "GLFW init failed!" << std::endl;
		return -1;
	}

	// set window configuration
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // only support modern opengl

	// create the window
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Audio", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "GLFW failed to create the window!" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Init glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW init failed!" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

  // Init TestEnvironment
  TestEnvironment testEnvironment;

  // init input
  Input::SetWindow(window);
  Input::SetListener(static_cast<InputListener*>(&testEnvironment));
  glfwSetKeyCallback(window, Input::OnKeyCallback);
  glfwSetMouseButtonCallback(window, Input::OnMouseButtonCallback);
  glfwSetScrollCallback(window, Input::OnMouseScrollCallback);
  glfwSetCursorPosCallback(window, Input::OnMouseMoveCallback);

  // init audio
  FMOD::System* FMODSystem = nullptr;
  FMOD::System_Create(&FMODSystem);
  if (FMODSystem)
  {
    int maxChannels = 50;
    FMOD_INITFLAGS flags = FMOD_INIT_NORMAL;
    void* extraDriverData = nullptr;
    FMODSystem->init(maxChannels, flags, extraDriverData);
  }
  else
  {
    FMOD_RESULT errorResult = FMOD::System_Create(&FMODSystem);
    std::cerr << "Failed to create FMOD system. Error: " << FMOD_ErrorString(errorResult) << std::endl;
  }

  // load audio
  FMOD::Sound* sound = nullptr;
  FMOD_RESULT errorResult = FMODSystem->createSound("sound.wav", FMOD_DEFAULT, nullptr, &sound);
  if(errorResult != FMOD_OK)
    std::cerr << "Failed to load sound. Error: " << FMOD_ErrorString(errorResult) << std::endl;

  auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::microseconds::zero());
 
  auto currentTime = std::chrono::steady_clock::now();
  auto previousTime = std::chrono::steady_clock::now();

	// main loop
	while (!glfwWindowShouldClose(window))
	{
    currentTime = std::chrono::steady_clock::now();

		glfwPollEvents();

    auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - previousTime);
    previousTime = currentTime;

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // play sound
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
      if (FMODSystem)
      {
        FMOD::Channel* channel = nullptr;
        bool paused = false;
        errorResult = FMODSystem->playSound(sound, nullptr, paused, &channel);
        if (errorResult != FMOD_OK)
          std::cerr << "Failed to play sound. Error: " << FMOD_ErrorString(errorResult) << std::endl;
      }
    }

    testEnvironment.Update((float)deltaTime.count() / 1000000);
    testEnvironment.Render();

    glfwSwapBuffers(window);

    if (FMODSystem)
    {
      FMODSystem->update();
    }
	}

  // unload audio
  if (sound)
  {
    sound->release();
  }

  // release audio
  if (FMODSystem)
  {
    FMODSystem->release();
  }

	glfwTerminate();

	return 0;
}



