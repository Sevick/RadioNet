//All Windows Media Player visual plugin code written by Greg Ellis
//
//Some winamp 5 plugin code taken from plainamp
//http://plainamp.sourceforge.net

//The sonique visualization code was put together by Greg Ellis based on code
//from Sharky08 on un4seen as well as many other code snippets from Xminion, Ian
//and other contributors found in old un4seen message forum posts

// Much of the winamp visualization code was taken from the BASS_WA project
// Much of code used by this class and BASS_WA was originally written by Saïd Bougribate,
// all BASS related functions and some improvements are written by Peter Hebels.
// This code is released under GNU GPL, which means you have to credit the authors if
// you use it in your applications.
//
// Other contributors: Greg Ellis, Silhwan and Svante Boberg
//
//Sonique OpenGL Visualization Rendering provided by Tuniac open source project
//Authors and site can be found here http://sourceforge.net/projects/tuniac/


typedef VisInfo* (WINAPI *QUERYMODULE)(void);
typedef HRESULT (CALLBACK *DllGetClassObjectFunc)(REFCLSID clsid, REFIID iid, LPVOID *ppv);
typedef long (WINAPI *BBPROC)(BBPLUGIN* BB);

class CCriticalSection
{
protected:
	HANDLE	m_hMutex;

public:
	CCriticalSection()
	{
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
	}

	~CCriticalSection()
	{
		CloseHandle(m_hMutex);
	}

	void Lock(void)
	{
		WaitForSingleObject(m_hMutex, INFINITE);
	}

	void Unlock(void)
	{
		ReleaseMutex(m_hMutex);
	}
};


class CAutoLock
{
private:
	CCriticalSection *	m_CS;

public:

	CAutoLock( CCriticalSection *CS )
	{
		m_CS = CS;
		m_CS->Lock();
	}

	~CAutoLock(void)
	{
		m_CS->Unlock();
	}
};


class CWinampWindow
{
public:
	CWinampWindow(){hwnd=NULL;bUsed=false;pointer=NULL;};
	~CWinampWindow(){};

	HWND hwnd;
	bool bUsed;
	void* pointer;
};

class CPlugin
{
public:
	CPlugin(HWND hWnd, HINSTANCE hInstance, LPCWSTR strPath, HWND hPluginWnd, int nWidth, int nHeight, bool bOpenGL, bool bOpenGLDB, int nIndex);
	~CPlugin();

	//plugin information
	int			m_nIndex;
	HWND		m_hWnd;
	HINSTANCE	m_hInstance;
	HBRUSH		hBrush;
	HSFX		handle;
	int			nPluginType;
	HSTREAM		m_hStream;
	int			m_nCurrentModule;
	int			m_nModuleCount;
	TCHAR		strAppPath[MAX_PATH];
	TCHAR		strVisIni[MAX_PATH];
	TCHAR		strPluginPath[MAX_PATH];
	char		strPluginName[MAX_PATH];
	TCHAR		wstrPluginName[MAX_PATH];
	DWORD		m_dwErrorCode;
	DWORD		m_dwFlags;
	vector<tstring> m_aryModuleNameW;
	vector<string> m_aryModuleName;
	
	//winamp stuff
	TCHAR strClassName[MAX_PATH];
	TCHAR strEmbedClassName[MAX_PATH];
	BOOL bWinampCreated;
	HWND m_hPluginWnd;
	BOOL bEnableRendering;
	BOOL bStopped;
	BOOL bMaximizeRestore;
	HWND hWinampWnd, hWinampWnd2;
	HANDLE hWinampThreadHandle;
	DWORD  dwWinampThreadID;
	UINT   winampPluginSamples;
	signed short		pcmBuffer[1152];
	signed short		fftBuffer[2048];
	CRITICAL_SECTION cslock;
	winampVisModule* pWinampVisModule[3];
	winampVisHeader* pWinampVisHeader;
	winampVisGetHeaderType visGetHeaderProc;
	static HWND embedWindow(embedWindowState *);
	static void CALLBACK WinampRenderTimer(UINT uId, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2);
	static LRESULT CALLBACK WinampWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK EmbedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI StartWinampThread(LPVOID);
	void DestroyWinampWindow();
	BOOL CreateWinampWindow();
	HANDLE CreateWinampThread();

	//sonique stuff
	char *pdata;
	BITMAPINFO bi;
	HDRAWDIB hdd;
	HBITMAP hSonBitmap, hOldSonBitmap;
	HDC hSonDC;
	unsigned char* sondata;

	bool			m_bEnableOpenGL;
	bool			m_bEnableOpenGLDoubleBuffer;
	HDC				m_glDC;
	HGLRC			m_glRC;
	unsigned long*	m_textureData;
	RECT			m_NextVisRect;
	RECT			m_PrevVisRect;
	int				m_LastWidth;
	int				m_LastHeight;
	int				iVisRes;
	VisInfo*	visInfo;
	VisData		visData;
	HMODULE		visHandle;
	HBITMAP		visBMP, hOldBitmap;
	HDC			visDC;
	BYTE		*visbuf;
	int			visWidth, visHeight;
	int			nVersion;
	QUERYMODULE queryModuleProc;
	
	//wmp stuff
	signed short			wmp_pcmBuffer[1152];
	signed short			wmp_fftBuffer[2048];
	DllGetClassObjectFunc	dllGetClassObjectProc;
	IWMPEffects2*			pWMP;
	
	//bassbox stuff
	BBPROC bbProc;
	BBPLUGIN bbPlugin;
	HDC   m_glDC_bbp;
	HGLRC m_glRC_bbp;
	float bb_fft[256];
	float bb_data[256];
	
	//plugin functions
	DWORD Flags(DWORD flags, DWORD mask);
	BOOL CreatePlugin();
	BOOL SetStream(HSTREAM hStream);
	BOOL StartPlugin();
	BOOL StopPlugin();
	BOOL Config();
	BOOL SetCurrentModule(int nModule);
	char* GetModuleName(int nModule);
	LPCWSTR GetModuleNameW(int nModule);
	char* GetName();
	LPCWSTR GetNameW();
	HBITMAP Render(HSTREAM hStream, HDC hDC);
	BOOL Resize(int nLeft, int nTop, int nWidth, int nHeight);
	BOOL Resize(int nWidth, int nHeight);
	BOOL FreePlugin();
	BOOL Clicked(int x, int y);
};
