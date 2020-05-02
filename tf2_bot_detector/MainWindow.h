#pragma once

#include "Actions.h"
#include "ActionManager.h"
#include "Clock.h"
#include "PlayerList.h"
#include "LobbyMember.h"
#include "PlayerStatus.h"
#include "TFConstants.h"
#include "IConsoleLineListener.h"

#include <imgui_desktop/Window.h>

#include <chrono>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace tf2_bot_detector
{
	class IConsoleLine;
	class IConsoleLineListener;

	struct PlayerScores
	{
		uint16_t m_Kills = 0;
		uint16_t m_Deaths = 0;
	};

	enum class PlayerMarkType
	{
		Cheater,
		Suspicious,
		Exploiter,
	};

	class MainWindow final : public ImGuiDesktop::Window, IConsoleLineListener
	{
	public:
		MainWindow();
		~MainWindow();

		void AddConsoleLineListener(IConsoleLineListener* listener);
		bool RemoveConsoleLineListener(IConsoleLineListener* listener);

	private:
		void OnDraw() override;
		void OnDrawScoreboard();
		void OnDrawChat();
		void OnDrawAppLog();

		void OnUpdate() override;
		size_t m_ParsedLineCount = 0;

		bool IsTimeEven() const;
		float TimeSine(float interval = 1.0f, float min = 0, float max = 1) const;

		void OnConsoleLineParsed(IConsoleLine& line);

		struct CustomDeleters
		{
			void operator()(FILE*) const;
		};
		std::unique_ptr<FILE, CustomDeleters> m_File;
		std::string m_FileLineBuf;
		std::optional<time_point_t> m_CurrentTimestamp;
		std::vector<std::unique_ptr<IConsoleLine>> m_ConsoleLines;

		size_t m_PrintingLineCount = 0;
		IConsoleLine* m_PrintingLines[512]{};
		void UpdatePrintingLines();

		void LogAction(std::string msg);
		std::vector<std::string> m_LogMessages;

		struct PlayerPrintData final
		{
			SteamID m_SteamID;
			std::string m_Name;
			PlayerScores m_Scores;
			uint16_t m_UserID;
			uint16_t m_Ping;
			TFTeam m_Team;
		};
		size_t GeneratePlayerPrintData(PlayerPrintData* begin, PlayerPrintData* end) const;

		std::optional<SteamID> FindSteamIDForName(const std::string_view& playerName) const;
		std::optional<LobbyMemberTeam> FindLobbyMemberTeam(const SteamID& id) const;
		std::optional<uint16_t> FindUserID(const SteamID& id) const;

		struct PlayerExtraData
		{
			PlayerStatus m_Status{};
			PlayerScores m_Scores{};
			TFTeam m_Team{};
			uint8_t m_ClientIndex{};
		};

		struct DelayedChatBan
		{
			time_point_t m_Timestamp;
			std::string m_PlayerName;
		};
		std::vector<DelayedChatBan> m_DelayedBans;
		void ProcessDelayedBans(time_point_t timestamp, const PlayerStatus& updatedStatus);

		std::vector<LobbyMember> m_CurrentLobbyMembers;
		std::vector<LobbyMember> m_PendingLobbyMembers;
		std::unordered_map<SteamID, PlayerExtraData> m_CurrentPlayerData;
		time_point_t m_OpenTime;
		PlayerList m_CheaterList;
		PlayerList m_SuspiciousList;
		PlayerList m_ExploiterList;

		bool MarkPlayer(const SteamID& id, PlayerMarkType markType);
		bool IsPlayerMarked(const SteamID& id, PlayerMarkType markType);

		void InitiateVotekick(const SteamID& id, KickReason reason);

		ActionManager m_ActionManager;
		std::set<IConsoleLineListener*> m_ConsoleLineListeners;
	};
}