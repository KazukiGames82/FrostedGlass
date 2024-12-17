
#pragma once
#pragma comment(lib, "dwmapi.lib")

// WinAPI
#include <windows.h>
#include <dwmapi.h>
#include <VersionHelpers.h>

// STL
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <regex>

// Rainmeter API
#include "../API/RainmeterAPI.h"

#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)