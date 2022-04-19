
class CFont
{
	HFONT				m_hFont;

public:
	CFont(int nHeight,               // height of font
		  int nWidth,                // average character width
		  int nEscapement,           // angle of escapement
		  int nOrientation,          // base-line orientation angle
		  int fnWeight,              // font weight
		  DWORD fdwItalic,           // italic attribute option
		  DWORD fdwUnderline,        // underline attribute option
		  DWORD fdwStrikeOut,        // strikeout attribute option
		  DWORD fdwCharSet,          // character set identifier
		  DWORD fdwOutputPrecision,  // output precision
		  DWORD fdwClipPrecision,    // clipping precision
		  DWORD fdwQuality,          // output quality
		  DWORD fdwPitchAndFamily,   // pitch and family
		  LPCTSTR lpszFace           // typeface 
		  );

	CFont();
	HFONT				Create(
		  int nHeight,               // height of font
		  int nWidth,                // average character width
		  int nEscapement,           // angle of escapement
		  int nOrientation,          // base-line orientation angle
		  int fnWeight,              // font weight
		  DWORD fdwItalic,           // italic attribute option
		  DWORD fdwUnderline,        // underline attribute option
		  DWORD fdwStrikeOut,        // strikeout attribute option
		  DWORD fdwCharSet,          // character set identifier
		  DWORD fdwOutputPrecision,  // output precision
		  DWORD fdwClipPrecision,    // clipping precision
		  DWORD fdwQuality,          // output quality
		  DWORD fdwPitchAndFamily,   // pitch and family
		  LPCTSTR lpszFace           // typeface 
		  );

	HFONT			Create(INT iHeight, BOOL bBold, BOOL bItalic, BOOL bStriked, BOOL bUnderline, LPTSTR lpszFontFace, UINT fQuality);

	void LoadFromWindow(HWND hWndFrom, INT iHeight = -1, INT bBold = -1, INT bItalic = -1, INT bStriked = -1, INT bUnderline = -1, LPTSTR lpszFontFace = NULL, UINT fQuality = -1);

	operator HFONT();
	HFONT operator=(HFONT);
	~CFont();
};