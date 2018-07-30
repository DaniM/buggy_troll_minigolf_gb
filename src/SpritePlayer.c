#pragma bank 2
#include "SpritePlayer.h"
#include "Keys.h"
#include "SpriteManager.h"
#include "ZGBMain.h"
#include "Trig.h"
#include "Scroll.h"
#include "Print.h"

/// math stuff

#define ABS( A ) ( A & 0x8000 ) ? -A : A
#define MAX( A, B ) ( ( (A) >= (B) ) ? (A) : (B) )
#define SIGN( A ) (  A & 0x8000  ) ? -1 : 1 


struct div
{
	INT8 div;
	INT8 rem;
};

typedef struct div DIV;

INT8 divide( INT8 a, INT8 b, DIV* result )
{
	INT8 abs_a = ABS( a );
	INT8 abs_b = ABS( b );

	switch( abs_b )
	{
		case 0:
		return -1;

		case 1:
		{
			result->div = abs_a;
			result->rem = 0;
			return 0;
		}
		default:
		result->div = 0;
		result->rem = 0;
		while( abs_a >= abs_b )
		{
			abs_a -= abs_b;
			result->div++;
		}	
	}

	result->rem = abs_a;

	return 0;
}


UINT8 bank_SPRITE_PLAYER = 2;

const UINT8 anim_idle[] = { 1, 0 };
const UINT8 cs_show[] = { 1, 0 };
const UINT8 cs_hidden[] = { 1, 1 };
const UINT8 bar_hidden[] = { 1, 3 };
const UINT8 bar_show[] = { 1, 0 };


enum PlayerState
{
	AIMING,
	SHOOTING,
	MOVING
};

typedef enum PlayerState PlayerState;

enum MovementType
{
	X_MOVE,
	Y_MOVE
};

typedef enum MovementType MovementType;

struct PlayerData
{
	INT8 angle;
};

typedef struct PlayerData PlayerData;

struct MovementData
{
	INT8 speed_x;
	INT8 speed_y;
	DIV proportion;
	UINT8 acc_error;
	UINT8 acc;
	MovementType mov_type;
	INT8 xstep;
	INT8 ystep;
};

typedef struct MovementData MovementData;

PlayerState current_state;

struct Sprite* cross_hair;
struct Sprite* power_bar[8];

UINT8 num_bars = 8;

UINT8 power_times[8] = {1,8,8,8,4,4,2,2};
UINT8 powers[8] = { 2, 2, 1, 1, 0, 0, 0, 0 };
UINT8 deccs[8] = { 1, 1, 1, 2, 4, 8, 8, 8  };
INT8 fill = 1;
INT8 power_times_idx = 0;
INT8 power_time = 0;

INT8 offset = 2; // shift
INT8 rotspeed = 1;

INT8 num_shots = 0;

UINT8 height = 0;

INT8 decc_count;
INT8 decc_step  = 8;
INT8 decc_force_frames = 0;
INT8 decc_force = 0;
MovementData pmovement;

void ChangeState( PlayerState next );

void calculateMovementData( INT8 dx, INT8 dy, MovementData* movement )
{
	INT8 abs_dx = ABS( dx );
	INT8 abs_dy = ABS( dy );
	movement->speed_x = dx;
	movement->speed_y = dy;
	movement->xstep = SIGN( dx );
	movement->ystep = SIGN( dy );
	
	if( abs_dx == 0 )
	{
		movement->mov_type = Y_MOVE;
		movement->proportion.div = 0;
		movement->proportion.rem = 0;
		movement->xstep = 0;
		/*
		DPRINT_POS(10, 0);
		DPrintf("d: %i ", movement->proportion.div );
		DPRINT_POS(10, 1);
		DPrintf("r: %i ", movement->proportion.rem );
		*/

	}
	else if( abs_dy == 0 )
	{
		movement->mov_type = X_MOVE;
		movement->proportion.div = 0;
		movement->proportion.rem = 0;
		movement->ystep = 0;
		/*
		DPRINT_POS(10, 0);
		DPrintf("d: %i ", movement->proportion.div );
		DPRINT_POS(10, 1);
		DPrintf("r: %i ", movement->proportion.rem );
		*/
	}
	else
	{
		if( abs_dx > abs_dy )
		{
			movement->mov_type = X_MOVE;
			divide( abs_dx, abs_dy, &(movement->proportion) );
			/*
			DPRINT_POS(10, 0);
			DPrintf("d: %i ", movement->proportion.div );
			DPRINT_POS(10, 1);
			DPrintf("r: %i ", movement->proportion.rem );
			*/
		}	
		else if( abs_dx < abs_dy )
		{
			movement->mov_type = Y_MOVE;
			divide( abs_dy,abs_dx, &(movement->proportion) );
			/*
			DPRINT_POS(10, 0);
			DPrintf("d: %i ", movement->proportion.div );
			DPRINT_POS(10, 1);
			DPrintf("r: %i ", movement->proportion.rem );
			*/
		}
		else
		{
			// same value		
			movement->mov_type = X_MOVE;
			movement->proportion.div = 1;
			movement->proportion.rem = 0;
			/*
			DPRINT_POS(10, 0);
			DPrintf("d: %i ", movement->proportion.div );
			DPRINT_POS(10, 1);
			DPrintf("r: %i ", movement->proportion.rem );
			*/
		}
	}
	//DPRINT_POS(0, 0);
	//DPrintf("x%iy%i", dx, dy);
	//DPRINT_POS(8, 0);
	//DPrintf("s%is%i", movement->xstep,  movement->ystep );
	DPRINT_POS(0, 0);
	DPrintf("x: %i ",dx );
	DPRINT_POS(0, 1);
	DPrintf("y: %i ",dy );
}

void updateVelocity( MovementData *movement )
{
	UINT8 id = GetScrollTile( (THIS->x+8) >> 3, (THIS->y+8) >> 3 );
	UINT8 updateVel = 0;
	UINT8 acc = 0;
	switch( id )
	{
		//case 10:
		//case 18:
		case 44:
		case 52:
		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = movement->speed_x > 0 ? 1 : 0;
			movement->speed_x++;
		}
		
		updateVel = 1;	
		break;
		//case 11:
		//case 20:
		case 43:
		case 53:
		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = movement->speed_y > 0 ? 1 : 0;
			movement->speed_y++;
		}

		updateVel = 1;
		break;
		//case 12:
		//case 19:
		case 46:
		case 54:
		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = movement->speed_x < 0 ? 1 : 0;
			movement->speed_x--;
		}

		updateVel = 1;	
		break;
		//case 13:
		//case 21:
		case 45:
		case 55:
		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = movement->speed_y < 0 ? 1 : 0;
			movement->speed_y--;
		}

		updateVel = 1;
		break;
	
		case 47:
		case 56:

		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = (movement->speed_y > 0 ? 1 : 0) && (movement->speed_x > 0 ? 1 : 0);

			movement->speed_x++;
			movement->speed_y++;
		}

		
		updateVel = 1;	
	
		break;

		case 48:
		case 57:

		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = (movement->speed_y > 0 ? 1 : 0)
			|| (movement->speed_x < 0 ? 1 : 0);

			movement->speed_x--;
			movement->speed_y++;
		}

		updateVel = 1;	
	
		break;

		case 49:
		case 59:

		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = (movement->speed_y < 0 ? 1 : 0)
			|| (movement->speed_x > 0 ? 1 : 0);

			movement->speed_x++;
			movement->speed_y--;
		}
		
		updateVel = 1;	
	
		break;

		case 50:
		case 58:
		//if( --decc_force < 0 )
		{
			decc_force = decc_force_frames;
			acc = (movement->speed_y < 0 ? 1 : 0)
			&& (movement->speed_x < 0 ? 1 : 0);

			movement->speed_x--;
			movement->speed_y--;
		}
		updateVel = 1;	
	
		break;

	}

	if( updateVel )
	{
		//acc = 1;
		DPRINT_POS(10, 0);
		DPrintf("Acc: %i  ", acc );

		if( power_times_idx < 1 )
		{
			power_times_idx++;
		}

		if( acc && power_times_idx < 2 )
		{
			power_times_idx++;
		}

		calculateMovementData( movement->speed_x, movement->speed_y, movement );
	}
	else
	{
		DPRINT_POS(10, 0);
		DPrintf("No Acc " );
	}
}

/*
void updateHeight()
{
	UINT8 id = GetScrollTile( (THIS->x+8) >> 3, (THIS->y+8) >> 3 );
	switch( id )
	{
		case 0:
		{
			height = 0;
		}
		break;
		case 2:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		{
			height = 1;

		break;
		case 5:
		{
			height = 2;
		}
		break;
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		{
			height = 3;
		}
		break;
		case 6:
		{
			height = 4;
		}
		break;
	}

}
*/
int inHoleTile()
{
	UINT8 id = GetScrollTile( ( THIS->x + 8 ) >> 3,( THIS->y + 8 ) >> 3);
	//DPRINT_POS(10, 0);
	//DPrintf("Tile: %i  ", id );

	switch( id )
	{
		case 9:
		case 8:
		case 4:
			return 1;
		default:
			return 0;
	}
}


void cleanMovementData( MovementData* movement )
{
	movement->acc = 0;
	movement->acc_error = 0;	
}

void move( MovementData* movement )
{
	INT8 sy = ABS( movement->speed_y ); 
	INT8 sx = ABS( movement->speed_x ); 

	movement->acc++;		
	//DPRINT_POS(1, 1);
	//DPrintf("Move:%i", movement->mov_type);

	switch( movement->mov_type )
	{
		case X_MOVE:
		{
			//DPRINT_POS(0, 1);
			//DPrintf("MoveX:%i", movement->acc);

			if(TranslateSprite(THIS, movement->xstep, 0)) {
				movement->xstep = -movement->xstep;
				if( power_times_idx > 2 )
				{
					--power_times_idx;
				}
  
			}

			if( movement->proportion.div > 0 )
			{
				if( movement->acc >= movement->proportion.div )
				{
					//DPRINT_POS(15, 1);
					//DPrintf("MoveY");
	
					movement->acc = 0;
					movement->acc_error += movement->proportion.rem;

					// move y
					if(TranslateSprite(THIS, 0, movement->ystep)) {
						movement->ystep = -movement->ystep;
						if( power_times_idx > 2 )
						{
							--power_times_idx;
						}
					}

					if(  movement->proportion.rem > 0 
					&& movement->acc_error > sy )
					{
						movement->acc_error  -= sy;	
						// move x another step
						if(TranslateSprite(THIS, movement->xstep, 0)) {
							movement->xstep = -movement->xstep;
							if( power_times_idx > 2 )
							{
								--power_times_idx;
							}
						}

					}
				}

			}
		}
		break;

		case Y_MOVE:
		{
			//DPRINT_POS(0, 1);
			//DPrintf("MoveY:%i", movement->acc);

			if( TranslateSprite( THIS, 0, movement->ystep ) ) {
				movement->ystep = -movement->ystep;
				if( power_times_idx > 2 )
				{
					--power_times_idx;
				}

			}

			if( movement->proportion.div > 0 )
			{
				if( movement->acc >= movement->proportion.div )
				{
					//DPRINT_POS(15, 1);
					//DPrintf("MoveX");

					movement->acc = 0;
					movement->acc_error += movement->proportion.rem;

					// move x
					if(TranslateSprite(THIS, movement->xstep, 0)) {
						movement->xstep = -movement->xstep;

						if( power_times_idx > 2 )
						{
							--power_times_idx;
						}
					}

					if(  movement->proportion.rem > 0 
					&& movement->acc_error > sx )
					{
						movement->acc_error  -= sx;	
						// move y another step
						if( TranslateSprite( THIS, 0, movement->ystep ) ) {
							movement->ystep = -movement->ystep;
							if( power_times_idx > 2 )
							{
								--power_times_idx;
							}

						}
					}
				}

			}	

		}
		break;
	}
}

void update_aim( PlayerData* data )
{
	INT8 dx;
	INT8 dy;

	if(KEY_PRESSED(J_UP)) {
		data->angle -= 	rotspeed;
	} 
	if(KEY_PRESSED(J_DOWN)) {
		data->angle += 	rotspeed;
	}

	// this will reset timers that why it's an if/elseif
	/*
	if(KEY_TICKED(J_LEFT)) {
		if( offset > 1 )
		{
			offset = offset >> 1;
		}
	}
	else if(KEY_TICKED(J_RIGHT)) {
		if( offset < 5 )
		{
			offset = offset << 1;
		}
	}
	*/

	if(KEY_TICKED( J_A ))
	{
		ChangeState( SHOOTING );
	}

	cross_hair->x = THIS->x - (COS(data->angle) >> offset);
	cross_hair->y = THIS->y + (COS(data->angle+64) >> offset);
	dx = -COS(data->angle) >> 3;
	dy = COS(data->angle+64) >> 3; 
	DPRINT_POS(0, 0);
	DPrintf("x: %i ",dx );
	DPRINT_POS(0, 1);
	DPrintf("y: %i ",dy );

}

void update_shooting( )
{
	UINT8 i = 0;
	power_time = power_time + 1;

	if( power_time >= power_times[power_times_idx] )
	{
		power_time = 0;

		if( fill < 0 )
		{
			//SetSpriteAnim( power_bar[power_times_idx], bar_hidden, 30 );
			/*
			for( i = 0; i != num_bars; ++i )
			{
				power_bar[i]->anim_data = 0;	
				power_bar[i]->current_frame = 3;
			}
			*/

			power_bar[power_times_idx]->anim_data = 0;	
			power_bar[power_times_idx]->current_frame = 3;
		
		}
		else
		{
			//SetSpriteAnim( power_bar[power_times_idx], bar_show, 30 );
			power_bar[power_times_idx]->anim_data = 0;	
			power_bar[power_times_idx]->current_frame = 0;

			/*
			for( i = 0; i != num_bars; ++i )
			{
				power_bar[i]->anim_data = 0;	
				power_bar[i]->current_frame = 0;
			}
			*/
		}

		power_times_idx += fill;
		if( power_times_idx >= num_bars )
		{
			power_times_idx = num_bars - 1;
			fill = -1;
		}
		else if( power_times_idx <  0 )
		{
			power_times_idx = 0;
			fill = 1;
		}
	}

	if( KEY_TICKED( J_A ) )
	{
		ChangeState( MOVING );
	}
}

void update_moving( )
{
	
	UINT8 prev_height = height;
	if( --decc_count <= 0 )
	{
		decc_count = decc_step;
		--power_times_idx;
	}
	if( --power_time <= 0 && power_times_idx >= 0 )
	{
		power_time = powers[power_times_idx];
		//DPRINT_POS(1, 1);
		//DPrintf("Move:%i", power_times_idx);

		move( &pmovement );
		//updateVelocity( &pmovement );
	}
	if( --decc_force <= 0 && power_times_idx >= 0 )
	{
		decc_force = deccs[power_times_idx];
		updateVelocity( &pmovement );
	}


	if( power_times_idx < 0 )
	{
		ChangeState( AIMING );
	}

}

void Start_SPRITE_PLAYER() 
{
	INT8 i = 0;
	
	PlayerData* data = (PlayerData*)THIS->custom_data;
	data->angle = 0;
	cross_hair = SpriteManagerAdd(SPRITE_CROSSHAIR, THIS->x, THIS->y);
	cross_hair->x = THIS->x - (COS(data->angle) >> offset);
	cross_hair->y = THIS->y + (SIN(data->angle) >> offset);

	for(i=0; i != num_bars; ++i)
	{
		power_bar[i] = SpriteManagerAdd( SPRITE_BAR, 60 + i*8, 132 );
		power_bar[i]->anim_data = 0;
	}

	
	THIS->coll_x = 8;
	THIS->coll_y = 8;
	THIS->coll_w = 4;
	THIS->coll_h = 4;

	
	ChangeState( AIMING );
	//ChangeState( SHOOTING );
}

void Update_SPRITE_PLAYER() 
{
	PlayerData* data = (PlayerData*)THIS->custom_data;

	INT8 i;
	INT8 tx = ( ( (THIS->x + 8) >> 3 ) << 3 ) + 4;
	INT8 ty = ( ( (THIS->y + 8) >> 3 ) << 3 ) + 4;
	INT8 cx = THIS->x + 8;
	INT8 cy = THIS->y + 8;
	INT8 mdx = ABS( (tx - cx) );
	INT8 mdy = ABS( (ty - cy) );
	//INT8 speedx = ABS( pmovement.speed_x );
	//INT8 speedy = ABS( pmovement.speed_y );

	/*	
	Sprite* spr;

	SPRITEMANAGER_ITERATE(i, spr) {
		if(spr->type == SPRITE_HOLE) {
			if(CheckCollision(THIS, spr)) {
				SetState(STATE_GAME);
			}
		}
	}
	*/

	if( inHoleTile() )
	{
		INT8 md = MAX( mdx ,  mdy );
		//INT8 max_speed = MAX( speedx, speedy );

		//DPRINT_POS(10, 0);
		//DPrintf("mdx: %i  ", speedx );
		//DPRINT_POS(10, 1);
		//DPrintf("mdy: %i  ", speedy );

		if( power_times_idx == 0 && md <= 2 )
		{
			SetState(STATE_GAME);
		}
	}
	else
	{
		//DPRINT_POS(10, 0);
		//DPrintf("NO HOLE TILE" );
		//DPRINT_POS(10, 1);
		//DPrintf("NO HOLE TILE" );
	}

	for(i=0;i!=num_bars;++i)
	{
		//60 + i*8, 132 
		power_bar[i]->x = scroll_x + 60  + i * 8;	
	}	

	switch( current_state )
	{
		case AIMING:
		{
			update_aim( data );
		}
		break;
		case SHOOTING:
		{
			update_shooting( );
		}
		break;
		case MOVING:
		{
			update_moving( );
		}
		break;
	}

}

void ChangeState(PlayerState next) 
{
	INT8 i = 0;
	INT8 dx;
	INT8 dy;
	PlayerData* data = (PlayerData*)THIS->custom_data;

	current_state = next;

	switch( current_state )
	{
		case AIMING:
			pmovement.speed_x = 0;
			pmovement.speed_y = 0;

			data->angle = 0;

			//SetSpriteAnim( THIS, anim_idle, 15 );
			//SetSpriteAnim( cross_hair, cs_show, 15 );
			THIS->anim_data = 0;
			THIS->current_frame = 0;
			cross_hair->anim_data = 0;
			cross_hair->current_frame = 0;

			for(i=0;i!=num_bars;++i)
			{
				//SetSpriteAnim( power_bar[i], bar_hidden, 15 );

				power_bar[i]->anim_data = 0;
				power_bar[i]->current_frame = 3;
			}	

			cross_hair->x = THIS->x - (COS(data->angle) >> offset);
			cross_hair->y = THIS->y + (SIN(data->angle) >> offset);
		break;
		case SHOOTING:
		{
			
			for(i=0;i!=num_bars;++i)
			{
				//SetSpriteAnim( power_bar[i], bar_hidden, 15 );
			//60 + i*8, 132 
				power_bar[i]->x = scroll_x + 60  + i * 8;	
				power_bar[i]->anim_data = 0;
				power_bar[i]->current_frame = 3;
			}
			fill = 1;
			power_time = 0;
			power_times_idx = 0;
		}
		break;
		case MOVING:
		{
			/*
			for(i=0;i!=num_bars;++i)
			{
				//SetSpriteAnim( power_bar[i], bar_hidden, 15 );

				power_bar[i]->anim_data = 0;
				power_bar[i]->current_frame = 3;
			}
			*/
			power_time = 0;
			decc_count = decc_step;
			decc_force = decc_force_frames;
			cross_hair->current_frame = 1;
			cleanMovementData( &pmovement );
			
			dx =-COS(data->angle) >> 3;	
			dy = COS(data->angle+64) >> 3; 

			calculateMovementData( dx, dy, &pmovement );
		}
		break;
	}	
}




void Destroy_SPRITE_PLAYER() {
}

