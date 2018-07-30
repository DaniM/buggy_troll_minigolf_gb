#pragma bank 2
#include "StateGame.h"
UINT8 bank_STATE_GAME = 2;

#include "../res/src/tiles.h"
#include "../res/src/font.h"
#include "../res/src/map.h"
#include "../res/src/window.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Print.h"
#include "string.h"

extern UINT8 n_sprite_types;

UINT8 collision_tiles[] = {1,26,0};

void Start_STATE_GAME() {
	UINT8 i;

	INIT_CONSOLE(font, 3, 2);

	SPRITES_8x16;
	for(i = 0; i != n_sprite_types; ++ i) {
		SpriteManagerLoad(i);
	}
	SHOW_SPRITES;

	scroll_target = SpriteManagerAdd(SPRITE_PLAYER, 50, 50);

	print_target = PRINT_WIN;\
	print_x = 0;\
	print_y = 0;\
	font_idx = 255 - 45;\
	InitScrollTiles(255 - 45, 45, font, 3);\
	WX_REG = 7;\
	WY_REG = (144 - (2 << 2));\
	SHOW_WIN;
	InitWindow(0, 0, 20, 3, window, 3, 0);
	PRINT_POS(1, 0);
	Printf("Power: ");

	InitScrollTiles(0, 68, tiles, 3);
	InitScroll(mapWidth, mapHeight, map, collision_tiles, 0, 3);
	//SpriteManagerAdd(SPRITE_HOLE, 70, 50 );

	SHOW_BKG;
}

void Update_STATE_GAME() {
}
