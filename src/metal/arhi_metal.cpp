// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ARHI_METAL)
#include "arhi_internal.h"

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
//#define MTK_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
//#include <AppKit/AppKit.hpp>
//#include <MetalKit/MetalKit.hpp>

bool Metal_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;
    available = false;
    return available;
}

RHIFactoryImpl* Metal_CreateFactory(const RHIFactoryDesc* desc)
{
    ARHI_UNUSED(desc);
    return nullptr;
}

#endif /* defined(ARHI_METAL) */
