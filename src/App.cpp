#include "stdafx.hpp"

#include "App.hpp"

#include "Vulkan/Pipeline.hpp"

namespace OselEngine {
App::App() {
}

App::~App() {
}

void App::run() {
	mRenderer.loadModels();
	mRenderer.updateDescriptorSets(mTexture);
	spdlog::info("Starting Phoenix Engine app");
	while (!mWindow.shouldClose()) {
		glfwPollEvents();
		mRenderer.drawFrame();
	}

	vkDeviceWaitIdle(mDevice.get());
}

} // namespace OselEngine
