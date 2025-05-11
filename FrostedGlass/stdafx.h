#pragma once

#pragma comment(lib, "dwmapi.lib")

#include <windows.h>

#include <algorithm>
#include <cwctype>
#include <dwmapi.h>
#include <string>
#include <unordered_map>

#include "../API/RainmeterAPI.h"

using wstring = std::wstring;
using MODE = bool;

namespace FrostedGlass
{
    namespace Types
    {
        enum class ACCENT : uint8_t
        {
            DEFAULT = 0, // * No accents are applied.
            BACKDROP = 1, // * Enables backdrop (solid window background).
            BACKDROP_TRASLUCENT = 2, // * Enables backdrop (translucent window background).
            BLUR = 3, // * Applies a Gaussian blur effect to the window background.
            ACRYLIC = 4,  // * Enables Windows 11-style acrylic material (blur + noise texture).
        };

        enum class MICA : uint8_t
        {
            DEFAULT = 1, // * No mica effects are applied.
            STANDARD = 2, // * Standard Mica effect (opaque textured background with theme-aware tint).
            ACRYLIC = 3, // * Hybrid Mica + Acrylic effect (textured semi-transparency).
            ALTERNATIVE = 4, // * Alternate Mica variant with reduced opacity.
        };

        enum class EFFECT : uint8_t
        {
            DEFAULT = 0, // * No effects are applied.
            LUMINANCE = 2, // * Adjusts background luminance for improved text contrast.
            FULLSCREEN = 4, // * Enables frosted glass effects in fullscreen mode.
            BOTH = 6, // * Combines both effects EFFECT::LUMINANCE with EFFECT::FULLSCREEN.
        };

        enum class SHADOW : uint16_t
        {
            DEFAULT = 0,   // * No shadows are applied.
            LEFT = 32,  // * Adds a drop shadow to the left window edge.
            TOP = 64,  // * Adds a drop shadow to the top window edge.
            RIGHT = 128, // * Adds a drop shadow to the right window edge.
            BOTTOM = 256, // * Adds a drop shadow to the bottom window edge.
            ALL = 480  // * Combines shadows on all window edges.
        };

        enum class CORNER : uint8_t
        {
            DEFAULT = 1, // * No rounded corners are applied.
            ROUND = 2, // * Adds rounded corners with a defined shadow. (8px radius)
            ROUND_SMALL = 3, // * Adds slightly rounded corners with a softer, less pronounced shadow. (4px radius)
            ROUND_SMALL_SHADOW = 4, // * Combines the rounded corners of CORNER::ROUND with the softer shadow of CORNER::ROUND_SMALL.
        };

        enum class STROKECOLOR : uint32_t
        {
            HIDDEN = 0xFFFFFFFE, // * No borders are applied.
            VISIBLE = 0xFFFFFFFF,  // * Shows a 1px solid border around the window.}
        };

        enum class BACKDROP : uint32_t
        {
            DEFAULT = 0x00000000, // * No colors are applied.
            DARK_BASE = 0x94202020, // * Dark theme base color (20% opacity dark gray).
            LIGHT_BASE = 0x00F3F3F3, // * Light theme base color (95% opacity light gray).
            DARK = 0x01000000, // * Dark theme layer 1 (minimal opacity black overlay).
            DARK2 = 0x3F000000, // * Dark theme layer 2 (25% opacity black overlay).
            DARK3 = 0x7E000000, // * Dark theme layer 3 (49% opacity black overlay).
            DARK4 = 0xBD000000, // * Dark theme layer 4 (74% opacity black overlay).
            DARK5 = 0xFF000000, // * Dark theme layer 5 (fully opaque black).
            LIGHT = 0x01FFFFFF, // * Light theme layer 1 (minimal opacity white overlay).
            LIGHT2 = 0x3FFFFFFF, // * Light theme layer 2 (25% opacity white overlay).
            LIGHT3 = 0x7EFFFFFF, // * Light theme layer 3 (49% opacity white overlay).
            LIGHT4 = 0xBDFFFFFF, // * Light theme layer 4 (74% opacity white overlay).
            LIGHT5 = 0xFFFFFFFF  // * Light theme layer 5 (fully opaque white).
        };

        enum class COMMAND : uint8_t
        {
            DEFAULT,
            BLUR_TOGGLE,
            BLUR_ENABLE,
            BLUR_DISABLE
        };
    }

    namespace Maps
    {
        static const std::unordered_map<wstring, Types::ACCENT> AccentMap =
        {
            {L"0", Types::ACCENT::DEFAULT},               {L"none", Types::ACCENT::DEFAULT},                             {L"default", Types::ACCENT::DEFAULT},
            {L"1", Types::ACCENT::BACKDROP},              {L"backdrop", Types::ACCENT::BACKDROP},
            {L"2", Types::ACCENT::BACKDROP_TRASLUCENT},   {L"traslucentbackdrop", Types::ACCENT::BACKDROP_TRASLUCENT},
            {L"3", Types::ACCENT::BLUR},                  {L"blur", Types::ACCENT::BLUR},
            {L"4", Types::ACCENT::ACRYLIC},               {L"acrylic", Types::ACCENT::ACRYLIC},
            {L"5", Types::ACCENT::DEFAULT},               {L"mica", Types::ACCENT::DEFAULT},
            {L"6", Types::ACCENT::DEFAULT},               {L"micaacrylic", Types::ACCENT::DEFAULT},
            {L"7", Types::ACCENT::DEFAULT},               {L"micaalt", Types::ACCENT::DEFAULT}
        };

        static const std::unordered_map<wstring, Types::EFFECT> EffectMap =
        {
            {L"0", Types::EFFECT::DEFAULT},      {L"none", Types::EFFECT::DEFAULT},            {L"default", Types::EFFECT::DEFAULT},
            {L"1", Types::EFFECT::LUMINANCE},    {L"luminance", Types::EFFECT::LUMINANCE},     {L"highcontrast", Types::EFFECT::LUMINANCE},
            {L"2", Types::EFFECT::FULLSCREEN},   {L"fullscreen", Types::EFFECT::FULLSCREEN},   {L"cover", Types::EFFECT::FULLSCREEN},
            {L"3", Types::EFFECT::BOTH},         {L"both", Types::EFFECT::BOTH}
        };

        static const std::unordered_map<wstring, Types::BACKDROP> BackdropMap =
        {
            {L"none", Types::BACKDROP::DEFAULT},           {L"default", Types::BACKDROP::DEFAULT},
            {L"lightbase", Types::BACKDROP::LIGHT_BASE},   {L"windowslight", Types::BACKDROP::LIGHT_BASE},
            {L"dark", Types::BACKDROP::DARK},              {L"d1", Types::BACKDROP::DARK},
            {L"light", Types::BACKDROP::LIGHT},            {L"l1", Types::BACKDROP::LIGHT},
            {L"dark2", Types::BACKDROP::DARK2},            {L"d2", Types::BACKDROP::DARK2},
            {L"light2", Types::BACKDROP::LIGHT2},          {L"l2", Types::BACKDROP::LIGHT2},
            {L"dark3", Types::BACKDROP::DARK3},            {L"d3", Types::BACKDROP::DARK3},
            {L"light3", Types::BACKDROP::LIGHT3},          {L"l3", Types::BACKDROP::LIGHT3},
            {L"darkbase", Types::BACKDROP::DARK_BASE},     {L"windowsdark", Types::BACKDROP::DARK_BASE},
            {L"dark4", Types::BACKDROP::DARK4},            {L"d4", Types::BACKDROP::DARK4},
            {L"light4", Types::BACKDROP::LIGHT4},          {L"l4", Types::BACKDROP::LIGHT4},
            {L"dark5", Types::BACKDROP::DARK5},            {L"d5", Types::BACKDROP::DARK5},
            {L"light5", Types::BACKDROP::LIGHT5},          {L"l5", Types::BACKDROP::LIGHT5},
        };

        static const std::unordered_map<wstring, Types::MICA> MicaMap =
        {
            {L"0", Types::MICA::DEFAULT},       {L"none", Types::MICA::DEFAULT},                 {L"default", Types::MICA::DEFAULT},
            {L"1", Types::MICA::DEFAULT},       {L"backdrop", Types::MICA::DEFAULT},
            {L"2", Types::MICA::DEFAULT},       {L"traslucentbackdrop", Types::MICA::DEFAULT},
            {L"3", Types::MICA::DEFAULT},       {L"blur", Types::MICA::DEFAULT},
            {L"4", Types::MICA::DEFAULT},       {L"acrylic", Types::MICA::DEFAULT},
            {L"5", Types::MICA::STANDARD},      {L"mica", Types::MICA::STANDARD},
            {L"6", Types::MICA::ACRYLIC},       {L"micaacrylic", Types::MICA::ACRYLIC},
            {L"7", Types::MICA::ALTERNATIVE},   {L"micaalt", Types::MICA::ALTERNATIVE}
        };

        static const std::unordered_map<wstring, Types::CORNER> CornerMap =
        {
            {L"0", Types::CORNER::DEFAULT},              {L"none", Types::CORNER::DEFAULT},                 {L"default", Types::CORNER::DEFAULT},
            {L"1", Types::CORNER::ROUND},                {L"round", Types::CORNER::ROUND},
            {L"2", Types::CORNER::ROUND_SMALL_SHADOW},   {L"roundws", Types::CORNER::ROUND_SMALL_SHADOW},   {L"roundsmallshadow", Types::CORNER::ROUND_SMALL_SHADOW},
            {L"3", Types::CORNER::ROUND_SMALL},          {L"roundsmall", Types::CORNER::ROUND_SMALL},
        };

        static const std::unordered_map<wstring, Types::COMMAND> CommandMap =
        {
            {L"toggleblur", Types::COMMAND::BLUR_TOGGLE},
            {L"enableblur", Types::COMMAND::BLUR_ENABLE},
            {L"disableblur", Types::COMMAND::BLUR_DISABLE}
        };
    }

    namespace System
    {
        enum class BUILDS : uint16_t
        {
            WINDOWS_10 = 16299,
            WINDOWS_11 = 22000,
            WINDOWS_MICA = 22621
        };

        class VersionState
        {
        private:
            static bool isInitialized;
            static bool isWin10;
            static bool isWin11;
            static bool isMica;

            static bool CheckBuild(const BUILDS _RequiredBuild) noexcept;

        public:
            static void Initialize() noexcept;
            static bool IsWin10() noexcept;
            static bool IsWin11() noexcept;
            static bool IsMica() noexcept;
        };
    }

    namespace API
    {
        enum class ATTRIBUTE : uint8_t
        {
            ACCENT_POLICY = 19
        };

        struct AccentPolicy
        {
            Types::ACCENT Accent;
            uint32_t Flags;
            Types::BACKDROP Color;
            BYTE Reserved;
        };

        struct CompositionAttribute
        {
            ATTRIBUTE Attribute;
            AccentPolicy* Data;
            DWORD Size;
        };

        using pSetWindowCompositionAttribute = void(__stdcall*)(HWND hHwnd, CompositionAttribute* Data);
        using pDwmSetWindowAttribute = void(__stdcall*)(HWND Hwnd, INT Attribute, LPCVOID Data, INT Size);

        class InstanceState
        {
        private:
            static bool isInitialized;

            static HINSTANCE User32Module;
            static HINSTANCE DwmapiModule;

            static uint32_t instances;

        public:
            static pSetWindowCompositionAttribute SetWindowCompositionAttribute;
            static pDwmSetWindowAttribute SetWindowAttribute;

            static void Initialize() noexcept;
            static bool IsUser32Loaded() noexcept;
            static bool IsDwmapiLoaded() noexcept;
            static void Finalize() noexcept;
        };

        class WindowStyler
        {
        private:
            static void SetMargin(HWND _Hwnd, int8_t _Margin) noexcept;
            static void SetCompositionAttribute(HWND _Hwnd, CompositionAttribute Data) noexcept;
            static void SetAttribute(HWND _Hwnd, const uint8_t _Attribute, const int& _Value) noexcept;
        public:
            static void SetAccent(HWND _Hwnd, const Types::ACCENT _Accent, const Types::EFFECT _Effect, const Types::SHADOW _Shadow, const Types::BACKDROP _Backdrop) noexcept;
            static void SetMica(HWND _Hwnd, const Types::MICA _Mica, const MODE _Focus) noexcept;
            static void SetCorner(HWND _Hwnd, const Types::CORNER _Corner) noexcept;
            static void SetStrokeColor(HWND _Hwnd, const Types::STROKECOLOR _StrokeColor) noexcept;
            static void SetDarkMode(HWND _Hwnd, const MODE _Mode) noexcept;
        };
    }

}
