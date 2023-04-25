#include "../Hooks.h"

//i stole this from mfed :)

static int PlayerIndex;
static uint32 CurrentFriendsID;

MAKE_HOOK(CVoiceStatus_IsPlayerBlocked, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 8D 95 ? ? ? ? 56 8B F1 8B 0D ? ? ? ? 52 FF 75 ? 8B 01 8B 40 ? FF D0 84 C0 75 ? 5E"), bool, __fastcall,
    void* ecx, void* edx, int playerIndex)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, playerIndex); }

    auto retaddr = DWORD(_ReturnAddress());

    static auto CVoiceStatus_IsPlayerBlocked_Call = g_Pattern.Find(L"client.dll", L"84 C0 BA ? ? ? ? 68");
    if (retaddr == CVoiceStatus_IsPlayerBlocked_Call)
    {
        PlayerIndex = playerIndex;
    }

    return Hook.Original<FN>()(ecx, edx, playerIndex);
}

MAKE_HOOK(VGui_MenuBuilder_AddMenuItem, g_Pattern.Find(L"client.dll", L"55 8B EC 56 8B F1 57 8B 7D ? 8B 46 ? 85 C0 74 ? 50 57 E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 8B 0E 8B 01 FF 90 ? ? ? ? 8B 0E 6A ? FF 76 ? 89 7E ? FF 75 ? 8B 01 FF 75 ? FF 90 ? ? ? ? 8B 36 8B C8 3B 8E ? ? ? ? 73 ? 3B 8E ? ? ? ? 7F ? 8B 86 ? ? ? ? 8D 14 49 39 4C 90 ? 75 ? 39 4C 90 ? 75 ? 8B 04 90 5F 5E 5D C2 ? ? 5F 33 C0 5E 5D C2 ? ? CC CC CC CC CC 55 8B EC 56"), void*, __fastcall,
    void* ecx, void* edx, const char* pszButtonText, const char* pszCommand, const char* pszCategoryName)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName); }

    auto retaddr = DWORD(_ReturnAddress());

    static auto VGui_MenuBuilder_AddMenuItem_Call = g_Pattern.Find(L"client.dll", L"8B 0D ? ? ? ? 8D 55 ? 52 8D 55 ? 52 8B 01 FF 50 ? FF 75 ? 8B 8E ? ? ? ? FF 75 ? E8");
    if (retaddr == VGui_MenuBuilder_AddMenuItem_Call && PlayerIndex != -1)
    {
        auto ret = Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName);

        PlayerInfo_t pi;

        if (I::EngineClient->GetPlayerInfo(PlayerIndex, &pi))
        {
            CurrentFriendsID = pi.friendsID;

            bool ignored = G::PlayerPriority[pi.friendsID].Mode == 1;
            bool marked = G::PlayerPriority[pi.friendsID].Mode == 4;

            Hook.Original<FN>()(ecx, edx, ignored ? "Unignore" : "Ignore", "fedignore", "fed");
            Hook.Original<FN>()(ecx, edx, marked ? "Unmark" : "Mark as Cheater", "fedmark", "fed");
        }

        return ret;
    }

    return Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName);
}

MAKE_HOOK(CTFClientScoreBoardDialog_OnCommand, g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC ? 56 57 8B 7D ? 8B F1 68 ? ? ? ? 57 E8 ? ? ? ? 83 C4 ? 85 C0 74"), void, __fastcall,
    void* ecx, void* edx, const char* command)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, command); return; }

    if (FNV1A::Hash(command) == FNV1A::HashConst("fedignore"))
    {
        if (CurrentFriendsID)
        {
            G::SwitchIgnore(CurrentFriendsID);
        }
    }

    if (FNV1A::Hash(command) == FNV1A::HashConst("fedmark"))
    {
        if (CurrentFriendsID)
        {
            G::SwitchMark(CurrentFriendsID);
        }
    }

    Hook.Original<FN>()(ecx, edx, command);
}