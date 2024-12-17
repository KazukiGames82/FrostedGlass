#pragma once
/*
	if (!accentType.empty())
	{
		if (_wcsicmp(accentType.c_str(), L"SOLIDBACKDROP") == 0)
		{
			m->OMT_Accent = DWMFA_SOLIDBACKDROP;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"TRANSPARENTBACKDROP") == 0)
		{
			m->OMT_Accent = DWMFA_COLORACCENT;
			m->OMT_Effect = DWMFE_LUMINANCE_HIGH;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"ACCENT") == 0)
		{
			m->OMT_Accent = DWMFA_COLORACCENT;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"BLUR") == 0)
		{
			m->OMT_Accent = DWMFA_BLURBEHIND;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"ACRYLIC") == 0)
		{
			m->OMT_Accent = DWMFA_ACRYLIC;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"MICA") == 0)
		{
			m->OMT_Mica = DWMFA_MICA_BASE;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"MICAACRYLIC") == 0)
		{
			m->OMT_Mica = DWMFA_MICA_ACRYLIC;
			return;
		}
		if (_wcsicmp(accentType.c_str(), L"MICAALT") == 0)
		{
			m->OMT_Mica = DWMFA_MICA_ALT;
			return;
		}
	}*/
/*if (compare(sargs, L"SETBACKDROP"))
{
	if (compare(sargs, L"LIGHT"))
	{
		if (compare(sargs, L"BASE")) m->Backdrop = DWMFB_WINLIGHT_BASE;
		if (compare(sargs, L"5")) m->Backdrop = DWMFB_LIGHT5;
		if (compare(sargs, L"4")) m->Backdrop = DWMFB_LIGHT4;
		if (compare(sargs, L"3")) m->Backdrop = DWMFB_LIGHT3;
		if (compare(sargs, L"2")) m->Backdrop = DWMFB_LIGHT2;
		if (compare(sargs, L"1")) m->Backdrop = DWMFB_LIGHT;
	}
	if (compare(sargs, L"DARK"))
	{
		if (compare(sargs, L"BASE")) m->Backdrop = DWMFB_WINDARK_BASE;
		if (compare(sargs, L"5")) m->Backdrop = DWMFB_DARK5;
		if (compare(sargs, L"4")) m->Backdrop = DWMFB_DARK4;
		if (compare(sargs, L"3")) m->Backdrop = DWMFB_DARK3;
		if (compare(sargs, L"2")) m->Backdrop = DWMFB_DARK2;
		if (compare(sargs, L"1")) m->Backdrop = DWMFB_DARK;
	}

	SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
	return;
}*/
/*if (compare(sargs, L"SETBLUR"))
{
	if (m->Accent == DWMFA_NONE && m->Mica == DWMFA_MICA_NONE) return;

	m->Accent = DWMFA_NONE;
	m->Mica = DWMFA_MICA_NONE;

	if (compare(sargs, L"DEFAULT"))
	{
		m->Accent = DWMFA_BLURBEHIND;
	}
	if (compare(sargs, L"ACCENT"))
	{
		m->Accent = DWMFA_COLORACCENT;
		SetCorner(m->skin, m->error_HModule, m->isWin11, DWMFC_DONOTROUND);
	}
	if (compare(sargs, L"BACKDROP"))
	{
		m->Accent = DWMFA_SOLIDBACKDROP;
	}
	if (compare(sargs, L"ACRYLIC"))
	{
		m->Accent = DWMFA_ACRYLIC;
	}
	if (compare(sargs, L"MICA"))
	{
		m->Accent = DWMFA_NONE;
		m->Mica = DWMFA_MICA_BASE;
		if (compare(sargs, L"ACRYLIC")) m->Mica = DWMFA_MICA_ACRYLIC;
		if (compare(sargs, L"ALT")) m->Mica = DWMFA_MICA_ALT;
	}

	m->temp_Accent = m->Accent;
	m->temp_Mica = m->Mica;

	SetAccent(m->skin, m->error_HModule, m->Accent, m->Effect, m->SquareBorder, m->Backdrop);
	SetMica(m->skin, m->error_HModule, m->isWin11Mica, m->Mica, m->MicaFocus);
	SetCorner(m->skin, m->error_HModule, m->isWin11, m->Corner);
	return;
}*/
