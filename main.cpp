#include "logger.h"
#include "FormLoader.h"
#include "Hooks.h"
#include "Events.h"
#include "Serialization.h"
#include "Settings.h"
#include "PapyrusAPI.h"

void InitListener(SKSE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kNewGame:
		Serialization::LoadChecks();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		Serialization::LoadChecks();
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		FormLoader::GetSingleton()->LoadAllForms();
		Settings::LoadSettings();
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
	SKSE::Init(skse);
	SetupLog();
	logger::info("loading SMI");

	FormLoader::GetSingleton()->CacheGameAddresses();
	SKSE::AllocTrampoline(42);
	Hooks::Install();
	Events::Register();

	auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener(InitListener)) {
		return false;
	}

	if (auto serialization = SKSE::GetSerializationInterface()) {
		serialization->SetUniqueID(Serialization::ID);
		serialization->SetSaveCallback(&Serialization::SaveCallback);
		serialization->SetLoadCallback(&Serialization::LoadCallback);
		serialization->SetRevertCallback(&Serialization::RevertCallback);
	}

	if (SKSE::GetPapyrusInterface()->Register(PapyrusAPI::Register)) {
		logger::info("Papyrus functions bound.");
	}
	else {
		SKSE::stl::report_and_fail("SMI-SKSE: Failure to register Papyrus bindings.");
	}

	return true;
}