#include "Engine.h"
#include "Player.h"
#include "TileTypes.h"
#include "FixedMath.h"

Player::Player()
{
	weapon.type = WeaponType_Pistol;
	weapon.ammo = 8;
	weapon.frame = 0;
	weapon.debounce = false;
}

void Player::update()
{
	bool strafe = Platform.readInput() & Input_Btn_A;
    
	int16_t movement = MOVEMENT;
	int16_t turn = TURN;
	int16_t cos_dir = FixedMath::Cos(direction);
	int16_t sin_dir = FixedMath::Sin(direction);
	int16_t oldX = x;
	int16_t oldZ = z;
	int16_t deltaX = 0, deltaZ = 0;
    
	updateWeapon();
    
	if (Platform.readInput() & Input_Dpad_Down)
	{
		deltaX -= (movement * cos_dir) >> (FIXED_SHIFT);
		deltaZ -= (movement * sin_dir) >> (FIXED_SHIFT);
	}
    
	if (Platform.readInput() & Input_Dpad_Up)
	{
		deltaX += (movement * cos_dir) >> (FIXED_SHIFT);
		deltaZ += (movement * sin_dir) >> (FIXED_SHIFT);
	}
    
	if (Platform.readInput() & Input_Dpad_Left)
	{
		if (strafe)
		{
			deltaX += (movement * sin_dir) >> (FIXED_SHIFT);
			deltaZ -= (movement * cos_dir) >> (FIXED_SHIFT);
		}
		else
			direction -= turn;
	}	
    
	if (Platform.readInput() & Input_Dpad_Right)
	{
		if (strafe)
		{
			deltaX -= (movement * sin_dir) >> (FIXED_SHIFT);
			deltaZ += (movement * cos_dir) >> (FIXED_SHIFT);
		}
		else
			direction += turn;
	}
  
	move(deltaX, deltaZ);

	//int16_t projectedX = x / CELL_SIZE;
	//int16_t projectedZ = z / CELL_SIZE;

	int16_t projectedX = x / CELL_SIZE + cos_dir / 19;
	int16_t projectedZ = z / CELL_SIZE + sin_dir / 19;

	engine.map.updateBufferPosition(projectedX - MAP_BUFFER_SIZE / 2, projectedZ - MAP_BUFFER_SIZE / 2);

	// Check for doors
	int8_t cellX = x / CELL_SIZE;
	int8_t cellZ = z / CELL_SIZE;

	engine.map.openDoorsAt(cellX, cellZ);

	if(cos_dir > 0)
	{
		engine.map.openDoorsAt(cellX + 1, cellZ);
	}
	else
	{
		engine.map.openDoorsAt(cellX - 1, cellZ);
	}
	if(sin_dir > 0)
	{
		engine.map.openDoorsAt(cellX, cellZ + 1);
	}
	else
	{
		engine.map.openDoorsAt(cellX, cellZ - 1);
	}

}

#ifdef USE_SIMPLE_COLLISIONS

bool Player::isPlayerColliding()
{
	for(int8_t n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		Actor& actor = engine.actors[n];
		if(actor.type != ActorType_Empty && actor.hp > 0 && actor.isPlayerColliding())
		{
			return true;
		}
	}

	return isPointColliding(x - MIN_WALL_DISTANCE, z - MIN_WALL_DISTANCE)
		|| isPointColliding(x + MIN_WALL_DISTANCE, z - MIN_WALL_DISTANCE)
		|| isPointColliding(x + MIN_WALL_DISTANCE, z + MIN_WALL_DISTANCE)
		|| isPointColliding(x - MIN_WALL_DISTANCE, z + MIN_WALL_DISTANCE);
}

bool Player::isPointColliding(int16_t pointX, int16_t pointZ)
{
	int8_t cellX = pointX / CELL_SIZE;
	int8_t cellZ = pointZ / CELL_SIZE;

	return (engine.map.isBlocked(cellX, cellZ));
}

void Player::move(int16_t deltaX, int16_t deltaZ)
{
	x += deltaX;
	z += deltaZ;

	if(isPlayerColliding())
	{
		z -= deltaZ;
		if(isPlayerColliding())
		{
			x -= deltaX;
			z += deltaZ;
			if(isPlayerColliding())
			{
				z -= deltaZ;
			}
		}
	}
}

#else

void Player::move(int16_t deltaX, int16_t deltaZ)
{
	for(int8_t n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		Actor& actor = engine.actors[n];
		if(actor.type != ActorType_Empty && actor.hp > 0)
		{
			int16_t diffX = mabs((x + deltaX) - actor.x);
			int16_t diffZ = mabs((z + deltaZ) - actor.z);

			if(diffX < MIN_ACTOR_DISTANCE && diffZ < MIN_ACTOR_DISTANCE)
			{
				if(diffX > diffZ)
				{
					if(x < actor.x)
					{
						deltaX = (actor.x - MIN_ACTOR_DISTANCE) - x;
					}
					else
					{
						deltaX = (actor.x + MIN_ACTOR_DISTANCE) - x;
					}
				}
				else
				{
					if(z < actor.z)
					{
						deltaZ = (actor.z - MIN_ACTOR_DISTANCE) - z;
					}
					else
					{
						deltaZ = (actor.z + MIN_ACTOR_DISTANCE) - z;
					}
				}
			}
		}
	}

	int8_t cellX = x / CELL_SIZE;
	int8_t cellZ = z / CELL_SIZE;

	if(deltaX < 0)
	{
		if(engine.map.isBlocked(cellX - 1, cellZ)
			|| (z < cellZ * CELL_SIZE + MIN_WALL_DISTANCE && engine.map.isBlocked(cellX - 1, cellZ - 1))
			|| (z > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && engine.map.isBlocked(cellX - 1, cellZ + 1)))
		{
			if(x + deltaX < cellX * CELL_SIZE + MIN_WALL_DISTANCE)
			{
				deltaX = (cellX * CELL_SIZE + MIN_WALL_DISTANCE) - x;
				cellX = x / CELL_SIZE;
			}
		}
	}
	else if(deltaX > 0)
	{
		if(engine.map.isBlocked(cellX + 1, cellZ)
			|| (z < cellZ * CELL_SIZE + MIN_WALL_DISTANCE && engine.map.isBlocked(cellX + 1, cellZ - 1))
			|| (z > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && engine.map.isBlocked(cellX + 1, cellZ + 1)))
		{
			if(x + deltaX > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE)
			{
				deltaX = (cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE) - x;
				cellX = x / CELL_SIZE;
			}
		}
	}

	if(deltaZ < 0)
	{
		if(engine.map.isBlocked(cellX, cellZ - 1)
			|| (x < cellX * CELL_SIZE + MIN_WALL_DISTANCE && engine.map.isBlocked(cellX - 1, cellZ - 1))
			|| (x > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && engine.map.isBlocked(cellX + 1, cellZ - 1)))
		{
			if(z + deltaZ < cellZ * CELL_SIZE + MIN_WALL_DISTANCE)
			{
				deltaZ = (cellZ * CELL_SIZE + MIN_WALL_DISTANCE) - z;
			}
		}
	}
	else if(deltaZ > 0)
	{
		if(engine.map.isBlocked(cellX, cellZ + 1)
			|| (x < cellX * CELL_SIZE + MIN_WALL_DISTANCE && engine.map.isBlocked(cellX - 1, cellZ + 1))
			|| (x > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && engine.map.isBlocked(cellX + 1, cellZ + 1)))
		{
			if(z + deltaZ > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE)
			{
				deltaZ = (cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE) - z;
			}
		}
	}


	x += deltaX;
	z += deltaZ;
}

#endif

#define NUM_WEAPON_FRAMES 4

void Player::updateWeapon()
{
	if (Platform.readInput() & Input_Btn_B)
	{
		if(!weapon.debounce)
		{
			weapon.debounce = true;
			if(weapon.shooting == false)
			{
				weapon.shooting = true;
				weapon.time = 0;
			}
		}
	}
	else
	{
		weapon.debounce = false;
	}

	if(weapon.shooting)
	{
		weapon.time++;

		switch(weapon.time)
		{
		case 2:
			weapon.frame = 1;
			break;
		case 4:
			shootWeapon();
			weapon.frame = 2;
			break;
		case 6:
			weapon.frame = 3;
			break;
		case 8:
			weapon.frame = 1;
			break;
		case 10:
			weapon.frame = 0;
			weapon.shooting = false;
			break;
		}
	}
}

void Player::init()
{
	// Find player start tile
	for(int8_t j = 0; j < MAP_SIZE; j += MAP_BUFFER_SIZE)
	{
		for(int8_t i = 0; i < MAP_SIZE; i += MAP_BUFFER_SIZE)
		{
			engine.map.updateBufferPosition(i, j);

			for(int8_t a = 0; a < MAP_BUFFER_SIZE; a++)
			{
				for(int8_t b = 0; b < MAP_BUFFER_SIZE; b++)
				{
					uint8_t tile = engine.map.getTileFast(b, a);

					if(tile >= Tile_PlayerStart_North && tile <= Tile_PlayerStart_West)
					{
						x = (i + b) * CELL_SIZE + CELL_SIZE / 2;
						z = (j + a) * CELL_SIZE + CELL_SIZE / 2;
						direction = (uint8_t)((tile - Tile_PlayerStart_North - 1) * DEGREES_90);
						return;
					}
				}
			}
		}
	}
}

void Player::shootWeapon()
{
	int16_t rotCos = FixedMath::Cos(-direction);
	int16_t rotSin = FixedMath::Sin(-direction);
	int8_t closestActor = -1;
	int16_t actorDistance = 0;
	
	for(int8_t n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(engine.actors[n].type != ActorType_Empty && engine.actors[n].hp > 0)
		{
			int16_t zt = (int16_t)(FIXED_TO_INT(rotCos * (int32_t)(engine.actors[n].x - x)) - FIXED_TO_INT(rotSin * (int32_t)(engine.actors[n].z - z)));
			int16_t xt = (int16_t)(FIXED_TO_INT(rotSin * (int32_t)(engine.actors[n].x - x)) + FIXED_TO_INT(rotCos * (int32_t)(engine.actors[n].z - z)));

			if(zt > CLIP_PLANE && xt > -ACTOR_HITBOX_SIZE / 2 && xt < ACTOR_HITBOX_SIZE / 2)
			{
				if(closestActor == -1 || zt < actorDistance)
				{
					closestActor = n;
					actorDistance = zt;
				}
			}
		}
	}

	if(closestActor != -1)
	{
		if(engine.map.isClearLine(x, z, engine.actors[closestActor].x, engine.actors[closestActor].z))
		{
			engine.actors[closestActor].damage(10);
			WARNING("BANG!\n");
		}
		else
		{
			WARNING("NOT A CLEAR LINE!\n");
		}
	}
	else
	{
		WARNING("NO TARGET!\n");
	}
}

