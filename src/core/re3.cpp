#include <csignal>
#define WITHWINDOWS
#include "common.h"
#if defined DETECT_JOYSTICK_MENU && defined XINPUT
#include <xinput.h>
#if !defined(PSAPI_VERSION) || (PSAPI_VERSION > 1)
#pragma comment( lib, "Xinput9_1_0.lib" )
#else
#pragma comment( lib, "Xinput.lib" )
#endif
#endif
#include "Renderer.h"
#include "Occlusion.h"
#include "Credits.h"
#include "Camera.h"
#include "Weather.h"
#include "Timecycle.h"
#include "Clock.h"
#include "World.h"
#include "Vehicle.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Boat.h"
#include "Heli.h"
#include "Automobile.h"
#include "Bike.h"
#include "Console.h"
#include "Debug.h"
#include "Hud.h"
#include "SceneEdit.h"
#include "Pad.h"
#include "PlayerPed.h"
#include "Radar.h"
#include "debugmenu.h"
#include "Frontend.h"
#include "WaterLevel.h"
#include "main.h"
#include "Script.h"
#include "MBlur.h"
#include "postfx.h"
#include "custompipes.h"
#include "MemoryHeap.h"
#include "FileMgr.h"
#include "Camera.h"
#include "MBlur.h"
#include "ControllerConfig.h"
#include "CarCtrl.h"
#include "Population.h"
#include "IniFile.h"
#include "Zones.h"

#include "crossplatform.h"

#ifndef _WIN32
#include "assert.h"
#include <stdarg.h>
#endif

#include <list>

#ifdef RWLIBS
extern "C" int vsprintf(char* const _Buffer, char const* const _Format, va_list  _ArgList);
#endif


#ifdef USE_PS2_RAND
unsigned long long myrand_seed = 1;
#else
unsigned long int myrand_seed = 1;
#endif

int
myrand(void)
{
#ifdef USE_PS2_RAND
	// Use our own implementation of rand, stolen from PS2
	myrand_seed = 0x5851F42D4C957F2D * myrand_seed + 1;
	return ((myrand_seed >> 32) & 0x7FFFFFFF);
#else
	// or original codewarrior rand
	myrand_seed = myrand_seed * 1103515245 + 12345;
	return((myrand_seed >> 16) & 0x7FFF);
#endif
}

void
mysrand(unsigned int seed)
{
	myrand_seed = seed;
}

#ifdef CUSTOM_FRONTEND_OPTIONS
#include "frontendoption.h"



#ifdef MORE_LANGUAGES
void LangPolSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_POLISH;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}

void LangRusSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_RUSSIAN;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}

void LangJapSelect(int8 action)
{
	if (action == FEOPTION_ACTION_SELECT) {
		FrontEndMenuManager.m_PrefsLanguage = CMenuManager::LANGUAGE_JAPANESE;
		FrontEndMenuManager.m_bFrontEnd_ReloadObrTxtGxt = true;
		FrontEndMenuManager.InitialiseChangedLanguageSettings();
		FrontEndMenuManager.SaveSettings();
	}
}
#endif

void
CustomFrontendOptionsPopulate(void)
{
	// Most of custom options are done statically in MenuScreensCustom.cpp, we add them here only if they're dependent to extra files

	int fd;
	// These work only if we have neo folder, so they're dynamically added
#ifdef EXTENDED_PIPELINES
	const char *vehPipelineNames[] = { "FED_MFX", "FED_NEO" };
	const char *off_on[] = { "FEM_OFF", "FEM_ON" };
	fd = CFileMgr::OpenFile("neo/neo.txd","r");
	if (fd) {
#ifdef GRAPHICS_MENU_OPTIONS
		FrontendOptionSetCursor(MENUPAGE_GRAPHICS_SETTINGS, -3, false);
		FrontendOptionAddSelect("FED_VPL", 0, 0, MENUALIGN_LEFT, vehPipelineNames, ARRAY_SIZE(vehPipelineNames), (int8*)&CustomPipes::VehiclePipeSwitch, false, nil, "Graphics", "VehiclePipeline");
		FrontendOptionAddSelect("FED_PRM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::RimlightEnable, false, nil, "Graphics", "NeoRimLight");
		FrontendOptionAddSelect("FED_WLM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::LightmapEnable, false, nil, "Graphics", "NeoLightMaps");
		FrontendOptionAddSelect("FED_RGL", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::GlossEnable, false, nil, "Graphics", "NeoRoadGloss");
#else
		FrontendOptionSetCursor(MENUPAGE_DISPLAY_SETTINGS, -3, false);
		FrontendOptionAddSelect("FED_VPL", 0, 0, MENUALIGN_LEFT, vehPipelineNames, ARRAY_SIZE(vehPipelineNames), (int8*)&CustomPipes::VehiclePipeSwitch, false, nil, "Graphics", "VehiclePipeline");
		FrontendOptionAddSelect("FED_PRM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::RimlightEnable, false, nil, "Graphics", "NeoRimLight");
		FrontendOptionAddSelect("FED_WLM", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::LightmapEnable, false, nil, "Graphics", "NeoLightMaps");
		FrontendOptionAddSelect("FED_RGL", 0, 0, MENUALIGN_LEFT, off_on, 2, (int8*)&CustomPipes::GlossEnable, false, nil, "Graphics", "NeoRoadGloss");
#endif
		CFileMgr::CloseFile(fd);
	}
#endif
	// Add outsourced language translations, if files are found
#ifdef MORE_LANGUAGES
	int fd2;
	FrontendOptionSetCursor(MENUPAGE_LANGUAGE_SETTINGS, 5, false);
#if 0
	if (fd = CFileMgr::OpenFile("text/polish.gxt")) {
		if (fd2 = CFileMgr::OpenFile("models/fonts_p.txd")) {
			FrontendOptionAddDynamic("FEL_POL", 0, 0, MENUALIGN_CENTER, nil, nil, LangPolSelect, nil, nil);
			CFileMgr::CloseFile(fd2);
		}
		CFileMgr::CloseFile(fd);
	}
#endif

	if (fd = CFileMgr::OpenFile("text/russian.gxt")) {
		if (fd2 = CFileMgr::OpenFile("models/fonts_r.txd")) {
			FrontendOptionAddDynamic("FEL_RUS", 0, 0, MENUALIGN_CENTER, nil, nil, LangRusSelect, nil, nil);
			CFileMgr::CloseFile(fd2);
		}
		CFileMgr::CloseFile(fd);
	}

#if 0
	if (fd = CFileMgr::OpenFile("text/japanese.gxt")) {
		if (fd2 = CFileMgr::OpenFile("models/fonts_j.txd")) {
			FrontendOptionAddDynamic("FEL_JAP", 0, 0, MENUALIGN_CENTER, nil, nil, LangJapSelect, nil, nil);
			CFileMgr::CloseFile(fd2);
		}
		CFileMgr::CloseFile(fd);
	}
#endif
#endif

}
#endif

#ifdef LOAD_INI_SETTINGS
#define MINI_CASE_SENSITIVE
#include "ini.h"

mINI::INIFile ini("reVC.ini");
mINI::INIStructure cfg;

bool ReadIniIfExists(const char *cat, const char *key, uint32 *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtoul(section.get(key).c_str(), &endPtr, 0);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, uint8 *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtoul(section.get(key).c_str(), &endPtr, 0);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, bool *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtoul(section.get(key).c_str(), &endPtr, 0);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, int32 *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtol(section.get(key).c_str(), &endPtr, 0);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, int8 *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtol(section.get(key).c_str(), &endPtr, 0);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, float *out)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		char *endPtr;
		*out = strtof(section.get(key).c_str(), &endPtr);
		return true;
	}
	return false;
}

bool ReadIniIfExists(const char *cat, const char *key, char *out, int size)
{
	mINI::INIMap<std::string> section = cfg.get(cat);
	if (section.has(key)) {
		strncpy(out, section.get(key).c_str(), size - 1);
		out[size - 1] = '\0';
		return true;
	}
	return false;
}

void StoreIni(const char *cat, const char *key, uint32 val)
{
	char temp[11];
	sprintf(temp, "%u", val);
	cfg[cat][key] = temp;
}

void StoreIni(const char *cat, const char *key, uint8 val)
{
	char temp[11];
	sprintf(temp, "%u", val);
	cfg[cat][key] = temp;
}

void StoreIni(const char *cat, const char *key, int32 val)
{
	char temp[11];
	sprintf(temp, "%d", val);
	cfg[cat][key] = temp;
}

void StoreIni(const char *cat, const char *key, int8 val)
{
	char temp[11];
	sprintf(temp, "%d", val);
	cfg[cat][key] = temp;
}

void StoreIni(const char *cat, const char *key, float val)
{
	char temp[50];
	sprintf(temp, "%f", val);
	cfg[cat][key] = temp;
}

void StoreIni(const char *cat, const char *key, char *val, int size)
{
	cfg[cat][key] = val;
}

const char *iniControllerActions[] = { "PED_FIREWEAPON", "PED_CYCLE_WEAPON_RIGHT", "PED_CYCLE_WEAPON_LEFT", "GO_FORWARD", "GO_BACK", "GO_LEFT", "GO_RIGHT", "PED_SNIPER_ZOOM_IN",
	"PED_SNIPER_ZOOM_OUT", "VEHICLE_ENTER_EXIT", "CAMERA_CHANGE_VIEW_ALL_SITUATIONS", "PED_JUMPING", "PED_SPRINT", "PED_LOOKBEHIND", "PED_DUCK", "PED_ANSWER_PHONE", 
#ifdef BIND_VEHICLE_FIREWEAPON
	"VEHICLE_FIREWEAPON",
#endif
	"VEHICLE_ACCELERATE", "VEHICLE_BRAKE", "VEHICLE_CHANGE_RADIO_STATION", "VEHICLE_HORN", "TOGGLE_SUBMISSIONS", "VEHICLE_HANDBRAKE", "PED_1RST_PERSON_LOOK_LEFT",
	"PED_1RST_PERSON_LOOK_RIGHT", "VEHICLE_LOOKLEFT", "VEHICLE_LOOKRIGHT", "VEHICLE_LOOKBEHIND", "VEHICLE_TURRETLEFT", "VEHICLE_TURRETRIGHT", "VEHICLE_TURRETUP", "VEHICLE_TURRETDOWN",
	"PED_CYCLE_TARGET_LEFT", "PED_CYCLE_TARGET_RIGHT", "PED_CENTER_CAMERA_BEHIND_PLAYER", "PED_LOCK_TARGET", "NETWORK_TALK", "PED_1RST_PERSON_LOOK_UP", "PED_1RST_PERSON_LOOK_DOWN",
	"_CONTROLLERACTION_36", "TOGGLE_DPAD", "SWITCH_DEBUG_CAM_ON", "TAKE_SCREEN_SHOT", "SHOW_MOUSE_POINTER_TOGGLE", "UNKNOWN_ACTION" };

const char *iniControllerTypes[] = { "kbd:", "2ndKbd:", "mouse:", "joy:" };

const char *iniMouseButtons[] = {"LEFT","MIDDLE","RIGHT","WHLUP","WHLDOWN","X1","X2"};

const char *iniKeyboardButtons[] = {"ESC","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
	"INS","DEL","HOME","END","PGUP","PGDN","UP","DOWN","LEFT","RIGHT","DIVIDE","TIMES","PLUS","MINUS","PADDEL",
	"PADEND","PADDOWN","PADPGDN","PADLEFT","PAD5","NUMLOCK","PADRIGHT","PADHOME","PADUP","PADPGUP","PADINS",
	"PADENTER", "SCROLL","PAUSE","BACKSP","TAB","CAPSLK","ENTER","LSHIFT","RSHIFT","SHIFT","LCTRL","RCTRL","LALT",
	"RALT", "LWIN", "RWIN", "APPS", "NULL"};

void LoadINIControllerSettings()
{
#ifdef DETECT_JOYSTICK_MENU
#ifdef XINPUT
	int storedJoy1 = -1;
	if (ReadIniIfExists("Controller", "JoystickName", &storedJoy1)) {
		CPad::XInputJoy1 = -1;
		CPad::XInputJoy2 = -1;
		XINPUT_STATE xstate;
		memset(&xstate, 0, sizeof(XINPUT_STATE));

		// Firstly confirm & set joy 1
		if (XInputGetState(storedJoy1, &xstate) == ERROR_SUCCESS) {
			CPad::XInputJoy1 = storedJoy1;
		}

		for (int i = 0; i <= 3; i++) {
			if (XInputGetState(i, &xstate) == ERROR_SUCCESS) {
				if (CPad::XInputJoy1 == -1)
					CPad::XInputJoy1 = i;
				else if (CPad::XInputJoy2 == -1 && i != CPad::XInputJoy1)
					CPad::XInputJoy2 = i;
			}
		}

		// There is no plug event on XInput, so let's leave XInputJoy1/2 as 0/1 respectively, and hotplug will be possible.
		if (CPad::XInputJoy1 == -1) {
			CPad::XInputJoy1 = 0;
			CPad::XInputJoy2 = 1;
		} else if (CPad::XInputJoy2 == -1) {
			CPad::XInputJoy2 = (CPad::XInputJoy1 + 1) % 4;
		}
	}
#else
	ReadIniIfExists("Controller", "JoystickName", gSelectedJoystickName, 128);
#endif
#endif
	// force to default GTA behaviour (never overwrite bindings on joy change/initialization) if user init'ed/set bindings before we introduced that
	if (!ReadIniIfExists("Controller", "PadButtonsInited", &ControlsManager.ms_padButtonsInited)) {
		ControlsManager.ms_padButtonsInited = cfg.get("Bindings").size() != 0 ? 16 : 0;
	}

	for (int32 i = 0; i < MAX_CONTROLLERACTIONS; i++) {
		char value[128];
		if (ReadIniIfExists("Bindings", iniControllerActions[i], value, 128)) {
			for (int32 j = 0; j < MAX_CONTROLLERTYPES; j++){
				ControlsManager.ClearSettingsAssociatedWithAction((e_ControllerAction)i, (eControllerType)j);
			}

			for (char *binding = strtok(value,", "); binding != nil; binding = strtok(nil, ", ")) {
				int contType = -1;
				for (int32 k = 0; k < ARRAY_SIZE(iniControllerTypes); k++) {
					int len = strlen(iniControllerTypes[k]);
					if (strncmp(binding, iniControllerTypes[k], len) == 0) {
						contType = k;
						binding += len;
						break;
					}
				}
				if (contType == -1)
					continue;

				int contKey;
				if (contType == JOYSTICK) {
					char *temp;
					contKey = strtol(binding, &temp, 0);

				} else if (contType == KEYBOARD || contType == OPTIONAL_EXTRA) {
					if (strlen(binding) == 1) {
						contKey = binding[0];
					} else if(strcmp(binding, "SPC") == 0) {
						contKey = ' ';
					} else {
						for (int32 k = 0; k < ARRAY_SIZE(iniKeyboardButtons); k++) {
							if(strcmp(binding, iniKeyboardButtons[k]) == 0) {
								contKey = 1000 + k;
								break;
							}
						}
					}
				} else if (contType == MOUSE) {
					for (int32 k = 0; k < ARRAY_SIZE(iniMouseButtons); k++) {
						if(strcmp(binding, iniMouseButtons[k]) == 0) {
							contKey = 1 + k;
							break;
						}
					}
				}

				ControlsManager.SetControllerKeyAssociatedWithAction((e_ControllerAction)i, contKey, (eControllerType)contType);
			}
		}
	}
}

void SaveINIControllerSettings()
{
	for (int32 i = 0; i < MAX_CONTROLLERACTIONS; i++) {
		char value[128] = { '\0' };

		// upper limit should've been GetNumOfSettingsForAction(i), but sadly even R* doesn't use it's own system correctly, and there are gaps between orders.
		for (int32 j = SETORDER_1; j < MAX_SETORDERS; j++){

			// We respect the m_ContSetOrder, and join/implode/order the bindings according to that; using comma as seperator.
			for (int32 k = 0; k < MAX_CONTROLLERTYPES; k++){
				if (ControlsManager.m_aSettings[i][k].m_ContSetOrder == j) {
					char next[32];
					if (k == JOYSTICK) {
						snprintf(next, 32, "%s%d,", iniControllerTypes[k], ControlsManager.m_aSettings[i][k].m_Key);

					} else if (k == KEYBOARD || k == OPTIONAL_EXTRA) {
						if (ControlsManager.m_aSettings[i][k].m_Key == ' ')
							snprintf(next, 32, "%sSPC,", iniControllerTypes[k]);
						else if (ControlsManager.m_aSettings[i][k].m_Key < 256)
							snprintf(next, 32, "%s%c,", iniControllerTypes[k], ControlsManager.m_aSettings[i][k].m_Key);
						else
							snprintf(next, 32, "%s%s,", iniControllerTypes[k], iniKeyboardButtons[ControlsManager.m_aSettings[i][k].m_Key - 1000]);

					} else if (k == MOUSE) {
						snprintf(next, 32, "%s%s,", iniControllerTypes[k], iniMouseButtons[ControlsManager.m_aSettings[i][k].m_Key - 1]);
					}
					strcat(value, next);
					break;
				}
			}
		}
		int len = strlen(value);
		if (len > 0)
			value[len - 1] = '\0'; // to remove comma

		StoreIni("Bindings", iniControllerActions[i], value, 128);
	}

#ifdef DETECT_JOYSTICK_MENU
#ifdef XINPUT
	StoreIni("Controller", "JoystickName", CPad::XInputJoy1);
#else
	StoreIni("Controller", "JoystickName", gSelectedJoystickName, 128);
#endif
#endif
	StoreIni("Controller", "PadButtonsInited", ControlsManager.ms_padButtonsInited);

	ini.write(cfg);
}

bool LoadINISettings()
{
	if (!ini.read(cfg))
		return false;

#ifdef IMPROVED_VIDEOMODE
	ReadIniIfExists("VideoMode", "Width", &FrontEndMenuManager.m_nPrefsWidth);
	ReadIniIfExists("VideoMode", "Height", &FrontEndMenuManager.m_nPrefsHeight);
	ReadIniIfExists("VideoMode", "Depth", &FrontEndMenuManager.m_nPrefsDepth);
	ReadIniIfExists("VideoMode", "Subsystem", &FrontEndMenuManager.m_nPrefsSubsystem);
	// Windowed mode is loaded below in CUSTOM_FRONTEND_OPTIONS section
#else
	ReadIniIfExists("Graphics", "VideoMode", &FrontEndMenuManager.m_nDisplayVideoMode);
#endif
	ReadIniIfExists("Controller", "HeadBob1stPerson", &TheCamera.m_bHeadBob);
	ReadIniIfExists("Controller", "HorizantalMouseSens", &TheCamera.m_fMouseAccelHorzntl);
	ReadIniIfExists("Controller", "InvertMouseVertically", &MousePointerStateHelper.bInvertVertically);
	ReadIniIfExists("Controller", "DisableMouseSteering", &CVehicle::m_bDisableMouseSteering);
	ReadIniIfExists("Controller", "Vibration", &FrontEndMenuManager.m_PrefsUseVibration);
	ReadIniIfExists("Audio", "SfxVolume", &FrontEndMenuManager.m_PrefsSfxVolume);
	ReadIniIfExists("Audio", "MusicVolume", &FrontEndMenuManager.m_PrefsMusicVolume);
	ReadIniIfExists("Audio", "MP3BoostVolume", &FrontEndMenuManager.m_PrefsMP3BoostVolume);
	ReadIniIfExists("Audio", "Radio", &FrontEndMenuManager.m_PrefsRadioStation);
#ifdef EXTERNAL_3D_SOUND
	ReadIniIfExists("Audio", "SpeakerType", &FrontEndMenuManager.m_PrefsSpeakers);
	ReadIniIfExists("Audio", "Provider", &FrontEndMenuManager.m_nPrefsAudio3DProviderIndex);
#endif
	ReadIniIfExists("Audio", "DynamicAcoustics", &FrontEndMenuManager.m_PrefsDMA);
	ReadIniIfExists("Display", "Brightness", &FrontEndMenuManager.m_PrefsBrightness);
	ReadIniIfExists("Display", "DrawDistance", &FrontEndMenuManager.m_PrefsLOD);
	ReadIniIfExists("Display", "Subtitles", &FrontEndMenuManager.m_PrefsShowSubtitles);
	ReadIniIfExists("Graphics", "AspectRatio", &FrontEndMenuManager.m_PrefsUseWideScreen);
	ReadIniIfExists("Graphics", "FrameLimiter", &FrontEndMenuManager.m_PrefsFrameLimiter);
#ifdef LEGACY_MENU_OPTIONS
	ReadIniIfExists("Graphics", "VSync", &FrontEndMenuManager.m_PrefsVsyncDisp);
	ReadIniIfExists("Graphics", "Trails", &CMBlur::BlurOn);
#endif
	ReadIniIfExists("General", "SkinFile", FrontEndMenuManager.m_PrefsSkinFile, 256);
	ReadIniIfExists("Controller", "Method", &FrontEndMenuManager.m_ControlMethod);
	ReadIniIfExists("General", "Language", &FrontEndMenuManager.m_PrefsLanguage);
	ReadIniIfExists("Display", "ShowHud", &FrontEndMenuManager.m_PrefsShowHud);
	ReadIniIfExists("Display", "RadarMode", &FrontEndMenuManager.m_PrefsRadarMode);
	ReadIniIfExists("Display", "ShowLegends", &FrontEndMenuManager.m_PrefsShowLegends);

#ifdef EXTENDED_COLOURFILTER
	ReadIniIfExists("CustomPipesValues", "PostFXIntensity", &CPostFX::Intensity);
#endif
#ifdef EXTENDED_PIPELINES
	ReadIniIfExists("CustomPipesValues", "NeoVehicleShininess", &CustomPipes::VehicleShininess);
	ReadIniIfExists("CustomPipesValues", "NeoVehicleSpecularity", &CustomPipes::VehicleSpecularity);
	ReadIniIfExists("CustomPipesValues", "RimlightMult", &CustomPipes::RimlightMult);
	ReadIniIfExists("CustomPipesValues", "LightmapMult", &CustomPipes::LightmapMult);
	ReadIniIfExists("CustomPipesValues", "GlossMult", &CustomPipes::GlossMult);
#endif
	ReadIniIfExists("Rendering", "BackfaceCulling", &gBackfaceCulling);
#ifdef NEW_RENDERER
	ReadIniIfExists("Rendering", "NewRenderer", &gbNewRenderer);
#endif

#ifdef PROPER_SCALING
	ReadIniIfExists("Draw", "ProperScaling", &CDraw::ms_bProperScaling);	
#endif
#ifdef FIX_RADAR
	ReadIniIfExists("Draw", "FixRadar", &CDraw::ms_bFixRadar);	
#endif
#ifdef FIX_SPRITES
	ReadIniIfExists("Draw", "FixSprites", &CDraw::ms_bFixSprites);	
#endif
#ifdef DRAW_GAME_VERSION_TEXT
	ReadIniIfExists("General", "DrawVersionText", &gbDrawVersionText);
#endif
#ifdef NO_MOVIES
	ReadIniIfExists("General", "NoMovies", &gbNoMovies);
#endif

#ifdef CUSTOM_FRONTEND_OPTIONS
	bool migrate = cfg.get("FrontendOptions").size() != 0;
	for (int i = 0; i < MENUPAGES; i++) {
		for (int j = 0; j < NUM_MENUROWS; j++) {
			CMenuScreenCustom::CMenuEntry &option = aScreens[i].m_aEntries[j];
			if (option.m_Action == MENUACTION_NOTHING)
				break;
				
			// CFO check
			if (option.m_Action < MENUACTION_NOTHING && option.m_CFO->save) {
				// Migrate from old .ini to new .ini
				// Old values can only be int8, new ones can contain float if it is slider
				if (migrate && ReadIniIfExists("FrontendOptions", option.m_CFO->save, (int8*)option.m_CFO->value))
					cfg["FrontendOptions"].remove(option.m_CFO->save);
				else if (option.m_Action == MENUACTION_CFO_SLIDER)
					ReadIniIfExists(option.m_CFO->saveCat, option.m_CFO->save, (float*)option.m_CFO->value);
				else
					ReadIniIfExists(option.m_CFO->saveCat, option.m_CFO->save, (int8*)option.m_CFO->value);

				if (option.m_Action == MENUACTION_CFO_SELECT) {
					option.m_CFOSelect->lastSavedValue = option.m_CFOSelect->displayedValue = *(int8*)option.m_CFO->value;
				}
			}
		}
	}
#endif

	// Fetched in above block, but needs evaluation
#ifdef PED_CAR_DENSITY_SLIDERS
	CPopulation::MaxNumberOfPedsInUse = DEFAULT_MAX_NUMBER_OF_PEDS * CIniFile::PedNumberMultiplier;
	CPopulation::MaxNumberOfPedsInUseInterior = DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR * CIniFile::PedNumberMultiplier;
	CCarCtrl::MaxNumberOfCarsInUse = DEFAULT_MAX_NUMBER_OF_CARS * CIniFile::CarNumberMultiplier;
#endif

	return true;
}

void SaveINISettings()
{
#ifdef IMPROVED_VIDEOMODE
	StoreIni("VideoMode", "Width", FrontEndMenuManager.m_nPrefsWidth);
	StoreIni("VideoMode", "Height", FrontEndMenuManager.m_nPrefsHeight);
	StoreIni("VideoMode", "Depth", FrontEndMenuManager.m_nPrefsDepth);
	StoreIni("VideoMode", "Subsystem", FrontEndMenuManager.m_nPrefsSubsystem);
	// Windowed mode is loaded below in CUSTOM_FRONTEND_OPTIONS section
#else
	StoreIni("Graphics", "VideoMode", FrontEndMenuManager.m_nDisplayVideoMode);
#endif
	StoreIni("Controller", "HeadBob1stPerson", TheCamera.m_bHeadBob);
	StoreIni("Controller", "HorizantalMouseSens", TheCamera.m_fMouseAccelHorzntl);
	StoreIni("Controller", "InvertMouseVertically", MousePointerStateHelper.bInvertVertically);
	StoreIni("Controller", "DisableMouseSteering", CVehicle::m_bDisableMouseSteering);
	StoreIni("Controller", "Vibration", FrontEndMenuManager.m_PrefsUseVibration);
	StoreIni("Audio", "SfxVolume", FrontEndMenuManager.m_PrefsSfxVolume);
	StoreIni("Audio", "MusicVolume", FrontEndMenuManager.m_PrefsMusicVolume);
	StoreIni("Audio", "MP3BoostVolume", FrontEndMenuManager.m_PrefsMP3BoostVolume);
	StoreIni("Audio", "Radio", FrontEndMenuManager.m_PrefsRadioStation);
#ifdef EXTERNAL_3D_SOUND
	StoreIni("Audio", "SpeakerType", FrontEndMenuManager.m_PrefsSpeakers);
	StoreIni("Audio", "Provider", FrontEndMenuManager.m_nPrefsAudio3DProviderIndex);
#endif
	StoreIni("Audio", "DynamicAcoustics", FrontEndMenuManager.m_PrefsDMA);
	StoreIni("Display", "Brightness", FrontEndMenuManager.m_PrefsBrightness);
	StoreIni("Display", "DrawDistance", FrontEndMenuManager.m_PrefsLOD);
	StoreIni("Display", "Subtitles", FrontEndMenuManager.m_PrefsShowSubtitles);
	StoreIni("Graphics", "AspectRatio", FrontEndMenuManager.m_PrefsUseWideScreen);
#ifdef LEGACY_MENU_OPTIONS
	StoreIni("Graphics", "VSync", FrontEndMenuManager.m_PrefsVsyncDisp);
	StoreIni("Graphics", "Trails", CMBlur::BlurOn);
#endif
	StoreIni("Graphics", "FrameLimiter", FrontEndMenuManager.m_PrefsFrameLimiter);
	StoreIni("General", "SkinFile", FrontEndMenuManager.m_PrefsSkinFile, 256);
	StoreIni("Controller", "Method", FrontEndMenuManager.m_ControlMethod);
	StoreIni("General", "Language", FrontEndMenuManager.m_PrefsLanguage);
	StoreIni("Display", "ShowHud", FrontEndMenuManager.m_PrefsShowHud);
	StoreIni("Display", "RadarMode", FrontEndMenuManager.m_PrefsRadarMode);
	StoreIni("Display", "ShowLegends", FrontEndMenuManager.m_PrefsShowLegends);

#ifdef EXTENDED_COLOURFILTER
	StoreIni("CustomPipesValues", "PostFXIntensity", CPostFX::Intensity);
#endif
#ifdef EXTENDED_PIPELINES
	StoreIni("CustomPipesValues", "NeoVehicleShininess", CustomPipes::VehicleShininess);
	StoreIni("CustomPipesValues", "NeoVehicleSpecularity", CustomPipes::VehicleSpecularity);
	StoreIni("CustomPipesValues", "RimlightMult", CustomPipes::RimlightMult);
	StoreIni("CustomPipesValues", "LightmapMult", CustomPipes::LightmapMult);
	StoreIni("CustomPipesValues", "GlossMult", CustomPipes::GlossMult);
#endif
	StoreIni("Rendering", "BackfaceCulling", gBackfaceCulling);
#ifdef NEW_RENDERER
	StoreIni("Rendering", "NewRenderer", gbNewRenderer);
#endif

#ifdef PROPER_SCALING	
	StoreIni("Draw", "ProperScaling", CDraw::ms_bProperScaling);	
#endif
#ifdef FIX_RADAR
	StoreIni("Draw", "FixRadar", CDraw::ms_bFixRadar);
#endif
#ifdef FIX_SPRITES
	StoreIni("Draw", "FixSprites", CDraw::ms_bFixSprites);	
#endif
#ifdef DRAW_GAME_VERSION_TEXT
	StoreIni("General", "DrawVersionText", gbDrawVersionText);
#endif
#ifdef NO_MOVIES
	StoreIni("General", "NoMovies", gbNoMovies);
#endif
#ifdef CUSTOM_FRONTEND_OPTIONS
	for (int i = 0; i < MENUPAGES; i++) {
		for (int j = 0; j < NUM_MENUROWS; j++) {
			CMenuScreenCustom::CMenuEntry &option = aScreens[i].m_aEntries[j];
			if (option.m_Action == MENUACTION_NOTHING)
				break;
				
			if (option.m_Action < MENUACTION_NOTHING && option.m_CFO->save) {
				if (option.m_Action == MENUACTION_CFO_SLIDER)
					StoreIni(option.m_CFO->saveCat, option.m_CFO->save, *(float*)option.m_CFO->value);
				else
					StoreIni(option.m_CFO->saveCat, option.m_CFO->save, *(int8*)option.m_CFO->value);
			}
		}
	}
#endif

	ini.write(cfg);
}

#endif

#ifdef DEBUGMENU
void WeaponCheat1();
void WeaponCheat2();
void WeaponCheat3();
void HealthCheat();
void VehicleCheat(int model);
void BlowUpCarsCheat();
void ChangePlayerCheat();
void MayhemCheat();
void EverybodyAttacksPlayerCheat();
void WeaponsForAllCheat();
void FastTimeCheat();
void SlowTimeCheat();
void MoneyCheat();
void ArmourCheat();
void WantedLevelUpCheat();
void WantedLevelDownCheat();
void SunnyWeatherCheat();
void CloudyWeatherCheat();
void RainyWeatherCheat();
void FoggyWeatherCheat();
void FastWeatherCheat();
void OnlyRenderWheelsCheat();
void ChittyChittyBangBangCheat();
void StrongGripCheat();
void SpecialCarCheats();
void PickUpChicksCheat();

DebugMenuEntry *carCol1;
DebugMenuEntry *carCol2;

void
SpawnCar(int id)
{
	CVector playerpos;
	CStreaming::RequestModel(id, 0);
	CStreaming::LoadAllRequestedModels(false);
	if(CStreaming::HasModelLoaded(id)){
		playerpos = FindPlayerCoors();
		int node;
		if(!CModelInfo::IsBoatModel(id)){
			node = ThePaths.FindNodeClosestToCoors(playerpos, 0, 100.0f, false, false);
			if(node < 0)
				return;
		}

		CVehicle *v;
		if(CModelInfo::IsBoatModel(id))
			v = new CBoat(id, RANDOM_VEHICLE);
		else if(CModelInfo::IsBikeModel(id))
			v = new CBike(id, RANDOM_VEHICLE);
		else
			v = new CAutomobile(id, RANDOM_VEHICLE);

		v->bHasBeenOwnedByPlayer = true;
		if(carCol1)
			DebugMenuEntrySetAddress(carCol1, &v->m_currentColour1);
		if(carCol2)
			DebugMenuEntrySetAddress(carCol2, &v->m_currentColour2);

		if(CModelInfo::IsBoatModel(id))
			v->SetPosition(TheCamera.GetPosition() + TheCamera.GetForward()*15.0f);
		else
			v->SetPosition(ThePaths.m_pathNodes[node].GetPosition());

		v->GetMatrix().GetPosition().z += 4.0f;
		v->SetOrientation(0.0f, 0.0f, 3.49f);
		v->SetStatus(STATUS_ABANDONED);
		v->m_nDoorLock = CARLOCK_UNLOCKED;
		CWorld::Add(v);
	}
}

static void
FixCar(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;
	veh->m_fHealth = 1000.0f;
	if(veh->IsCar()){
		((CAutomobile*)veh)->Damage.SetEngineStatus(0);
		((CAutomobile*)veh)->Fix();
	}else if(veh->IsBike()){
		((CBike*)veh)->Fix();
	}
}

#ifdef MAP_ENHANCEMENTS
static void
TeleportToWaypoint(void)
{
	if (CRadar::TargetMarkerId == -1)
		return;
	CEntity* pEntityToTeleport = FindPlayerEntity();
	CVector vNewPos = CRadar::TargetMarkerPos;
	CStreaming::LoadScene(vNewPos);
	CStreaming::LoadSceneCollision(vNewPos);
	vNewPos.z = CWorld::FindGroundZForCoord(vNewPos.x, vNewPos.y) + pEntityToTeleport->GetDistanceFromCentreOfMassToBaseOfModel();
	pEntityToTeleport->Teleport(vNewPos);
}
#endif

static void
SwitchCarCollision(void)
{
	if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
		FindPlayerVehicle()->bUsesCollision = !FindPlayerVehicle()->bUsesCollision;
}

static void
ToggleComedy(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;
	veh->bComedyControls = !veh->bComedyControls;
}

static void
PlaceOnRoad(void)
{
	CVehicle *veh = FindPlayerVehicle();
	if(veh == nil)
		return;

	if(veh->IsCar())
		((CAutomobile*)veh)->PlaceOnRoadProperly();
}

static void
ResetCamStatics(void)
{
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
}

#ifdef MISSION_SWITCHER
int8 nextMissionToSwitch = 0;
static void
SwitchToMission(void)
{
	CTheScripts::SwitchToMission(nextMissionToSwitch);
}
#endif

static const char *carnames[] = {
	"landstal", "idaho", "stinger", "linerun", "peren", "sentinel", "rio", "firetruk", "trash", "stretch", "manana",
	"infernus", "voodoo", "pony", "mule", "cheetah", "ambulan", "fbicar", "moonbeam", "esperant", "taxi", "washing",
	"bobcat", "mrwhoop", "bfinject", "hunter", "police", "enforcer", "securica", "banshee", "predator", "bus",
	"rhino", "barracks", "cuban", "chopper", "angel", "coach", "cabbie", "stallion", "rumpo", "rcbandit", "romero",
	"packer", "sentxs", "admiral", "squalo", "seaspar", "pizzaboy", "gangbur", "airtrain", "deaddodo", "speeder",
	"reefer", "tropic", "flatbed", "yankee", "caddy", "zebra", "topfun", "skimmer", "pcj600", "faggio", "freeway",
	"rcbaron", "rcraider", "glendale", "oceanic", "sanchez", "sparrow", "patriot", "lovefist", "coastg", "dinghy",
	"hermes", "sabre", "sabretur", "pheonix", "walton", "regina", "comet", "deluxo", "burrito", "spand", "marquis",
	"baggage", "kaufman", "maverick", "vcnmav", "rancher", "fbiranch", "virgo", "greenwoo", "jetmax", "hotring",
	"sandking", "blistac", "polmav", "boxville", "benson", "mesa", "rcgoblin", "hotrina", "hotrinb",
	"bloodra", "bloodrb", "vicechee"
};

static CTweakVar** TweakVarsList;
static int TweakVarsListSize = -1;
static bool bAddTweakVarsNow = false;
static const char *pTweakVarsDefaultPath = NULL;

void CTweakVars::Add(CTweakVar *var)
{
	if(TweakVarsListSize == -1) {
		TweakVarsList = (CTweakVar**)malloc(64 * sizeof(CTweakVar*));
		TweakVarsListSize = 0;
	}
	if(TweakVarsListSize > 63)
		TweakVarsList = (CTweakVar**) realloc(TweakVarsList, (TweakVarsListSize + 1) * sizeof(CTweakVar*));

	TweakVarsList[TweakVarsListSize++] = var;
//	TweakVarsList.push_back(var);
	
	if ( bAddTweakVarsNow )
		var->AddDBG(pTweakVarsDefaultPath);
}

void CTweakVars::AddDBG(const char *path)
{
	pTweakVarsDefaultPath = path;

	for(int i = 0; i < TweakVarsListSize; ++i)
		TweakVarsList[i]->AddDBG(pTweakVarsDefaultPath);
	
	bAddTweakVarsNow = true;
}

void CTweakSwitch::AddDBG(const char *path)
{		
	DebugMenuEntry *e = DebugMenuAddVar(m_pPath == NULL ? path : m_pPath, m_pVarName, (int32_t *)m_pIntVar, m_pFunc, 1, m_nMin, m_nMax, m_aStr);
	DebugMenuEntrySetWrap(e, true);
}
	
void CTweakFunc::AddDBG  (const char *path) { DebugMenuAddCmd     (m_pPath == NULL ? path : m_pPath, m_pVarName, m_pFunc); }
void CTweakBool::AddDBG  (const char *path) { DebugMenuAddVarBool8(m_pPath == NULL ? path : m_pPath, m_pVarName, (int8_t *)m_pBoolVar,  NULL); }
void CTweakInt8::AddDBG  (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int8_t *)m_pIntVar,   NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt8::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint8_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakInt16::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int16_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt16::AddDBG(const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint16_t *)m_pIntVar, NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakInt32::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (int32_t *)m_pIntVar,  NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakUInt32::AddDBG(const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (uint32_t *)m_pIntVar, NULL, m_nStep, m_nLoawerBound, m_nUpperBound, NULL); }
void CTweakFloat::AddDBG (const char *path) { DebugMenuAddVar     (m_pPath == NULL ? path : m_pPath, m_pVarName, (float *)m_pIntVar,    NULL, m_nStep, m_nLoawerBound, m_nUpperBound); }

/*
static const char *wt[] = {
			"Sunny", "Cloudy", "Rainy", "Foggy"
		};

SETTWEAKPATH("TEST");		
TWEAKSWITCH(CWeather::NewWeatherType, 0, 3, wt, NULL);
*/

void
switchWeather(void)
{
	CWeather::StreamAfterRainTimer = 0;
}

void
DebugMenuPopulate(void)
{
	if(1){
		static const char *weathers[] = {
			"Sunny", "Cloudy", "Rainy", "Foggy", "Extrasunny", "Stormy"
		};
		static const char *extracols[] = {
			"1 - Malibu club",
			"2 - Strib club",
			"3 - Hotel",
			"4 - Bank",
			"5 - Police HQ",
			"6 - Mall",
			"7 - Rifle Range",
			"8 - Mansion",
			"9 - Dirt ring",
			"10 - Blood ring",
			"11 - Hot ring",
			"12 - Concert hall",
			"13 - Auntie Poulets",
			"14 - Intro at docks",
			"15 - Biker bar",
			"16 - Intro cafe",
			"17 - Studio",
			"18", "19", "20", "21", "22", "23", "24"
		};
		DebugMenuEntry *e;
		e = DebugMenuAddVar("Time & Weather", "Current Hour", &CClock::GetHoursRef(), nil, 1, 0, 23, nil);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Current Minute", &CClock::GetMinutesRef(),
			[](){ CWeather::InterpolationValue = CClock::GetMinutes()/60.0f; }, 1, 0, 59, nil);
			DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "Old Weather", (int16*)&CWeather::OldWeatherType, switchWeather, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		e = DebugMenuAddVar("Time & Weather", "New Weather", (int16*)&CWeather::NewWeatherType, switchWeather, 1, 0, 5, weathers);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVarBool32("Time & Weather", "Extracolours On", &CTimeCycle::m_bExtraColourOn, nil);
		DebugMenuAddVar("Time & Weather", "Extracolour", &CTimeCycle::m_ExtraColour, nil, 1, 0, 23, extracols);
		DebugMenuAddVar("Time & Weather", "Time scale", (float*)&CTimer::GetTimeScale(), nil, 0.1f, 0.0f, 10.0f);

		DebugMenuAddCmd("Cheats", "Weapon set 1", WeaponCheat1);
		DebugMenuAddCmd("Cheats", "Weapon set 2", WeaponCheat2);
		DebugMenuAddCmd("Cheats", "Weapon set 3", WeaponCheat3);
		DebugMenuAddCmd("Cheats", "Money", MoneyCheat);
		DebugMenuAddCmd("Cheats", "Health", HealthCheat);
		DebugMenuAddCmd("Cheats", "Wanted level up", WantedLevelUpCheat);
		DebugMenuAddCmd("Cheats", "Wanted level down", WantedLevelDownCheat);
		DebugMenuAddCmd("Cheats", "Tank", []() { VehicleCheat(MI_TAXI); });
		DebugMenuAddCmd("Cheats", "Blow up cars", BlowUpCarsCheat);
		DebugMenuAddCmd("Cheats", "Change player", ChangePlayerCheat);
		DebugMenuAddCmd("Cheats", "Mayhem", MayhemCheat);
		DebugMenuAddCmd("Cheats", "Everybody attacks player", EverybodyAttacksPlayerCheat);
		DebugMenuAddCmd("Cheats", "Weapons for all", WeaponsForAllCheat);
		DebugMenuAddCmd("Cheats", "Fast time", FastTimeCheat);
		DebugMenuAddCmd("Cheats", "Slow time", SlowTimeCheat);
		DebugMenuAddCmd("Cheats", "Armour", ArmourCheat);
		DebugMenuAddCmd("Cheats", "Sunny weather", SunnyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Cloudy weather", CloudyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Rainy weather", RainyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Foggy weather", FoggyWeatherCheat);
		DebugMenuAddCmd("Cheats", "Fast weather", FastWeatherCheat);
		DebugMenuAddCmd("Cheats", "Only render wheels", OnlyRenderWheelsCheat);
		DebugMenuAddCmd("Cheats", "Chitty chitty bang bang", ChittyChittyBangBangCheat);
		DebugMenuAddCmd("Cheats", "Strong grip", StrongGripCheat);
		DebugMenuAddCmd("Cheats", "Special car", SpecialCarCheats);
		DebugMenuAddCmd("Cheats", "Pickup chicks", PickUpChicksCheat);

		static int spawnCarId = MI_LANDSTAL;
		e = DebugMenuAddVar("Spawn", "Spawn Car ID", &spawnCarId, nil, 1, MI_LANDSTAL, MI_VICECHEE, carnames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddCmd("Spawn", "Spawn Car", [](){
			if(spawnCarId == MI_CHOPPER ||
			   spawnCarId == MI_AIRTRAIN ||
			   spawnCarId == MI_DEADDODO)
				return;
			SpawnCar(spawnCarId);
		});
		static uint8 dummy;
		carCol1 = DebugMenuAddVar("Spawn", "First colour", &dummy, nil, 1, 0, 255, nil);
		carCol2 = DebugMenuAddVar("Spawn", "Second colour", &dummy, nil, 1, 0, 255, nil);
		DebugMenuAddCmd("Spawn", "Spawn Stinger", [](){ SpawnCar(MI_STINGER); });
		DebugMenuAddCmd("Spawn", "Spawn Infernus", [](){ SpawnCar(MI_INFERNUS); });
		DebugMenuAddCmd("Spawn", "Spawn Cheetah", [](){ SpawnCar(MI_CHEETAH); });
		DebugMenuAddCmd("Spawn", "Spawn Phoenix", [](){ SpawnCar(MI_PHEONIX); });
		DebugMenuAddCmd("Spawn", "Spawn Banshee", [](){ SpawnCar(MI_BANSHEE); });
		DebugMenuAddCmd("Spawn", "Spawn Esperanto", [](){ SpawnCar(MI_ESPERANT); });
		DebugMenuAddCmd("Spawn", "Spawn Stallion", [](){ SpawnCar(MI_STALLION); });
		DebugMenuAddCmd("Spawn", "Spawn Admiral", [](){ SpawnCar(MI_ADMIRAL); });
		DebugMenuAddCmd("Spawn", "Spawn Washington", [](){ SpawnCar(MI_WASHING); });
		DebugMenuAddCmd("Spawn", "Spawn Taxi", [](){ SpawnCar(MI_TAXI); });
		DebugMenuAddCmd("Spawn", "Spawn Police", [](){ SpawnCar(MI_POLICE); });
		DebugMenuAddCmd("Spawn", "Spawn Enforcer", [](){ SpawnCar(MI_ENFORCER); });
		DebugMenuAddCmd("Spawn", "Spawn Cuban", [](){ SpawnCar(MI_CUBAN); });
		DebugMenuAddCmd("Spawn", "Spawn Voodoo", [](){ SpawnCar(MI_VOODOO); });
		DebugMenuAddCmd("Spawn", "Spawn BF injection", [](){ SpawnCar(MI_BFINJECT); });
		DebugMenuAddCmd("Spawn", "Spawn Maverick", [](){ SpawnCar(MI_MAVERICK); });
		DebugMenuAddCmd("Spawn", "Spawn VCN Maverick", [](){ SpawnCar(MI_VCNMAV); });
		DebugMenuAddCmd("Spawn", "Spawn Sparrow", [](){ SpawnCar(MI_SPARROW); });
		DebugMenuAddCmd("Spawn", "Spawn Sea Sparrow", [](){ SpawnCar(MI_SEASPAR); });
		DebugMenuAddCmd("Spawn", "Spawn Hunter", [](){ SpawnCar(MI_HUNTER); });
		DebugMenuAddCmd("Spawn", "Spawn Rhino", [](){ SpawnCar(MI_RHINO); });
		DebugMenuAddCmd("Spawn", "Spawn Firetruck", [](){ SpawnCar(MI_FIRETRUCK); });
		DebugMenuAddCmd("Spawn", "Spawn Predator", [](){ SpawnCar(MI_PREDATOR); });
		DebugMenuAddCmd("Spawn", "Spawn PCJ 600", [](){ SpawnCar(MI_PCJ600); });
		DebugMenuAddCmd("Spawn", "Spawn Faggio", [](){ SpawnCar(MI_FAGGIO); });
		DebugMenuAddCmd("Spawn", "Spawn Freeway", [](){ SpawnCar(MI_FREEWAY); });
		DebugMenuAddCmd("Spawn", "Spawn Squalo", [](){ SpawnCar(MI_SQUALO); });
		DebugMenuAddCmd("Spawn", "Spawn Skimmer", [](){ SpawnCar(MI_SKIMMER); });

		DebugMenuAddVarBool8("Render", "Draw hud", &CHud::m_Wants_To_Draw_Hud, nil);
#ifdef PROPER_SCALING	
		DebugMenuAddVarBool8("Render", "Proper Scaling", &CDraw::ms_bProperScaling, nil);
#endif
#ifdef FIX_RADAR
		DebugMenuAddVarBool8("Render", "Fix Radar", &CDraw::ms_bFixRadar, nil);
#endif
#ifdef FIX_SPRITES
		DebugMenuAddVarBool8("Render", "Fix Sprites", &CDraw::ms_bFixSprites, nil);
#endif
		DebugMenuAddVarBool8("Render", "Backface Culling", &gBackfaceCulling, nil);
		DebugMenuAddVarBool8("Render", "PS2 Alpha test Emu", &gPS2alphaTest, nil);
		DebugMenuAddVarBool8("Render", "Frame limiter", &FrontEndMenuManager.m_PrefsFrameLimiter, nil);
		DebugMenuAddVarBool8("Render", "VSynch", &FrontEndMenuManager.m_PrefsVsync, nil);
		DebugMenuAddVar("Render", "Max FPS", &RsGlobal.maxFPS, nil, 1, 1, 1000, nil);
#ifdef NEW_RENDERER
		DebugMenuAddVarBool8("Render", "New Renderer", &gbNewRenderer, nil);
extern bool gbRenderRoads;
extern bool gbRenderEverythingBarRoads;
extern bool gbRenderFadingInUnderwaterEntities;
extern bool gbRenderFadingInEntities;
extern bool gbRenderWater;
extern bool gbRenderBoats;
extern bool gbRenderVehicles;
extern bool gbRenderWorld0;
extern bool gbRenderWorld1;
extern bool gbRenderWorld2;
		DebugMenuAddVarBool8("Debug Render", "gbRenderRoads", &gbRenderRoads, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderEverythingBarRoads", &gbRenderEverythingBarRoads, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderFadingInUnderwaterEntities", &gbRenderFadingInUnderwaterEntities, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderFadingInEntities", &gbRenderFadingInEntities, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderWater", &gbRenderWater, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderBoats", &gbRenderBoats, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderVehicles", &gbRenderVehicles, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderWorld0", &gbRenderWorld0, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderWorld1", &gbRenderWorld1, nil);
		DebugMenuAddVarBool8("Debug Render", "gbRenderWorld2", &gbRenderWorld2, nil);
#endif

#ifdef EXTENDED_COLOURFILTER
		static const char *filternames[] = { "None", "Simple", "Normal", "Mobile" };
		e = DebugMenuAddVar("Render", "Colourfilter", &CPostFX::EffectSwitch, nil, 1, CPostFX::POSTFX_OFF, CPostFX::POSTFX_MOBILE, filternames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVar("Render", "Intensity", &CPostFX::Intensity, nil, 0.05f, 0, 10.0f);
		DebugMenuAddVarBool8("Render", "Blur", &CPostFX::BlurOn, nil);
		DebugMenuAddVarBool8("Render", "Motion Blur", &CPostFX::MotionBlurOn, nil);
#endif
		DebugMenuAddVar("Render", "Drunkness", &CMBlur::Drunkness, nil, 0.05f, 0, 1.0f);
#ifndef MASTER
		DebugMenuAddVarBool8("Render", "Occlusion debug", &bDispayOccDebugStuff, nil);
#endif
#ifdef LIBRW
		DebugMenuAddVarBool32("Render", "MatFX env map apply light", &rw::MatFX::envMapApplyLight, nil);
		DebugMenuAddVarBool32("Render", "MatFX env map flip U", &rw::MatFX::envMapFlipU, nil);
		DebugMenuAddVarBool32("Render", "MatFX env map use matcolor", &rw::MatFX::envMapUseMatColor, nil);
#endif
#ifdef EXTENDED_PIPELINES
		static const char *vehpipenames[] = { "MatFX", "Neo" };
		e = DebugMenuAddVar("Render", "Vehicle Pipeline", &CustomPipes::VehiclePipeSwitch, nil,
			1, CustomPipes::VEHICLEPIPE_MATFX, CustomPipes::VEHICLEPIPE_NEO, vehpipenames);
		DebugMenuEntrySetWrap(e, true);
		DebugMenuAddVar("Render", "Neo Vehicle Shininess", &CustomPipes::VehicleShininess, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVar("Render", "Neo Vehicle Specularity", &CustomPipes::VehicleSpecularity, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo Ped Rim light enable", &CustomPipes::RimlightEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::RimlightMult, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo World Lightmaps enable", &CustomPipes::LightmapEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::LightmapMult, nil, 0.1f, 0, 1.0f);
		DebugMenuAddVarBool8("Render", "Neo Road Gloss enable", &CustomPipes::GlossEnable, nil);
		DebugMenuAddVar("Render", "Mult", &CustomPipes::GlossMult, nil, 0.1f, 0, 1.0f);
#endif
		DebugMenuAddVarBool8("Debug Render", "Show Ped Paths", &gbShowPedPaths, nil);
		DebugMenuAddVarBool8("Debug Render", "Show Car Paths", &gbShowCarPaths, nil);
		DebugMenuAddVarBool8("Debug Render", "Show Car Path Links", &gbShowCarPathsLinks, nil);
		DebugMenuAddVarBool8("Debug Render", "Show Collision Lines", &gbShowCollisionLines, nil);
		DebugMenuAddVarBool8("Debug Render", "Show Collision Polys", &gbShowCollisionPolys, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't render Buildings", &gbDontRenderBuildings, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't render Big Buildings", &gbDontRenderBigBuildings, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't render Peds", &gbDontRenderPeds, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't render Vehicles", &gbDontRenderVehicles, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't render Objects", &gbDontRenderObjects, nil);
		DebugMenuAddVarBool8("Debug Render", "Don't Render Water", &gbDontRenderWater, nil);
		
		
#ifdef DRAW_GAME_VERSION_TEXT
		DebugMenuAddVarBool8("Debug", "Version Text", &gbDrawVersionText, nil);
#endif
		DebugMenuAddVarBool8("Debug", "Show DebugStuffInRelease", &gbDebugStuffInRelease, nil);
#ifdef TIMEBARS
		DebugMenuAddVarBool8("Debug", "Show Timebars", &gbShowTimebars, nil);
#endif
#ifndef FINAL
		DebugMenuAddVarBool8("Debug", "Use debug render groups", &bDebugRenderGroups, nil);
		DebugMenuAddVarBool8("Debug", "Print Memory Usage", &gbPrintMemoryUsage, nil);
#ifdef USE_CUSTOM_ALLOCATOR
		DebugMenuAddCmd("Debug", "Parse Heap", ParseHeap);
#endif
#endif

		DebugMenuAddVarBool8("Debug", "pad 1 -> pad 2", &CPad::m_bMapPadOneToPadTwo, nil);
#ifdef GTA_SCENE_EDIT
		DebugMenuAddVarBool8("Debug", "Edit on", &CSceneEdit::m_bEditOn, nil);
#endif
		//DebugMenuAddCmd("Debug", "Start Credits", CCredits::Start);
		//DebugMenuAddCmd("Debug", "Stop Credits", CCredits::Stop);

#ifdef RELOADABLES
// maybe put it back if we have more to reload 
//		DebugMenuAddCmd("Reload", "HUD.TXD", CHud::ReloadTXD);
#endif

#ifdef MAP_ENHANCEMENTS
		DebugMenuAddCmd("Game", "Teleport to map waypoint", TeleportToWaypoint);
#endif
		DebugMenuAddCmd("Game", "Fix Car", FixCar);
		DebugMenuAddCmd("Game", "Place Car on Road", PlaceOnRoad);
		DebugMenuAddCmd("Game", "Switch car collision", SwitchCarCollision);
		DebugMenuAddCmd("Game", "Toggle Comedy Controls", ToggleComedy);


#ifdef MISSION_SWITCHER
		DebugMenuEntry *missionEntry;
		static const char* missions[] = {
			"Initial", "Intro", "An Old Friend", "The Party", "Back Alley Brawl", "Jury Fury", "Riot",
			"Treacherous Swine", "Mall Shootout", "Guardian Angels", "Sir, Yes Sir!", "All Hands On Deck!",
			"The Chase", "Phnom Penh '86", "The Fastest Boat", "Supply & Demand", "Rub Out", "Death Row",
			"Four Iron", "Demolition Man", "Two Bit Hit", "No Escape?", "The Shootist", "The Driver",
			"The Job", "Gun Runner", "Boomshine Saigon", "Recruitment Drive", "Dildo Dodo", "Martha's Mug Shot",
			"G-spotlight", "Shakedown", "Bar Brawl", "Cop Land", "Spilling the Beans", "Hit the Courier",
			"Printworks Buy", "Sunshine Autos", "Interglobal Films Buy", "Cherry Popper Icecreams Buy",
			"Kaufman Cabs Buy", "Malibu Club Buy", "The Boatyard Buy", "Pole Position Club Buy", "El Swanko Casa Buy",
			"Links View Apartment Buy", "Hyman Condo Buy", "Ocean Heighs Aprt. Buy", "1102 Washington Street Buy",
			"Vice Point Buy", "Skumole Shack Buy", "Cap the Collector", "Keep your Friends Close...",
			"Alloy Wheels of Steel", "Messing with the Man", "Hog Tied", "Stunt Boat Challenge", "Cannon Fodder",
			"Naval Engagement", "Trojan Voodoo", "Juju Scramble", "Bombs Away!", "Dirty Lickin's", "Love Juice",
			"Psycho Killer", "Publicity Tour", "Weapon Range", "Road Kill", "Waste the Wife", "Autocide",
			"Check Out at the Check In", "Loose Ends", "V.I.P.", "Friendly Rivalry", "Cabmaggedon", "TAXI DRIVER",
			"PARAMEDIC", "FIREFIGHTER", "VIGILANTE", "HOTRING", "BLOODRING", "DIRTRING", "Sunshine Autos Races",
			"Distribution", "Downtown Chopper Checkpoint", "Ocean Beach Chopper Checkpoint", "Vice Point Chopper Checkpoint",
			"Little Haiti Chopper Checkpoint", "Trial by Dirt", "Test Track", "PCJ Playground", "Cone Crazy",
			"PIZZA BOY", "RC Raider Pickup", "RC Bandit Race", "RC Baron Race", "Checkpoint Charlie"
		};

		missionEntry = DebugMenuAddVar("Game", "Select mission", &nextMissionToSwitch, nil, 1, 0, ARRAY_SIZE(missions) - 1, missions);
		DebugMenuEntrySetWrap(missionEntry, true);
		DebugMenuAddCmd("Game", "Start selected mission ", SwitchToMission);
#endif
		extern bool PrintDebugCode;
		extern int16 DebugCamMode;
		DebugMenuAddVarBool8("Cam", "Use mouse Cam", &CCamera::m_bUseMouse3rdPerson, nil);
#ifdef FREE_CAM
		DebugMenuAddVarBool8("Cam", "Free Cam", &CCamera::bFreeCam, nil);
#endif
		DebugMenuAddVarBool8("Cam", "Print Debug Code", &PrintDebugCode, nil);
		DebugMenuAddVar("Cam", "Cam Mode", &DebugCamMode, nil, 1, 0, CCam::MODE_EDITOR, nil);
		DebugMenuAddCmd("Cam", "Normal", []() { DebugCamMode = 0; });
		DebugMenuAddCmd("Cam", "Reset Statics", ResetCamStatics);

		CTweakVars::AddDBG("Debug");
	}
}
#endif

#ifndef __MWERKS__
#ifndef MASTER
const int   re3_buffsize = 1024;
static char re3_buff[re3_buffsize];
#endif

#ifndef MASTER
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func)
{
#ifdef _WIN32
	int nCode;

	strcpy_s(re3_buff, re3_buffsize, "Assertion failed!" );
	strcat_s(re3_buff, re3_buffsize, "\n" );	
	
	strcat_s(re3_buff, re3_buffsize, "File: ");
	strcat_s(re3_buff, re3_buffsize, filename );
	strcat_s(re3_buff, re3_buffsize, "\n" );	

	strcat_s(re3_buff, re3_buffsize, "Line: " );
	_itoa_s( lineno, re3_buff + strlen(re3_buff), re3_buffsize - strlen(re3_buff), 10 );
	strcat_s(re3_buff, re3_buffsize, "\n");
	
	strcat_s(re3_buff, re3_buffsize, "Function: ");
	strcat_s(re3_buff, re3_buffsize, func );
	strcat_s(re3_buff, re3_buffsize, "\n" );	
	
	strcat_s(re3_buff, re3_buffsize, "Expression: ");
	strcat_s(re3_buff, re3_buffsize, expr);
	strcat_s(re3_buff, re3_buffsize, "\n");

	strcat_s(re3_buff, re3_buffsize, "\n" );
	strcat_s(re3_buff, re3_buffsize, "(Press Retry to debug the application)");


	nCode = ::MessageBoxA(nil, re3_buff, "REVC Assertion Failed!",
		MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

	if (nCode == IDABORT)
	{
		raise(SIGABRT);
		_exit(3);
	}

	if (nCode == IDRETRY)
	{
		__debugbreak();
		return;
	}

	if (nCode == IDIGNORE)
		return;

	abort();
#else
	// TODO
	printf("\nREVC ASSERT FAILED\n\tFile: %s\n\tLine: %d\n\tFunction: %s\n\tExpression: %s\n",filename,lineno,func,expr);
	assert(false);
#endif
}
#endif

void re3_debug(const char *format, ...)
{
#ifndef MASTER
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
#else
	vsprintf(re3_buff, format, va);
#endif
	va_end(va);

	printf("%s", re3_buff);
	CDebug::DebugAddText(re3_buff);
#endif
}

#ifndef MASTER
void re3_trace(const char *filename, unsigned int lineno, const char *func, const char *format, ...)
{
	char buff[re3_buffsize *2];
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
	va_end(va);
	
	sprintf_s(buff, re3_buffsize * 2, "[%s.%s:%d]: %s", filename, func, lineno, re3_buff);
#else
	vsprintf(re3_buff, format, va);
	va_end(va);
	
	sprintf(buff, "[%s.%s:%d]: %s", filename, func, lineno, re3_buff);
#endif

	OutputDebugString(buff);
}
#endif

#ifndef MASTER
void re3_usererror(const char *format, ...)
{
	va_list va;
	va_start(va, format);
#ifdef _WIN32
	vsprintf_s(re3_buff, re3_buffsize, format, va);
	va_end(va);
	
	::MessageBoxA(nil, re3_buff, "REVC Error!",
		MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

	raise(SIGABRT);
	_exit(3);
#else
	vsprintf(re3_buff, format, va);
	printf("\nREVC Error!\n\t%s\n",re3_buff);
	assert(false);
#endif
}
#endif
#endif

#ifdef VALIDATE_SAVE_SIZE
int32 _saveBufCount;
#endif
