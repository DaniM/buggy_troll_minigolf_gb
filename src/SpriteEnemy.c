#pragma bank 2
#include "SpriteEnemy.h"
#include "SpriteManager.h"

UINT8 bank_SPRITE_ENEMY = 2;


struct EnemyInfo
{
	INT8 vy;
};

typedef struct EnemyInfo EnemyInfo;

void Start_SPRITE_ENEMY()
{
	EnemyInfo* data = (EnemyInfo*)THIS->custom_data;
	data->vy = 1;
}

void Update_SPRITE_ENEMY()
{
	EnemyInfo* data = (EnemyInfo*)THIS->custom_data;
	if( TranslateSprite( THIS, 0, data->vy ) )
	{
		data->vy = -data->vy;
	}
}

void Destroy_SPRITE_ENEMY()
{}
