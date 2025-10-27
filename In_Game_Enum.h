/*==============================================================================

	Header For Enum Class [In_Game_Enum.h]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#ifndef IN_GAME_ENUM_H
#define IN_GAME_ENUM_H

enum class GameScene
{
	Title,
	Gameplay,
	Ending
};

enum class GameplaySequence
{
	Exploration,
	Battle,
	Grow,
	Dialogue,
	FinalBattle
};

enum class Day : int
{
	Day1 = 1,
	Day2 = 2,
	Day3 = 3,
	Day4 = 4,
	Day5 = 5,
	Day6 = 6,
	Day7 = 7,
	Final = 8
};


#endif // IN_GAME_ENUM_H
