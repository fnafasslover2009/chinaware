#include "../../SDK.h"

class WeaponESP
{
public:
	const char* GetWeaponName(int weaponID, int itemIndex, EWeaponType weaponType);
};

ADD_FEATURE(WeaponESP, WESP)