#pragma once

class CEntity;
class CVehicle;

enum {
	PED_LIGHTS_WALK,
	PED_LIGHTS_WALK_BLINK,
	PED_LIGHTS_DONT_WALK,

	CAR_LIGHTS_GREEN = 0,
	CAR_LIGHTS_YELLOW,
	CAR_LIGHTS_RED,
	CAR_LIGHTS_NONE
};

class CTrafficLights
{
public:
	static bool bGreenLightsCheat;

	static void DisplayActualLight(CEntity *ent);
	static void ScanForLightsOnMap(void);
	static int FindTrafficLightType(CEntity *light);
	static uint8 LightForPeds(void);
	static uint8 LightForCars1(void);
	static uint8 LightForCars2(void);
	static uint8 LightForCars1_Visual(void);
	static uint8 LightForCars2_Visual(void);
	static bool ShouldCarStopForLight(CVehicle*, bool);
	static bool ShouldCarStopForBridge(CVehicle*);
};
