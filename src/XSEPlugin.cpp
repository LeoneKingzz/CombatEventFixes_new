#include "hook.h"

// constexpr auto kTrampolineSize = 14;

void MessageHandler(SKSE::MessagingInterface::Message *a_msg)
{
    switch (a_msg->type)
    {
    case SKSE::MessagingInterface::kDataLoaded:
        // hooks::animEventHandler::Register(false, true);
        hooks::CombatEventFixes::install();
        // hooks::InputEventHandler::SinkEventHandlers();
        break;

    case SKSE::MessagingInterface::kPostPostLoad:
        // hooks::CombatEventFixes::GetSingleton()->init();
        break;

    default:

        break;
    }
}

void Init()
{
    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", MessageHandler);
}

void Load([[maybe_unused]] bool VR)
{
    hooks::CombatEventFixes::install_protected();
}

void onSKSEInit()
{
    // const auto papyrus = SKSE::GetPapyrusInterface();
    // papyrus->Register(hooks::CombatEventFixes::BindPapyrusFunctions);
}

void PreLoad()
{
}