
#include "StdAfx.h"
#include "Plugin.h"

/**************************************
*              VARIABLES              *
**************************************/

bool isValidWinVersion;
int references = 0;

/**************************************
*              FUNCTIONS              *
**************************************/

inline static bool isAtLeastWin10Build(const DWORD& buildNumber)
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

	while (in.size() > end && in[end] == L' ') 
		++end;
	
	in.erase(0, end);
	
	if (_wcsnicmp(in.c_str(), search.c_str(), search.size()) == 0)
	{
		in.erase(0, search.size());
		return true;
	}
	return false;
}
inline static int setChoice(Measure*& m, const std::unordered_map<std::wstring, int>& map, std::wstring& value, const int& defValue, const int& hasColor, const int& hasAlpha)
{
	std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) { return std::toupper(c, std::locale()); });

	auto it = map.find(value);

	if (hasColor) return (!value.empty() && !(it != map.end())) ? setColor(rgbaToHex(m, value, hasAlpha)) : (it != map.end()) ? it->second : defValue;

	return (it != map.end()) ? it->second : defValue;
}
inline static std::wstring rgbaToHex(Measure*& m, const std::wstring& color, const bool& useAlpha)
{
	std::wregex hexPattern(L"^(#[a-fA-F0-9]{6}|#[a-fA-F0-9]{8}|[a-fA-F0-9]{6}|[a-fA-F0-9]{8})$");
	
	if (std::regex_match(color, hexPattern)) return color;

	int r = NULL, g = NULL, b = NULL, a = NULL;

	if (swscanf_s(color.c_str(), L"%d,%d,%d,%d", &r, &g, &b, &a) >= 3) 
	{
		std::wstringstream ss;

		r = (r > 255 ? 255 : (r < 0 ? 0 : r));
		g = (g > 255 ? 255 : (g < 0 ? 0 : g));
		b = (b > 255 ? 255 : (b < 0 ? 0 : b));
		ss << std::hex << std::setw(2) << std::setfill(L'0') << r
			<< std::hex << std::setw(2) << std::setfill(L'0') << g
			<< std::hex << std::setw(2) << std::setfill(L'0') << b;

		if (useAlpha)
		{
			a = (a > 255 ? 255 : (a < 0 ? 0 : a));	
			ss << std::hex << std::setw(2) << std::setfill(L'0') << a;
		}
		return ss.str();
	}
	m->errorColor = true;
	return L"000000";
}
inline static uint32_t setColor(std::wstring& skinColor)
{
	uint32_t color = DWMFB_NONE;

	if (_wcsnicmp(skinColor.c_str(), L"#", 1) == 0U) 
		skinColor = skinColor.substr(1, skinColor.size());
	
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

inline static void loadModule(Measure*& m)
{
	if (m->errorHModule || SetWindowCompositionAttribute == NULL)
	{
		if ((hModule = LoadLibrary(TEXT("user32.dll"))))
		{
			m->errorHModule = false;
			SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
			GetWindowCompositionAttribute = (pGetWindowCompositionAttribute)GetProcAddress(hModule, "GetWindowCompositionAttribute");
		}
		if ((hDwmApi = LoadLibrary(L"DWMAPI.dll")))
		{
			m->errorHDwmapi = false;
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

inline static void getMeasureOptions(void*& rm, Measure*& m)
{
	m->BlurEnabled = (RmReadInt(rm, L"BlurEnabled", 1) == 1) ? true : false;
	m->IgnoreWarnings = (RmReadInt(rm, L"IgnoreWarnings", 0) == 1) ? true : false;
	m->DynamicVariables = (RmReadInt(rm, L"DynamicVariables", 0) == 1) ? true : false;
	m->Disabled = (RmReadInt(rm, L"Disabled", 0) == 1) ? true : false;
	m->OMT_DarkMode = (RmReadInt(rm, L"DarkMode", 0) == 1) ? true : false;
	m->OMT_MicaFocus = (RmReadInt(rm, L"MicaOnFocus", 0) == 1) ? true : false;
}
inline static void getAccent(void*& rm, Measure*& m)
{
	static const std::unordered_map<std::wstring, int> accentMap =
	{
		{L"BACKDROP", DWMFA_BACKDROP},
		{L"TRASLUCENTBACKDROP", DWMFA_TRASLUCENTBACKDROP},
		{L"BLUR", DWMFA_BLURBEHIND},
		{L"ACRYLIC", DWMFA_ACRYLIC}
	};
	static const std::unordered_map<std::wstring, int> micaMap =
	{
		{L"MICA", DWMFM_MICA},
		{L"MICAACRYLIC", DWMFM_MICA_ACRYLIC},
		{L"MICAALT", DWMFM_MICA_ALT}
	};
	
	std::wstring accentType = RmReadString(rm, L"Type", L"BLUR");
	
	m->OMT_Accent = setChoice(m, accentMap, accentType, DWMFA_NONE, false, false);
	m->OMT_Mica = setChoice(m, micaMap, accentType, DWMFM_NONE, false, false);
}
inline static void getEffect(void*& rm, Measure*& m)
{
	static const std::unordered_map<std::wstring, int> EffectMap =
	{
		{L"LUMINANCE", DWMFE_LUMINANCE},
		{L"FULLSCREEN", DWMFE_FULLSCREEN_LOW}
	};

	std::wstring effectType = RmReadString(rm, L"Effect", L"");

	m->OMT_Effect = setChoice(m, EffectMap, effectType, DWMFE_NONE, false, false);
}
inline static void getSquareBorder(void*& rm, Measure*& m)
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
			m->errorSquareBorder = true;
			m->OMT_SquareBorder = DWMFSB_NONE;
			break;
		}
	}
}
inline static void getBackdrop(void*& rm, Measure*& m)
{
	static const std::unordered_map<std::wstring, int> backdropMap =
	{
		{L"LIGHTBASE", DWMFB_LIGHT_BASE},
		{L"DARK", DWMFB_DARK},
		{L"LIGHT", DWMFB_LIGHT},
		{L"DARK2", DWMFB_DARK2},
		{L"LIGHT2", DWMFB_LIGHT2},
		{L"DARK3", DWMFB_DARK3},
		{L"LIGHT3", DWMFB_LIGHT3},
		{L"DARKBASE", DWMFB_DARK_BASE},
		{L"DARK4", DWMFB_DARK4},
		{L"LIGHT4", DWMFB_LIGHT4},
		{L"DARK5", DWMFB_DARK5},
		{L"LIGHT5", DWMFB_LIGHT5}
	};

	std::wstring backdropType = RmReadString(rm, L"Backdrop", L"");

	m->OMT_Backdrop = setChoice(m, backdropMap, backdropType, DWMFB_NONE, true, true);
}
inline static void getRoundCorner(void*& rm, Measure*& m)
{
	static const std::unordered_map<std::wstring, int> cornerMap =
	{
		{L"ROUND", DWMFRC_ROUND},
		{L"ROUNDWS", DWMFRC_ROUNDWS},
		{L"ROUNDSMALL", DWMFRC_ROUNDSMALL}
	};

	std::wstring cornerType = RmReadString(rm, L"Corner", L"");

	m->OMT_Corner = setChoice(m, cornerMap, cornerType, DWMFRC_DONOTROUND, false, false);
}
inline static void getRoundBorder(void*& rm, Measure*& m)
{
	if (RmReadInt(rm, L"BorderVisible", 1) == 0)
	{
		m->OMT_RoundBorder = DWMFRB_HIDDEN;
		return;
	}
		std::wstring borderColor = RmReadString(rm, L"BorderColor", L"");
		
		if (borderColor.empty())
		{
			m->OMT_RoundBorder = DWMFRB_VISIBLE;
			return;
		}
			if (_wcsicmp(borderColor.c_str(), L"BACKDROP") == 0)
			{
				static const std::unordered_map<std::wstring, int> backdropMap =
				{
					{L"LIGHTBASE", DWMFB_LIGHT_BASE},
					{L"DARK", DWMFB_DARK},
					{L"LIGHT", DWMFB_LIGHT},
					{L"DARK2", DWMFB_DARK2},
					{L"LIGHT2", DWMFB_LIGHT2},
					{L"DARK3", DWMFB_DARK3},
					{L"LIGHT3", DWMFB_LIGHT3},
					{L"DARKBASE", DWMFB_DARK_BASE},
					{L"DARK4", DWMFB_DARK4},
					{L"LIGHT4", DWMFB_LIGHT4},
					{L"DARK5", DWMFB_DARK5},
					{L"LIGHT5", DWMFB_LIGHT5}
				};

				std::wstring backdropType = RmReadString(rm, L"Backdrop", L"");

				m->OMT_RoundBorder = setChoice(m, backdropMap, backdropType, DWMFB_NONE, true, false);
				return;
			}
				m->OMT_RoundBorder = setColor(rgbaToHex(m, borderColor, false));
}
inline static void BackwardsCompability(Measure*& m)
{
	if (m->BlurEnabled && !m->Disabled) return;

		m->temp_Accent = m->OMT_Accent;
		m->temp_Mica = m->OMT_Mica;
		m->temp_Corner = m->OMT_Corner;

		m->OMT_Accent = DWMFA_NONE;
		m->OMT_Mica = DWMFM_NONE;
		m->OMT_Corner = DWMFRC_DONOTROUND;	
}

/*-----------------------------------*/

inline static void SetAccent(const HWND& hwnd, const bool& skinError, const int& skinAccent, const int& skinEffect, const int& skinSquareBorder, const int& skinBackdrop)
{
	if (skinError) return;

	AccentPolicy policy = { skinAccent, (skinEffect | skinSquareBorder) , skinBackdrop, 0 };
	WinCompAttrData data = { WCA_ACCENT_POLICY, &policy, sizeof(policy) };
	SetWindowCompositionAttribute(hwnd, &data);
}
inline static void SetMica(const HWND& hwnd, const bool& skinError, const bool& skinWindowMica, const int& skinMica, const bool& skinMicaFocus)
{
	if (skinError) return;
	if (!skinWindowMica) return;

	if (skinMica == DWMFM_NONE)
	{
		SetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &skinMica, sizeof(skinMica));
		return;
	}
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

inline static void checkFeatures(Measure*& m)
{ 
	// -- DARK MODE -- //
	if (m->OMT_DarkMode == TRUE && !m->isWin11)
	{
		m->warnDarkMode = true;
		m->OMT_DarkMode = FALSE;
	}
	// -- ACCENT -- //
	if (m->OMT_Accent != DWMFA_NONE && m->OMT_Accent != DWMFA_TRASLUCENTBACKDROP && m->OMT_Accent != DWMFA_BLURBEHIND && m->OMT_Accent != DWMFA_BACKDROP && !m->isWin11)
	{
		if (m->OMT_Accent == DWMFA_ACRYLIC) 
			m->warnAccent = true;
		m->OMT_Accent = DWMFA_BLURBEHIND;
	}
	// -- MICA -- //
	if (m->OMT_Mica != DWMFM_NONE && !m->isWin11Mica)
	{
		m->warnMica = true;
		m->OMT_Accent = DWMFA_BLURBEHIND;
		m->OMT_Mica = DWMFM_NONE;
		m->MicaFocus = FALSE;
	}
	// -- CORNER -- //
	if (m->OMT_Corner != DWMFRC_DONOTROUND && !m->isWin11)
	{
		m->warnCorner = true;
		m->OMT_Corner = DWMFRC_DONOTROUND;
	}
	// -- ROUND BORDER -- //
	if (m->OMT_RoundBorder != DWMFRB_VISIBLE && !m->isWin11)
	{
		m->warnRoundBorder = true;
		m->OMT_RoundBorder = DWMFRB_VISIBLE;
	}
	// -- EFFECT -- //
	if (m->OMT_Accent == DWMFA_BLURBEHIND && m->OMT_Effect != DWMFE_FULLSCREEN_LOW)
		m->OMT_Effect = DWMFE_NONE;
	if (m->OMT_Accent == DWMFA_TRASLUCENTBACKDROP)
		m->OMT_Effect |= DWMFE_LUMINANCE;
	if (m->OMT_Mica == DWMFM_NONE && m->OMT_Effect >= DWMFE_FULLSCREEN_LOW)
	{
		m->OMT_SquareBorder = DWMFSB_NONE;
		m->OMT_Corner = DWMFRC_DONOTROUND;
	}
}
inline static void checkErrors(void*& rm, Measure*& m)
{
	if (m->errorAccent)		RmLogF(rm, LOG_ERROR, L"[Type] - Invalid type format, expected | between tokens");
	if (m->errorSquareBorder)	RmLogF(rm, LOG_ERROR, L"[Border] - Invalid border format, expected | between tokens");
	if (m->errorColor)			RmLogF(rm, LOG_ERROR, L"[Backdrop/BorderColor] - Invalid color format");

	if (m->IgnoreWarnings) return;

		if (m->warnDarkMode)		RmLogF(rm, LOG_WARNING, L"[Dark Mode] - Required Windows 11 build 22000.");
		if (m->warnAccent)			RmLogF(rm, LOG_WARNING, L"[Type - Acrylic] - Required Windows 11 build 22000.");
		if (m->warnMica)			RmLogF(rm, LOG_WARNING, L"[Type - Mica] - Required Windows 11 build 22621.");
		if (m->warnCorner)			RmLogF(rm, LOG_WARNING, L"[Corner] - Required Windows 11 build 22000.");
		if (m->warnRoundBorder)	RmLogF(rm, LOG_WARNING, L"[Border Visible/Border Color] - Required Windows 11 build 22000.");
}

/**************************************
*              RAINMETER              *
**************************************/

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	if (!IsWindows10OrGreater()) return;

	isValidWinVersion = true;

	Measure* m = new Measure;

	m->isWin11Mica = isAtLeastWin10Build(BUILD_22H2);
	m->isWin11 = m->isWin11Mica ? true : isAtLeastWin10Build(BUILD_WIN11);
	m->isWin10 = isAtLeastWin10Build(BUILD_1803);

	m->Skin = RmGetSkinWindow(rm);
	loadModule(m);
	*data = m;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	if (!isValidWinVersion) return;

	Measure* m = (Measure*)data;

	getMeasureOptions(rm, m);
	getAccent(rm, m);
	getEffect(rm, m);
	getSquareBorder(rm, m);
	getBackdrop(rm, m);
	getRoundCorner(rm, m);
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
		SetAccent(m->Skin, m->errorHModule, m->OMT_Accent, m->OMT_Effect, m->OMT_SquareBorder, m->OMT_Backdrop);
	
	if (m->Mica != m->OMT_Mica && !m->OMT_MicaFocus)
		SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->OMT_Mica, m->OMT_MicaFocus);
	
	if (m->Corner != m->OMT_Corner)
		SetCorner(m->Skin, m->errorHModule, m->isWin11, m->OMT_Corner);

	if (m->RoundBorder != m->OMT_RoundBorder)
		SetRoundBorder(m->Skin, m->errorHModule, m->isWin11, m->OMT_RoundBorder);

	SetDarkMode(m->Skin, m->errorHModule, m->isWin11, m->OMT_DarkMode);

	checkErrors(rm, m);

	m->DarkMode			= m->OMT_DarkMode;
	m->MicaFocus		= m->OMT_MicaFocus;
	m->Accent			= m->OMT_Accent;
	m->Mica				= m->OMT_Mica;
	m->SquareBorder		= m->OMT_SquareBorder;
	m->Backdrop			= m->OMT_Backdrop;
	m->Corner			= m->OMT_Corner;
	m->RoundBorder		= m->OMT_RoundBorder;
	m->Effect			= m->OMT_Effect;
}

PLUGIN_EXPORT double Update(void* data)
{
	if (!isValidWinVersion) return 0.0;

	Measure* m = (Measure*)data;

	if ((m->Mica != DWMFM_NONE) && (m->MicaFocus)) 
		SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);

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

	if (m->errorHModule) return;

	if (compare(sargs, L"TOGGLEBLUR"))
	{
		if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE)
		{
			m->Accent = m->temp_Accent;
			m->Mica = m->temp_Mica;

			SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
			return;
		}
		else
		{
			m->temp_Accent = m->Accent;
			m->temp_Mica = m->Mica;
			m->Accent = DWMFA_NONE;
			m->Mica = DWMFM_NONE;

			SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			SetCorner(m->Skin, m->errorHModule, m->isWin11, DWMFRC_DONOTROUND);
			return;
		}
	}
	if (compare(sargs, L"ENABLEBLUR"))
	{
		if (m->Accent != DWMFA_NONE || m->Mica != DWMFM_NONE) return;

		m->Accent = m->temp_Accent;
		m->Mica = m->temp_Mica;

		SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
		SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
		SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
		return;
	}
	if (compare(sargs, L"DISABLEBLUR"))
	{
		if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;

		m->temp_Accent = m->Accent;
		m->temp_Mica = m->Mica;
		m->Accent = DWMFA_NONE;
		m->Mica = DWMFM_NONE;

		SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
		SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
		SetCorner(m->Skin, m->errorHModule, m->isWin11, DWMFRC_DONOTROUND);
		return;
	}

	if (m->isWin11Mica)
	{
		if (compare(sargs, L"TOGGLEFOCUS"))
		{
			m->MicaFocus = !m->MicaFocus;

			SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
		if (compare(sargs, L"ENABLEFOCUS"))
		{
			m->MicaFocus = true;

			SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
		if (compare(sargs, L"DISABLEFOCUS"))
		{
			m->MicaFocus = false;

			SetMica(m->Skin, m->errorHModule, m->isWin11Mica, m->Mica, m->MicaFocus);
			return;
		}
	}

	if (m->isWin11)
	{
		if (compare(sargs, L"TOGGLEMODE"))
		{
			m->DarkMode = !m->DarkMode;

			SetDarkMode(m->Skin, m->errorHModule, m->isWin11, m->DarkMode);
			return;
		}
		if (compare(sargs, L"LIGHTMODE"))
		{
			m->DarkMode = FALSE;

			SetDarkMode(m->Skin, m->errorHModule, m->isWin11, m->DarkMode);
			return;
		}
		if (compare(sargs, L"DARKMODE"))
		{
			m->DarkMode = TRUE;

			SetDarkMode(m->Skin, m->errorHModule, m->isWin11, m->DarkMode);
			return;
		}
		
		if (compare(sargs, L"TOGGLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;

			if (m->Corner == DWMFRC_DONOTROUND)
			{
				m->Corner = m->temp_Corner;
				m->temp_Corner = DWMFRC_DONOTROUND;
			}
			else
			{
				m->temp_Corner = m->Corner;
				m->Corner = DWMFRC_DONOTROUND;
			}

			SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"ENABLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;
			if (m->Corner != DWMFRC_DONOTROUND) return;

			m->Corner = m->temp_Corner;

			SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"DISABLECORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;
			if (m->Corner == DWMFRC_DONOTROUND) return;

			m->temp_Corner = m->Corner;
			m->Corner = DWMFRC_DONOTROUND;

			SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
			return;
		}
		if (compare(sargs, L"SETCORNER"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;

			m->Corner = DWMFRC_DONOTROUND;

			if (compare(sargs, L"ROUNDSMALL")) m->Corner = DWMFRC_ROUNDSMALL;
			if (compare(sargs, L"ROUNDWS")) m->Corner = DWMFRC_ROUNDWS;
			if (compare(sargs, L"ROUND")) m->Corner = DWMFRC_ROUND;

			m->temp_Corner = m->Corner;

			SetCorner(m->Skin, m->errorHModule, m->isWin11, m->Corner);
			return;
		}
		
		if (compare(sargs, L"TOGGLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;

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

			SetRoundBorder(m->Skin, m->errorHModule, m->isWin11, m->RoundBorder);
			return;
		}
		if (compare(sargs, L"ENABLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;
			if (m->RoundBorder != DWMFRB_HIDDEN) return;

			m->RoundBorder = m->temp_RoundBorder;

			SetRoundBorder(m->Skin, m->errorHModule, m->isWin11, m->RoundBorder);
			return;
		}
		if (compare(sargs, L"DISABLEBORDERS"))
		{
			if (m->Accent == DWMFA_NONE && m->Mica == DWMFM_NONE) return;
			if (m->RoundBorder == DWMFRB_HIDDEN) return;

			m->temp_RoundBorder = m->RoundBorder;
			m->RoundBorder = DWMFRB_HIDDEN;

			SetRoundBorder(m->Skin, m->errorHModule, m->isWin11, m->RoundBorder);
			return;
		}
		
		if (compare(sargs, L"TOGGLEBACKDROP"))
		{
			if (m->Backdrop == DWMFB_DARK || m->Backdrop == DWMFB_LIGHT) m->Backdrop = m->Backdrop == DWMFB_DARK ? DWMFB_LIGHT : DWMFB_DARK;
			if (m->Backdrop == DWMFB_DARK2 || m->Backdrop == DWMFB_LIGHT2) m->Backdrop = m->Backdrop == DWMFB_DARK2 ? DWMFB_LIGHT2 : DWMFB_DARK2;
			if (m->Backdrop == DWMFB_DARK3 || m->Backdrop == DWMFB_LIGHT3) m->Backdrop = m->Backdrop == DWMFB_DARK3 ? DWMFB_LIGHT3 : DWMFB_DARK3;
			if (m->Backdrop == DWMFB_DARK4 || m->Backdrop == DWMFB_LIGHT4) m->Backdrop = m->Backdrop == DWMFB_DARK4 ? DWMFB_LIGHT4 : DWMFB_DARK4;
			if (m->Backdrop == DWMFB_DARK5 || m->Backdrop == DWMFB_LIGHT5) m->Backdrop = m->Backdrop == DWMFB_DARK5 ? DWMFB_LIGHT5 : DWMFB_DARK2;
			if (m->Backdrop == DWMFB_DARK_BASE || m->Backdrop == DWMFB_LIGHT_BASE) m->Backdrop = m->Backdrop == DWMFB_DARK_BASE ? DWMFB_LIGHT_BASE : DWMFB_DARK_BASE;

			SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
		if (compare(sargs, L"LIGHTBACKDROP"))
		{
			if (m->Backdrop == DWMFB_DARK) m->Backdrop = DWMFB_LIGHT;
			if (m->Backdrop == DWMFB_DARK2) m->Backdrop = DWMFB_LIGHT2;
			if (m->Backdrop == DWMFB_DARK3) m->Backdrop = DWMFB_LIGHT3;
			if (m->Backdrop == DWMFB_DARK4) m->Backdrop = DWMFB_LIGHT4;
			if (m->Backdrop == DWMFB_DARK5) m->Backdrop = DWMFB_LIGHT5;
			if (m->Backdrop == DWMFB_DARK_BASE) m->Backdrop = DWMFB_LIGHT_BASE;

			SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
		if (compare(sargs, L"DARKBACKDROP"))
		{
			if (m->Backdrop == DWMFB_LIGHT) m->Backdrop = DWMFB_DARK;
			if (m->Backdrop == DWMFB_LIGHT2) m->Backdrop = DWMFB_DARK2;
			if (m->Backdrop == DWMFB_LIGHT3) m->Backdrop = DWMFB_DARK3;
			if (m->Backdrop == DWMFB_LIGHT4) m->Backdrop = DWMFB_DARK4;
			if (m->Backdrop == DWMFB_LIGHT5) m->Backdrop = DWMFB_DARK5;
			if (m->Backdrop == DWMFB_LIGHT_BASE) m->Backdrop = DWMFB_DARK_BASE;

			SetAccent(m->Skin, m->errorHModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
			return;
		}
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
		SetAccent(m->Skin, m->errorHModule, DWMFA_NONE, DWMFE_NONE, DWMFSB_NONE, DWMFB_NONE);

	if (m->Mica != DWMFM_NONE) 
		SetMica(m->Skin, m->errorHModule, m->isWin11Mica, DWMFM_NONE, false);

	if (m->Corner != DWMFRC_DONOTROUND)
		SetCorner(m->Skin, m->errorHModule, m->isWin11, DWMFRC_DONOTROUND);

	if (m->RoundBorder != DWMFRB_VISIBLE)
		SetRoundBorder(m->Skin, m->errorHModule, m->isWin11, DWMFRB_VISIBLE);

	unloadModule();

	delete(m);
}