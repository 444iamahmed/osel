#pragma once

#include "Vulkan/VulkanDevice.hpp"
#include "Vulkan/VulkanWindow.hpp"
#include "Vulkan/VulkanRenderer.hpp"
#include "Vulkan/Texture.hpp"

namespace OselEngine {
class App {
public:
	static constexpr int WIDTH =
		800; // Should we make width and height a property of the window rather
			 // than the app
	static constexpr int HEIGHT = 600;

	uint32_t currentFrameIndex = 0;

	App();
	~App();

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void run();

private:
	Vulkan::Window mWindow{WIDTH, HEIGHT, "Phoenix"};
	Vulkan::Device mDevice{mWindow};
	Vulkan::Renderer mRenderer{mWindow, mDevice};
	Vulkan::Texture mTexture{"assets/textures/test_image.png", mDevice};
};
} // namespace OselEngine
