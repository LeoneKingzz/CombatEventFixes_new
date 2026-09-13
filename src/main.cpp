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

void Init() {
    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", MessageHandler);
}

void Load(bool VR) {
    hooks::CombatEventFixes::install_protected();
}

void onSKSEInit() {
    // const auto papyrus = SKSE::GetPapyrusInterface();
    // papyrus->Register(hooks::CombatEventFixes::BindPapyrusFunctions);
}

void PreLoad() {
    
}

void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto       sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

SKSEPluginInfo(
    .Version = Plugin::VERSION,
    .Name = Plugin::NAME.data(),
    .Author = "LeoneKingzz",
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {

    InitializeLog();
    logger::info("Loaded plugin");
    
    SKSE::Init(a_skse, false);
    // SKSE::AllocTrampoline(kTrampolineSize);
    PreLoad();
    onSKSEInit();
    Init();
    const auto ver = a_skse->RuntimeVersion();
    if (ver == SKSE::RUNTIME_LATEST_VR) {
        Load(true);
    } else {
        Load(false);
    }

    return true;
}
