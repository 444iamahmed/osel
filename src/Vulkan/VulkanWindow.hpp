#pragma once

#include "../Window.hpp"

namespace OselEngine {
namespace Vulkan {
class Window : public OselEngine::Window {
public:
	Window(int inWidth, int inHeight, std::string inName);
	~Window() {};

	void createSurface(VkInstance& instance, VkSurfaceKHR& surface) const;
	VkExtent2D getExtent() {return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; };

   protected:
	void setupHandle() override {};
};
}  // namespace Vulkan
}  // namespace PhoenixEngine
