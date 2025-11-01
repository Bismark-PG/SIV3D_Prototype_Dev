/*==============================================================================

	Manager For Game Resource [Resource_Texture.cpp]

	Author : Team Re:ing >> name

	Note :

==============================================================================*/
#include "stdafx.h" 

void Init_Audio()
{
	AudioAsset::Register(U"Chase", U"Assets/Sound/Chaser.wav");
	AudioAsset::Register(U"Ending_Bad", U"Assets/Sound/Ending_Bad.wav");
	AudioAsset::Register(U"Ending_Good", U"Assets/Sound/Ending_Good.wav");
	AudioAsset::Register(U"Explorer", U"Assets/Sound/Explorer.mp3");
	AudioAsset::Register(U"Final_Battle", U"Assets/Sound/Final_Boss.mp3");
	AudioAsset::Register(U"Battle", U"Assets/Sound/Normal_Battle.wav");
	AudioAsset::Register(U"Scary", U"Assets/Sound/Scary.wav");

	AudioAsset::Load(U"Chase");
	AudioAsset::Load(U"Ending_Bad");
	AudioAsset::Load(U"Ending_Good");
	AudioAsset::Load(U"Explorer");
	AudioAsset::Load(U"Final_Battle");
	AudioAsset::Load(U"Battle");
	AudioAsset::Load(U"Scary");
}
