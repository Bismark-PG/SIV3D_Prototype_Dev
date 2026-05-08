/*==============================================================================

	Manage Story [Story.h]

	Author : Team Re:ing >> Bismark, Rin

	Note :

==============================================================================*/
#pragma once
#include <Siv3D.hpp>
#include "Player.h"

enum class StoryID
{
	Story_For_Skip,
	Skip_Input_Player_Name,
	Skip_Input_Partner_Name,

	Opening1,
	Opening2,
	Opening3,

	Tutorial_Name,
	Tutorial_Battle,
	Tutorial_Introduce,
	Tutorial_End,
	Day1_End,

	Day2_Start,

	FinalBattle_Start,
	Ending_Bad,
	Ending_Good,
	NONE
};

struct ScriptLine
{
	String speaker;
	String text;
};

enum class StoryState
{
	Talking,
	NamingPartner,
	NamingPlayer
};

enum class StoryRenderType
{
	Standalone,
	Overlay    
};

class Story_Manager
{
public:
	Story_Manager(PlayerStats& playerStats);

	void StartStory(StoryID id);
	bool Update();
	void Draw() const;
	bool IsActive() const;

	StoryID GetLastStoryID() const;

	StoryRenderType GetRenderType() const;
	String GetCurrentBGM() const;

private:
	// --- Core Data ---
	PlayerStats& m_playerStats;
	String m_playerName = U"主人公";

	// --- Fonts ---
	Font m_font;
	Font m_speakerFont;
	Font m_promptFont;
	Font m_nameInputFont;

	// --- UI Assets ---
	Texture m_protagonistTexture;

	// --- UI Rects & Positions ---
	RectF m_textBox;
	RectF m_speakerBoxBase;
	Vec2 m_protagonistPos;

	Vec2 m_aibo_Pos;
	Optional<BattleAnim> m_aibo_Anime;

	// --- Special Speaker Names ---
	const String m_narrationSpeaker = U"ナレーション";
	const String m_protagonistSpeaker = U"Player";
	const String m_scriptProtagonist = U"主人公";

	const String m_inputPartnerTag = U"INPUT_PARTNER_NAME";
	const String m_inputPlayerTag = U"INPUT_PLAYER_NAME";
	const String m_inputForSkip = U"INPUT_NAME";

	const String m_mob1 = U"女学生１";
	const String m_mob2 = U"女学生２";
	const String m_mob3 = U"マスク女";
	const String m_mob4 = U"口裂け女";

	// --- State ---
	bool m_isActive = false;
	Array<ScriptLine> m_currentScript;
	size_t m_currentLineIndex = 0;
	StoryID m_lastStoryID = StoryID::NONE;
	StoryState m_state = StoryState::Talking;

	StoryRenderType m_currentRenderType = StoryRenderType::Overlay;
	Texture m_background;
	String m_currentBGM;

	bool m_showNameWarning = false;

	// --- RPG-Style Name Input ---
	String m_nameInputBuffer;
	Point m_gridCursor;      
	RectF m_nameDisplayBox;  
	RectF m_charGridBox;     
	Font m_nameBufferFont;

	static const Array<Array<String>> m_charGrid;
	static const int m_gridCols = 10;
	static const int m_gridRows = 10;
	static const int m_maxNameLength = 8;

	void DrawNamingUI(const String& titleText) const;

	void DrawDuplicateNameWarning() const;

	Point FindNextValidCursorPos(Point currentPos, int dx, int dy);
};
