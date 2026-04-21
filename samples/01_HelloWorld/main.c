// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "arhi.h"

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#   define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#   define GLFW_EXPOSE_NATIVE_COCOA
#   include <Foundation/Foundation.h>
#   include <QuartzCore/CAMetalLayer.h>
#else
#   define GLFW_EXPOSE_NATIVE_X11
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#else
#  include <GLFW/glfw3native.h>
#endif

#include <stdlib.h>
#include <string.h> // memset
#include <assert.h>
#define ARHI_UNUSED(x) (void)(x)

static void error_callback(int error, const char* description)
{
    ARHI_UNUSED(error);
    ARHI_UNUSED(description);
    //fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ARHI_UNUSED(scancode);
    ARHI_UNUSED(mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    RHIFactoryDesc factoryDesc = {
        .preferredBackend = RHIBackend_Vulkan,
#if defined(_DEBUG)
        .validationMode = RHIValidationMode_Enabled,
#else
        .validationMode = RHIValidationMode_Disabled,
#endif
    };

    RHIFactory factory = RHIFactoryCreate(&factoryDesc);
    RHIAdapter adapter = RHIFactoryGetBestAdapter(factory);

    RHIAdapterInfo adapterInfo;
    RHIAdapterGetInfo(adapter, &adapterInfo);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Alimer RHI", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    RHISurfaceSource surfaceSource = NULL;
    switch (glfwGetPlatform())
    {
        case GLFW_PLATFORM_WIN32:
            surfaceSource = RHISurfaceSourceCreateFromWin32(glfwGetWin32Window(window));
            break;

        default:
            surfaceSource = NULL;
            break;
    }
    if (!surfaceSource)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    RHISurface surface = RHISurfaceCreate(factory, surfaceSource);

    glfwSetKeyCallback(window, key_callback);
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        ARHI_UNUSED(ratio);

        glfwPollEvents();
    }

    RHISurfaceRelease(surface);
    RHIFactoryRelease(factory);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
