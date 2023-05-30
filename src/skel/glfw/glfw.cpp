#if defined RW_GL3 && !defined LIBRW_SDL2

#ifdef _WIN32
#include <shlobj.h>
#include <basetsd.h>
#include <mmsystem.h>
#include <regstr.h>
#include <shellapi.h>
#include <windowsx.h>

DWORD _dwOperatingSystemVersion;
#include "resource.h"
#else
long _dwOperatingSystemVersion;
#ifndef __SWITCH__
#ifndef __APPLE__
#include <sys/sysinfo.h>
#else
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#endif
#endif
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stddef.h>
#endif

#include "common.h"
#if (defined(_MSC_VER))
#include <tchar.h>
#endif /* (defined(_MSC_VER)) */
#include <stdio.h>
#include "rwcore.h"
#include "skeleton.h"
#include "platform.h"
#include "crossplatform.h"

#include "main.h"
#include "FileMgr.h"
#include "Text.h"
#include "Pad.h"
#include "Timer.h"
#include "DMAudio.h"
#include "ControllerConfig.h"
#include "Frontend.h"
#include "Game.h"
#include "PCSave.h"
#include "MemoryCard.h"
#include "Sprite2d.h"
#include "AnimViewer.h"
#include "Font.h"
#include "MemoryMgr.h"

// This is defined on project-level, via premake5 or cmake
#ifdef GET_KEYBOARD_INPUT_FROM_X11
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#endif

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#define MAX_SUBSYSTEMS		(16)

rw::EngineOpenParams openParams;

static RwBool		  ForegroundApp = TRUE;
static RwBool		  WindowIconified = FALSE;
static RwBool		  WindowFocused = TRUE;

static RwBool		  RwInitialised = FALSE;

static RwSubSystemInfo GsubSysInfo[MAX_SUBSYSTEMS];
static RwInt32		GnumSubSystems = 0;
static RwInt32		GcurSel = 0, GcurSelVM = 0;

static RwBool useDefault;

// What is that for anyway?
#ifndef IMPROVED_VIDEOMODE
static RwBool defaultFullscreenRes = TRUE;
#else
static RwBool defaultFullscreenRes = FALSE;
static RwInt32 bestWndMode = -1;
#endif

static psGlobalType PsGlobal;


#define PSGLOBAL(var) (((psGlobalType *)(RsGlobal.ps))->var)

size_t _dwMemAvailPhys;
RwUInt32 gGameState;

#ifdef DETECT_JOYSTICK_MENU
char gSelectedJoystickName[128] = "";
#endif

/*
 *****************************************************************************
 */
void _psCreateFolder(const char *path)
{
#ifdef _WIN32
	HANDLE hfle = CreateFile(path, GENERIC_READ, 
									FILE_SHARE_READ,
									nil,
									OPEN_EXISTING,
									FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
									nil);

	if ( hfle == INVALID_HANDLE_VALUE )
		CreateDirectory(path, nil);
	else
		CloseHandle(hfle);
#else
	struct stat info;
	char fullpath[PATH_MAX];
	realpath(path, fullpath);

	if (lstat(fullpath, &info) != 0) {
		if (errno == ENOENT || (errno != EACCES && !S_ISDIR(info.st_mode))) {
			mkdir(fullpath, 0755);
		}
	}
#endif
}

/*
 *****************************************************************************
 */
const char *_psGetUserFilesFolder()
{
#if defined USE_MY_DOCUMENTS && defined _WIN32
	HKEY hKey = NULL;

	static CHAR szUserFiles[256];

	if ( RegOpenKeyEx(HKEY_CURRENT_USER,
						REGSTR_PATH_SPECIAL_FOLDERS,
						REG_OPTION_RESERVED,
						KEY_READ,
						&hKey) == ERROR_SUCCESS )
	{
		DWORD KeyType;
		DWORD KeycbData = sizeof(szUserFiles);
		if ( RegQueryValueEx(hKey,
							"Personal",
							NULL,
							&KeyType,
							(LPBYTE)szUserFiles,
							&KeycbData) == ERROR_SUCCESS )
		{
			RegCloseKey(hKey);
			strcat(szUserFiles, "\\GTA Vice City User Files");
			_psCreateFolder(szUserFiles);
			return szUserFiles;
		}	

		RegCloseKey(hKey);		
	}
	
	strcpy(szUserFiles, "data");
	return szUserFiles;
#else
	static char szUserFiles[256];
	strcpy(szUserFiles, "userfiles");
	_psCreateFolder(szUserFiles);
	return szUserFiles;
#endif
}

/*
 *****************************************************************************
 */
RwBool
psCameraBeginUpdate(RwCamera *camera)
{
	if ( !RwCameraBeginUpdate(Scene.camera) )
	{
		ForegroundApp = FALSE;
		RsEventHandler(rsACTIVATE, (void *)FALSE);
		return FALSE;
	}
	
	return TRUE;
}

/*
 *****************************************************************************
 */
void
psCameraShowRaster(RwCamera *camera)
{
#ifdef LEGACY_MENU_OPTIONS
	if (FrontEndMenuManager.m_PrefsVsync || FrontEndMenuManager.m_bMenuActive)
#else
	if (FrontEndMenuManager.m_PrefsFrameLimiter || FrontEndMenuManager.m_bMenuActive)
#endif
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPWAITVSYNC);
	else
		RwCameraShowRaster(camera, PSGLOBAL(window), rwRASTERFLIPDONTWAIT);

	return;
}

/*
 *****************************************************************************
 */
RwImage *
psGrabScreen(RwCamera *pCamera)
{
#ifndef LIBRW
	RwRaster *pRaster = RwCameraGetRaster(pCamera);
	if (RwImage *pImage = RwImageCreate(pRaster->width, pRaster->height, 32)) {
		RwImageAllocatePixels(pImage);
		RwImageSetFromRaster(pImage, pRaster);
		return pImage;
	}
#else
	rw::Image *image = RwCameraGetRaster(pCamera)->toImage();
	image->removeMask();
	if(image)
		return image;
#endif
	return nil;
}

/*
 *****************************************************************************
 */
#ifdef _WIN32
#pragma comment( lib, "Winmm.lib" ) // Needed for time
RwUInt32
psTimer(void)
{
	RwUInt32 time;

	TIMECAPS TimeCaps;
	
	timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
	
	timeBeginPeriod(TimeCaps.wPeriodMin);
	
	time = (RwUInt32) timeGetTime();

	timeEndPeriod(TimeCaps.wPeriodMin);
	
	return time;
}
#else
double
psTimer(void)
{
	struct timespec start; 
#if defined(CLOCK_MONOTONIC_RAW)
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
#elif defined(CLOCK_MONOTONIC_FAST)
	clock_gettime(CLOCK_MONOTONIC_FAST, &start);
#else
	clock_gettime(CLOCK_MONOTONIC, &start);
#endif
	return start.tv_sec * 1000.0 + start.tv_nsec/1000000.0;
}
#endif       


/*
 *****************************************************************************
 */
void
psMouseSetPos(RwV2d *pos)
{
	glfwSetCursorPos(PSGLOBAL(window), pos->x, pos->y);
	
	PSGLOBAL(lastMousePos.x) = (RwInt32)pos->x;

	PSGLOBAL(lastMousePos.y) = (RwInt32)pos->y;

	return;
}

/*
 *****************************************************************************
 */
RwMemoryFunctions*
psGetMemoryFunctions(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	return &memFuncs;
#else
	return nil;
#endif
}

/*
 *****************************************************************************
 */
RwBool
psInstallFileSystem(void)
{
	return (TRUE);
}


/*
 *****************************************************************************
 */
RwBool
psNativeTextureSupport(void)
{
	return true;
}

/*
 *****************************************************************************
 */
#ifdef UNDER_CE
#define CMDSTR	LPWSTR
#else
#define CMDSTR	LPSTR
#endif

/*
 *****************************************************************************
 */

#ifdef __SWITCH__

static HidVibrationValue SwitchVibrationValues[2];
static HidVibrationDeviceHandle SwitchVibrationDeviceHandles[2][2];
static HidVibrationDeviceHandle SwitchVibrationDeviceGC;

static PadState SwitchPad;

static Result HidInitializationResult[2];
static Result HidInitializationGCResult;

static void _psInitializeVibration()
{
	HidInitializationResult[0] = hidInitializeVibrationDevices(SwitchVibrationDeviceHandles[0], 2, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
	if(R_FAILED(HidInitializationResult[0])) {
		printf("Failed to initialize VibrationDevice for Handheld Mode\n");
	}
	HidInitializationResult[1] = hidInitializeVibrationDevices(SwitchVibrationDeviceHandles[1], 2, HidNpadIdType_No1, HidNpadStyleSet_NpadFullCtrl);
	if(R_FAILED(HidInitializationResult[1])) {
		printf("Failed to initialize VibrationDevice for Detached Mode\n");
	}
	HidInitializationGCResult = hidInitializeVibrationDevices(&SwitchVibrationDeviceGC, 1, HidNpadIdType_No1, HidNpadStyleTag_NpadGc);
	if(R_FAILED(HidInitializationResult[1])) {
		printf("Failed to initialize VibrationDevice for GC Mode\n");
	}

	SwitchVibrationValues[0].freq_low  = 160.0f;
	SwitchVibrationValues[0].freq_high = 320.0f;

	padConfigureInput(1, HidNpadStyleSet_NpadFullCtrl);
	padInitializeDefault(&SwitchPad);
}

static void _psHandleVibration()
{
	padUpdate(&SwitchPad);

	uint8 target_device = padIsHandheld(&SwitchPad) ? 0 : 1;

	if(R_SUCCEEDED(HidInitializationResult[target_device])) {
		CPad* pad = CPad::GetPad(0);

		// value conversion based on SDL2 switch port
		SwitchVibrationValues[0].amp_high = SwitchVibrationValues[0].amp_low = pad->ShakeFreq == 0 ? 0.0f : 320.0f;
		SwitchVibrationValues[0].freq_low = pad->ShakeFreq == 0.0 ? 160.0f : (float)pad->ShakeFreq * 1.26f;
		SwitchVibrationValues[0].freq_high = pad->ShakeFreq == 0.0 ? 320.0f : (float)pad->ShakeFreq * 1.26f;

		if (pad->ShakeDur < CTimer::GetTimeStepInMilliseconds())
			pad->ShakeDur = 0;
		else
			pad->ShakeDur -= CTimer::GetTimeStepInMilliseconds();
		if (pad->ShakeDur == 0) pad->ShakeFreq = 0;


		if(target_device == 1 && R_SUCCEEDED(HidInitializationGCResult)) {
			// gamecube rumble
			hidSendVibrationGcErmCommand(SwitchVibrationDeviceGC, pad->ShakeFreq > 0 ? HidVibrationGcErmCommand_Start : HidVibrationGcErmCommand_Stop);
		}

		memcpy(&SwitchVibrationValues[1], &SwitchVibrationValues[0], sizeof(HidVibrationValue));
		hidSendVibrationValues(SwitchVibrationDeviceHandles[target_device], SwitchVibrationValues, 2);
	}
}
#else
static void _psInitializeVibration() {}
static void _psHandleVibration() {}
#endif

/*
 *****************************************************************************
 */
RwBool
psInitialize(void)
{
	PsGlobal.lastMousePos.x = PsGlobal.lastMousePos.y = 0.0f;

	RsGlobal.ps = &PsGlobal;
	
	PsGlobal.fullScreen = FALSE;
	PsGlobal.cursorIsInWindow = FALSE;
	WindowFocused = TRUE;
	WindowIconified = FALSE;
	
	PsGlobal.joy1id	= -1;
	PsGlobal.joy2id	= -1;

	CFileMgr::Initialise();
	
#ifdef PS2_MENU
	CPad::Initialise();
	CPad::GetPad(0)->Mode = 0;

	CGame::frenchGame = false;
	CGame::germanGame = false;
	CGame::nastyGame = true;
	CMenuManager::m_PrefsAllowNastyGame = true;

#ifndef _WIN32
	// Mandatory for Linux(Unix? Posix?) to set lang. to environment lang.
	setlocale(LC_ALL, "");	

	char *systemLang, *keyboardLang;

	systemLang = setlocale (LC_ALL, NULL);
	keyboardLang = setlocale (LC_CTYPE, NULL);
	
	short lang;
	lang = !strncmp(systemLang, "fr_",3) ? LANG_FRENCH :
					!strncmp(systemLang, "de_",3) ? LANG_GERMAN :
					!strncmp(systemLang, "en_",3) ? LANG_ENGLISH :
					!strncmp(systemLang, "it_",3) ? LANG_ITALIAN :
					!strncmp(systemLang, "es_",3) ? LANG_SPANISH :
					LANG_OTHER;
#else
	WORD lang	= PRIMARYLANGID(GetSystemDefaultLCID());
#endif

	if ( lang  == LANG_ITALIAN )
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_ITALIAN;
	else if ( lang  == LANG_SPANISH )
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_SPANISH;
	else if ( lang  == LANG_GERMAN )
	{
		CGame::germanGame = true;
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_GERMAN;
	}
	else if ( lang  == LANG_FRENCH )
	{
		CGame::frenchGame = true;
		CGame::nastyGame = false;
		CMenuManager::m_PrefsAllowNastyGame = false;
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_FRENCH;
	}
	else
		CMenuManager::m_PrefsLanguage = CMenuManager::LANGUAGE_AMERICAN;

	FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();

	TheMemoryCard.Init();
#else
	C_PcSave::SetSaveDirectory(_psGetUserFilesFolder());
	
	InitialiseLanguage();

#endif

	_psInitializeVibration();
	
	gGameState = GS_START_UP;
	TRACE("gGameState = GS_START_UP");
#ifdef _WIN32
	OSVERSIONINFO verInfo;
	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	GetVersionEx(&verInfo);
	
	_dwOperatingSystemVersion = OS_WIN95;
	
	if ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		if ( verInfo.dwMajorVersion == 4 )
		{
			debug("Operating System is WinNT\n");
			_dwOperatingSystemVersion = OS_WINNT;
		}
		else if ( verInfo.dwMajorVersion == 5 )
		{
			debug("Operating System is Win2000\n");
			_dwOperatingSystemVersion = OS_WIN2000;
		}
		else if ( verInfo.dwMajorVersion > 5 )
		{
			debug("Operating System is WinXP or greater\n");
			_dwOperatingSystemVersion = OS_WINXP;
		}
	}
	else if ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
		if ( verInfo.dwMajorVersion > 4 || verInfo.dwMajorVersion == 4 && verInfo.dwMinorVersion != 0 )
		{
			debug("Operating System is Win98\n");
			_dwOperatingSystemVersion = OS_WIN98;
		}
		else
		{
			debug("Operating System is Win95\n");
			_dwOperatingSystemVersion = OS_WIN95;
		}
	}
#else
	_dwOperatingSystemVersion = OS_WINXP; // To fool other classes
#endif

	
#ifndef PS2_MENU
	FrontEndMenuManager.LoadSettings();
#endif


#ifdef _WIN32
	MEMORYSTATUS memstats;
	GlobalMemoryStatus(&memstats);

	_dwMemAvailPhys = memstats.dwAvailPhys;

	debug("Physical memory size %u\n", memstats.dwTotalPhys);
	debug("Available physical memory %u\n", memstats.dwAvailPhys);
#elif defined (__APPLE__)
	uint64_t size = 0;
	uint64_t page_size = 0;
	size_t uint64_len = sizeof(uint64_t);
	size_t ull_len = sizeof(unsigned long long);
	sysctl((int[]){CTL_HW, HW_PAGESIZE}, 2, &page_size, &ull_len, NULL, 0);
	sysctl((int[]){CTL_HW, HW_MEMSIZE}, 2, &size, &uint64_len, NULL, 0);
	vm_statistics_data_t vm_stat;
	mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
	host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stat, &count);
	_dwMemAvailPhys = (uint64_t)(vm_stat.free_count * page_size);
	debug("Physical memory size %llu\n", _dwMemAvailPhys);
	debug("Available physical memory %llu\n", size);
#elif defined (__SWITCH__)
	svcGetInfo(&_dwMemAvailPhys, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
	debug("Physical memory size %llu\n", _dwMemAvailPhys);
#else
#ifndef __APPLE__
 	struct sysinfo systemInfo;
	sysinfo(&systemInfo);
	_dwMemAvailPhys = systemInfo.freeram;
	debug("Physical memory size %u\n", systemInfo.totalram);
	debug("Available physical memory %u\n", systemInfo.freeram);
#else
	uint64_t size = 0;
	uint64_t page_size = 0;
	size_t uint64_len = sizeof(uint64_t);
	size_t ull_len = sizeof(unsigned long long);
	sysctl((int[]){CTL_HW, HW_PAGESIZE}, 2, &page_size, &ull_len, NULL, 0);
	sysctl((int[]){CTL_HW, HW_MEMSIZE}, 2, &size, &uint64_len, NULL, 0);
	vm_statistics_data_t vm_stat;
	mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
	host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stat, &count);
	_dwMemAvailPhys = (uint64_t)(vm_stat.free_count * page_size);
	debug("Physical memory size %llu\n", _dwMemAvailPhys);
	debug("Available physical memory %llu\n", size);
#endif
	_dwOperatingSystemVersion = OS_WINXP; // To fool other classes
#endif
  
  TheText.Unload();

	return TRUE;
}


/*
 *****************************************************************************
 */
void
psTerminate(void)
{
	return;
}

/*
 *****************************************************************************
 */
static RwChar **_VMList;

RwInt32 _psGetNumVideModes()
{
	return RwEngineGetNumVideoModes();
}

/*
 *****************************************************************************
 */
RwBool _psFreeVideoModeList()
{
	RwInt32 numModes;
	RwInt32 i;
	
	numModes = _psGetNumVideModes();
	
	if ( _VMList == nil )
		return TRUE;
	
	for ( i = 0; i < numModes; i++ )
	{
		RwFree(_VMList[i]);
	}
	
	RwFree(_VMList);
	
	_VMList = nil;
	
	return TRUE;
}
							
/*
 *****************************************************************************
 */							
RwChar **_psGetVideoModeList()
{
	RwInt32 numModes;
	RwInt32 i;
	
	if ( _VMList != nil )
	{
		return _VMList;
	}
	
	numModes = RwEngineGetNumVideoModes();
	
	_VMList = (RwChar **)RwCalloc(numModes, sizeof(RwChar*));
	
	for ( i = 0; i < numModes; i++	)
	{
		RwVideoMode			vm;
		
		RwEngineGetVideoModeInfo(&vm, i);
		
		if ( vm.flags & rwVIDEOMODEEXCLUSIVE )
		{
			_VMList[i] = (RwChar*)RwCalloc(100, sizeof(RwChar));
			rwsprintf(_VMList[i],"%d X %d X %d", vm.width, vm.height, vm.depth);
		}
		else
			_VMList[i] = nil;
	}
	
	return _VMList;
}

/*
 *****************************************************************************
 */
void _psSelectScreenVM(RwInt32 videoMode)
{
	RwTexDictionarySetCurrent( nil );
	
	FrontEndMenuManager.UnloadTextures();
	
	if (!_psSetVideoMode(RwEngineGetCurrentSubSystem(), videoMode))
	{
		RsGlobal.quit = TRUE;

		printf("ERROR: Failed to select new screen resolution\n");
	}
	else
		FrontEndMenuManager.LoadAllTextures();
}

/*
 *****************************************************************************
 */

RwBool IsForegroundApp()
{
	return !!ForegroundApp;
}
/*
UINT GetBestRefreshRate(UINT width, UINT height, UINT depth)
{
	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
	
	ASSERT(d3d != nil);
	
	UINT refreshRate = INT_MAX;
	D3DFORMAT format;

	if ( depth == 32 )
		format = D3DFMT_X8R8G8B8;
	else if ( depth == 24 )
		format = D3DFMT_R8G8B8;
	else
		format = D3DFMT_R5G6B5;
	
	UINT modeCount = d3d->GetAdapterModeCount(GcurSel);
	
	for ( UINT i = 0; i < modeCount; i++ )
	{
		D3DDISPLAYMODE mode;
		
		d3d->EnumAdapterModes(GcurSel, i, &mode);
		
		if ( mode.Width == width && mode.Height == height && mode.Format == format )
		{
			if ( mode.RefreshRate == 0 )
				return 0;

			if ( mode.RefreshRate < refreshRate && mode.RefreshRate >= 60 )
				refreshRate = mode.RefreshRate;
		}
	}
	
#ifdef FIX_BUGS
	d3d->Release();
#endif
	
	if ( refreshRate == -1 )
		return -1;

	return refreshRate;
}
*/
/*
 *****************************************************************************
 */
RwBool
psSelectDevice()
{
	RwVideoMode			vm;
	RwInt32				subSysNum;
	RwInt32				AutoRenderer = 0;
	

	RwBool modeFound = FALSE;
	
	if ( !useDefault )
	{
		GnumSubSystems = RwEngineGetNumSubSystems();
		if ( !GnumSubSystems )
		{
			 return FALSE;
		}
		
		/* Just to be sure ... */
		GnumSubSystems = (GnumSubSystems > MAX_SUBSYSTEMS) ? MAX_SUBSYSTEMS : GnumSubSystems;
		
		/* Get the names of all the sub systems */
		for (subSysNum = 0; subSysNum < GnumSubSystems; subSysNum++)
		{
			RwEngineGetSubSystemInfo(&GsubSysInfo[subSysNum], subSysNum);
		}
		
		/* Get the default selection */
		GcurSel = RwEngineGetCurrentSubSystem();
#ifdef IMPROVED_VIDEOMODE
		if(FrontEndMenuManager.m_nPrefsSubsystem < GnumSubSystems)
			GcurSel = FrontEndMenuManager.m_nPrefsSubsystem;
#endif
	}
	
	/* Set the driver to use the correct sub system */
	if (!RwEngineSetSubSystem(GcurSel))
	{
		return FALSE;
	}

#ifdef IMPROVED_VIDEOMODE
	FrontEndMenuManager.m_nPrefsSubsystem = GcurSel;
#endif

#ifndef IMPROVED_VIDEOMODE
	if ( !useDefault )
	{
		if ( _psGetVideoModeList()[FrontEndMenuManager.m_nDisplayVideoMode] && FrontEndMenuManager.m_nDisplayVideoMode )
		{
			FrontEndMenuManager.m_nPrefsVideoMode = FrontEndMenuManager.m_nDisplayVideoMode;
			GcurSelVM = FrontEndMenuManager.m_nDisplayVideoMode;
		}
		else
		{
#ifdef DEFAULT_NATIVE_RESOLUTION
			// get the native video mode
			HDC hDevice = GetDC(NULL);
			int w = GetDeviceCaps(hDevice, HORZRES);
			int h = GetDeviceCaps(hDevice, VERTRES);
			int d = GetDeviceCaps(hDevice, BITSPIXEL);
#else
			const int w = 640;
			const int h = 480;
			const int d = 16;
#endif
			while ( !modeFound && GcurSelVM < RwEngineGetNumVideoModes() )
			{
				RwEngineGetVideoModeInfo(&vm, GcurSelVM);
				if ( defaultFullscreenRes	&& vm.width	 != w 
											|| vm.height != h
											|| vm.depth	 != d
											|| !(vm.flags & rwVIDEOMODEEXCLUSIVE) )
					++GcurSelVM;
				else
					modeFound = TRUE;
			}
			
			if ( !modeFound )
			{
#ifdef DEFAULT_NATIVE_RESOLUTION
				GcurSelVM = 1;
#else
				printf("WARNING: Cannot find 640x480 video mode, selecting device cancelled\n");
				return FALSE;
#endif
			}
		}
	}
#else
	if ( !useDefault )
	{
		if(FrontEndMenuManager.m_nPrefsWidth == 0 ||
		   FrontEndMenuManager.m_nPrefsHeight == 0 ||
		   FrontEndMenuManager.m_nPrefsDepth == 0){
			// Defaults if nothing specified
			const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			FrontEndMenuManager.m_nPrefsWidth = mode->width;
			FrontEndMenuManager.m_nPrefsHeight = mode->height;
			FrontEndMenuManager.m_nPrefsDepth = 32;
			FrontEndMenuManager.m_nPrefsWindowed = 0;
		}

		// Find the videomode that best fits what we got from the settings file
		RwInt32 bestFsMode = -1;
		RwInt32 bestWidth = -1;
		RwInt32 bestHeight = -1;
		RwInt32 bestDepth = -1;
		for(GcurSelVM = 0; GcurSelVM < RwEngineGetNumVideoModes(); GcurSelVM++){
			RwEngineGetVideoModeInfo(&vm, GcurSelVM);

			if (!(vm.flags & rwVIDEOMODEEXCLUSIVE)){
				bestWndMode = GcurSelVM;
			} else {
				// try the largest one that isn't larger than what we wanted
				if(vm.width >= bestWidth && vm.width <= FrontEndMenuManager.m_nPrefsWidth &&
				   vm.height >= bestHeight && vm.height <= FrontEndMenuManager.m_nPrefsHeight &&
				   vm.depth >= bestDepth && vm.depth <= FrontEndMenuManager.m_nPrefsDepth){
					bestWidth = vm.width;
					bestHeight = vm.height;
					bestDepth = vm.depth;
					bestFsMode = GcurSelVM;
				}
			}
		}

		if(bestFsMode < 0){
			printf("WARNING: Cannot find desired video mode, selecting device cancelled\n");
			return FALSE;
		}
		GcurSelVM = bestFsMode;

		FrontEndMenuManager.m_nDisplayVideoMode = GcurSelVM;
		FrontEndMenuManager.m_nPrefsVideoMode = FrontEndMenuManager.m_nDisplayVideoMode;

		FrontEndMenuManager.m_nSelectedScreenMode = FrontEndMenuManager.m_nPrefsWindowed;
	}
#endif

	RwEngineGetVideoModeInfo(&vm, GcurSelVM);

#ifdef IMPROVED_VIDEOMODE
	if (FrontEndMenuManager.m_nPrefsWindowed)
		GcurSelVM = bestWndMode;

	// Now GcurSelVM is 0 but vm has sizes(and fullscreen flag) of the video mode we want, that's why we changed the rwVIDEOMODEEXCLUSIVE conditions below
	FrontEndMenuManager.m_nPrefsWidth = vm.width;
	FrontEndMenuManager.m_nPrefsHeight = vm.height;
	FrontEndMenuManager.m_nPrefsDepth = vm.depth;
#endif

#ifndef PS2_MENU
	FrontEndMenuManager.m_nCurrOption = 0;
#endif
	
	/* Set up the video mode and set the apps window
	* dimensions to match */
	if (!RwEngineSetVideoMode(GcurSelVM))
	{
		return FALSE;
	}
	/*
	TODO
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
	{
		debug("%dx%dx%d", vm.width, vm.height, vm.depth);
		
		UINT refresh = GetBestRefreshRate(vm.width, vm.height, vm.depth);
		
		if ( refresh != (UINT)-1 )
		{
			debug("refresh %d", refresh);
			RwD3D8EngineSetRefreshRate((RwUInt32)refresh);
		}
	}
	*/
#ifndef IMPROVED_VIDEOMODE
	if (vm.flags & rwVIDEOMODEEXCLUSIVE)
	{
		RsGlobal.maximumWidth = vm.width;
		RsGlobal.maximumHeight = vm.height;
		RsGlobal.width = vm.width;
		RsGlobal.height = vm.height;
		
		PSGLOBAL(fullScreen) = TRUE;
	}
#else
		RsGlobal.maximumWidth = FrontEndMenuManager.m_nPrefsWidth;
		RsGlobal.maximumHeight = FrontEndMenuManager.m_nPrefsHeight;
		RsGlobal.width = FrontEndMenuManager.m_nPrefsWidth;
		RsGlobal.height = FrontEndMenuManager.m_nPrefsHeight;
		
		PSGLOBAL(fullScreen) = !FrontEndMenuManager.m_nPrefsWindowed;
#endif

#ifdef MULTISAMPLING
	RwD3D8EngineSetMultiSamplingLevels(1 << FrontEndMenuManager.m_nPrefsMSAALevel);
#endif
	return TRUE;
}

#ifndef GET_KEYBOARD_INPUT_FROM_X11
void keypressCB(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif
void resizeCB(GLFWwindow* window, int width, int height);
void scrollCB(GLFWwindow* window, double xoffset, double yoffset);
void cursorCB(GLFWwindow* window, double xpos, double ypos);
void cursorEnterCB(GLFWwindow* window, int entered);
void windowFocusCB(GLFWwindow* window, int focused);
void windowIconifyCB(GLFWwindow* window, int iconified);
void joysChangeCB(int jid, int event);

bool IsThisJoystickBlacklisted(int i)
{
#ifndef DETECT_JOYSTICK_MENU
	return false;
#else
	if (glfwJoystickIsGamepad(i))
		return false;

	const char* joyname = glfwGetJoystickName(i);

	if (gSelectedJoystickName[0] != '\0' &&
		strncmp(joyname, gSelectedJoystickName, strlen(gSelectedJoystickName)) == 0)
		return false;

	return true;
#endif
}

void _InputInitialiseJoys()
{
	PSGLOBAL(joy1id) = -1;
	PSGLOBAL(joy2id) = -1;

	// Load our gamepad mappings.
#define SDL_GAMEPAD_DB_PATH "gamecontrollerdb.txt"
	FILE *f = fopen(SDL_GAMEPAD_DB_PATH, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *db = (char*)malloc(fsize + 1);
		if (fread(db, 1, fsize, f) == fsize) {
			db[fsize] = '\0';

			if (glfwUpdateGamepadMappings(db) == GLFW_FALSE)
				Error("glfwUpdateGamepadMappings didn't succeed, check " SDL_GAMEPAD_DB_PATH ".\n");
		} else
			Error("fread on " SDL_GAMEPAD_DB_PATH " wasn't successful.\n");

		free(db);
		fclose(f);
	} else
		printf("You don't seem to have copied " SDL_GAMEPAD_DB_PATH " file from reVC/gamefiles to GTA: Vice City directory. Some gamepads may not be recognized.\n");

#undef SDL_GAMEPAD_DB_PATH

	// But always overwrite it with the one in SDL_GAMECONTROLLERCONFIG.
	char const* EnvControlConfig = getenv("SDL_GAMECONTROLLERCONFIG");
	if (EnvControlConfig != nil) {
		glfwUpdateGamepadMappings(EnvControlConfig);
	}

	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
		if (glfwJoystickPresent(i) && !IsThisJoystickBlacklisted(i)) {
			if (PSGLOBAL(joy1id) == -1)
				PSGLOBAL(joy1id) = i;
			else if (PSGLOBAL(joy2id) == -1)
				PSGLOBAL(joy2id) = i;
			else
				break;
		}
	}

	if (PSGLOBAL(joy1id) != -1) {
		int count;
		glfwGetJoystickButtons(PSGLOBAL(joy1id), &count);
#ifdef DETECT_JOYSTICK_MENU
		strcpy(gSelectedJoystickName, glfwGetJoystickName(PSGLOBAL(joy1id)));
#endif
		ControlsManager.InitDefaultControlConfigJoyPad(count);
	}
}

int lastCursorMode = GLFW_CURSOR_HIDDEN;
long _InputInitialiseMouse(bool exclusive)
{
	// Disabled = keep cursor centered and hide
	lastCursorMode = exclusive ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN;
	glfwSetInputMode(PSGLOBAL(window), GLFW_CURSOR, lastCursorMode);
	return 0;
}

void _InputShutdownMouse()
{
	// Not needed
}

// Not "needs exclusive" on GLFW, but more like "needs to change mode"
bool _InputMouseNeedsExclusive()
{
	// That was the cause of infamous mouse bug on Win.
	
	RwVideoMode vm;
	RwEngineGetVideoModeInfo(&vm, GcurSelVM);

	// If windowed, free the cursor on menu(where this func. is called and DISABLED-HIDDEN transition is done accordingly)
	// If it's fullscreen, be sure that it didn't stuck on HIDDEN.
	return !(vm.flags & rwVIDEOMODEEXCLUSIVE) || lastCursorMode == GLFW_CURSOR_HIDDEN;
}

void psPostRWinit(void)
{
	RwVideoMode vm;
	RwEngineGetVideoModeInfo(&vm, GcurSelVM);

	glfwSetFramebufferSizeCallback(PSGLOBAL(window), resizeCB);
#ifndef IGNORE_MOUSE_KEYBOARD
#ifndef GET_KEYBOARD_INPUT_FROM_X11
	glfwSetKeyCallback(PSGLOBAL(window), keypressCB);
#endif
	glfwSetScrollCallback(PSGLOBAL(window), scrollCB);
	glfwSetCursorPosCallback(PSGLOBAL(window), cursorCB);
	glfwSetCursorEnterCallback(PSGLOBAL(window), cursorEnterCB);
#endif
	glfwSetWindowIconifyCallback(PSGLOBAL(window), windowIconifyCB);
	glfwSetWindowFocusCallback(PSGLOBAL(window), windowFocusCB);
	glfwSetJoystickCallback(joysChangeCB);

	_InputInitialiseJoys();
	_InputInitialiseMouse(false);

	if(!(vm.flags & rwVIDEOMODEEXCLUSIVE))
		glfwSetWindowSize(PSGLOBAL(window), RsGlobal.maximumWidth, RsGlobal.maximumHeight);

	// Make sure all keys are released
	CPad::GetPad(0)->Clear(true);
	CPad::GetPad(1)->Clear(true);
}

/*
 *****************************************************************************
 */
RwBool _psSetVideoMode(RwInt32 subSystem, RwInt32 videoMode)
{
	RwInitialised = FALSE;
	
	RsEventHandler(rsRWTERMINATE, nil);
	
	GcurSel = subSystem;
	GcurSelVM = videoMode;
	
	useDefault = TRUE;
	
	if ( RsEventHandler(rsRWINITIALIZE, &openParams) == rsEVENTERROR )
		return FALSE;

	RwInitialised = TRUE;
	useDefault = FALSE;
	
	RwRect r;
	
	r.x = 0;
	r.y = 0;
	r.w = RsGlobal.maximumWidth;
	r.h = RsGlobal.maximumHeight;

	RsEventHandler(rsCAMERASIZE, &r);
	
	psPostRWinit();
	
	return TRUE;
}
 
 
/*
 *****************************************************************************
 */
static RwChar **
CommandLineToArgv(RwChar *cmdLine, RwInt32 *argCount)
{
	RwInt32 numArgs = 0;
	RwBool inArg, inString;
	RwInt32 i, len;
	RwChar *res, *str, **aptr;

	len = strlen(cmdLine);

	/* 
	 * Count the number of arguments...
	 */
	inString = FALSE;
	inArg = FALSE;

	for(i=0; i<=len; i++)
	{
		if( cmdLine[i] == '"' )
		{
			inString = !inString;
		}

		if( (cmdLine[i] <= ' ' && !inString) || i == len )
		{
			if( inArg ) 
			{
				inArg = FALSE;
				
				numArgs++;
			}
		} 
		else if( !inArg )
		{
			inArg = TRUE;
		}
	}

	/* 
	 * Allocate memory for result...
	 */
	res = (RwChar *)malloc(sizeof(RwChar *) * numArgs + len + 1);
	str = res + sizeof(RwChar *) * numArgs;
	aptr = (RwChar **)res;

	strcpy(str, cmdLine);

	/*
	 * Walk through cmdLine again this time setting pointer to each arg...
	 */
	inArg = FALSE;
	inString = FALSE;

	for(i=0; i<=len; i++)
	{
		if( cmdLine[i] == '"' )
		{
			inString = !inString;
		}

		if( (cmdLine[i] <= ' ' && !inString) || i == len )
		{
			if( inArg ) 
			{
				if( str[i-1] == '"' )
				{
					str[i-1] = '\0';
				}
				else
				{
					str[i] = '\0';
				}
				
				inArg = FALSE;
			}
		} 
		else if( !inArg && cmdLine[i] != '"' )
		{
			inArg = TRUE; 
			
			*aptr++ = &str[i];
		}
	}

	*argCount = numArgs;

	return (RwChar **)res;
}

/*
 *****************************************************************************
 */
void InitialiseLanguage()
{
#ifndef _WIN32
	// Mandatory for Linux(Unix? Posix?) to set lang. to environment lang.
	setlocale(LC_ALL, "");	

	char *systemLang, *keyboardLang;

	systemLang = setlocale (LC_ALL, NULL);
	keyboardLang = setlocale (LC_CTYPE, NULL);
	
	short primUserLCID, primSystemLCID;
	primUserLCID = primSystemLCID = !strncmp(systemLang, "fr_",3) ? LANG_FRENCH :
					!strncmp(systemLang, "de_",3) ? LANG_GERMAN :
					!strncmp(systemLang, "en_",3) ? LANG_ENGLISH :
					!strncmp(systemLang, "it_",3) ? LANG_ITALIAN :
					!strncmp(systemLang, "es_",3) ? LANG_SPANISH :
					LANG_OTHER;

	short primLayout = !strncmp(keyboardLang, "fr_",3) ? LANG_FRENCH : (!strncmp(keyboardLang, "de_",3) ? LANG_GERMAN : LANG_ENGLISH);

	short subUserLCID, subSystemLCID;
	subUserLCID = subSystemLCID = !strncmp(systemLang, "en_AU",5) ? SUBLANG_ENGLISH_AUS : SUBLANG_OTHER;
	short subLayout = !strncmp(keyboardLang, "en_AU",5) ? SUBLANG_ENGLISH_AUS : SUBLANG_OTHER;

#else
	WORD primUserLCID	= PRIMARYLANGID(GetSystemDefaultLCID());
	WORD primSystemLCID = PRIMARYLANGID(GetUserDefaultLCID());
	WORD primLayout		= PRIMARYLANGID((DWORD)GetKeyboardLayout(0));
	
	WORD subUserLCID	= SUBLANGID(GetSystemDefaultLCID());
	WORD subSystemLCID	= SUBLANGID(GetUserDefaultLCID());
	WORD subLayout		= SUBLANGID((DWORD)GetKeyboardLayout(0));
#endif
	if (   primUserLCID	  == LANG_GERMAN
		|| primSystemLCID == LANG_GERMAN
		|| primLayout	  == LANG_GERMAN )
	{
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		CGame::germanGame = true;
	}
	
	if (   primUserLCID	  == LANG_FRENCH
		|| primSystemLCID == LANG_FRENCH
		|| primLayout	  == LANG_FRENCH )
	{
		CGame::nastyGame = false;
		FrontEndMenuManager.m_PrefsAllowNastyGame = false;
		CGame::frenchGame = true;
	}
	
	if (   subUserLCID	 == SUBLANG_ENGLISH_AUS
		|| subSystemLCID == SUBLANG_ENGLISH_AUS
		|| subLayout	 == SUBLANG_ENGLISH_AUS )
		CGame::noProstitutes = true;

#ifdef NASTY_GAME
	CGame::nastyGame = true;
	FrontEndMenuManager.m_PrefsAllowNastyGame = true;
	CGame::noProstitutes = false;
#endif
	
	int32 lang;
	
	switch ( primSystemLCID )
	{
		case LANG_GERMAN:
		{
			lang = LANG_GERMAN;
			break;
		}
		case LANG_FRENCH:
		{
			lang = LANG_FRENCH;
			break;
		}
		case LANG_SPANISH:
		{
			lang = LANG_SPANISH;
			break;
		}
		case LANG_ITALIAN:
		{
			lang = LANG_ITALIAN;
			break;
		}
		default:
		{
			lang = ( subSystemLCID == SUBLANG_ENGLISH_AUS ) ? -99 : LANG_ENGLISH;
			break;
		}
	}
	
	FrontEndMenuManager.OS_Language = primUserLCID;

	switch ( lang )
	{
		case LANG_GERMAN:
		{
			FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_GERMAN;
			break;
		}
		case LANG_SPANISH:
		{
			FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_SPANISH;
			break;
		}
		case LANG_FRENCH:
		{
			FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_FRENCH;
			break;
		}
		case LANG_ITALIAN:
		{
			FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_ITALIAN;
			break;
		}
		default:
		{
			FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_AMERICAN;
			break;
		}
	}

#ifndef _WIN32
	// TODO this is needed for strcasecmp to work correctly across all languages, but can these cause other problems??
	setlocale(LC_CTYPE, "C");
	setlocale(LC_COLLATE, "C");
	setlocale(LC_NUMERIC, "C");
#endif

	TheText.Unload();
	TheText.Load();
}

/*
 *****************************************************************************
 */

void HandleExit()
{
#ifdef _WIN32
	MSG message;
	while ( PeekMessage(&message, nil, 0U, 0U, PM_REMOVE|PM_NOYIELD) )
	{
		if( message.message == WM_QUIT )
		{
			RsGlobal.quit = TRUE;
		}
		else
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
#else
	// We now handle terminate message always, why handle on some cases?
	return;
#endif
}

#ifndef _WIN32
void terminateHandler(int sig, siginfo_t *info, void *ucontext) {
	RsGlobal.quit = TRUE;
}

#ifdef FLUSHABLE_STREAMING
void dummyHandler(int sig){
	// Don't kill the app pls
}
#endif
#endif

void resizeCB(GLFWwindow* window, int width, int height) {
	/*
	* Handle event to ensure window contents are displayed during re-size
	* as this can be disabled by the user, then if there is not enough
	* memory things don't work.
	*/
	/* redraw window */

	if (RwInitialised && gGameState == GS_PLAYING_GAME)
	{
		RsEventHandler(rsIDLE, (void *)TRUE);
	}

	if (RwInitialised && height > 0 && width > 0) {
		RwRect r;

		// TODO fix artifacts of resizing with mouse
		RsGlobal.maximumHeight = height;
		RsGlobal.maximumWidth = width;

		r.x = 0;
		r.y = 0;
		r.w = width;
		r.h = height;

		RsEventHandler(rsCAMERASIZE, &r);
	}
//	glfwSetWindowPos(window, 0, 0);
}

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
	PSGLOBAL(mouseWheel) = yoffset;
}

bool lshiftStatus = false;
bool rshiftStatus = false;

#ifndef GET_KEYBOARD_INPUT_FROM_X11
int keymap[GLFW_KEY_LAST + 1];

static void
initkeymap(void)
{
	int i;
	for (i = 0; i < GLFW_KEY_LAST + 1; i++)
		keymap[i] = rsNULL;

	keymap[GLFW_KEY_SPACE] = ' ';
	keymap[GLFW_KEY_APOSTROPHE] = '\'';
	keymap[GLFW_KEY_COMMA] = ',';
	keymap[GLFW_KEY_MINUS] = '-';
	keymap[GLFW_KEY_PERIOD] = '.';
	keymap[GLFW_KEY_SLASH] = '/';
	keymap[GLFW_KEY_0] = '0';
	keymap[GLFW_KEY_1] = '1';
	keymap[GLFW_KEY_2] = '2';
	keymap[GLFW_KEY_3] = '3';
	keymap[GLFW_KEY_4] = '4';
	keymap[GLFW_KEY_5] = '5';
	keymap[GLFW_KEY_6] = '6';
	keymap[GLFW_KEY_7] = '7';
	keymap[GLFW_KEY_8] = '8';
	keymap[GLFW_KEY_9] = '9';
	keymap[GLFW_KEY_SEMICOLON] = ';';
	keymap[GLFW_KEY_EQUAL] = '=';
	keymap[GLFW_KEY_A] = 'A';
	keymap[GLFW_KEY_B] = 'B';
	keymap[GLFW_KEY_C] = 'C';
	keymap[GLFW_KEY_D] = 'D';
	keymap[GLFW_KEY_E] = 'E';
	keymap[GLFW_KEY_F] = 'F';
	keymap[GLFW_KEY_G] = 'G';
	keymap[GLFW_KEY_H] = 'H';
	keymap[GLFW_KEY_I] = 'I';
	keymap[GLFW_KEY_J] = 'J';
	keymap[GLFW_KEY_K] = 'K';
	keymap[GLFW_KEY_L] = 'L';
	keymap[GLFW_KEY_M] = 'M';
	keymap[GLFW_KEY_N] = 'N';
	keymap[GLFW_KEY_O] = 'O';
	keymap[GLFW_KEY_P] = 'P';
	keymap[GLFW_KEY_Q] = 'Q';
	keymap[GLFW_KEY_R] = 'R';
	keymap[GLFW_KEY_S] = 'S';
	keymap[GLFW_KEY_T] = 'T';
	keymap[GLFW_KEY_U] = 'U';
	keymap[GLFW_KEY_V] = 'V';
	keymap[GLFW_KEY_W] = 'W';
	keymap[GLFW_KEY_X] = 'X';
	keymap[GLFW_KEY_Y] = 'Y';
	keymap[GLFW_KEY_Z] = 'Z';
	keymap[GLFW_KEY_LEFT_BRACKET] = '[';
	keymap[GLFW_KEY_BACKSLASH] = '\\';
	keymap[GLFW_KEY_RIGHT_BRACKET] = ']';
	keymap[GLFW_KEY_GRAVE_ACCENT] = '`';
	keymap[GLFW_KEY_ESCAPE] = rsESC;
	keymap[GLFW_KEY_ENTER] = rsENTER;
	keymap[GLFW_KEY_TAB] = rsTAB;
	keymap[GLFW_KEY_BACKSPACE] = rsBACKSP;
	keymap[GLFW_KEY_INSERT] = rsINS;
	keymap[GLFW_KEY_DELETE] = rsDEL;
	keymap[GLFW_KEY_RIGHT] = rsRIGHT;
	keymap[GLFW_KEY_LEFT] = rsLEFT;
	keymap[GLFW_KEY_DOWN] = rsDOWN;
	keymap[GLFW_KEY_UP] = rsUP;
	keymap[GLFW_KEY_PAGE_UP] = rsPGUP;
	keymap[GLFW_KEY_PAGE_DOWN] = rsPGDN;
	keymap[GLFW_KEY_HOME] = rsHOME;
	keymap[GLFW_KEY_END] = rsEND;
	keymap[GLFW_KEY_CAPS_LOCK] = rsCAPSLK;
	keymap[GLFW_KEY_SCROLL_LOCK] = rsSCROLL;
	keymap[GLFW_KEY_NUM_LOCK] = rsNUMLOCK;
	keymap[GLFW_KEY_PRINT_SCREEN] = rsNULL;
	keymap[GLFW_KEY_PAUSE] = rsPAUSE;

	keymap[GLFW_KEY_F1] = rsF1;
	keymap[GLFW_KEY_F2] = rsF2;
	keymap[GLFW_KEY_F3] = rsF3;
	keymap[GLFW_KEY_F4] = rsF4;
	keymap[GLFW_KEY_F5] = rsF5;
	keymap[GLFW_KEY_F6] = rsF6;
	keymap[GLFW_KEY_F7] = rsF7;
	keymap[GLFW_KEY_F8] = rsF8;
	keymap[GLFW_KEY_F9] = rsF9;
	keymap[GLFW_KEY_F10] = rsF10;
	keymap[GLFW_KEY_F11] = rsF11;
	keymap[GLFW_KEY_F12] = rsF12;
	keymap[GLFW_KEY_F13] = rsNULL;
	keymap[GLFW_KEY_F14] = rsNULL;
	keymap[GLFW_KEY_F15] = rsNULL;
	keymap[GLFW_KEY_F16] = rsNULL;
	keymap[GLFW_KEY_F17] = rsNULL;
	keymap[GLFW_KEY_F18] = rsNULL;
	keymap[GLFW_KEY_F19] = rsNULL;
	keymap[GLFW_KEY_F20] = rsNULL;
	keymap[GLFW_KEY_F21] = rsNULL;
	keymap[GLFW_KEY_F22] = rsNULL;
	keymap[GLFW_KEY_F23] = rsNULL;
	keymap[GLFW_KEY_F24] = rsNULL;
	keymap[GLFW_KEY_F25] = rsNULL;
	keymap[GLFW_KEY_KP_0] = rsPADINS;
	keymap[GLFW_KEY_KP_1] = rsPADEND;
	keymap[GLFW_KEY_KP_2] = rsPADDOWN;
	keymap[GLFW_KEY_KP_3] = rsPADPGDN;
	keymap[GLFW_KEY_KP_4] = rsPADLEFT;
	keymap[GLFW_KEY_KP_5] = rsPAD5;
	keymap[GLFW_KEY_KP_6] = rsPADRIGHT;
	keymap[GLFW_KEY_KP_7] = rsPADHOME;
	keymap[GLFW_KEY_KP_8] = rsPADUP;
	keymap[GLFW_KEY_KP_9] = rsPADPGUP;
	keymap[GLFW_KEY_KP_DECIMAL] = rsPADDEL;
	keymap[GLFW_KEY_KP_DIVIDE] = rsDIVIDE;
	keymap[GLFW_KEY_KP_MULTIPLY] = rsTIMES;
	keymap[GLFW_KEY_KP_SUBTRACT] = rsMINUS;
	keymap[GLFW_KEY_KP_ADD] = rsPLUS;
	keymap[GLFW_KEY_KP_ENTER] = rsPADENTER;
	keymap[GLFW_KEY_KP_EQUAL] = rsNULL;
	keymap[GLFW_KEY_LEFT_SHIFT] = rsLSHIFT;
	keymap[GLFW_KEY_LEFT_CONTROL] = rsLCTRL;
	keymap[GLFW_KEY_LEFT_ALT] = rsLALT;
	keymap[GLFW_KEY_LEFT_SUPER] = rsLWIN;
	keymap[GLFW_KEY_RIGHT_SHIFT] = rsRSHIFT;
	keymap[GLFW_KEY_RIGHT_CONTROL] = rsRCTRL;
	keymap[GLFW_KEY_RIGHT_ALT] = rsRALT;
	keymap[GLFW_KEY_RIGHT_SUPER] = rsRWIN;
	keymap[GLFW_KEY_MENU] = rsNULL;
}

void
keypressCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key <= GLFW_KEY_LAST && action != GLFW_REPEAT) {
		RsKeyCodes ks = (RsKeyCodes)keymap[key];

		if (key == GLFW_KEY_LEFT_SHIFT)
			lshiftStatus = action != GLFW_RELEASE;

		if (key == GLFW_KEY_RIGHT_SHIFT)
			rshiftStatus = action != GLFW_RELEASE;

		if (action == GLFW_RELEASE) RsKeyboardEventHandler(rsKEYUP, &ks);
		else if (action == GLFW_PRESS) RsKeyboardEventHandler(rsKEYDOWN, &ks);
	}
}

#else

uint32 keymap[512]; // 256 ascii + 256 KeySyms between 0xff00 - 0xffff
bool keyStates[512];
uint32 keyCodeToKeymapIndex[256]; // cache for physical keys

#define KEY_MAP_OFFSET (0xff00 - 256)
static void
initkeymap(void)
{
	Display *display = glfwGetX11Display();
	int i;

	for (i = 0; i < ARRAY_SIZE(keymap); i++)
		keymap[i] = rsNULL;

	// You can add new ASCII mappings to here freely (but beware that if right hand side of assignment isn't supported on CFont, it'll be blank/won't work on binding screen)
	// Right hand side of assigments should always be uppercase counterpart of character
	keymap[XK_space] = ' ';
	keymap[XK_apostrophe] = '\'';
	keymap[XK_ampersand] = '&';
	keymap[XK_percent] = '%';
	keymap[XK_dollar] = '$';
	keymap[XK_comma] = ',';
	keymap[XK_minus] = '-';
	keymap[XK_period] = '.';
	keymap[XK_slash] = '/';
	keymap[XK_question] = '?';
	keymap[XK_exclam] = '!';
	keymap[XK_quotedbl] = '"';
	keymap[XK_colon] = ':';
	keymap[XK_semicolon] = ';';
	keymap[XK_equal] = '=';
	keymap[XK_bracketleft] = '[';
	keymap[XK_backslash] = '\\';
	keymap[XK_bracketright] = ']';
	keymap[XK_grave] = '`';
	keymap[XK_0] = '0';
	keymap[XK_1] = '1';
	keymap[XK_2] = '2';
	keymap[XK_3] = '3';
	keymap[XK_4] = '4';
	keymap[XK_5] = '5';
	keymap[XK_6] = '6';
	keymap[XK_7] = '7';
	keymap[XK_8] = '8';
	keymap[XK_9] = '9';
	keymap[XK_a] = 'A';
	keymap[XK_b] = 'B';
	keymap[XK_c] = 'C';
	keymap[XK_d] = 'D';
	keymap[XK_e] = 'E';
	keymap[XK_f] = 'F';
	keymap[XK_g] = 'G';
	keymap[XK_h] = 'H';
	keymap[XK_i] = 'I';
	keymap[XK_I] = 'I'; // Turkish I problem
	keymap[XK_j] = 'J';
	keymap[XK_k] = 'K';
	keymap[XK_l] = 'L';
	keymap[XK_m] = 'M';
	keymap[XK_n] = 'N';
	keymap[XK_o] = 'O';
	keymap[XK_p] = 'P';
	keymap[XK_q] = 'Q';
	keymap[XK_r] = 'R';
	keymap[XK_s] = 'S';
	keymap[XK_t] = 'T';
	keymap[XK_u] = 'U';
	keymap[XK_v] = 'V';
	keymap[XK_w] = 'W';
	keymap[XK_x] = 'X';
	keymap[XK_y] = 'Y';
	keymap[XK_z] = 'Z';

	// Some of regional but ASCII characters that GTA supports
	keymap[XK_agrave] = 0x00c0;
	keymap[XK_aacute] = 0x00c1;
	keymap[XK_acircumflex] = 0x00c2;
	keymap[XK_adiaeresis] = 0x00c4;

	keymap[XK_ae] = 0x00c6;

	keymap[XK_egrave] = 0x00c8;
	keymap[XK_eacute] = 0x00c9;
	keymap[XK_ecircumflex] = 0x00ca;
	keymap[XK_ediaeresis] = 0x00cb;

	keymap[XK_igrave] = 0x00cc;
	keymap[XK_iacute] = 0x00cd;
	keymap[XK_icircumflex] = 0x00ce;
	keymap[XK_idiaeresis] = 0x00cf;

	keymap[XK_ccedilla] = 0x00c7;
	keymap[XK_odiaeresis] = 0x00d6;
	keymap[XK_udiaeresis] = 0x00dc;

	// These are 0xff00 - 0xffff range of KeySym's, and subtracting KEY_MAP_OFFSET is needed
	keymap[XK_Escape - KEY_MAP_OFFSET] = rsESC;
	keymap[XK_Return - KEY_MAP_OFFSET] = rsENTER;
	keymap[XK_Tab - KEY_MAP_OFFSET] = rsTAB;
	keymap[XK_BackSpace - KEY_MAP_OFFSET] = rsBACKSP;
	keymap[XK_Insert - KEY_MAP_OFFSET] = rsINS;
	keymap[XK_Delete - KEY_MAP_OFFSET] = rsDEL;
	keymap[XK_Right - KEY_MAP_OFFSET] = rsRIGHT;
	keymap[XK_Left - KEY_MAP_OFFSET] = rsLEFT;
	keymap[XK_Down - KEY_MAP_OFFSET] = rsDOWN;
	keymap[XK_Up - KEY_MAP_OFFSET] = rsUP;
	keymap[XK_Page_Up - KEY_MAP_OFFSET] = rsPGUP;
	keymap[XK_Page_Down - KEY_MAP_OFFSET] = rsPGDN;
	keymap[XK_Home - KEY_MAP_OFFSET] = rsHOME;
	keymap[XK_End - KEY_MAP_OFFSET] = rsEND;
	keymap[XK_Caps_Lock - KEY_MAP_OFFSET] = rsCAPSLK;
	keymap[XK_Scroll_Lock - KEY_MAP_OFFSET] = rsSCROLL;
	keymap[XK_Num_Lock - KEY_MAP_OFFSET] = rsNUMLOCK;
	keymap[XK_Pause - KEY_MAP_OFFSET] = rsPAUSE;

	keymap[XK_F1 - KEY_MAP_OFFSET] = rsF1;
	keymap[XK_F2 - KEY_MAP_OFFSET] = rsF2;
	keymap[XK_F3 - KEY_MAP_OFFSET] = rsF3;
	keymap[XK_F4 - KEY_MAP_OFFSET] = rsF4;
	keymap[XK_F5 - KEY_MAP_OFFSET] = rsF5;
	keymap[XK_F6 - KEY_MAP_OFFSET] = rsF6;
	keymap[XK_F7 - KEY_MAP_OFFSET] = rsF7;
	keymap[XK_F8 - KEY_MAP_OFFSET] = rsF8;
	keymap[XK_F9 - KEY_MAP_OFFSET] = rsF9;
	keymap[XK_F10 - KEY_MAP_OFFSET] = rsF10;
	keymap[XK_F11 - KEY_MAP_OFFSET] = rsF11;
	keymap[XK_F12 - KEY_MAP_OFFSET] = rsF12;
	keymap[XK_F13 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F14 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F15 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F16 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F17 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F18 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F19 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F20 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F21 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F22 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F23 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F24 - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_F25 - KEY_MAP_OFFSET] = rsNULL;

	keymap[XK_KP_0 - KEY_MAP_OFFSET] = rsPADINS;
	keymap[XK_KP_1 - KEY_MAP_OFFSET] = rsPADEND;
	keymap[XK_KP_2 - KEY_MAP_OFFSET] = rsPADDOWN;
	keymap[XK_KP_3 - KEY_MAP_OFFSET] = rsPADPGDN;
	keymap[XK_KP_4 - KEY_MAP_OFFSET] = rsPADLEFT;
	keymap[XK_KP_5 - KEY_MAP_OFFSET] = rsPAD5;
	keymap[XK_KP_6 - KEY_MAP_OFFSET] = rsPADRIGHT;
	keymap[XK_KP_7 - KEY_MAP_OFFSET] = rsPADHOME;
	keymap[XK_KP_8 - KEY_MAP_OFFSET] = rsPADUP;
	keymap[XK_KP_9 - KEY_MAP_OFFSET] = rsPADPGUP;
	keymap[XK_KP_Insert - KEY_MAP_OFFSET] = rsPADINS;
	keymap[XK_KP_End - KEY_MAP_OFFSET] = rsPADEND;
	keymap[XK_KP_Down - KEY_MAP_OFFSET] = rsPADDOWN;
	keymap[XK_KP_Page_Down - KEY_MAP_OFFSET] = rsPADPGDN;
	keymap[XK_KP_Left - KEY_MAP_OFFSET] = rsPADLEFT;
	keymap[XK_KP_Begin - KEY_MAP_OFFSET] = rsPAD5;
	keymap[XK_KP_Right - KEY_MAP_OFFSET] = rsPADRIGHT;
	keymap[XK_KP_Home - KEY_MAP_OFFSET] = rsPADHOME;
	keymap[XK_KP_Up - KEY_MAP_OFFSET] = rsPADUP;
	keymap[XK_KP_Page_Up - KEY_MAP_OFFSET] = rsPADPGUP;

	keymap[XK_KP_Decimal - KEY_MAP_OFFSET] = rsPADDEL;
	keymap[XK_KP_Divide - KEY_MAP_OFFSET] = rsDIVIDE;
	keymap[XK_KP_Multiply - KEY_MAP_OFFSET] = rsTIMES;
	keymap[XK_KP_Subtract - KEY_MAP_OFFSET] = rsMINUS;
	keymap[XK_KP_Add - KEY_MAP_OFFSET] = rsPLUS;
	keymap[XK_KP_Enter - KEY_MAP_OFFSET] = rsPADENTER;
	keymap[XK_KP_Equal - KEY_MAP_OFFSET] = rsNULL;
	keymap[XK_Shift_L - KEY_MAP_OFFSET] = rsLSHIFT;
	keymap[XK_Control_L - KEY_MAP_OFFSET] = rsLCTRL;
	keymap[XK_Alt_L - KEY_MAP_OFFSET] = rsLALT;
	keymap[XK_Super_L - KEY_MAP_OFFSET] = rsLWIN;
	keymap[XK_Shift_R - KEY_MAP_OFFSET] = rsRSHIFT;
	keymap[XK_Control_R - KEY_MAP_OFFSET] = rsRCTRL;
	keymap[XK_Alt_R - KEY_MAP_OFFSET] = rsRALT;
	keymap[XK_Super_R - KEY_MAP_OFFSET] = rsRWIN;
	keymap[XK_Menu - KEY_MAP_OFFSET] = rsNULL;

	// Cache the key codes' key symbol equivelants, otherwise we will have to do it on each frame
	// KeyCode is always in [0,255], and represents a physical key

	int min_keycode, max_keycode, keysyms_per_keycode;
	KeySym *keymap, *origkeymap;

	char *keyboardLang = setlocale (LC_CTYPE, NULL);
	setlocale(LC_CTYPE, "");

	XDisplayKeycodes(display, &min_keycode, &max_keycode);
	origkeymap = XGetKeyboardMapping(display, min_keycode, (max_keycode - min_keycode + 1), &keysyms_per_keycode);
	keymap = origkeymap;
	for (int i = min_keycode; i <= max_keycode; i++) {
		int  j, lastKeysym;

		lastKeysym = keysyms_per_keycode - 1;
		while ((lastKeysym >= 0) && (keymap[lastKeysym] == NoSymbol))
			lastKeysym--;

		for (j = 0; j <= lastKeysym; j++) {
			KeySym ks = keymap[j];

			if (ks == NoSymbol)
				continue;

			if (ks < 256) {
				keyCodeToKeymapIndex[i] = ks;
				break;
			} else if (ks >= 0xff00 && ks < 0xffff) {
				keyCodeToKeymapIndex[i] = ks - KEY_MAP_OFFSET;
				break;
			}
		}
		keymap += keysyms_per_keycode;
	}
	XFree(origkeymap);

	setlocale(LC_CTYPE, keyboardLang);
}
#undef KEY_MAP_OFFSET

void checkKeyPresses()
{
	Display *display = glfwGetX11Display();
	char keys[32];
	XQueryKeymap(display, keys);
	for (int i = 0; i < sizeof(keys); i++) {
		for (int j = 0; j < 8; j++) {
			KeyCode keycode = 8 * i + j;
			uint32 keymapIndex = keyCodeToKeymapIndex[keycode];
			if (keymapIndex != 0) {
				int rsCode = keymap[keymapIndex];
				if (rsCode == rsNULL)
					continue;

				bool pressed = WindowFocused && !!(keys[i] & (1 << j));

				// idk why R* does that
				if (rsCode == rsLSHIFT)
					lshiftStatus = pressed;
				else if (rsCode == rsRSHIFT)
					rshiftStatus = pressed;

				if (keyStates[keymapIndex] != pressed) {
					if (pressed) {
						RsKeyboardEventHandler(rsKEYDOWN, &rsCode);
					} else {
						RsKeyboardEventHandler(rsKEYUP, &rsCode);
					}
				}

				keyStates[keymapIndex] = pressed;
			}
		}
	}

}
#endif

// R* calls that in ControllerConfig, idk why
void
_InputTranslateShiftKeyUpDown(RsKeyCodes *rs) {
	RsKeyboardEventHandler(lshiftStatus ? rsKEYDOWN : rsKEYUP, &(*rs = rsLSHIFT));
	RsKeyboardEventHandler(rshiftStatus ? rsKEYDOWN : rsKEYUP, &(*rs = rsRSHIFT));
}

// TODO this only works in frontend(and luckily only frontend use this). Fun fact: if I get pos manually in game, glfw reports that it's > 32000
void
cursorCB(GLFWwindow* window, double xpos, double ypos) {
	if (!FrontEndMenuManager.m_bMenuActive)
		return;
	
	int winw, winh;
	glfwGetWindowSize(PSGLOBAL(window), &winw, &winh);
	FrontEndMenuManager.m_nMouseTempPosX = xpos * (RsGlobal.maximumWidth / winw);
	FrontEndMenuManager.m_nMouseTempPosY = ypos * (RsGlobal.maximumHeight / winh);
}

void
cursorEnterCB(GLFWwindow* window, int entered) {
	PSGLOBAL(cursorIsInWindow) = !!entered;
}

void
windowFocusCB(GLFWwindow* window, int focused) {
	WindowFocused = !!focused;
}

void
windowIconifyCB(GLFWwindow* window, int iconified) {
	WindowIconified = !!iconified;
}

/*
 *****************************************************************************
 */
#ifdef _WIN32
int PASCAL
WinMain(HINSTANCE instance,
	HINSTANCE prevInstance	__RWUNUSED__,
	CMDSTR cmdLine,
	int cmdShow)
{

	RwInt32 argc;
	RwChar** argv;
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, nil, SPIF_SENDCHANGE);

#ifndef MASTER
	if (strstr(cmdLine, "-console"))
	{
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

#else
int
main(int argc, char *argv[])
{
#endif
	RwV2d pos;
	RwInt32 i;

#ifdef USE_CUSTOM_ALLOCATOR
	InitMemoryMgr();
#endif

#if !defined(_WIN32) && !defined(__SWITCH__)
	struct sigaction act;
	act.sa_sigaction = terminateHandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
#ifdef FLUSHABLE_STREAMING
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = dummyHandler;
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
#endif
#endif

	/* 
	 * Initialize the platform independent data.
	 * This will in turn initialize the platform specific data...
	 */
	if( RsEventHandler(rsINITIALIZE, nil) == rsEVENTERROR )
	{
		return FALSE;
	}

#ifdef _WIN32
	/*
	 * Get proper command line params, cmdLine passed to us does not
	 * work properly under all circumstances...
	 */
	cmdLine = GetCommandLine();

	/*
	 * Parse command line into standard (argv, argc) parameters...
	 */
	argv = CommandLineToArgv(cmdLine, &argc);


	/* 
	 * Parse command line parameters (except program name) one at 
	 * a time BEFORE RenderWare initialization...
	 */
#endif
	for(i=1; i<argc; i++)
	{
		RsEventHandler(rsPREINITCOMMANDLINE, argv[i]);
	}

	/*
	 * Parameters to be used in RwEngineOpen / rsRWINITIALISE event
	 */

	openParams.width = RsGlobal.maximumWidth;
	openParams.height = RsGlobal.maximumHeight;
	openParams.windowtitle = RsGlobal.appName;
	openParams.window = &PSGLOBAL(window);
	
	ControlsManager.MakeControllerActionsBlank();
	ControlsManager.InitDefaultControlConfiguration();

	/* 
	 * Initialize the 3D (RenderWare) components of the app...
	 */
	if( rsEVENTERROR == RsEventHandler(rsRWINITIALIZE, &openParams) )
	{
		RsEventHandler(rsTERMINATE, nil);

		return 0;
	}

#ifdef _WIN32
	HWND wnd = glfwGetWin32Window(PSGLOBAL(window));

	HICON icon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN_ICON));

	SendMessage(wnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
#endif

	psPostRWinit();

	ControlsManager.InitDefaultControlConfigMouse(MousePointerStateHelper.GetMouseSetUp());

//	glfwSetWindowPos(PSGLOBAL(window), 0, 0);

	/* 
	 * Parse command line parameters (except program name) one at 
	 * a time AFTER RenderWare initialization...
	 */
	for(i=1; i<argc; i++)
	{
		RsEventHandler(rsCOMMANDLINE, argv[i]);
	}

	/* 
	 * Force a camera resize event...
	 */
	{
		RwRect r;

		r.x = 0;
		r.y = 0;
		r.w = RsGlobal.maximumWidth;
		r.h = RsGlobal.maximumHeight;

		RsEventHandler(rsCAMERASIZE, &r);
	}
#ifdef _WIN32
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, FALSE, nil, SPIF_SENDCHANGE);
	

	STICKYKEYS SavedStickyKeys;
	SavedStickyKeys.cbSize = sizeof(STICKYKEYS);
	
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	
	STICKYKEYS NewStickyKeys;
	NewStickyKeys.cbSize = sizeof(STICKYKEYS);
	NewStickyKeys.dwFlags = SKF_TWOKEYSOFF;
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &NewStickyKeys, SPIF_SENDCHANGE);
#endif

	{
		CFileMgr::SetDirMyDocuments();
		
#ifdef LOAD_INI_SETTINGS
		// At this point InitDefaultControlConfigJoyPad must have set all bindings to default and ms_padButtonsInited to number of detected buttons.
		// We will load stored bindings below, but let's cache ms_padButtonsInited before LoadINIControllerSettings and LoadSettings clears it,
		// so we can add new joy bindings **on top of** stored bindings.
		int connectedPadButtons = ControlsManager.ms_padButtonsInited;
#endif

		int32 gta3set = CFileMgr::OpenFile("gta_vc.set", "r");
		
		if ( gta3set )
		{
			ControlsManager.LoadSettings(gta3set);
			CFileMgr::CloseFile(gta3set);
		}
		
		CFileMgr::SetDir("");

#ifdef LOAD_INI_SETTINGS
		LoadINIControllerSettings();
		if (connectedPadButtons != 0)
			ControlsManager.InitDefaultControlConfigJoyPad(connectedPadButtons); // add (connected-saved) amount of new button assignments on top of ours

		// these have 2 purposes: creating .ini at the start, and adding newly introduced settings to old .ini at the start
		SaveINISettings();
		SaveINIControllerSettings();
#endif
	}
	
#ifdef _WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS);
#endif

#ifdef PS2_MENU
	int32 r = TheMemoryCard.CheckCardStateAtGameStartUp(CARD_ONE);
	if (   r == CMemoryCard::ERR_DIRNOENTRY  || r == CMemoryCard::ERR_NOFORMAT
		&& r != CMemoryCard::ERR_OPENNOENTRY && r != CMemoryCard::ERR_NONE )
	{
		LoadingScreen(nil, nil, "loadsc0");
		
		TheText.Unload();
		TheText.Load();
		
		CFont::Initialise();
		
		FrontEndMenuManager.DrawMemoryCardStartUpMenus();
	}
#endif
	
	initkeymap();

	while ( TRUE )
	{
		RwInitialised = TRUE;
		
		/* 
		* Set the initial mouse position...
		*/
		pos.x = RsGlobal.maximumWidth * 0.5f;
		pos.y = RsGlobal.maximumHeight * 0.5f;

		RsMouseSetPos(&pos);
		
		/*
		* Enter the message processing loop...
		*/

#ifndef MASTER
		if (gbModelViewer) {
			// This is TheModelViewer in LCS
			LoadingScreen("Loading the ModelViewer", NULL, GetRandomSplashScreen());
			CAnimViewer::Initialise();
			CTimer::Update();
#ifndef PS2_MENU
			FrontEndMenuManager.m_bGameNotLoaded = false;
#endif
		}
#endif

#ifdef PS2_MENU
		if (TheMemoryCard.m_bWantToLoad)
			LoadSplash(GetLevelSplashScreen(CGame::currLevel));
		
		TheMemoryCard.m_bWantToLoad = false;
		
		CTimer::Update();
		
		while( !RsGlobal.quit && !(FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad) && !glfwWindowShouldClose(PSGLOBAL(window)) )
#else
		while( !RsGlobal.quit && !FrontEndMenuManager.m_bWantToRestart && !glfwWindowShouldClose(PSGLOBAL(window)))
#endif
		{
			glfwPollEvents();
#ifdef GET_KEYBOARD_INPUT_FROM_X11
			checkKeyPresses();
#endif
#ifndef MASTER
			if (gbModelViewer) {
				// This is TheModelViewerCore in LCS
				TheModelViewer();
			} else
#endif
			if ( ForegroundApp )
			{
				switch ( gGameState )
				{
					case GS_START_UP:
					{
#ifdef NO_MOVIES
						gGameState = GS_INIT_ONCE;
#else
						gGameState = GS_INIT_LOGO_MPEG;
#endif
						TRACE("gGameState = GS_INIT_ONCE");
						break;
					}

				    case GS_INIT_LOGO_MPEG:
					{
					    //if (!startupDeactivate)
						//    PlayMovieInWindow(cmdShow, "movies\\Logo.mpg");
					    gGameState = GS_LOGO_MPEG;
					    TRACE("gGameState = GS_LOGO_MPEG;");
					    break;
				    }

				    case GS_LOGO_MPEG:
					{
//					    CPad::UpdatePads();

//					    if (startupDeactivate || ControlsManager.GetJoyButtonJustDown() != 0)
						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetLeftMouseJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetEnterJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetCharJustDown(' '))
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetAltJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetTabJustDown())
//						    ++gGameState;

					    break;
				    }

				    case GS_INIT_INTRO_MPEG:
					{
//#ifndef NO_MOVIES
//					    CloseClip();
//					    CoUninitialize();
//#endif
//
//					    if (CMenuManager::OS_Language == LANG_FRENCH || CMenuManager::OS_Language == LANG_GERMAN)
//						    PlayMovieInWindow(cmdShow, "movies\\GTAtitlesGER.mpg");
//					    else
//						    PlayMovieInWindow(cmdShow, "movies\\GTAtitles.mpg");

					    gGameState = GS_INTRO_MPEG;
					    TRACE("gGameState = GS_INTRO_MPEG;");
					    break;
				    }

				    case GS_INTRO_MPEG:
					{
//					    CPad::UpdatePads();
//
//					    if (startupDeactivate || ControlsManager.GetJoyButtonJustDown() != 0)
						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetLeftMouseJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetEnterJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetCharJustDown(' '))
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetAltJustDown())
//						    ++gGameState;
//					    else if (CPad::GetPad(0)->GetTabJustDown())
//						    ++gGameState;

					    break;
				    }

					case GS_INIT_ONCE:
					{
						//CoUninitialize();
						
#ifdef PS2_MENU
						extern char version_name[64];
						if ( CGame::frenchGame || CGame::germanGame )
							LoadingScreen(NULL, version_name, "loadsc24");
						else
							LoadingScreen(NULL, version_name, "loadsc0");
						
						printf("Into TheGame!!!\n");
#else				
						LoadingScreen(nil, nil, "loadsc0");
						// LoadingScreen(nil, nil, "loadsc0"); // duplicate
#endif
						if ( !CGame::InitialiseOnceAfterRW() )
							RsGlobal.quit = TRUE;
						
#ifdef PS2_MENU
						gGameState = GS_INIT_PLAYING_GAME;
#else
						gGameState = GS_INIT_FRONTEND;
						TRACE("gGameState = GS_INIT_FRONTEND;");
#endif
						break;
					}
#ifndef PS2_MENU
					case GS_INIT_FRONTEND:
					{
						LoadingScreen(nil, nil, "loadsc0");
						// LoadingScreen(nil, nil, "loadsc0"); // duplicate
						
						FrontEndMenuManager.m_bGameNotLoaded = true;
						
						FrontEndMenuManager.m_bStartUpFrontEndRequested = true;
						
						if ( defaultFullscreenRes )
						{
							defaultFullscreenRes = FALSE;
							FrontEndMenuManager.m_nPrefsVideoMode = GcurSelVM;
							FrontEndMenuManager.m_nDisplayVideoMode = GcurSelVM;
						}
						
						gGameState = GS_FRONTEND;
						TRACE("gGameState = GS_FRONTEND;");
						break;
					}
					
					case GS_FRONTEND:
					{
						if(!WindowIconified)
							RsEventHandler(rsFRONTENDIDLE, nil);

#ifdef PS2_MENU
						if ( !FrontEndMenuManager.m_bMenuActive || TheMemoryCard.m_bWantToLoad )
#else
						if ( !FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_bWantToLoad )
#endif
						{
							gGameState = GS_INIT_PLAYING_GAME;
							TRACE("gGameState = GS_INIT_PLAYING_GAME;");
						}

#ifdef PS2_MENU
						if (TheMemoryCard.m_bWantToLoad )
#else
						if ( FrontEndMenuManager.m_bWantToLoad )
#endif
						{
							InitialiseGame();
							FrontEndMenuManager.m_bGameNotLoaded = false;
							gGameState = GS_PLAYING_GAME;
							TRACE("gGameState = GS_PLAYING_GAME;");
						}
						break;
					}
#endif
					
					case GS_INIT_PLAYING_GAME:
					{
#ifdef PS2_MENU
						CGame::Initialise("DATA\\GTA3.DAT");
						
						//LoadingScreen("Starting Game", NULL, GetRandomSplashScreen());
					
						if (   TheMemoryCard.CheckCardInserted(CARD_ONE) == CMemoryCard::NO_ERR_SUCCESS
							&& TheMemoryCard.ChangeDirectory(CARD_ONE, TheMemoryCard.Cards[CARD_ONE].dir)
							&& TheMemoryCard.FindMostRecentFileName(CARD_ONE, TheMemoryCard.MostRecentFile) == true
							&& TheMemoryCard.CheckDataNotCorrupt(TheMemoryCard.MostRecentFile))
						{
							strcpy(TheMemoryCard.LoadFileName, TheMemoryCard.MostRecentFile);
							TheMemoryCard.b_FoundRecentSavedGameWantToLoad = true;
					
							if (CMenuManager::m_PrefsLanguage != TheMemoryCard.GetLanguageToLoad())
							{
								CMenuManager::m_PrefsLanguage = TheMemoryCard.GetLanguageToLoad();
								TheText.Unload();
								TheText.Load();
							}
					
							CGame::currLevel = (eLevelName)TheMemoryCard.GetLevelToLoad();
						}
#else
						InitialiseGame();

						FrontEndMenuManager.m_bGameNotLoaded = false;
#endif
						gGameState = GS_PLAYING_GAME;
						TRACE("gGameState = GS_PLAYING_GAME;");
						break;
					}
					
					case GS_PLAYING_GAME:
					{
						float ms = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
						if ( RwInitialised )
						{
							if (!FrontEndMenuManager.m_PrefsFrameLimiter || (1000.0f / (float)RsGlobal.maxFPS) < ms)
								RsEventHandler(rsIDLE, (void *)TRUE);
						}
						break;
					}
				}
			}
			else
			{
				if ( RwCameraBeginUpdate(Scene.camera) )
				{
					RwCameraEndUpdate(Scene.camera);
					ForegroundApp = TRUE;
					RsEventHandler(rsACTIVATE, (void *)TRUE);
				}
				
			}
		}

		
		/* 
		* About to shut down - block resize events again...
		*/
		RwInitialised = FALSE;
		
		FrontEndMenuManager.UnloadTextures();
#ifdef PS2_MENU	
		if ( !(FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad))
			break;
#else
		if ( !FrontEndMenuManager.m_bWantToRestart )
			break;
#endif
		
		CPad::ResetCheats();
		CPad::StopPadsShaking();
		
		DMAudio.ChangeMusicMode(MUSICMODE_DISABLE);
		
#ifdef PS2_MENU
		CGame::ShutDownForRestart();
#endif
		
		CTimer::Stop();
		
#ifdef PS2_MENU
		if (FrontEndMenuManager.m_bWantToRestart || TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
		{
			if (TheMemoryCard.b_FoundRecentSavedGameWantToLoad)
			{
				FrontEndMenuManager.m_bWantToRestart = true;
				TheMemoryCard.m_bWantToLoad = true;
			}

			CGame::InitialiseWhenRestarting();
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			FrontEndMenuManager.m_bWantToRestart = false;
			
			continue;
		}
		
		CGame::ShutDown();	
		CTimer::Stop();
		
		break;
#else
		if ( FrontEndMenuManager.m_bWantToLoad )
		{
			CGame::ShutDownForRestart();
			CGame::InitialiseWhenRestarting();
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			LoadSplash(GetLevelSplashScreen(CGame::currLevel));
			FrontEndMenuManager.m_bWantToLoad = false;
		}
		else
		{
#ifndef MASTER
			if ( gbModelViewer )
				CAnimViewer::Shutdown();
			else
#endif
			if ( gGameState == GS_PLAYING_GAME )
				CGame::ShutDown();
			
			CTimer::Stop();
			
			if ( FrontEndMenuManager.m_bFirstTime == true )
			{
				gGameState = GS_INIT_FRONTEND;
				TRACE("gGameState = GS_INIT_FRONTEND;");
			}
			else
			{
				gGameState = GS_INIT_PLAYING_GAME;
				TRACE("gGameState = GS_INIT_PLAYING_GAME;");
			}
		}
		
		FrontEndMenuManager.m_bFirstTime = false;
		FrontEndMenuManager.m_bWantToRestart = false;
#endif
	}
	
#ifndef MASTER
	if ( gbModelViewer )
		CAnimViewer::Shutdown();
	else
#endif
	if ( gGameState == GS_PLAYING_GAME )
		CGame::ShutDown();

	DMAudio.Terminate();
	
	_psFreeVideoModeList();


	/*
	 * Tidy up the 3D (RenderWare) components of the application...
	 */
	RsEventHandler(rsRWTERMINATE, nil);

	/*
	 * Free the platform dependent data...
	 */
	RsEventHandler(rsTERMINATE, nil);

#ifdef _WIN32
	/* 
	 * Free the argv strings...
	 */
	free(argv);
	
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &SavedStickyKeys, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, TRUE, nil, SPIF_SENDCHANGE);
	SetErrorMode(0);
#endif

	return 0;
}

/*
 *****************************************************************************
 */

RwV2d leftStickPos;
RwV2d rightStickPos;

void CapturePad(RwInt32 padID)
{
	int8 glfwPad = -1;

	if( padID == 0 )
		glfwPad = PSGLOBAL(joy1id);
	else if( padID == 1)
		glfwPad = PSGLOBAL(joy2id);
	else
		assert("invalid padID");
	
	if ( glfwPad == -1 )
		return;
	
	int numButtons, numAxes;
	const uint8 *buttons = glfwGetJoystickButtons(glfwPad, &numButtons);
	const float *axes = glfwGetJoystickAxes(glfwPad, &numAxes);
	GLFWgamepadstate gamepadState;

	if (ControlsManager.m_bFirstCapture == false) {
		memcpy(&ControlsManager.m_OldState, &ControlsManager.m_NewState, sizeof(ControlsManager.m_NewState));
	} else {
		// In case connected gamepad doesn't have L-R trigger axes.
		ControlsManager.m_NewState.mappedButtons[15] = ControlsManager.m_NewState.mappedButtons[16] = 0;
	}

	ControlsManager.m_NewState.buttons = (uint8*)buttons;
	ControlsManager.m_NewState.numButtons = numButtons;
	ControlsManager.m_NewState.id = glfwPad;
	ControlsManager.m_NewState.isGamepad = glfwGetGamepadState(glfwPad, &gamepadState);
	if (ControlsManager.m_NewState.isGamepad) {
		memcpy(&ControlsManager.m_NewState.mappedButtons, gamepadState.buttons, sizeof(gamepadState.buttons));
		float lt = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER], rt = gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

		// glfw returns 0.0 for non-existent axises(which is bullocks) so we treat it as deadzone, and keep value of previous frame.
		// otherwise if this axis is present, -1 = released, 1 = pressed
		if (lt != 0.0f)
			ControlsManager.m_NewState.mappedButtons[15] = lt > -0.8f;

		if (rt != 0.0f)
			ControlsManager.m_NewState.mappedButtons[16] = rt > -0.8f;
	}
	// TODO? L2-R2 axes(not buttons-that's fine) on joysticks that don't have SDL gamepad mapping AREN'T handled, and I think it's impossible to do without mapping.

	if (ControlsManager.m_bFirstCapture == true) {
		memcpy(&ControlsManager.m_OldState, &ControlsManager.m_NewState, sizeof(ControlsManager.m_NewState));
		
		ControlsManager.m_bFirstCapture = false;
	}

	RsPadButtonStatus bs;
	bs.padID = padID;

	RsPadEventHandler(rsPADBUTTONUP, (void *)&bs);
	
	// Gamepad axes are guaranteed to return 0.0f if that particular gamepad doesn't have that axis.
	// And that's really good for sticks, because gamepads return 0.0 for them when sticks are in released state.
	if ( glfwPad != -1 ) {
		leftStickPos.x = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] : numAxes >= 1 ? axes[0] : 0.0f;
		leftStickPos.y = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] : numAxes >= 2 ? axes[1] : 0.0f;

		rightStickPos.x = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] : numAxes >= 3 ? axes[2] : 0.0f;
		rightStickPos.y = ControlsManager.m_NewState.isGamepad ? gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] : numAxes >= 4 ? axes[3] : 0.0f;
	}
	
	{
		if (CPad::m_bMapPadOneToPadTwo)
			bs.padID = 1;
		
		RsPadEventHandler(rsPADBUTTONUP,   (void *)&bs);
		RsPadEventHandler(rsPADBUTTONDOWN, (void *)&bs);
	}
	
	{
		if (CPad::m_bMapPadOneToPadTwo)
			bs.padID = 1;
		
		CPad *pad = CPad::GetPad(bs.padID);

		if ( Abs(leftStickPos.x)  > 0.3f )
			pad->PCTempJoyState.LeftStickX	= (int32)(leftStickPos.x  * 128.0f);
		
		if ( Abs(leftStickPos.y)  > 0.3f )
			pad->PCTempJoyState.LeftStickY	= (int32)(leftStickPos.y  * 128.0f);
		
		if ( Abs(rightStickPos.x) > 0.3f )
			pad->PCTempJoyState.RightStickX = (int32)(rightStickPos.x * 128.0f);

		if ( Abs(rightStickPos.y) > 0.3f )
			pad->PCTempJoyState.RightStickY = (int32)(rightStickPos.y * 128.0f);
	}

	_psHandleVibration();

	return;
}

void joysChangeCB(int jid, int event)
{
	if (event == GLFW_CONNECTED && !IsThisJoystickBlacklisted(jid)) {
		if (PSGLOBAL(joy1id) == -1) {
			PSGLOBAL(joy1id) = jid;
#ifdef DETECT_JOYSTICK_MENU
			strcpy(gSelectedJoystickName, glfwGetJoystickName(jid));
#endif
			// This is behind LOAD_INI_SETTINGS, because otherwise the Init call below will destroy/overwrite your bindings.
#ifdef LOAD_INI_SETTINGS
			int count;
			glfwGetJoystickButtons(PSGLOBAL(joy1id), &count);
			ControlsManager.InitDefaultControlConfigJoyPad(count);
#endif
		} else if (PSGLOBAL(joy2id) == -1)
			PSGLOBAL(joy2id) = jid;

	} else if (event == GLFW_DISCONNECTED) {
		if (PSGLOBAL(joy1id) == jid) {
			PSGLOBAL(joy1id) = -1;
		} else if (PSGLOBAL(joy2id) == jid)
			PSGLOBAL(joy2id) = -1;
	}
}

#if (defined(_MSC_VER))
int strcasecmp(const char* str1, const char* str2)
{
	return _strcmpi(str1, str2);
}
int strncasecmp(const char *str1, const char *str2, size_t len)
{
	return _strnicmp(str1, str2, len);
}
#endif
#endif
