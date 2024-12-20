#pragma once

/**************************************
*              CONSTANTS              *
**************************************/

constexpr uint8_t WCA_ACCENT_POLICY      = 19; // No description provided 
constexpr uint8_t IMMERSIVE_DARK_MODE    = 20; // Allows a skin to either use the accent color, or dark, according to the user Color Mode preferences.

constexpr DWORD BUILD_22H2               = 22621; // Windows 11 22H2 first to support mica properly
constexpr DWORD BUILD_WIN11              = 22000; // Windows 11 first "stable" build
constexpr DWORD BUILD_1803               = 17134; // Windows 10 1803 (April 2018 Update)

constexpr uint8_t HEX_BASE               = 16;
constexpr uint8_t RGB_STR_SIZE           = 6;
constexpr uint8_t RGBA_STR_SIZE          = 8;
constexpr uint8_t BIT_WISE_SHIFT_4       = 4;
constexpr uint8_t BIT_WISE_SHIFT_8       = 8;
constexpr uint8_t BIT_WISE_SHIFT_24      = 24;
constexpr uint32_t MASK_AA               = 0xFF;
constexpr uint32_t MASK_BB               = 0xFF00;
constexpr uint32_t MASK_GG               = 0xFF0000;
constexpr uint32_t MASK_RR               = 0xFF000000;

/*****************************************
*              ENUMERATIONS              *
*****************************************/

enum DWM_FROSTEDGLASS_ACCENT : uint8_t
{
    DWMFA_NONE                           = 0, // [Default] - Remove any blur -
    DWMFA_BACKDROP                       = 1, // [Solid Color] - Use backdrop color -
    DWMFA_TRASLUCENTBACKDROP             = 2, // [Solid Color] - Use backdrop + alpha color -
    DWMFA_BLURBEHIND                     = 3, // [Blur] - Use basic blur -
    DWMFA_ACRYLIC                        = 4, // [Blur] - Use Acrylic blur -
    DWMFA_HOSTBACKDROP                   = 5, // [UNUSED] - No description provided - 
    DWMFA_TRANSPARENTFULL                = 6, // [UNUSED] - No description provided -
};

enum DWM_FROSTEDGLASS_MICA : uint8_t
{
    DWMFM_NONE                           = 1, // [Default] - Remove any mica effect -
    DWMFM_MICA                           = 2, // [Mica] - Use Mica base effect -
    DWMFM_MICA_ACRYLIC                   = 3, // [Mica] - Use Mica Acrylic effect -
    DWMFM_MICA_ALT                       = 4, // [Mica] - Use Mica Alt effect -
};

enum DWM_FROSTEDGLASS_SQUARE_BORDER : uint16_t
{
    DWMFSB_NONE                          = 0,   // [Default] - Remove borders -
    DWMFSB_LEFT                          = 32,  // [Border] - Add left border -
    DWMFSB_TOP                           = 64,  // [Border] - Add top border -
    DWMFSB_RIGHT                         = 128, // [Border] - Add right border -
    DWMFSB_BOTTOM                        = 256, // [Border] - Add bottom border - 
    DWMFSB_ALL                           = 480  // [Border] - Add all border sides -
};

enum DWM_FROSTEDGLASS_BACKDROP : uint32_t
{
    DWMFB_NONE                           = 0x00000000, // [Default] - Use default color for backdrop -
    DWMFB_DARK_BASE                      = 0x94202020, // [Windows Dark] - Use windows dark color, with 58% opacity -
    DWMFB_LIGHT_BASE                     = 0x00F3F3F3,  // [Windows Light] - Use windows light color, with 0% opacity -

    // [DEPRECATED] - DON'T USE ANYMORE | STILL WORKING FOR BWC - //
    DWMFB_DARK                           = 0x01000000, // [Dark 1] - Use full black color, with 1% opacity -
    DWMFB_DARK2                          = 0x3F000000, // [Dark 2] - Use full black color, with 25% opacity -
    DWMFB_DARK3                          = 0x7E000000, // [Dark 3] - Use full black color, with 50% opacity -
    DWMFB_DARK4                          = 0xBD000000, // [Dark 4] - Use full black color, with 75% opacity -
    DWMFB_DARK5                          = 0xFF000000, // [Dark 5] - Use full black color, with 100% opacity -
    DWMFB_LIGHT                          = 0x01FFFFFF, // [Light 1] - Use full white color, with 1% opacity -
    DWMFB_LIGHT2                         = 0x3FFFFFFF, // [Light 2] - Use full white color, with 25% opacity -
    DWMFB_LIGHT3                         = 0x7EFFFFFF, // [Light 3] - Use full white color, with 50% opacity -
    DWMFB_LIGHT4                         = 0xBDFFFFFF, // [Light 4] - Use full white color, with 75% opacity -
    DWMFB_LIGHT5                         = 0xFFFFFFFF, // [Light 5] - Use full white color, with 100% opacity -
};

enum DWM_FROSTEDGLASS_ROUND_CORNER : uint8_t
{
    DWMFRC_DEFAULT                       = 0, // [UNUSED] - Let the system decide whether or not to round skin corners -
    DWMFRC_DONOTROUND                    = 1, // [Default] - Never round skin corners -
    DWMFRC_ROUND                         = 2, // [Corner] Round the corners if appropriate -
    DWMFRC_ROUNDSMALL                    = 3, // [Corner] Round the corners if appropriate, with a small radius -
    DWMFRC_ROUNDWS                       = 4  // [Corner] - Round the corners if appropriate, with less shadow -
};

enum DWM_FROSTEDGLASS_ROUND_BORDER : uint32_t
{
    DWMFRB_VISIBLE                       = 0xFFFFFFFF, // [Default] - Use default border color -
    DWMFRB_HIDDEN                        = 0xFFFFFFFE,  // [Hidden] - Remove border color -
};

enum DWM_FROSTEDGLASS_EFFECT : uint8_t
{
    DWMFE_NONE                           = 0, // [Default] - Remove any effect -
    DWMFE_LUMINANCE                      = 2, // [Effect] - Use high luminance for backdrop -
    DWMFE_FULLSCREEN_LOW                 = 4, // [Experimental] - Applies complete blur to current screen -
    DWMFE_FULLSCREEN_HIGH                = 6  // [Experimental] - Applies complete blur to current screen, with luminance effect -
};

/***************************************
*              STRUCTURES              *
***************************************/

struct AccentPolicy
{
    int nAccentState                     = NULL;
    int nFlags                           = NULL;
    int nColor                           = NULL;
    int nAnimationId                     = NULL;
};

struct WinCompAttrData
{
    int nAttribute                       = NULL;
    void* pData                          = nullptr;
    int ulDataSize                       = NULL;
};

struct Measure
{
    bool isWin11Mica                     = false;
    bool isWin11                         = false;
    bool isWin10                         = false;
    /*--------------------------------*/
    bool errorHModule                    = true;
    bool errorHDwmapi                    = true;
    bool errorAccent                     = false;
    bool errorSquareBorder               = false;
    bool errorColor                      = false;
    /*--------------------------------*/
    bool warnDarkMode                    = false;
    bool warnAccent                      = false;
    bool warnMica                        = false;
    bool warnCorner                      = false;
    bool warnRoundBorder                 = false;
    /*--------------------------------*/
    HWND Skin                            = NULL;
    bool BlurEnabled                     = false;
    bool IgnoreWarnings                  = false;
    bool DynamicVariables                = false;
    bool Disabled                        = false;
    /*--------------------------------*/
    BOOL OMT_DarkMode                    = FALSE;
    bool OMT_MicaFocus                   = false;
    uint8_t OMT_Accent                   = DWMFA_NONE;
    uint8_t OMT_Mica                     = DWMSBT_NONE;
    uint16_t OMT_SquareBorder            = DWMFSB_NONE;
    uint32_t OMT_Backdrop                = DWMFB_NONE;
    uint8_t OMT_Corner                   = DWMFRC_DONOTROUND;
    uint32_t OMT_RoundBorder             = DWMFRB_VISIBLE;
    uint8_t OMT_Effect                   = DWMFE_NONE;
    /*--------------------------------*/
    BOOL DarkMode                        = FALSE;
    bool MicaFocus                       = false;
    uint8_t Accent                       = DWMFA_NONE;
    uint8_t Mica                         = DWMFM_NONE;
    uint16_t SquareBorder                = DWMFSB_NONE;
    uint32_t Backdrop                    = DWMFB_NONE;
    uint8_t Corner                       = DWMFRC_DONOTROUND;
    uint32_t RoundBorder                 = DWMFRB_VISIBLE;
    uint8_t Effect                       = DWMFE_NONE;
    /*--------------------------------*/
    uint8_t temp_Accent                  = DWMFA_NONE;
    uint8_t temp_Mica                    = DWMSBT_NONE;
    uint8_t temp_Corner                  = DWMFRC_DONOTROUND;
    uint32_t temp_RoundBorder            = DWMFRB_VISIBLE;
};

/************************************
*              MODULES              *
************************************/

HINSTANCE hModule = NULL;
HMODULE hDwmApi = NULL;

typedef bool(WINAPI* pSetWindowCompositionAttribute)(HWND, WinCompAttrData*);
typedef bool(WINAPI* pGetWindowCompositionAttribute)(HWND, WinCompAttrData*);
typedef HRESULT(WINAPI* pDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);

static pSetWindowCompositionAttribute SetWindowCompositionAttribute = NULL;
static pGetWindowCompositionAttribute GetWindowCompositionAttribute = NULL;
static pDwmSetWindowAttribute SetWindowAttribute = NULL;

/****************************************
*              DECLARATION              *
****************************************/

inline static bool isAtLeastWin10Build(const DWORD& buildNumber);

/*--------------------------------*/

inline static bool compare(std::wstring& in, const std::wstring& search);
inline static int setChoice(Measure*& m, const std::unordered_map<std::wstring, int>& map, std::wstring& value, const int& defValue, const int& hasColor, const int& hasAlpha);
inline static std::wstring rgbaToHex(Measure*& m, const std::wstring& color, const bool& useAlpha);
inline static uint32_t setColor(std::wstring& skinColor);

/*--------------------------------*/

inline static void loadModule(Measure*& m);
inline static void unloadModule();

/*--------------------------------*/

inline static void getMeasureOptions(void*& rm, Measure*& m);
inline static void getAccent(void*& rm, Measure*& m);
inline static void getEffect(void*& rm, Measure*& m);
inline static void getSquareBorder(void*& rm, Measure*& m);
inline static void getBackdrop(void*& rm, Measure*& m);
inline static void getRoundCorner(void*& rm, Measure*& m);
inline static void getRoundBorder(void*& rm, Measure*& m);
inline static void BackwardsCompability(Measure*& m);

/*--------------------------------*/

inline static void SetAccent(const HWND& hwnd, const bool& skinError, const int& skinAccent, const int& skinEffect, const int& skinSquareBorder, const int& skinBackdrop);
inline static void SetMica(const HWND& hwnd, const bool& skinError, const bool& skinWindowMica, const int& skinMica, const bool& skinMicaFocus);
inline static void SetCorner(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const int& skinCorner);
inline static void SetRoundBorder(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const int& skinRoundBorder);
inline static void SetDarkMode(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const BOOL& skinDarkMode);

/*--------------------------------*/

inline static void checkFeatures(Measure*& m);
inline static void checkErrors(void*& rm, Measure*& m);

/*--------------------------------*/

PLUGIN_EXPORT void Initialize(void** data, void* rm);
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue);
PLUGIN_EXPORT double Update(void* data);
PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args);
PLUGIN_EXPORT void Finalize(void* data);
