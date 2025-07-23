## Vulkan
- [ ] Rewrite a good CMakeLists.txt for separating wayland to other surfaces
- [ ] Cap the recreation of the swapchain when resize is active to have less freeze while resizing and make it faster
- [ ] Remove unecessary check for NULL values in cleanup

## Window api's
- [ ] Update Wayland functions using documentation and without the tutorial
- [ ] Do a X11 and Windows.h struct window to contain surface and use it in vulkan
- [ ] Allow touchpad and keyboards
- [ ] Handle window decoration if available else => do own decoration