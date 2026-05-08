/*==============================================================================

	Manager For Game Resource [Resource_Texture.cpp]

	Author : Team Re:ing >> name

	Note :

==============================================================================*/
#include "stdafx.h" 

void Init_Audio()
{
	// --- BGM --- //
	AudioAsset::Register(U"Chase",					U"../Assets/Sound/Chaser.wav");
	AudioAsset::Register(U"Ending_Bad",				U"../Assets/Sound/Ending_Bad.wav");
	AudioAsset::Register(U"Ending_Good",			U"../Assets/Sound/Ending_Good.wav");
	AudioAsset::Register(U"Explorer",				U"../Assets/Sound/Explorer.mp3");
	AudioAsset::Register(U"Final_Battle",			U"../Assets/Sound/Final_Boss.mp3");
	AudioAsset::Register(U"Battle",					U"../Assets/Sound/Normal_Battle.wav");	
	AudioAsset::Register(U"Scary",					U"../Assets/Sound/Scary.wav");
	AudioAsset::Register(U"禁止区",					U"../Assets/Sound/禁止区.mp3");
	AudioAsset::Register(U"君が残したメロディー",	U"../Assets/Sound/君が残したメロディー.mp3");
	AudioAsset::Register(U"歪む霊域",				U"../Assets/Sound/歪む霊域.mp3");

	// --- SFX --- //
}
