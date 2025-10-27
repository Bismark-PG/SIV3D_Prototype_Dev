/*==============================================================================

	Manager For Game Resource [Resource_Texture.cpp]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#include "stdafx.h" 

void Init_Texture()
{
	TextureAsset::Register(U"Sample_Player", U"Resource/Sample_Player.png");
	TextureAsset::Register(U"Sample_Enemy", U"Resource/Sample_Enemy.png");
	TextureAsset::Register(U"Sample_Map", U"Resource/Sample_Map.png");
	TextureAsset::Register(U"Sample_Chara", U"Resource/Sample_Chara.png");

	TextureAsset::Register(U"Map_Enemy_Slime", U"Resource/Sample_Enemy.png"); // Debug : use Sample_Enemy
}
