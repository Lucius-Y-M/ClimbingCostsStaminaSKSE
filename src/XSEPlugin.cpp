#pragma once

#include "../include/AliasSKSE.h"
#include "../include/Hook.h"


#define DLLEXPORT __declspec(dllexport)








void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	const auto level = a_level;

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}


		/* directly taken from the plugin example by Ershin */
		// template <typename T>
		// void RegisterCondition()
		// {
		// 	auto result = OAR_API::Conditions::AddCustomCondition<T>();
		// 	switch (result)
		// 	{
		// 	case OAR_API::Conditions::APIResult::OK:
		// 		logger::info("Registered {} condition!", T::CONDITION_NAME);
		// 		break;
		// 	case OAR_API::Conditions::APIResult::AlreadyRegistered:
		// 		logger::warn("Condition {} is already registered!",
		// 				T::CONDITION_NAME);
		// 		break;
		// 	case OAR_API::Conditions::APIResult::Invalid:
		// 		logger::error("Condition {} is invalid!", T::CONDITION_NAME);
		// 		break;
		// 	case OAR_API::Conditions::APIResult::Failed:
		// 		logger::error("Failed to register condition {}!", T::CONDITION_NAME);
		// 		break;
		// 	}
		// }

		// void InitMessaging()
		// {
		// 	logger::trace("Initializing messaging listener...");
		// 	const auto intfc = SKSE::GetMessagingInterface();
		// 	if (!intfc->RegisterListener([](SKSE::MessagingInterface::Message* a_msg)
		// 	{
		// 		if (a_msg->type == SKSE::MessagingInterface::kPostLoad)
		// 		{
		// 			OAR_API::Conditions::GetAPI(OAR_API::Conditions::InterfaceVersion::V2);
		// 			if (g_oarConditionsInterface)
		// 			{
		// 				RegisterCondition<Conditions::ExampleCondition>();
		// 				RegisterCondition<Conditions::CompareGraphVariableNiPoint3DotCondition>();
		// 			}
		// 			else
		// 			{
		// 				logger::error("Failed to request Open Animation Replacer API"sv);
		// 			}
		// 		}
		// 	}))
		// 	{
		// 		stl::report_and_fail("Failed to initialize message listener.");
		// 	}
		// }


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	SKSE::Init(a_skse);
	// InitMessaging();

	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());



		SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message * msg) {

			if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
				if (Hook::Install()) {
					logger::info("-- DLL installation success.");
				}
				else {
					logger::error("!! DLL initialization failed, please check log for what happened, and report to mod author if necessary.");
				}
			}
		
		});



	return true;
}





extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}