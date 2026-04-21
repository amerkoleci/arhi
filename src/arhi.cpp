// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "arhi_internal.h"

static RHILogLevel s_LogLevel = RHILogLevel_Off;
static RHILogCallback s_LogFunc = nullptr;
static void* s_userData = nullptr;

RHILogLevel arhiGetLogLevel(void)
{
    return s_LogLevel;
}

void arhiSetLogLevel(RHILogLevel level)
{
    s_LogLevel = level;
}

void arhiSetLogCallback(RHILogCallback func, void* userData)
{
    s_LogFunc = func;
    s_userData = userData;
}
