#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Renderer.h"
//#include "vulkan/vulkan.hpp"
//#include <vulkan/vulkan_raii.hpp>

/// <summary>
/// alt sur une lige et les flaiche pour le boujer
/// controle D duplkique  et shift pour deléctrioner
/// muticurser : alt souri er tu peux taper sur plusier lige en mame temp
/// Alt entrée sur les void pour les maitre facilemant dans .cpp
/// </summary>
/// 
class Application
{
public:
	int width = 800;
	int heitght = 600;

	GLFWwindow* window = nullptr;
	Renderer *renderer = nullptr;

	void Init();
	void Run();
	void Destroy();

private:

	void InitWindow();
	void InitVulkan();

	void DestroyWindow();
	void DestroyVulkan();



};

