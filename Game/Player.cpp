#include "Engine.h"
#include "Player.h"
#include "TileTypes.h"
#include "FixedMath.h"
#include "Sounds.h"

Player::Player()
{
}

void Player::update()
{
	int16_t cos_dir = FixedMath::Cos(direction);
	int16_t sin_dir = FixedMath::Sin(direction);

	if(hp > 0)
	{
		bool strafe = Platform.readInput() & Input_Btn_A;

		if(Platform.readInput() == Input_Btn_A)
		{
			if(!ticksSinceStrafePressed)
			{
				ticksSinceStrafePressed = 1;
			}
			else if(ticksSinceStrafePressed > 1)
			{
				ticksSinceStrafePressed = 0;
				if(weapon.type == WeaponType_Pistol)
				{
					if(inventory.hasMachineGun)
					{
						weapon.type = WeaponType_MachineGun;
					}
					else if(inventory.hasChainGun)
					{
						weapon.type = WeaponType_ChainGun;
					}
					else weapon.type = WeaponType_Knife;
				}
				else if(weapon.type == WeaponType_MachineGun)
				{
					if(inventory.hasChainGun)
					{
						weapon.type = WeaponType_ChainGun;
					}
					else weapon.type = WeaponType_Knife;
				}
				else if(weapon.type == WeaponType_ChainGun)
				{
					weapon.type = WeaponType_ChainGun;
				}
				else if(weapon.ammo > 0)
				{
					weapon.type = WeaponType_Pistol;
				}
			}
		}
		else if(!Platform.readInput())
		{
			if(ticksSinceStrafePressed > 0)
			{
				ticksSinceStrafePressed ++;
				if(ticksSinceStrafePressed > 5)
				{
					ticksSinceStrafePressed = 0;
				}
			}
		}
		else ticksSinceStrafePressed = 0;

		int16_t movement = MOVEMENT;
		int16_t turn = TURN;
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

		// Check for doors
		int8_t cellX = WORLD_TO_CELL(x);
		int8_t cellZ = WORLD_TO_CELL(z);

		engine.map.openDoorsAt(cellX, cellZ, Direction_None);

		if(mabs(cos_dir) > mabs(sin_dir))
		{
			if(cos_dir > 0)
			{
				engine.map.openDoorsAt(cellX + 1, cellZ, Direction_East);
			}
			else
			{
				engine.map.openDoorsAt(cellX - 1, cellZ, Direction_West);
			}
		}
		else
		{
			if(sin_dir > 0)
			{
				engine.map.openDoorsAt(cellX, cellZ + 1, Direction_South);
			}
			else
			{
				engine.map.openDoorsAt(cellX, cellZ - 1, Direction_North);
			}
		}

		// Collect any items
		for(int8_t n = 0; n < MAX_ACTIVE_ITEMS; n++)
		{
			if(engine.map.items[n].type != 0 && engine.map.items[n].x == cellX && engine.map.items[n].z == cellZ)
			{
				bool collected = true;

				// Collect this item
				switch(engine.map.items[n].type)
				{
				case Tile_Item_MachineGun:
					weapon.ammo = min(weapon.ammo + 8, 99);
					Platform.playSound(Sound_CollectAmmo);
					weapon.type = WeaponType_MachineGun;
					inventory.hasMachineGun = true;
					break;
				case Tile_Item_Clip:
					if(weapon.ammo < 99)
					{
						if(weapon.ammo == 0 && weapon.type == WeaponType_Knife)
						{
							weapon.type = WeaponType_Pistol;
						}
						weapon.ammo = min(weapon.ammo + 8, 99);
					}
					else collected = false;
					break;
				case Tile_Item_FirstAid:
					if(hp < 100)
						hp = min(100, hp + 25);
					else collected = false;
					break;
				case Tile_Item_Food:
					if(hp < 100)
						hp = min(100, hp + 10);
					else collected = false;
					break;
				}
				if(collected)
				{
					Platform.playSound(Sound_CollectAmmo);
					engine.map.items[n].type = 0;
					engine.map.markItemCollected(engine.map.items[n].spawnId);
				}
			}
		}
	}
	else
	{
		int16_t rotCos = FixedMath::Cos(-direction);
		int16_t rotSin = FixedMath::Sin(-direction);
		int16_t xt = (int16_t)(FIXED_TO_INT(rotSin * (int32_t)(engine.actors[killer].x - x)) + FIXED_TO_INT(rotCos * (int32_t)(engine.actors[killer].z - z)));

		if(xt > 0)
		{
			direction += TURN;
		}
		else
		{
			direction -= TURN;
		}
		rotCos = FixedMath::Cos(-direction);
		rotSin = FixedMath::Sin(-direction);

		int16_t newXt = (int16_t)(FIXED_TO_INT(rotSin * (int32_t)(engine.actors[killer].x - x)) + FIXED_TO_INT(rotCos * (int32_t)(engine.actors[killer].z - z)));
		if((xt < 0 && newXt >= 0) || (xt > 0 && newXt <= 0))
		{
			engine.gameState = GameState_Dead;
			engine.frameCount = 0;
		}
		engine.renderer.damageIndicator = 5;
	}

	// Update the stream position
	int16_t projectedX = WORLD_TO_CELL(x) + cos_dir / 19;
	int16_t projectedZ = WORLD_TO_CELL(z) + sin_dir / 19;

	engine.map.updateBufferPosition(projectedX - MAP_BUFFER_SIZE / 2, projectedZ - MAP_BUFFER_SIZE / 2);
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
	int8_t cellX = WORLD_TO_CELL(pointX);
	int8_t cellZ = WORLD_TO_CELL(pointZ);

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
			weapon.frame = 2;
			shootWeapon();
			break;
		case 6:
			if(weapon.type == WeaponType_MachineGun)
			{
				weapon.frame = 1;
			}
			else weapon.frame = 3;
			break;
		case 8:
			if(weapon.type == WeaponType_MachineGun)
			{
				if(Platform.readInput() & Input_Btn_B)
				{
					weapon.time = 2;
				}
				else
				{
					weapon.frame = 0;
					weapon.shooting = false;
				}
			}
			else weapon.frame = 1;
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
	if(hp == 0)
	{
		weapon.type = WeaponType_Pistol;
		weapon.ammo = 8;
		hp = 100;
		inventoryFlags = 0;
	}

	weapon.frame = 0;
	weapon.debounce = false;

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
						x = CELL_TO_WORLD(i + b) + CELL_SIZE / 2;
						z = CELL_TO_WORLD(j + a) + CELL_SIZE / 2;
						direction = (uint8_t)((tile - Tile_PlayerStart_North - 1) * DEGREES_90);
					}
				}
			}
		}
	}
}

void Player::shootWeapon()
{
	Platform.playSound(Sound_AttackPistol);

	if(weapon.type != WeaponType_Knife)
	{
		weapon.ammo--;
	}

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

			if(zt > CLIP_PLANE && xt > -ACTOR_HITBOX_SIZE / 2 && xt < ACTOR_HITBOX_SIZE / 2 && (zt < INT_TO_FIXED(CELL_SIZE) || weapon.type != WeaponType_Knife))
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
		if(weapon.type == WeaponType_Knife)
		{
			uint8_t damage = getRandomNumber() >> 4;
			engine.actors[closestActor].damage(damage);
		}
		else if(engine.map.isClearLine(x, z, engine.actors[closestActor].x, engine.actors[closestActor].z))
		{
			int8_t dist = engine.actors[closestActor].getPlayerCellDistance();
			int damage;

			if (dist < 2)
				damage = getRandomNumber() / 4;
			else if (dist<4)
				damage = getRandomNumber() / 6;
			else
			{
				if ( (getRandomNumber() / 12) < dist)           // missed
					goto missed;
				damage = getRandomNumber() / 6;
			}
			
			engine.actors[closestActor].damage(damage);
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

missed:
	if(weapon.type != WeaponType_Knife && weapon.ammo == 0)
	{
		weapon.type = WeaponType_Knife;
		weapon.time = 0;
		weapon.frame = 0;
		weapon.shooting = false;
	}

}

void Player::damage(uint8_t amount)
{
	WARNING("Player damage: %d\n", (int)amount);
	engine.renderer.damageIndicator = 5;

	if(amount > hp)
	{
		hp = 0;
		// Dead
	}
	else
	{
		hp -= amount;
	}
}
