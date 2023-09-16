#include "stdafx.h"
#include "RGMain.h"
#include "NewChat.h"
#include "ZConfiguration.h"
#include "ZRule.h"
#include "VoiceChat.h"
#include "FileInfo.h"
#include "ZInput.h"
#include <cstdint>
#include "dxerr.h"
#include "defer.h"
#include <string>

static optional<RGMain> g_RGMain;

RGMain& GetRGMain() { return g_RGMain.value(); }
void CreateRGMain() { g_RGMain.emplace(); }
void DestroyRGMain() { g_RGMain.reset(); }

void RGMain::OnAppCreate(){}
void RGMain::OnReset() {}
void RGMain::OnDrawLobby(MDrawContext* pDC) {}
void RGMain::OnGameCreate() {}
void RGMain::OnDrawGame() {}

void RGMain::OnCreateDevice()
{
	auto&& Cfg = *ZGetConfiguration();
	auto&& c = *Cfg.GetChat();
	m_Chat.emplace(c.Font, c.BoldFont, c.FontSize);
	GetChat().SetBackgroundColor(c.BackgroundColor);
#ifdef _VOICE_CHAT
	m_VoiceChat.OnCreateDevice();
#endif
}

void RGMain::OnDrawGameInterface(MDrawContext* pDC)
{
		GetChat().OnDraw(pDC);
#ifdef _VOICE_CHAT
		m_VoiceChat.OnDraw(pDC);
#endif
}

bool RGMain::OnGameInput()
{
	return GetChat().IsInputEnabled();
}

void RGMain::Resize(int w, int h)
{
	GetChat().Resize(w, h);
}

std::pair<PlayerFoundStatus, ZCharacter*> FindSinglePlayer(const char * NameSubstring)
{
	bool Found = false;
	ZCharacter* FoundChar = nullptr;

	for (auto Item : *ZGetCharacterManager())
	{
		auto Char = Item.second;

		if (!strstr(Char->GetUserName(), NameSubstring))
			continue;

		if (Found)
			return{ PlayerFoundStatus::TooManyFound, nullptr };

		Found = true;
		FoundChar = Char;
	}

	if (Found)
		return{ PlayerFoundStatus::FoundOne, FoundChar };

	return{ PlayerFoundStatus::NotFound, nullptr };
}

RGMain::RGMain() = default;
RGMain::~RGMain() {
	//
}

void RGMain::OnUpdate(double Elapsed)
{
	LastTime = Time;

	Time += Elapsed;

	{
		std::lock_guard<std::mutex> lock(QueueMutex);

		for (auto it = QueuedInvokations.begin(); it != QueuedInvokations.end(); it++)
		{
			auto item = *it;

			if (Time < item.Time)
				continue;

			item.fn();

			it = QueuedInvokations.erase(it);

			if (it == QueuedInvokations.end())
				break;
		}
	}

	GetChat().OnUpdate(Elapsed);
}

bool RGMain::OnEvent(MEvent *pEvent)
{
	GetChat().OnEvent(pEvent);

	bool IsChatVisible;
#ifdef _NEW_CHAT
	IsChatVisible = GetChat().IsInputEnabled();
#else
	IsChatVisible = ZGetCombatInterface()->IsChat();
#endif

	bool ret = false;

#ifdef _VOICE_CHAT
	static bool LastState = false;
	bool CurState = ZIsActionKeyPressed(ZACTION_VOICE_CHAT);

	ret = [&]
	{
		if (CurState && !LastState)
		{
			m_VoiceChat.StartRecording();

			return true;
		}
		else if (!CurState && LastState)
		{
			m_VoiceChat.StopRecording();

			return true;
		}

		return false;
	}();

	LastState = CurState;
#endif
	return IsChatVisible;

}

bool RGMain::IsCursorEnabled() const
{
	bool bEnabled = false;
	auto&& Cfg = *ZGetConfiguration();

	if (Cfg.GetChat()->EnableCursor && GetChat().IsInputEnabled())
		bEnabled = true;

	return bEnabled;
}

void RGMain::OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length)
{
#ifdef _VOICE_CHAT
	m_VoiceChat.OnReceiveVoiceChat(Char, Buffer, Length);
#endif
}