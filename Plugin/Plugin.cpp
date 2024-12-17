
#include "StdAfx.h"
#include "Plugin.h"

/**********************************************
*            CHANGELOG 1.1.2 - 1.2.0          *
**********************************************/
/*
*	Measure:
*		- Added RGBA compatibility for "Backdrop" option.
*		- Added "Screen" and "ScreenLuminance" to "Effect" option.
*		- Added "BorderColor" option.
*		- Added "IgnoreWarnings" option.
*	Plugin:
*		- Fixed memory leak.
*		- Reduced compilation time.
*		- Reduced memory usage.
*		- 
*		-
*
*
*/

/**************************************
*              VARIABLES              *
**************************************/

bool isValidWinVersion;
int references = 0;

/**************************************
*              FUNCTIONS              *
**************************************/

inline static bool IsAtLeastWin10Build(DWORD buildNumber)
{
	if (!IsWindows10OrGreater()) return false;

	const auto MASK_ = VerSetConditionMask(0, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	OSVERSIONINFOEXW osvi{};
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwBuildNumber = buildNumber;
	return VerifyVersionInfo(&osvi, VER_BUILDNUMBER, MASK_) != FALSE;
}

/*-----------------------------------*/

inline static bool compare(std::wstring& in, const std::wstring& search)
{
	uint8_t end = 0;
	while (in.size() > end && in[end] == L' ') ++end;
	in.erase(0, end);
	if (_wcsnicmp(in.c_str(), search.c_str(), search.size()) == 0)
	{
		in.erase(0, search.size());
		return true;
	}
	return false;
}
inline static std::wstring rgbaToHex(Measure* m, const std::wstring& color, const bool& useAlpha)
{
	std::wregex hexPattern(L"^(#[a-fA-F0-9]{6}|#[a-fA-F0-9]{8}|[a-fA-F0-9]{6}|[a-fA-F0-9]{8})$");
	if (std::regex_match(color, hexPattern)) return color;

	int r = NULL, g = NULL, b = NULL, a = NULL;

	if (swscanf_s(color.c_str(), L"%d,%d,%d,%d", &r, &g, &b, &a) >= 3) {
		std::wstringstream ss;
	
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		ss << std::hex << std::setw(2) << std::setfill(L'0') << r
			<< std::hex << std::setw(2) << std::setfill(L'0') << g
			<< std::hex << std::setw(2) << std::setfill(L'0') << b;

		if (useAlpha)
		{
			if (a < 0) a = 0;
			if (a > 255) a = 255;
		
			ss << std::hex << std::setw(2) << std::setfill(L'0') << a;
		}
		return ss.str();
	}
	m->Error_Color = true;
	return L"000000";
}
inline static uint32_t setColor(std::wstring& skinColor)
{
	uint32_t color = DWMFB_NONE;

	if (_wcsnicmp(skinColor.c_str(), L"#", 1) == 0U) skinColor = skinColor.substr(1, skinColor.size());
	
	const size_t strSize = skinColor.size();
		
	color = std::stoul(skinColor, nullptr, HEX_BASE);

	if (strSize < RGBA_STR_SIZE) {
		color <<= BIT_WISE_SHIFT_4;
		if (strSize == RGB_STR_SIZE)
			color <<= BIT_WISE_SHIFT_4;
	}
	color = (((color & MASK_AA) << BIT_WISE_SHIFT_24) |
		((color & MASK_RR) >> BIT_WISE_SHIFT_24) |
		((color & MASK_BB) << BIT_WISE_SHIFT_8) |
		((color & MASK_GG) >> BIT_WISE_SHIFT_8));

	return color;
}

/*-----------------------------------*/

inline static void loadModule(Measure* m)
{
	if (m->error_HModule || SetWindowCompositionAttribute == NULL)
	{

		if ((hModule = LoadLibrary(TEXT("user32.dll"))))
		{
			m->error_HModule = false;
			SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
			GetWindowCompositionAttribute = (pGetWindowCompositionAttribute)GetProcAddress(hModule, "GetWindowCompositionAttribute");
		}
		if ((hDwmApi = LoadLibrary(L"DWMAPI.dll")))
		{
			m->error_HDwmapi = false;
			SetWindowAttribute = (pDwmSetWindowAttribute)GetProcAddress(hDwmApi, "DwmSetWindowAttribute");
		}
		if (SetWindowCompositionAttribute == NULL) RmLog(LOG_ERROR, L"Could not load the SetWindowCompositionAttribute function from user32.dll, did Microsoft remove it?");

		if (GetWindowCompositionAttribute == NULL) RmLog(LOG_ERROR, L"Could not load the GetWindowCompositionAttribute function from user32.dll, did Microsoft remove it?");

		if (SetWindowAttribute == NULL) RmLog(LOG_ERROR, L"Could not load the DwmSetWindowAttribute function from DWMAPI.dll");
	}
	references++;
}
inline static void unloadModule()
{
	references--;
	if (references <= 0)
	{
		FreeLibrary(hModule);
		FreeLibrary(hDwmApi);
		hModule = NULL;
		hDwmApi = NULL;
		SetWindowCompositionAttribute = NULL;
		SetWindowAttribute = NULL;
	}
}

/*-----------------------------------*/

inline static void getMeasureOptions(void* rm, Measure* m)
{
	m->BlurEnabled = (RmReadInt(rm, L"BlurEnabled", 1) == 1) ? true : false;
	m->IgnoreWarnings = (RmReadInt(rm, L"IgnoreWarnings", 0) == 1) ? true : false;
	m->DynamicVariables = (RmReadInt(rm, L"DynamicVariables", 0) == 1) ? true : false;
	m->Disabled = (RmReadInt(rm, L"Disabled", 0) == 1) ? true : false;
	m->OMT_DarkMode = (RmReadInt(rm, L"DarkMode", 0) == 1) ? true : false;
	m->OMT_MicaFocus = (RmReadInt(rm, L"MicaOnFocus", 0) == 1) ? true : false;
}
inline static void getAccent(void* rm, Measure* m)
{
	std::wstring accentType = RmReadString(rm, L"Type", L"BLUR");
	m->OMT_Accent = DWMFA_NONE;
	m->OMT_Mica = DWMFA_MICA_NONE;
	m->OMT_Effect = DWMFE_NONE;

	while (!accentType.empty())
	{
		if (compare(accentType, L"BLUR")) m->OMT_Accent = DWMFA_BLURBEHIND;
		if (compare(accentType, L"SOLIDBACKDROP")) m->OMT_Accent = DWMFA_SOLIDBACKDROP;
		if (compare(accentType, L"TRANSPARENTBACKDROP"))
		{
			m->OMT_Accent = DWMFA_COLORACCENT;
			m->OMT_Effect = DWMFE_LUMINANCE_HIGH;
		}
		if (compare(accentType, L"ACRYLIC")) m->OMT_Accent = DWMFA_ACRYLIC;
		if (compare(accentType, L"MICA"))
		{
			m->OMT_Mica = DWMFA_MICA_BASE;
			if (compare(accentType, L"ACRYLIC")) m->OMT_Mica = DWMFA_MICA_ACRYLIC;
			if (compare(accentType, L"ALT")) m->OMT_Mica = DWMFA_MICA_ALT;
		}
		if (compare(accentType, L"NONE")) break;
		if (compare(accentType, L"LUMINANCE")) m->OMT_Effect = DWMFE_LUMINANCE_HIGH;
		if (compare(accentType, L"FULLSCREEN")) m->OMT_Effect |= DWMFE_FULLSCREEN_LOW;

		if (!accentType.empty() && !compare(accentType, L"|"))
		{
			m->Error_Accent = true;
			break;
		}
	}
}
inline static void getSquareBorder(void* rm, Measure* m)
{
	std::wstring borderType = RmReadString(rm, L"Border", L"");
	m->OMT_SquareBorder = DWMFSB_NONE;

	while (!borderType.empty())
	{
		if (compare(borderType, L"TOP")) m->OMT_SquareBorder |= DWMFSB_TOP;
		if (compare(borderType, L"LEFT")) m->OMT_SquareBorder |= DWMFSB_LEFT;
		if (compare(borderType, L"RIGHT")) m->OMT_SquareBorder |= DWMFSB_RIGHT;
		if (compare(borderType, L"BOTTOM")) m->OMT_SquareBorder |= DWMFSB_BOTTOM;
		if (compare(borderType, L"NONE")) break;
		if (compare(borderType, L"ALL"))
		{
			m->OMT_SquareBorder = DWMFSB_ALL; 
			break;
		}

		if (!borderType.empty() && !compare(borderType, L"|"))
		{
			m->Error_SquareBorder = true;
			m->OMT_SquareBorder = DWMFSB_NONE;
			break;
		}
	}
}
inline static void getBackdrop(void* rm, Measure* m)
{
	std::wstring backdropType = RmReadString(rm, L"Backdrop", L"");
	m->OMT_Backdrop = DWMFB_NONE;

	if (!backdropType.empty())
	{
		if (_wcsicmp(backdropType.c_str(), L"DARK") == 0)
		{
			m->OMT_Backdrop = DWMFB_DARK;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHT") == 0)
		{
			m->OMT_Backdrop = DWMFB_LIGHT;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"DARK2") == 0)
		{
			m->OMT_Backdrop = DWMFB_DARK2;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHT2") == 0)
		{
			m->OMT_Backdrop = DWMFB_LIGHT2;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"DARK3") == 0)
		{
			m->OMT_Backdrop = DWMFB_DARK3;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHT3") == 0)
		{
			m->OMT_Backdrop = DWMFB_LIGHT3;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"DARK4") == 0)
		{
			m->OMT_Backdrop = DWMFB_DARK4;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHT4") == 0)
		{
			m->OMT_Backdrop = DWMFB_LIGHT4;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"DARK5") == 0)
		{
			m->OMT_Backdrop = DWMFB_DARK5;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHT5") == 0)
		{
			m->OMT_Backdrop = DWMFB_LIGHT5;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"DARKBASE") == 0)
		{
			m->OMT_Backdrop = DWMFB_WINDARK_BASE;
			return;
		}
		if (_wcsicmp(backdropType.c_str(), L"LIGHTBASE") == 0)
		{
			m->OMT_Backdrop = DWMFB_WINLIGHT_BASE;
			return;
		}
		m->OMT_Backdrop = setColor(rgbaToHex(m, backdropType, true));
	}
}
inline static void getCorner(void* rm, Measure* m)
{
	std::wstring cornerType = RmReadString(rm, L"Corner", L"");
	m->OMT_Corner = DWMFC_DONOTROUND;

	if (!cornerType.empty())
	{
		if (_wcsicmp(cornerType.c_str(), L"ROUND") == 0)
		{
			m->OMT_Corner = DWMFC_ROUND;
			return;
		}
		if (_wcsicmp(cornerType.c_str(), L"ROUNDWS") == 0)
		{
			m->OMT_Corner = DWMFC_ROUNDWS;
			return;
		}
		if (_wcsicmp(cornerType.c_str(), L"ROUNDSMALL") == 0)
		{
			m->OMT_Corner = DWMFC_ROUNDSMALL;
			return;
		}
	}
}
inline static void getRoundBorder(void* rm, Measure* m)
{
	bool borderVisible = (RmReadInt(rm, L"BorderVisible", 1) == 1) ? true : false;
	m->OMT_RoundBorder = DWMFRB_HIDDEN;

	if (borderVisible)
	{
		std::wstring borderColor = RmReadString(rm, L"BorderColor", L"");
		m->OMT_RoundBorder = DWMFRB_VISIBLE;

		if (!borderColor.empty())
		{
			m->OMT_RoundBorder = setColor(rgbaToHex(m, borderColor, false));
			return;
		}
	}
}
inline static void BackwardsCompability(Measure* m)
{
	if (!m->BlurEnabled || m->Disabled)
	{
		m->temp_Accent = m->OMT_Accent;
		m->temp_Mica = m->OMT_Mica;
		m->temp_Corner = m->OMT_Corner;

		m->OMT_Accent = DWMFA_NONE;
		m->OMT_Mica = DWMFA_MICA_NONE;
		m->OMT_Corner = DWMFC_DONOTROUND;	
	}
}

/*-----------------------------------*/

inline static void SetAccent(const HWND& hwnd, const bool& skinError, const int& skinAccent, const int& skinEffect, const int& skinSquareBorder, const int& skinBackdrop)
{
	if (skinError) return;

	AccentPolicy policy = { skinAccent, (skinSquareBorder | skinEffect) , skinBackdrop, 1 };
	WinCompAttrData data = { WCA_ACCENT_POLICY, &policy, sizeof(policy) };
	SetWindowCompositionAttribute(hwnd, &data);
}
inline static void SetMica(const HWND& hwnd, const bool& skinError, const bool& skinWindowMica, const int& skinMica, const bool& skinMicaFocus)
{
	if (skinError) return;
	if (!skinWindowMica) return;

	SetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &skinMica, sizeof(skinMica));

	if (skinMica == DWMFA_MICA_NONE) return;

	if (!skinMicaFocus) SetWindowPos(hwnd, hwnd, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	SetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &skinMica, sizeof(skinMica));

	margins = { 0 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);
}
inline static void SetCorner(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const int& skinCorner)
{
	if (skinError) return;
	if (!skinWindow11) return;

	SetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &skinCorner, sizeof(skinCorner));
}
inline static void SetRoundBorder(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const int& skinRoundBorder)
{
	if (skinError) return;
	if (!skinWindow11) return;

	SetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &skinRoundBorder, sizeof(skinRoundBorder));
}
inline static void SetDarkMode(const HWND& hwnd, const bool& skinError, const bool& skinWindow11, const BOOL& skinDarkMode)
{
	if (skinError) return;
	if (!skinWindow11) return;

	SetWindowAttribute(hwnd, IMMERSIVE_DARK_MODE, &skinDarkMode, sizeof(skinDarkMode));
}

/*-----------------------------------*/

inline static void checkFeatures(Measure* m)
{ 
	if (m->OMT_DarkMode == TRUE && !m->isWin11)
	{
		m->OMT_DarkMode = FALSE;
		m->Warn_DarkMode = true;
	}
	if (m->OMT_Accent != DWMFA_NONE && m->OMT_Accent != DWMFA_COLORACCENT && m->OMT_Accent != DWMFA_BLURBEHIND && m->OMT_Accent != DWMFA_SOLIDBACKDROP && !m->isWin11)
	{
		if (m->OMT_Accent == DWMFA_ACRYLIC) m->Warn_Accent = true;

		m->OMT_Accent = DWMFA_BLURBEHIND;
	}
	if (m->OMT_Mica != DWMFA_MICA_NONE && !m->isWin11Mica)
	{
		m->OMT_Accent = DWMFA_BLURBEHIND;
		m->OMT_Mica = DWMFA_MICA_NONE;
		m->MicaFocus = FALSE;
		m->Warn_Mica = true;
	}
	if (m->OMT_Corner != DWMFC_DONOTROUND && !m->isWin11)
	{
		m->OMT_Corner = DWMFC_DONOTROUND;
		m->Warn_Corner = true;
	}
	if (m->OMT_RoundBorder != DWMFRB_VISIBLE && !m->isWin11)
	{
		m->OMT_RoundBorder = DWMFRB_VISIBLE;
		m->Warn_RoundBorder = true;
	}
	if (m->OMT_Accent == DWMFA_BLURBEHIND && m->OMT_Effect != DWMFE_FULLSCREEN_LOW)
	{
		m->OMT_Effect = DWMFE_NONE;
	}
	if (m->OMT_Mica == DWMFA_MICA_NONE && m->OMT_Effect >= DWMFE_FULLSCREEN_LOW)
	{
		m->OMT_SquareBorder = DWMFSB_NONE;
		m->OMT_Corner = DWMFC_DONOTROUND;
	}
}
inline static void checkErrors(void* rm, Measure* m)
{
	if (m->Error_Accent)			RmLogF(rm, LOG_ERROR, L"[Type] - Invalid type format, expected | between tokens");
	if (m->Error_SquareBorder)		RmLogF(rm, LOG_ERROR, L"[Border] - Invalid border format, expected | between tokens");
	if (m->Error_Color)				RmLogF(rm, LOG_ERROR, L"[Backdrop/BorderColor] - Invalid color format");

	if (m->IgnoreWarnings) return;

	if (m->Warn_DarkMode)			RmLogF(rm, LOG_WARNING, L"[Dark Mode] - Required Windows 11 build 22000.");
	if (m->Warn_Accent)				RmLogF(rm, LOG_WARNING, L"[Type - Acrylic] - Required Windows 11 build 22000.");
	if (m->Warn_Mica)				RmLogF(rm, LOG_WARNING, L"[Type - Mica] - Required Windows 11 build 22621.");
	if (m->Warn_Corner)				RmLogF(rm, LOG_WARNING, L"[Corner] - Required Windows 11 build 22000.");
	if (m->Warn_RoundBorder)		RmLogF(rm, LOG_WARNING, L"[Border Visible] - Required Windows 11 build 22000.");
}

/**************************************
*              RAINMETER              *
**************************************/

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	if (!IsWindows10OrGreater()) return;

	isValidWinVersion = true;

	Measure* m = new Measure;

	m->isWin11Mica = IsAtLeastWin10Build(BUILD_22H2);
	m->isWin11 = m->isWin11Mica ? true : IsAtLeastWin10Build(BUILD_WIN11);
	m->isWin10 = IsAtLeastWin10Build(BUILD_1803);

	m->skin = RmGetSkinWindow(rm);
	loadModule(m);
	*data = m;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	if (!isValidWinVersion) return;

	Measure* m = (Measure*)data;

	getMeasureOptions(rm, m);
	getAccent(rm, m);
	getSquareBorder(rm, m);
	getBackdrop(rm, m);
	getCorner(rm, m);
	getRoundBorder(rm, m);
	checkFeatures(m);
	BackwardsCompability(m);

	if ((m->DarkMode		== m->OMT_DarkMode) &&
		(m->MicaFocus		== m->OMT_MicaFocus) &&
		(m->Accent			== m->OMT_Accent) &&
		(m->Mica			== m->OMT_Mica) &&
		(m->SquareBorder	== m->OMT_SquareBorder) &&
		(m->Backdrop		== m->OMT_Backdrop) &&
		(m->Corner			== m->OMT_Corner) &&
		(m->RoundBorder		== m->OMT_RoundBorder) &&
		(m->Effect			== m->OMT_Effect)) return;

	if (m->Accent != m->OMT_Accent || m->Effect != m->OMT_Effect || m->SquareBorder != m->OMT_SquareBorder || m->Backdrop != m->OMT_Backdrop)
		SetAccent(m->skin, m->error_HModule, m->OMT_Accent, m->OMT_Effect, m->OMT_SquareBorder, m->OMT_Backdrop);
	
	if (m->MicaFocus != m->OMT_MicaFocus || m->Mica != m->OMT_Mica)
		SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->OMT_Mica, m->OMT_MicaFocus);
	
	if (m->Corner != m->OMT_Corner)
		SetCorner(m->skin, m->error_HModule, m->isWin11, m->OMT_Corner);

	if (m->RoundBorder != m->OMT_RoundBorder)
		SetRoundBorder(m->skin, m->error_HModule, m->isWin11, m->OMT_RoundBorder);

	SetDarkMode(m->skin, m->error_HModule, m->isWin11, m->OMT_DarkMode);

	checkErrors(rm, m);

	m->DarkMode		= m->OMT_DarkMode;
	m->MicaFocus	= m->OMT_MicaFocus;
	m->Accent		= m->OMT_Accent;
	m->Mica			= m->OMT_Mica;
	m->SquareBorder	= m->OMT_SquareBorder;
	m->Backdrop		= m->OMT_Backdrop;
	m->Corner		= m->OMT_Corner;
	m->RoundBorder	= m->OMT_RoundBorder;
	m->Effect		= m->OMT_Effect;
}

PLUGIN_EXPORT double Update(void* data)
{
	if (!isValidWinVersion) return 0.0;

	Measure* m = (Measure*)data;

	if ((m->Mica != DWMFA_MICA_NONE) && (m->MicaFocus)) 
		SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);

	return 0.0;
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	if (!isValidWinVersion)
	{
		RmLog(LOG_WARNING, L"FrostedGlass commands are supported only on Windows 10 and 11.");
		return;
	}

	Measure* m = (Measure*)data;
	std::wstring sargs = args;

	if (m->error_HModule) return;

	if (m->isWin11Mica)
	{
		if (compare(sargs, L"TOGGLEFOCUS"))
		{
			m->MicaFocus = m->MicaFocus == true ? false : true;

			SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
		if (compare(sargs, L"ENABLEFOCUS"))
		{
			m->MicaFocus = true;

			SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
		if (compare(sargs, L"DISABLEFOCUS"))
		{
			m->MicaFocus = false;

			SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
	}

	if (m->isWin11)
	{
		if (compare(sargs, L"TOGGLEMODE"))
		{
			m->DarkMode = m->DarkMode == TRUE ? FALSE : TRUE;

			SetDarkMode(m->skin, m->error_HModule, m->isWin11, m->DarkMode);
			return;
		}
		if (compare(sargs, L"LIGHTMODE"))
		{
			m->DarkMode = FALSE;

			SetDarkMode(m->skin, m->error_HModule, m->isWin11, m->DarkMode);
			return;
		}
		if (compare(sargs, L"DARKMODE"))
		{
			m->DarkMode = TRUE;

			SetDarkMode(m->skin, m->error_HModule, m->isWin11, m->DarkMode);
			return;
		}
		
		if (compare(sargs, L"TOGGLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;

			if (m->Corner == DWMFC_DONOTROUND)
			{
				m->Corner = m->temp_Corner;
				m->temp_Corner = DWMFC_DONOTROUND;
			}
			else
			{
				m->temp_Corner = m->Corner;
				m->Corner = DWMFC_DONOTROUND;
			}

			SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"ENABLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;
			if (m->Corner != DWMFC_DONOTROUND) return;

			m->Corner = m->temp_Corner;

			SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"DISABLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;
			if (m->Corner == DWMFC_DONOTROUND) return;

			m->temp_Corner = m->Corner;
			m->Corner = DWMFC_DONOTROUND;

			SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"SETCORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;

			m->Corner = DWMFC_DONOTROUND;

			if (compare(sargs, L"ROUNDSMALL")) m->Corner = DWMFC_ROUNDSMALL;
			if (compare(sargs, L"ROUNDWS")) m->Corner = DWMFC_ROUNDWS;
			if (compare(sargs, L"ROUND")) m->Corner = DWMFC_ROUND;

			m->temp_Corner = m->Corner;

			SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
			return;
		}
		
		if (compare(sargs, L"TOGGLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;

			if (m->RoundBorder == DWMFRB_HIDDEN)
			{
				m->RoundBorder = m->temp_RoundBorder;
				m->temp_RoundBorder = DWMFRB_HIDDEN;
			}
			else
			{
				m->temp_RoundBorder = m->RoundBorder;
				m->RoundBorder = DWMFRB_HIDDEN;
			}

			SetRoundBorder(m->skin, m->error_HModule, m->isWin11, m->RoundBorder);
			return;
		}
		if (compare(sargs, L"ENABLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;
			if (m->RoundBorder != DWMFRB_HIDDEN) return;

			m->RoundBorder = m->temp_RoundBorder;

			SetRoundBorder(m->skin, m->error_HModule, m->isWin11, m->RoundBorder);
			return;
		}
		if (compare(sargs, L"DISABLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;
			if (m->RoundBorder == DWMFRB_HIDDEN) return;

			m->temp_RoundBorder = m->RoundBorder;
			m->RoundBorder = DWMFRB_HIDDEN;

			SetRoundBorder(m->skin, m->error_HModule, m->isWin11, m->RoundBorder);
			return;
		}
		
		if (compare(sargs, L"TOGGLEBACKDROP"))
		{
			if (m->Backdrop == DWMFB_DARK || m->Backdrop == DWMFB_LIGHT) m->Backdrop = m->Backdrop == DWMFB_DARK ? DWMFB_LIGHT : DWMFB_DARK;
			if (m->Backdrop == DWMFB_DARK2 || m->Backdrop == DWMFB_LIGHT2) m->Backdrop = m->Backdrop == DWMFB_DARK2 ? DWMFB_LIGHT2 : DWMFB_DARK2;
			if (m->Backdrop == DWMFB_DARK3 || m->Backdrop == DWMFB_LIGHT3) m->Backdrop = m->Backdrop == DWMFB_DARK3 ? DWMFB_LIGHT3 : DWMFB_DARK3;
			if (m->Backdrop == DWMFB_DARK4 || m->Backdrop == DWMFB_LIGHT4) m->Backdrop = m->Backdrop == DWMFB_DARK4 ? DWMFB_LIGHT4 : DWMFB_DARK4;
			if (m->Backdrop == DWMFB_DARK5 || m->Backdrop == DWMFB_LIGHT5) m->Backdrop = m->Backdrop == DWMFB_DARK5 ? DWMFB_LIGHT5 : DWMFB_DARK2;
			if (m->Backdrop == DWMFB_WINDARK_BASE || m->Backdrop == DWMFB_WINLIGHT_BASE) m->Backdrop = m->Backdrop == DWMFB_WINDARK_BASE ? DWMFB_WINLIGHT_BASE : DWMFB_WINDARK_BASE;

			SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
		if (compare(sargs, L"LIGHTBACKDROP"))
		{
			if (m->Backdrop == DWMFB_DARK) m->Backdrop = DWMFB_LIGHT;
			if (m->Backdrop == DWMFB_DARK2) m->Backdrop = DWMFB_LIGHT2;
			if (m->Backdrop == DWMFB_DARK3) m->Backdrop = DWMFB_LIGHT3;
			if (m->Backdrop == DWMFB_DARK4) m->Backdrop = DWMFB_LIGHT4;
			if (m->Backdrop == DWMFB_DARK5) m->Backdrop = DWMFB_LIGHT5;
			if (m->Backdrop == DWMFB_WINDARK_BASE) m->Backdrop = DWMFB_WINLIGHT_BASE;

			SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
		if (compare(sargs, L"DARKBACKDROP"))
		{
			if (m->Backdrop == DWMFB_LIGHT) m->Backdrop = DWMFB_DARK;
			if (m->Backdrop == DWMFB_LIGHT2) m->Backdrop = DWMFB_DARK2;
			if (m->Backdrop == DWMFB_LIGHT3) m->Backdrop = DWMFB_DARK3;
			if (m->Backdrop == DWMFB_LIGHT4) m->Backdrop = DWMFB_DARK4;
			if (m->Backdrop == DWMFB_LIGHT5) m->Backdrop = DWMFB_DARK5;
			if (m->Backdrop == DWMFB_WINLIGHT_BASE) m->Backdrop = DWMFB_WINDARK_BASE;

			SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
	}

	if (compare(sargs, L"TOGGLEBLUR"))
	{
		if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE)
		{
			m->Accent = m->temp_Accent;
			m->Mica = m->temp_Mica;

			SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
			return;
		}
		else
		{
			m->temp_Accent = m->Accent;
			m->temp_Mica = m->Mica;
			m->Accent = DWMFA_NONE;
			m->Mica = DWMFA_MICA_NONE;

			SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			SetCorner(m->skin, m->error_HModule, m->isWin11, DWMFC_DONOTROUND);
			return;
		}
	}
	if (compare(sargs, L"ENABLEBLUR"))
	{
		if (m->Accent != DWMFA_NONE || m->Mica != DWMFA_MICA_NONE) return;

		m->Accent = m->temp_Accent;
		m->Mica = m->temp_Mica;

		SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
		SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
		SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
		return;
	}
	if (compare(sargs, L"DISABLEBLUR"))
	{
		if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;

		m->temp_Accent = m->Accent;
		m->temp_Mica = m->Mica;
		m->Accent = DWMFA_NONE;
		m->Mica = DWMFA_MICA_NONE;

		SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
		SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
		SetCorner(m->skin, m->error_HModule, m->isWin11, DWMFC_DONOTROUND);
		return;
	}
}

PLUGIN_EXPORT void Finalize(void* data)
{
	if (!isValidWinVersion)
	{
		RmLog(LOG_WARNING, L"FrostedGlass plugin is supported only on Windows 10 and 11.");
		return;
	}

	Measure* m = (Measure*)data;

	if (m->Accent != DWMFA_NONE || m->Effect != DWMFE_NONE || m->SquareBorder != DWMFSB_NONE || m->Backdrop != DWMFB_NONE)
		SetAccent(m->skin, m->error_HModule, DWMFA_NONE, DWMFE_NONE, DWMFSB_NONE, DWMFB_NONE);

	if (m->Mica != DWMFA_MICA_NONE)
		SetMica(m->skin, m->error_HModule, m->isWin11Mica, DWMFA_MICA_NONE, false);

	if (m->Corner != DWMFC_DONOTROUND)
		SetCorner(m->skin, m->error_HModule, m->isWin11, DWMFC_DONOTROUND);

	unloadModule();

	delete(m);
}