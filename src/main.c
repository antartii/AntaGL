#include <vulkan/vulkan.h>

#include "utils.h"
#include "vulkan_wrapper.h"
#include "wayland.h"

VkRect2D pick_swap_extent()
{

}

void create_swapchain(struct engine *engine)
{
    VkSurfaceCapabilitiesKHR swapchain_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physical_device, engine->surface, &swapchain_capabilities);
}

void init_vulkan(struct engine *engine)
{
    char app_name[11] = "AntaEngine";
    uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
    uint32_t vulkan_api_version = VK_API_VERSION_1_2;

    engine->name = "AntaEngine";
    engine->version = VK_MAKE_VERSION(1, 0, 0);

    create_vulkan_instance(engine, app_name, app_version, vulkan_api_version);
    if ((engine->physical_device = pick_physical_device(engine->instance)) == NULL)
        error("Couldn't pick a physical device\n", engine);
    create_device(engine);
    create_swapchain(engine);
    create_command_buffers(engine);
    create_sync_objects(engine);
}

void test_wayland()
{
    struct window win = {0};

    if (!init_wayland(&win))
        exit(1);

    while (wl_display_dispatch(win.display) != -1) {
        
    }

    end_wayland(&win);
}

int main(const int argc, const char **argv, const char **env)
{
    struct engine engine;

    init_vulkan(&engine);

    // draw_frame(&engine);
    cleanup(&engine);

    printf("Testing wayland\n");

    test_wayland();
    return 0;
}
