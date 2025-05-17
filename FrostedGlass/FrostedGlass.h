#pragma once

#include "stdafx.h"

using namespace FrostedGlass;
using namespace Types;

class Measure
{
private:
    void* m_Rm = nullptr;
    HWND m_Hwnd = nullptr;
    
    bool m_IsShadowValid = true;
    bool m_IsColorValid = true;

    bool m_IsDarkModeSupported = true;
    bool m_IsAcrylicSupported = true;
    bool m_IsMicaSupported = true;
    bool m_IsCornerSupported = true;
    bool m_IsStrokeColorSupported = true;
    bool m_IsLuminanceSupported = true;

    bool m_IsDisabled =  false ;
    bool m_IgnoreWarnings = false;
    bool m_IsDarkMode[2] = { false };
    bool m_IsMicaOnFocus[2] = { false };
    bool m_IsBorderVisible[2] = { false };

    ACCENT m_Accent[3] = { ACCENT::DEFAULT };
    EFFECT m_Effect[2] = { EFFECT::DEFAULT };
    SHADOW m_Shadow[2] = { SHADOW::DEFAULT };
    BACKDROP m_Backdrop[2] = { BACKDROP::DEFAULT };
    MICA m_Mica[3] = { MICA::DEFAULT };
    CORNER m_Corner[3] = { CORNER::DEFAULT };
    STROKECOLOR m_StrokeColor[2] = { STROKECOLOR::VISIBLE };

    template<typename A>
    A SetValueA(const std::unordered_map<wstring, A>& _Map, wstring& _Value, const A _DefValue);
    template<typename B>
    B SetValueB(const std::unordered_map<wstring, B>& _Map, wstring& _Value, const B _DefValue);

    void GetBasic();
    void GetAccent();
    void GetEffect();
    void GetShadow();
    void GetBackdrop();
    void GetCorner();
    void GetStrokeColor();

    bool IsNewerData() const;
    void SetNewerData();

    void CheckFeatures();
    bool CheckBlur();
    void CheckErrors() const;

public:

    Measure() {};
    ~Measure() {};

    void Initialize(void* rm);
    void Reload();
    void Update() const;
    void ExecuteBang(LPCWSTR args);
    void Finalize();

    void setValidColor(const bool _IsValid);
};

//-----------------------------------//
//            DECLARATION            //
//-----------------------------------//

PLUGIN_EXPORT void Initialize(void** data, void* rm);
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue);
PLUGIN_EXPORT double Update(void* data);
PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args);
PLUGIN_EXPORT void Finalize(void* data);