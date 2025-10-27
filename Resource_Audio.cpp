/*==============================================================================

	Manager For Game Resource [Resource_Texture.cpp]

	Author : Team Re:ing >> name

	Note :

==============================================================================*/
#include "stdafx.h" 

void Init_Audio()
{
	AudioAsset::Register(U"Sample_Main", U"Resource/Main.mp3");
	AudioAsset::Register(U"Sample_Battle", U"Resource/Battle.mp3");
	AudioAsset::Register(U"Sample_Talk", U"Resource/Talk.mp3");
	AudioAsset::Register(U"Sample_Ending", U"Resource/Ending.mp3");

	AudioAsset::Load(U"Sample_Main");
	AudioAsset::Load(U"Sample_Battle");
	AudioAsset::Load(U"Sample_Talk");
	AudioAsset::Load(U"Sample_Ending");
}
