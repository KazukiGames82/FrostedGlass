#include "stdafx.h"
#include "FrostedGlass.h"

using namespace FrostedGlass;

//-----------------------------------//
//             RAINMETER             //
//-----------------------------------//

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
    System::VersionState::Initialize();

    if (!System::VersionState::IsWin10()) return;

	Measure* measure = new Measure();
	*data = measure;
	measure->Initialize(rm);
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	if (!System::VersionState::IsWin10()) return;

	Measure* measure = (Measure*)data;
	*maxValue = 3;
	measure->Reload();
}

PLUGIN_EXPORT double Update(void* data)
{
	if (!System::VersionState::IsWin10()) return -1;

	Measure* measure = (Measure*)data;
	measure->Update();

    return 0.0;
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	if (!System::VersionState::IsWin10())
	{
		RmLog(LOG_WARNING, L"FrostedGlass commands are supported only on Windows 10 and 11.");
		return;
	}

	Measure* measure = (Measure*)data;
	measure->ExecuteBang(args);
}

PLUGIN_EXPORT void Finalize(void* data)
{
	if (!System::VersionState::IsWin10())
	{
		RmLog(LOG_WARNING, L"FrostedGlass plugin is supported only on Windows 10 and 11.");
		return;
	}

	Measure* measure = (Measure*)data;
	measure->Finalize();
	delete(measure);
}

//-----------------------------------//
//              MEASURE              //
//-----------------------------------//

wstring Measure::ToWstring(const uint8_t _Value) noexcept
{
    //return std::to_wstring(static_cast<unsigned int>(_Value));
    wchar_t buffer[4];
    swprintf_s(buffer, L"%u", _Value);
    return buffer;
}

void Measure::ToLowerCase(wstring& _Input) noexcept
{
    //std::transform(_Input.begin(), _Input.end(), _Input.begin(), std::tolower);
    std::transform(_Input.begin(), _Input.end(), _Input.begin(),
                   [](wchar_t c) { return std::towlower(c); });
}

void Measure::ToRemoveSpace(wstring& _Input) noexcept
{
    _Input.erase(_Input.begin(),
                 std::find_if_not(_Input.begin(), _Input.end(),
                                  [](wchar_t c) { return std::iswspace(c); }));
}

bool Measure::Evaluate(wstring& _Input, const wstring& _Search) noexcept
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



uint8_t Measure::ClampColor(const int _Value) noexcept
{
    return static_cast<uint8_t>((_Value >= 255) ? 255 : ((_Value <= 0) ? 0 : _Value));
}

bool Measure::IsValidHexColor(wstring& _Color) noexcept
{
    if (_Color[0] == L'#')
        _Color.erase(0, 1);

    const size_t length = _Color.length();
    if (length != 6 && length != 8) return false;

    //return std::all_of(_Color.begin(), _Color.end(), std::iswxdigit);

    return std::all_of(_Color.begin(), _Color.end(), [](wchar_t c) { return std::iswxdigit(c); });
}

bool Measure::IsValidDecColor(wstring& _Color) noexcept
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


wstring Measure::GetColor(wstring& _Color) noexcept
{
    if (IsValidHexColor(_Color) || IsValidDecColor(_Color)) return _Color;

    this->m_IsColorValid = false;
    return L"000000";
}

uint32_t Measure::SetColor(wstring& _Color) noexcept
{
    if (_Color.size() == 6)
        _Color += L"00";

    const uint32_t _HexColor = std::stoul(_Color, nullptr, 16);
    uint32_t postColor (((_HexColor >> 24) & 0xFF) |
                        ((_HexColor >> 8)  & 0xFF00) |
                        ((_HexColor << 8)  & 0xFF0000) |
                        ((_HexColor << 24) & 0xFF000000));

    if (postColor <= 0) return 0x000001;

    return postColor;
}



template<typename A>
A Measure::SetValueA(const std::unordered_map<wstring, A>& _Map, wstring& _Value, const A _DefValue)
{
    if (_Value.empty()) return _DefValue;

    ToLowerCase(_Value);

    auto it = _Map.find(_Value);
    if (it != _Map.end()) return it->second;

    return _DefValue;
}

template<typename B>
B Measure::SetValueB(const std::unordered_map<wstring, B>& _Map, wstring& _Value, const B _DefValue)
{
    if (_Value.empty()) return _DefValue;

    ToLowerCase(_Value);

    auto it = _Map.find(_Value);
    if (it != _Map.end()) return it->second;

    return static_cast<B>(SetColor(GetColor(_Value)));
}



void Measure::GetBasic()
{
    m_IsDisabled = static_cast<bool>(RmReadInt(m_Rm, L"DISABLED", (1 - RmReadInt(m_Rm, L"BLURENABLED", 1))));
    m_IgnoreWarnings = static_cast<bool>(RmReadInt(m_Rm, L"IGNOREWARNINGS", 0));

    m_IsDarkMode[0] = static_cast<bool>(RmReadInt(m_Rm, L"DARKMODE", 0));
    m_IsMicaOnFocus[0] = static_cast<bool>(RmReadInt(m_Rm, L"MICAONFOCUS", 0));
    m_IsBorderVisible[0] = static_cast<bool>(RmReadInt(m_Rm, L"BORDERVISIBLE", 1));
}

void Measure::GetAccent()
{
    uint8_t h_ValueInt8 = static_cast<uint8_t>(RmReadInt(m_Rm, L"ACCENT", RmReadInt(m_Rm, L"TYPE", 0)));

    if (h_ValueInt8 > 0)
    {
        wstring h_ValueConverted = ToWstring(h_ValueInt8);
        m_Accent[0] = SetValueA(Maps::AccentMap, h_ValueConverted, ACCENT::BLUR);
        m_Mica[0] = SetValueA(Maps::MicaMap, h_ValueConverted, MICA::DEFAULT);
        return;
    }

    wstring h_ValueString = RmReadString(m_Rm, L"ACCENT", RmReadString(m_Rm, L"TYPE", L""));
    m_Accent[0] = SetValueA(Maps::AccentMap, h_ValueString, ACCENT::BLUR);
    m_Mica[0] = SetValueA(Maps::MicaMap, h_ValueString, MICA::DEFAULT);
}

void Measure::GetEffect()
{
    uint8_t h_ValueInt8 = static_cast<uint8_t>(RmReadInt(m_Rm, L"EFFECT", 0));

    if (h_ValueInt8 > 0)
    {
        m_Effect[0] = SetValueA(Maps::EffectMap, ToWstring(h_ValueInt8), EFFECT::DEFAULT);
        return;
    }

    wstring h_ValueString = RmReadString(m_Rm, L"EFFECT", L"");
    m_Effect[0] = SetValueA(Maps::EffectMap, h_ValueString, EFFECT::DEFAULT);
}

void Measure::GetShadow()
{
    wstring h_ValueString = RmReadString(m_Rm, L"SHADOW", RmReadString(m_Rm, L"BORDER", L""));

    if (h_ValueString.empty()) return;

    ToLowerCase(h_ValueString);
    while (!h_ValueString.empty())
    {
        if (Evaluate(h_ValueString, L"top")) m_Shadow[0] = static_cast<SHADOW>(static_cast<uint16_t>(m_Shadow[0]) | static_cast<uint16_t>(SHADOW::TOP));
        if (Evaluate(h_ValueString, L"left")) m_Shadow[0] = static_cast<SHADOW>(static_cast<uint16_t>(m_Shadow[0]) | static_cast<uint16_t>(SHADOW::LEFT));
        if (Evaluate(h_ValueString, L"right")) m_Shadow[0] = static_cast<SHADOW>(static_cast<uint16_t>(m_Shadow[0]) | static_cast<uint16_t>(SHADOW::RIGHT));
        if (Evaluate(h_ValueString, L"bottom")) m_Shadow[0] = static_cast<SHADOW>(static_cast<uint16_t>(m_Shadow[0]) | static_cast<uint16_t>(SHADOW::BOTTOM));
        if (Evaluate(h_ValueString, L"none"))
        {
            m_Shadow[0] = SHADOW::DEFAULT;
            break;
        }
        if (Evaluate(h_ValueString, L"all"))
        {
            m_Shadow[0] = SHADOW::ALL;
            break;
        }
        if (!h_ValueString.empty() && !Evaluate(h_ValueString, L"|"))
        {
            m_IsShadowValid = false;
            m_Shadow[0] = SHADOW::DEFAULT;
            break;
        }
    }
}

void Measure::GetBackdrop()
{
    wstring h_ValueString = RmReadString(m_Rm, L"BACKDROP", L"");
    m_Backdrop[0] = SetValueB(Maps::BackdropMap, h_ValueString, BACKDROP::DEFAULT);
}

void Measure::GetCorner()
{
    uint8_t h_ValueInt8 = static_cast<uint8_t>(RmReadInt(m_Rm, L"CORNER", 0));

    if (h_ValueInt8 > 0)
    {
        m_Corner[0] = SetValueA(Maps::CornerMap, ToWstring(h_ValueInt8), CORNER::DEFAULT);
        return;
    }

    wstring h_ValueString = RmReadString(m_Rm, L"CORNER", L"");
    m_Corner[0] = SetValueA(Maps::CornerMap, h_ValueString, CORNER::DEFAULT);
}

void Measure::GetStrokeColor()
{
    if (!m_IsBorderVisible[0])
    {
        m_StrokeColor[0] = STROKECOLOR::HIDDEN;
        return;
    }

    wstring h_ValueString = RmReadString(m_Rm, L"STROKECOLOR", RmReadString(m_Rm, L"BORDERCOLOR", L""));

    if (h_ValueString.empty() || (_wcsicmp(h_ValueString.c_str(), L"DEFAULT") == 0))
    {
        m_StrokeColor[0] = STROKECOLOR::VISIBLE;
        return;
    }

    if ((_wcsicmp(h_ValueString.c_str(), L"BACKDROP") == 0))
    {
        m_StrokeColor[0] = static_cast<STROKECOLOR>(static_cast<uint32_t>(m_Backdrop[0]) & 0x00FFFFFF);
        return;        
    }

    m_StrokeColor[0] = static_cast<STROKECOLOR>(SetColor(GetColor(h_ValueString)) & 0x00FFFFFF);
}



bool Measure::IsNewerData() const
{
    if (m_IsDarkMode[0] == m_IsDarkMode[1] &&
        m_IsMicaOnFocus[0] == m_IsMicaOnFocus[1] &&
        m_IsBorderVisible[0] == m_IsBorderVisible[1] &&
        m_Accent[0] == m_Accent[1] &&
        m_Mica[0] == m_Mica[1] &&
        m_Effect[0] == m_Effect[1] &&
        m_Shadow[0] == m_Shadow[1] &&
        m_Backdrop[0] == m_Backdrop[1] &&
        m_Corner[0] == m_Corner[1] &&
        m_StrokeColor[0] == m_StrokeColor[1]) return false;

    return true;
}

void Measure::SetNewerData()
{
    m_IsDarkMode[1] = m_IsDarkMode[0];
    m_IsMicaOnFocus[1] = m_IsMicaOnFocus[0];
    m_IsBorderVisible[1] = m_IsBorderVisible[0];
    m_Accent[1] = m_Accent[0];
    m_Mica[1] = m_Mica[0];
    m_Effect[1] = m_Effect[0];
    m_Shadow[1] = m_Shadow[0];
    m_Backdrop[1] = m_Backdrop[0];
    m_Corner[1] = m_Corner[0];
    m_StrokeColor[1] = m_StrokeColor[0];
}



void Measure::CheckFeatures()
{
    if (m_IsDarkMode[0] && !System::VersionState::IsWin11())
    {
        m_IsDarkModeSupported = false;
        m_IsDarkMode[0] = false;
    }

    if (m_Accent[0] == ACCENT::ACRYLIC && !System::VersionState::IsWin11())
    {
        m_IsAcrylicSupported = false;
        m_Accent[0] = ACCENT::BLUR;
    }

    if (m_Mica[0] != MICA::DEFAULT && !System::VersionState::IsMica())
    {
        m_IsMicaSupported = false;
        m_IsMicaOnFocus[0] = false;
        m_Accent[0] = ACCENT::BLUR;
        m_Mica[0] = MICA::DEFAULT;
    }

    if (m_Corner[0] != CORNER::DEFAULT && !System::VersionState::IsWin11())
    {
        m_IsCornerSupported = false;
        m_Corner[0] = CORNER::DEFAULT;
    }

    if (m_StrokeColor[0] != STROKECOLOR::VISIBLE && !System::VersionState::IsWin11()) 
    {
        m_IsStrokeColorSupported = false;
        m_StrokeColor[0] = STROKECOLOR::VISIBLE;
    }

    if (m_Accent[0] == ACCENT::BLUR && m_Effect[0] == EFFECT::LUMINANCE)
    {
        m_IsLuminanceSupported = false;
        m_Effect[0] = EFFECT::DEFAULT;
    }

    // ----- Tweaks ----- //

    if (m_Accent[0] == ACCENT::BACKDROP)
    {
        m_Shadow[0] = SHADOW::DEFAULT;
    }

    if (m_Accent[0] == ACCENT::BACKDROP_TRASLUCENT)
    {
        m_Effect[0] = static_cast<EFFECT>(static_cast<uint16_t>(m_Effect[0]) | static_cast<uint16_t>(EFFECT::LUMINANCE));
        m_Shadow[0] = SHADOW::DEFAULT;
    }

    if (m_Mica[0] == MICA::DEFAULT && m_Effect[0] >= EFFECT::FULLSCREEN)
    {
        m_Shadow[0] = SHADOW::DEFAULT;
        m_Corner[0] = CORNER::DEFAULT;
    }
}

bool Measure::CheckBlur()
{
    if (!m_IsDisabled) return true;

    m_Accent[2] = m_Accent[0];
    m_Mica[2] = m_Mica[0];
    m_Corner[2] = m_Corner[0];
    m_Accent[0] = ACCENT::DEFAULT;
    m_Mica[0] = MICA::DEFAULT;
    m_Corner[0] = CORNER::DEFAULT;

    return false;
}

void Measure::CheckErrors() const
{
    if (!m_IsShadowValid) RmLogF(m_Rm, LOG_ERROR, L"[Border] - Invalid border format, expected | between tokens");
    if (!m_IsColorValid) RmLogF(m_Rm, LOG_ERROR, L"[Backdrop/BorderColor] - Invalid color format");

    if (m_IgnoreWarnings) return;

    if (!m_IsDarkModeSupported) RmLogF(m_Rm, LOG_WARNING, L"[Dark Mode] - Required Windows 11 build %d.", System::BUILDS::WINDOWS_11);
    if (!m_IsAcrylicSupported) RmLogF(m_Rm, LOG_WARNING, L"[Type - Acrylic] - Required Windows 11 build %d.", System::BUILDS::WINDOWS_11);
    if (!m_IsMicaSupported) RmLogF(m_Rm, LOG_WARNING, L"[Type - Mica] - Required Windows 11 build %d.", System::BUILDS::WINDOWS_MICA);
    if (!m_IsCornerSupported) RmLogF(m_Rm, LOG_WARNING, L"[Corner] - Required Windows 11 build %d.", System::BUILDS::WINDOWS_11);
    if (!m_IsStrokeColorSupported) RmLogF(m_Rm, LOG_WARNING, L"[Border Visible/Border Color] - Required Windows 11 build %d.", System::BUILDS::WINDOWS_11);
    if (!m_IsLuminanceSupported) RmLogF(m_Rm, LOG_WARNING, L"[Effect] - Unable to use this effect 'Luminance' when 'Type=Blur'.");

    if (wcslen(RmReadString(m_Rm, L"BLURENABLED", L"")) > 0) RmLogF(m_Rm, LOG_WARNING, L"'BLURENABLED' option is being deprecated. Use 'DISABLED' option instead.");
    if (wcslen(RmReadString(m_Rm, L"BORDER", L"")) > 0) RmLogF(m_Rm, LOG_WARNING, L"'BORDER' option is being deprecated. Use 'SHADOW' option instead.");
}



void Measure::Initialize(void* rm)
{
    m_Rm = rm;
    m_Hwnd = RmGetSkinWindow(rm);
    API::InstanceState::Initialize();
}

void Measure::Reload()
{
    GetBasic();
    GetAccent();
    GetEffect();
    GetShadow();
    GetBackdrop();
    GetCorner();
    GetStrokeColor();

    CheckFeatures();
    CheckBlur();

    if (!IsNewerData()) return;

    if ((m_Accent[0] != m_Accent[1]) ||
        (m_Effect[0] != m_Effect[1]) ||
        (m_Shadow[0] != m_Shadow[1]) ||
        (m_Backdrop[0] != m_Backdrop[1]))
        API::WindowStyler::SetAccent(m_Hwnd, m_Accent[0], m_Effect[0], m_Shadow[0], m_Backdrop[0]);

    if (m_Mica[0] != m_Mica[1] || !m_IsMicaOnFocus[0])
        API::WindowStyler::SetMica(m_Hwnd, m_Mica[0], m_IsMicaOnFocus[0]);

    if (m_Corner[0] != m_Corner[1])
        API::WindowStyler::SetCorner(m_Hwnd, m_Corner[0]);

    if (m_StrokeColor[0] != m_StrokeColor[1])
        API::WindowStyler::SetStrokeColor(m_Hwnd, m_StrokeColor[0]);

    API::WindowStyler::SetDarkMode(m_Hwnd, m_IsDarkMode[0]);

    CheckErrors();

    SetNewerData();
}

void Measure::Update() const
{
    if (m_Mica[1] != MICA::DEFAULT && m_IsMicaOnFocus[1])
        API::WindowStyler::SetMica(m_Hwnd, m_Mica[1], m_IsMicaOnFocus[1]);
}

void Measure::ExecuteBang(LPCWSTR args)
{
    if (!API::InstanceState::IsUser32Loaded() && !API::InstanceState::IsDwmapiLoaded()) return;

    wstring sargs = args;

    switch (SetValueA(Maps::CommandMap, sargs, COMMAND::DEFAULT))
    {
        case COMMAND::BLUR_TOGGLE:
            if (m_Accent[1] == ACCENT::DEFAULT && m_Mica[1] == MICA::DEFAULT)
            {
                m_Accent[1] = m_Accent[2];
                m_Mica[1] = m_Mica[2];
                m_Corner[1] = m_Corner[2];
            }
            else
            {
                m_Accent[2] = m_Accent[1];
                m_Mica[2] = m_Mica[1];
                m_Corner[2] = m_Corner[1];
                m_Accent[1] = ACCENT::DEFAULT;
                m_Mica[1] = MICA::DEFAULT;
                m_Corner[1] = CORNER::DEFAULT;
            }
            API::WindowStyler::SetAccent(m_Hwnd, m_Accent[1], m_Effect[1], m_Shadow[1], m_Backdrop[1]);
            API::WindowStyler::SetMica(m_Hwnd, m_Mica[1], m_IsMicaOnFocus[1]);
            API::WindowStyler::SetCorner(m_Hwnd, m_Corner[1]);
            break;
        case COMMAND::BLUR_ENABLE:
            if (m_Accent[1] == ACCENT::DEFAULT && m_Mica[1] == MICA::DEFAULT)
            {
                m_Accent[1] = m_Accent[2];
                m_Mica[1] = m_Mica[2];
                m_Corner[1] = m_Corner[2];
                API::WindowStyler::SetAccent(m_Hwnd, m_Accent[1], m_Effect[1], m_Shadow[1], m_Backdrop[1]);
                API::WindowStyler::SetMica(m_Hwnd, m_Mica[1], m_IsMicaOnFocus[1]);
                API::WindowStyler::SetCorner(m_Hwnd, m_Corner[1]);
            }
            break;
        case COMMAND::BLUR_DISABLE:
            if (m_Accent[1] != ACCENT::DEFAULT || m_Mica[1] != MICA::DEFAULT)
            {
                m_Accent[2] = m_Accent[1];
                m_Mica[2] = m_Mica[1];
                m_Corner[2] = m_Corner[1];
                m_Accent[1] = ACCENT::DEFAULT;
                m_Mica[1] = MICA::DEFAULT;
                m_Corner[1] = CORNER::DEFAULT;
                API::WindowStyler::SetAccent(m_Hwnd, m_Accent[1], m_Effect[1], m_Shadow[1], m_Backdrop[1]);
                API::WindowStyler::SetMica(m_Hwnd, m_Mica[1], m_IsMicaOnFocus[1]);
                API::WindowStyler::SetCorner(m_Hwnd, CORNER::DEFAULT);
            }
            break;
        default:
            break;
    }
}

void Measure::Finalize()
{
    if (m_Accent[1] != ACCENT::DEFAULT ||
        m_Effect[1] != EFFECT::DEFAULT ||
        m_Shadow[1] != SHADOW::DEFAULT ||
        m_Backdrop[1] != BACKDROP::DEFAULT)
        API::WindowStyler::SetAccent(m_Hwnd, ACCENT::DEFAULT, EFFECT::DEFAULT, SHADOW::DEFAULT, BACKDROP::DEFAULT);

    if (m_Mica[1] != MICA::DEFAULT)
        API::WindowStyler::SetMica(m_Hwnd, MICA::DEFAULT, false);

    if (m_Corner[1] != CORNER::DEFAULT)
        API::WindowStyler::SetCorner(m_Hwnd, CORNER::DEFAULT);

    if (m_StrokeColor[1] != STROKECOLOR::VISIBLE)
        API::WindowStyler::SetStrokeColor(m_Hwnd, STROKECOLOR::VISIBLE);

    API::InstanceState::Finalize();
    this->~Measure();
}