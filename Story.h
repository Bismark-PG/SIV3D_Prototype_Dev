/*==============================================================================

	Manage Story [Story.h]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#pragma once
#include <Siv3D.hpp>
#include "Player.h"

enum class StoryID
{
	Opening1,
	Opening2,
	Opening3,
	Tutorial_Name,
	Tutorial_Battle,
	Tutorial_Introduce,
	Tutorial_End,
	Day2_Start,
	FinalBattle_Start,
	Ending_Clear,
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

class Story_Manager
{
public:
	Story_Manager(PlayerStats& playerStats);

	void StartStory(StoryID id);
	bool Update();
	void Draw() const;
	bool IsActive() const;

	StoryID GetLastStoryID() const;

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
	Circle m_npcSilhouette;

	RectF m_nameInputModal;
	RectF m_nameInputTextBox;

	// --- Special Speaker Names ---
	const String m_narrationSpeaker = U"ナレーション";
	const String m_protagonistSpeaker = U"Player";
	const String m_scriptProtagonist = U"主人公";

	const String m_inputPartnerTag = U"INPUT_PARTNER_NAME";
	const String m_inputPlayerTag = U"INPUT_PLAYER_NAME";

	// --- State ---
	bool m_isActive = false;
	Array<ScriptLine> m_currentScript;
	size_t m_currentLineIndex = 0;
	StoryID m_lastStoryID = StoryID::NONE;

	StoryState m_state = StoryState::Talking;

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
};
