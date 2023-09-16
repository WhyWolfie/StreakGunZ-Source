#include "stdafx.h"
#include "ZConfiguration.h"
#include "NewChat.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include "Config.h"
#include <cstdint>
#include "ZOptionInterface.h"
#include "ZTestGame.h"


struct BoolResult
{
	bool Success;
	bool Value;
};
static BoolResult ParseBool(int argc, char** argv)
{
	auto Argument = argv[1];
	if (!_stricmp(Argument, "true") || !strcmp(Argument, "1")) {
		return{ true, true };
	}
	else if (!_stricmp(Argument, "false") || !strcmp(Argument, "0")) {
		return{ true, false };
	}

	return{ false, false };
}

// Sets the value of a bool according to the given arguments, and outputs feedback to user in chat.
//
// If no arguments are given, the value is toggled.
// If one argument is given, it's parsed by ParseBool: 
// "true" or "1" sets the bool to true,
// and "false" or "0" sets it to false.
//
// Returns false on invalid arguments. No change to the variable is applied in this case.
static bool SetBool(const char* Name, bool& Value, int argc, char** argv)
{
	if (argc < 1 || argc > 2) {
		assert(false);
		return false;
	}

	if (argc > 1)
	{
		// We got an argument. Parse it into a bool.
		auto ret = ParseBool(argc, argv);
		if (!ret.Success)
		{
			ZChatOutputF("%s is not a valid bool argument.", argv[1]);
			return false;
		}

		Value = ret.Value;
	}
	else
	{
		// No arguments. Toggle the value.
		Value = !Value;
	}

	ZChatOutputF("%s %s.", Name, Value ? "enabled" : "disabled");
	return true;
}


void LoadRGCommands(ZChatCmdManager& CmdManager)
{

#ifdef _VOICE_CHAT
	CmdManager.AddCommand(0, "mute", [](const char* line, int argc, char** const argv) {
		auto ret = FindSinglePlayer(argv[1]);

		if (!ret.second)
		{
			switch (ret.first)
			{
			case PlayerFoundStatus::NotFound:
				ZChatOutputF("No player with %s in their name was found", argv[1]);
				break;

			case PlayerFoundStatus::TooManyFound:
				ZChatOutputF("Too many players with %s in their name was found", argv[1]);
				break;

			default:
				ZChatOutputF("Unknown error %d", static_cast<int>(ret.first));
			};

			return;
		}

		bool b = GetRGMain().MutePlayer(ret.second->GetUID());

		ZChatOutputF("%s has been %s", ret.second->GetUserName(), b ? "muted" : "unmuted");
	}, CCF_ALL, 1, 1, true, "/mute <nameuser>", "");
#endif
}