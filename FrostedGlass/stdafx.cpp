#include "stdafx.h"
#include "FrostedGlass.h"

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

        PSET_COMPOSITION InstanceState::SetWindowCompositionAttribute = nullptr;
        PSET_ATTRIBUTE InstanceState::SetWindowAttribute = nullptr;

        // ----------------------------------------------------------- //

        void InstanceState::LoadUser32() noexcept
        {
            User32Module = LoadLibrary(TEXT("user32.dll"));

            if (User32Module)
            {
                RmLog(LOG_DEBUG, L"FrostedGlass: Loaded User32.dll Module...");
                DisableThreadLibraryCalls(User32Module);
                SetWindowCompositionAttribute = reinterpret_cast<PSET_COMPOSITION>(GetProcAddress(User32Module, "SetWindowCompositionAttribute"));
            }
            if (!SetWindowCompositionAttribute) RmLog(LOG_ERROR, L"FrostedGlass: Could not load SetWindowCompositionAttribute from user32.dll");
        }

        void InstanceState::UnloadUser32() noexcept
        {
            if (!IsUser32Loaded()) return;

            RmLog(LOG_DEBUG, L"FrostedGlass: Unloaded User32.dll Module...");
            FreeLibrary(User32Module);
            SetWindowCompositionAttribute = nullptr;
        }

        void InstanceState::LoadDwmapi() noexcept
        {
            DwmapiModule = LoadLibrary(TEXT("DWMAPI.dll"));

            if (DwmapiModule)
            {
                RmLog(LOG_DEBUG, L"FrostedGlass: Loaded Dwmapi.dll Module...");
                DisableThreadLibraryCalls(DwmapiModule);
                SetWindowAttribute = reinterpret_cast<PSET_ATTRIBUTE>(GetProcAddress(DwmapiModule, "DwmSetWindowAttribute"));
            }
            if (!SetWindowAttribute) RmLog(LOG_ERROR, L"FrostedGlass: Could not load DwmSetWindowAttribute from DWMAPI.dll");
        }

        void InstanceState::UnloadDwmapi() noexcept
        {
            if (!IsDwmapiLoaded()) return;
         
            RmLog(LOG_DEBUG, L"FrostedGlass: Unloaded Dwmapi.dll Module...");
            FreeLibrary(DwmapiModule);
            SetWindowAttribute = nullptr;
        }

        // ------------------------------ //

        void InstanceState::Initialize() noexcept
        {
            if (!isInitialized)
            {
                isInitialized = true;
                LoadUser32();
                LoadDwmapi();
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
            UnloadUser32();
            UnloadDwmapi();
        }

        // ----------------------------------------------------------- //

        wstring StringHelper::ToWstring(const uint8_t _Value) noexcept
        {
            wchar_t buffer[4];
            swprintf_s(buffer, L"%u", _Value);
            return buffer;
        }

        void StringHelper::ToLowerCase(wstring& _Input) noexcept
        {
            std::transform(_Input.begin(), _Input.end(), _Input.begin(),
                           [](wchar_t c) { return std::towlower(c); });
        }

        void StringHelper::ToRemoveSpace(wstring& _Input) noexcept
        {
            _Input.erase(_Input.begin(),
                         std::find_if_not(_Input.begin(), _Input.end(),
                                          [](wchar_t c) { return std::iswspace(c); }));
        }

        bool StringHelper::Evaluate(wstring& _Input, const wstring& _Search) noexcept
        {
            //ToRemoveSpace(_Input);
            //
            //const size_t pos = _Search.size();
            //if (_Input.substr(0, pos) != _Search) return false;
            //
            //_Input.erase(0, pos);
            //return true;

            ToRemoveSpace(_Input);

            const size_t pos = _Search.size();
            if (_Input.compare(0, pos, _Search) != 0) return false;

            _Input.erase(0, pos);
            return true;
        }

        // ----------------------------------------------------------- //

        uint8_t ColorHelper::ClampColor(const int _Value) noexcept
        {
            return static_cast<uint8_t>((_Value >= 255) ? 255 : ((_Value <= 0) ? 0 : _Value));
        }

        bool ColorHelper::IsValidHexColor(wstring& _Color) noexcept
        {
            if (_Color[0] == L'#')
                _Color.erase(0, 1);

            const size_t length = _Color.length();
            if (length != 6 && length != 8) return false;

            //return std::all_of(_Color.begin(), _Color.end(), std::iswxdigit);

            return std::all_of(_Color.begin(), _Color.end(), [](wchar_t c) { return std::iswxdigit(c); });
        }

        bool ColorHelper::IsValidDecColor(wstring& _Color) noexcept
        {
            int channels[4] = { 0 };
            const int count = swscanf_s(_Color.c_str(), L"%d,%d,%d,%d", &channels[0], &channels[1], &channels[2], &channels[3]);

            if (count < 3) return false;

            //if (swscanf_s(_Color.c_str(), L"%d,%d,%d,%d", &channels[0], &channels[1], &channels[2], &channels[3]) < 3) return false;

            wchar_t hexColor[9];
            swprintf_s(hexColor, L"%02X%02X%02X%02X", ClampColor(channels[0]), ClampColor(channels[1]), ClampColor(channels[2]), ClampColor(channels[3]));
            _Color = hexColor;
            return true;
        }

        // ------------------------------ //

        wstring ColorHelper::GetColor(Measure* _Measure, wstring& _Color) noexcept
        {
            if (API::ColorHelper::IsValidHexColor(_Color) || API::ColorHelper::IsValidDecColor(_Color)) return _Color;

            _Measure->setValidColor(false);
            return L"000000";
        }

        uint32_t ColorHelper::SetColor(wstring& _Color) noexcept
        {
            if (_Color.size() == 6)
                _Color += L"00";

            const uint32_t _HexColor = std::stoul(_Color, nullptr, 16);
            uint32_t postColor(((_HexColor >> 24) & 0xFF) |
                               ((_HexColor >> 8) & 0xFF00) |
                               ((_HexColor << 8) & 0xFF0000) |
                               ((_HexColor << 24) & 0xFF000000));

            if (postColor <= 0) return 0x000001;

            return postColor;
        }

        // ----------------------------------------------------------- //

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

        // ------------------------------ //

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
