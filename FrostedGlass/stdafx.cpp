#include "stdafx.h"

namespace FrostedGlass
{
    namespace System
    {
        bool VersionState::isInitialized = false;
        bool VersionState::isWin10 = false;
        bool VersionState::isWin11 = false;
        bool VersionState::isMica = false;

        bool VersionState::CheckBuild(const BUILDS _RequiredBuild) noexcept
        {
            static const DWORDLONG mask = VerSetConditionMask(0, VER_BUILDNUMBER, VER_GREATER_EQUAL);
            OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, static_cast<DWORD>(_RequiredBuild) };
            return VerifyVersionInfo(&osvi, VER_BUILDNUMBER, mask) != FALSE;
        }

        void VersionState::Initialize() noexcept
        {
            if (isInitialized) return;

            isInitialized = true;
            isWin10 = CheckBuild(BUILDS::WINDOWS_10);

            if (!isWin10) return;

            isMica = CheckBuild(BUILDS::WINDOWS_MICA);
            isWin11 = isMica || CheckBuild(BUILDS::WINDOWS_11);
        }

        bool VersionState::IsWin10() noexcept
        {
            return isWin10;
        }

        bool VersionState::IsWin11() noexcept
        {
            return isWin11;
        }

        bool VersionState::IsMica() noexcept
        {
            return isMica;
        }
    }

    namespace API
    {
        bool InstanceState::isInitialized = false;

        HINSTANCE InstanceState::User32Module = nullptr;
        HINSTANCE InstanceState::DwmapiModule = nullptr;

        uint32_t InstanceState::instances = 0;

        pSetWindowCompositionAttribute InstanceState::SetWindowCompositionAttribute = nullptr;
        pDwmSetWindowAttribute InstanceState::SetWindowAttribute = nullptr;

        void InstanceState::Initialize() noexcept
        {
            if (!isInitialized)
            {
                isInitialized = true;
                if (User32Module == nullptr && SetWindowCompositionAttribute == nullptr)
                {
                    User32Module = LoadLibrary(TEXT("user32.dll"));
                    if (User32Module)
                    {
                        RmLog(LOG_DEBUG, L"FrostedGlass: Loaded User32.dll Module...");
                        DisableThreadLibraryCalls(User32Module);
                        SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(GetProcAddress(User32Module, "SetWindowCompositionAttribute"));
                    }
                    if (!SetWindowCompositionAttribute) RmLog(LOG_ERROR, L"FrostedGlass: Could not load SetWindowCompositionAttribute from user32.dll");
                }

                if (DwmapiModule == nullptr && SetWindowAttribute == nullptr)
                {
                    DwmapiModule = LoadLibrary(TEXT("DWMAPI.dll"));
                    if (DwmapiModule)
                    {
                        RmLog(LOG_DEBUG, L"FrostedGlass: Loaded Dwmapi.dll Module...");
                        DisableThreadLibraryCalls(DwmapiModule);
                        SetWindowAttribute = reinterpret_cast<pDwmSetWindowAttribute>(GetProcAddress(DwmapiModule, "DwmSetWindowAttribute"));
                    }
                    if (!SetWindowAttribute) RmLog(LOG_ERROR, L"FrostedGlass: Could not load DwmSetWindowAttribute from DWMAPI.dll");
                }
            }
            ++instances;
        }

        bool InstanceState::IsUser32Loaded() noexcept
        {
            return User32Module;
        }

        bool InstanceState::IsDwmapiLoaded() noexcept
        {
            return DwmapiModule;
        }

        void InstanceState::Finalize() noexcept
        {
            if (--instances > 0) return;

            isInitialized = false;

            if (User32Module)
            {
                RmLog(LOG_DEBUG, L"FrostedGlass: Unloading User32.dll Module...");
                FreeLibrary(User32Module);
                SetWindowCompositionAttribute = nullptr;
            }
            if (DwmapiModule)
            {
                RmLog(LOG_DEBUG, L"FrostedGlass: Unloading Dwmapi.dll Module...");
                FreeLibrary(DwmapiModule);
                SetWindowAttribute = nullptr;
            }
        }

        void WindowStyler::SetMargin(HWND _Hwnd, int8_t _Margin) noexcept
        {
            const MARGINS margins = { _Margin };
            DwmExtendFrameIntoClientArea(_Hwnd, &margins);
        }

        void WindowStyler::SetCompositionAttribute(HWND _Hwnd, CompositionAttribute _Data) noexcept
        {
            InstanceState::SetWindowCompositionAttribute(_Hwnd, &_Data);
        }

        void WindowStyler::SetAttribute(HWND _Hwnd, const uint8_t _Attribute, const int& _Value) noexcept
        {
            InstanceState::SetWindowAttribute(_Hwnd, _Attribute, &_Value, sizeof(_Value));
        }

        void WindowStyler::SetAccent(HWND _Hwnd, const Types::ACCENT _Accent, const Types::EFFECT _Effect, const Types::SHADOW _Shadow, const Types::BACKDROP _Backdrop) noexcept
        {
            if (!InstanceState::IsUser32Loaded()) return;

            AccentPolicy policy = {};
            policy.Accent = _Accent;
            policy.Flags = static_cast<uint32_t>(static_cast<uint8_t>(_Effect) + static_cast<uint16_t>(_Shadow));
            policy.Color = _Backdrop;

            CompositionAttribute data = {};
            data.Attribute = ATTRIBUTE::ACCENT_POLICY;
            data.Data = &policy;
            data.Size = sizeof(policy);

            SetCompositionAttribute(_Hwnd, data);
        }

        void WindowStyler::SetMica(HWND _Hwnd, const Types::MICA _Mica, const MODE _Focus) noexcept
        {
            if (!InstanceState::IsDwmapiLoaded() || !System::VersionState::IsMica()) return;

            if (!_Focus) SetWindowPos(_Hwnd, NULL, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

            SetMargin(_Hwnd, -1);
            SetAttribute(_Hwnd, DWMWA_SYSTEMBACKDROP_TYPE, static_cast<int>(_Mica));
            SetMargin(_Hwnd, 0);
        }

        void WindowStyler::SetCorner(HWND _Hwnd, const Types::CORNER _Corner) noexcept
        {
            if (!InstanceState::IsDwmapiLoaded() || !System::VersionState::IsWin11()) return;

            SetAttribute(_Hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, static_cast<int>(_Corner));
        }

        void WindowStyler::SetStrokeColor(HWND _Hwnd, const Types::STROKECOLOR _StrokeColor) noexcept
        {
            if (!InstanceState::IsDwmapiLoaded() || !System::VersionState::IsWin11()) return;

            SetAttribute(_Hwnd, DWMWA_BORDER_COLOR, static_cast<int>(_StrokeColor));
        }

        void WindowStyler::SetDarkMode(HWND _Hwnd, const MODE _Mode) noexcept
        {
            if (!InstanceState::IsDwmapiLoaded() || !System::VersionState::IsWin11()) return;

            SetAttribute(_Hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, static_cast<int>(_Mode));
        }

    }
}