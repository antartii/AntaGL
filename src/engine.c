#include "engine.h"

static int engine_error(engine_t engine, const char *msg, const bool is_critical)
{
    write(STDERR_FILENO, msg, strlen(msg));

    if (is_critical) {
        engine_cleanup(engine);
        exit(ENGINE_ERROR_CODE_DEFAULT);
    }
    else
        return ENGINE_ERROR_CODE_DEFAULT;
}

static void vulkan_cleanup(engine_t engine)
{
    if (engine->device) {
        if (engine->swapchain) vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
        if (engine->swapchain_images) {
            for (uint32_t i = 0; i < engine->swapchain_images_count; ++i)
                vkDestroyImageView(engine->device, engine->swapchain_image_views[i], NULL);
            free(engine->swapchain_images);
        }
        vkDestroyDevice(engine->device, NULL);
    }
    if (engine->instance) {
        #ifdef DEBUG
        if (engine->debug_messenger) engine->vulkan_extensions_functions.vkDestroyDebugUtilsMessengerEXT(engine->instance, engine->debug_messenger, NULL);
        #endif
        if (engine->surface) vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
        vkDestroyInstance(engine->instance, NULL);
    }
}

void engine_cleanup(engine_t engine)
{
    if (!engine)
        return;

    vulkan_cleanup(engine);

    end_wayland(engine->window);

    free(engine->window);
    free(engine);
}

static void engine_init(engine_t engine, const char *application_name, uint32_t application_version)
{
    if (!engine)
        engine_error(engine, "engine_init: engine_t engine is NULL\n", true);

    // todo: find a better way to set them manually + do resize
    engine->window->width = 800;
    engine->window->height = 600;

    if (!init_wayland(engine->window))
        engine_error(engine, "engine_init: wayland window couldn't be inited\n", true);
    
    if (!vulkan_create_instance(&engine->instance, ENGINE_NAME, ENGINE_VERSION, application_name, application_version)
        || !vulkan_init_extensions_functions(engine->instance, &engine->vulkan_extensions_functions)
        #ifdef DEBUG
        || !vulkan_setup_debug_messenger(engine->instance, &engine->debug_messenger, engine->vulkan_extensions_functions.vkCreateDebugUtilsMessengerEXT)
        #endif
        || !vulkan_create_surface(engine->instance, engine->window, &engine->surface)
        || !vulkan_pick_physical_device(engine->instance, &engine->physical_device)
        || !vulkan_create_logical_device(engine->physical_device, engine->surface, &engine->device, &engine->graphic_queue, &engine->present_queue)
        || !vulkan_create_swapchain(engine->physical_device, engine->device, engine->surface, engine->window, &engine->swapchain, &engine->swapchain_image_format, &engine->swapchain_extent, &engine->swapchain_images_count, &engine->swapchain_images)
        || !vulkan_create_image_view(engine->device, engine->swapchain_image_format, engine->swapchain_images_count, engine->swapchain_images, &engine->swapchain_image_views)
    )
        engine_error(engine, "engine_init: failed to init the engine\n", true);
}

engine_t engine_create(const char *application_name, const struct version application_version)
{
    engine_t engine = calloc(1, sizeof(struct engine));
    engine->window = calloc(1, sizeof(struct window));
    

    if (!engine)
        engine_error(engine, "engine_create: engine_t engine is NULL\n", true);
    engine_init(engine, application_name, VK_MAKE_VERSION(application_version.major, application_version.minor, application_version.patch));

    return engine;
}
