#include "../Hooks.h"

int ColorToInt(const Color_t& col)
{
    return col.r << 16 | col.g << 8 | col.b;
}

int feetcolor()
{
    if (Vars::Visuals::Particles::Feet::ColorType.Value == 0)
    {
        return ColorToInt(Colors::FeetColor);
    }
    else if (Vars::Visuals::Particles::Feet::ColorType.Value == 1)
    {
        return 2;
    }
    else if (Vars::Visuals::Particles::Feet::ColorType.Value == 2)
    {
        return 1;
    }
    else if (Vars::Visuals::Particles::Feet::ColorType.Value == 3)
    {
        return ColorToInt(Utils::Rainbow());
    }
    else return 1;
}

//Credits to mfed
MAKE_HOOK(C_TFPlayer_FireEvent, g_Pattern.Find(L"client.dll", L"55 8B EC 8B 45 ? 81 EC ? ? ? ? 53 56 57 8B F9 3D"), void, __fastcall,
    CBaseEntity* ecx, void* edx, const Vector& origin, const QAngle& angles, int event_, const char* options)
{
    static auto SpawnHalloweenSpellFootsteps = reinterpret_cast<void* (__thiscall*)(CBaseEntity*, ParticleAttachment_t, int)>(g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? 83 EC ? 53 8B 5D"));

    if (!Vars::Visuals::Particles::Feet::Enabled.Value || event_ != 7001 || ecx != g_EntityCache.GetLocal())
    {
        return Hook.Original<FN>()(ecx, edx, origin, angles, event_, options);
    }

    if (Vars::Visuals::Particles::Feet::DashOnly.Value && G::Teleporting == true) //this is better than just showing when you have the key down, i guess
    {
        SpawnHalloweenSpellFootsteps(g_EntityCache.GetLocal(), PATTACH_ABSORIGIN, feetcolor());
    }
    else {
        SpawnHalloweenSpellFootsteps(g_EntityCache.GetLocal(), PATTACH_ABSORIGIN, feetcolor());
    }

    return Hook.Original<FN>()(ecx, edx, origin, angles, event_, options);
}