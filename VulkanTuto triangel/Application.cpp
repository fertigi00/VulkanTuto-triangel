#include "Application.h"

void Application::Init()
{
    InitWindow();
    InitVulkan();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        renderer->drawFrame();
    }
    renderer->device.waitIdle();
}

void Application::Destroy() //destroy inver de init
{
    DestroyVulkan();
    DestroyWindow();
}

void Application::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


    window = glfwCreateWindow(800, 600, "VulkanTUTO:)", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, renderer->framebufferResizeCallback);
    
}


void Application::InitVulkan()
{
	renderer = new Renderer();

	renderer->SetWindow(window);

	renderer->CreatInstance();
    renderer->CreateSurface();
    renderer->pickPhysicalDevice();
    renderer->createLogicalDevice();
    renderer->createSwapChain();
    renderer->createImageViews();
	renderer->createDescriptorSetLayout();
    renderer->createGraphicsPipeline();
    renderer->createCommandPool();
    renderer->createCommandBuffer();
    renderer->createSyncObjects();
	renderer->createVertexBuffer();
	renderer->createIndexBuffer();
	renderer->createUniformBuffers();
	renderer->createDescriptorPool();
    renderer->createDescriptorSets();
}

void Application::DestroyWindow()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::DestroyVulkan()
{

}
