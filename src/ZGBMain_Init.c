#pragma bank 1
#include "ZGBMain.h"
UINT8 init_bank = 1;


#include "StateGame.h"

#include "SpritePlayer.h"
#include "SpriteEnemy.h"
#include "SpriteCrosshair.h"
#include "SpriteBall.h"
#include "SpriteBar.h"
#include "SpriteHole.h"

#include "../res/src/player.h"
#include "../res/src/enemy.h"
#include "../res/src/crosshair.h"
#include "../res/src/ball.h"
#include "../res/src/bar.h"
#include "../res/src/hole.h"



UINT8 next_state = STATE_GAME;

SET_N_STATES(N_STATES);
SET_N_SPRITE_TYPES(N_SPRITE_TYPES);

void InitStates() {
	INIT_STATE(STATE_GAME);
}

void InitSprites() {
	INIT_SPRITE(SPRITE_PLAYER, player, 3, FRAME_16x16, 2);
	INIT_SPRITE(SPRITE_ENEMY, enemy, 3, FRAME_16x16, 2);
	INIT_SPRITE(SPRITE_CROSSHAIR, crosshair, 3, FRAME_16x16, 2);
	INIT_SPRITE(SPRITE_BALL, ball, 3, FRAME_16x16, 2);
	INIT_SPRITE(SPRITE_BAR, bar, 3, FRAME_8x16, 4);
	INIT_SPRITE(SPRITE_HOLE, hole, 3, FRAME_16x16, 1);

}
