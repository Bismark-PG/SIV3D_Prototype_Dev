/*==============================================================================

	Manager For Game Resource [Resource_Texture.cpp]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#include "stdafx.h" 

void Init_Texture()
{
	TextureAsset::Register(U"Battle_Back1", U"Assets/background/battle_back1.png");
	TextureAsset::Register(U"Battle_Back2", U"Assets/background/battle_back2.png");
	TextureAsset::Register(U"Battle_Back3", U"Assets/background/battle_back3.png");
	TextureAsset::Register(U"Grow_Back1", U"Assets/background/graw_back1.png");
	TextureAsset::Register(U"Grow_Back2", U"Assets/background/graw_back2.png");
	TextureAsset::Register(U"Grow_Back3", U"Assets/background/graw_back3.png");

	TextureAsset::Register(U"Player", U"Assets/Me.png");
}
