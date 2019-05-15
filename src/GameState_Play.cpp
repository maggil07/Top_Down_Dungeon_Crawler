#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"



GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
    : GameState(game)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void GameState_Play::init(const std::string & levelPath)
{
    loadLevel(levelPath);

	m_bg_music = m_entityManager.addEntity("music");
	m_bg_music->addComponent<CMusic>(m_game.getAssets().getMusic("PlayBG"), true, true);
}

void GameState_Play::loadLevel(const std::string & filename)
{
    m_entityManager = EntityManager();

	//Read in level config file
	std::ifstream fin(m_levelPath);
	std::string token;

	while (fin.good())
	{
		fin >> token;
		
		if (token == "Player")
		{
			int posX, posY;
			int bX, bY;
			float player_speed;
			int HP;

			fin >> posX >> posY >> bX >> bY >> player_speed >> HP;

			m_playerConfig.X = posX;
			m_playerConfig.Y = posY;
			m_playerConfig.CX = bX;
			m_playerConfig.CY = bY;
			m_playerConfig.SPEED = player_speed;
			m_playerConfig.HitPoints = HP;
		}
		else if (token == "Tile")
		{
			std::string name;
			int rX, rY;
			int posX, posY;
			int bM, bV;

			fin >> name >> rX >> rY >> posX >> posY >> bM >> bV;

			// Calculate where to add tile entities
			Vec2 tilePos = Vec2(rX*1280, rY*768) + Vec2(posX*64 + 32, posY*64 + 32);

			auto tiles = m_entityManager.addEntity("tile");

			tiles->addComponent<CTransform>(tilePos);
			tiles->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			tiles->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(name).getSize(), bM, bV);
			tiles->addComponent<CDraggable>(true);

			loadedAnimations.push_back(name); // As we load in animations, keep record for editing entities 

		}
		else if (token == "Item")
		{
			std::string name;
			int RX, RY, TX, TY, BM, BV, W, P, L;
			fin >> name >> RX >> RY >> TX >> TY >> BM >> BV >> W >> P >> L;
			auto item = m_entityManager.addEntity(token);
			item->addComponent<CTransform>((Vec2(RX * 1280, RY * 768) + Vec2(TX * 64 + 32, TY * 64 + 32)));
			item->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			item->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(name).getSize(), BM, BV);
			item->addComponent<CItem>(W, P, L);
		}
		else if (token == "NPC")
		{
			std::string name;
			int rX, rY;
			int posX, posY;
			int bM, bV;
			std::string AI_type;
			int Speed;

			fin >> name >> rX >> rY >> posX >> posY >> bM >> bV >> AI_type >> Speed;

			// Calculate where to add NPCs
			Vec2 NPC_Pos = Vec2(rX * 1280, rY * 768) + Vec2(posX * 64 + 32, posY * 64 + 32);

			auto enemies = m_entityManager.addEntity("npc");

			enemies->addComponent<CTransform>(NPC_Pos);
			enemies->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			enemies->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(name).getSize(), bM, bV);
			enemies->addComponent<CDraggable>(true);

			loadedAnimations.push_back(name); // As we load in animations, keep record for editing entities 

			//AI behavour
			if (AI_type == "Follow")
			{
				enemies->addComponent<CFollowPlayer>(NPC_Pos, Speed);
			}
			else if(AI_type == "Patrol")
			{
				int numPatrolPoints;
				int X, Y;
				
				fin >> numPatrolPoints;

				//Coords for each patrol point
				std::vector<Vec2> patrolPoints;

				//Add each patrol point to the correct location 
				for (int i = 0; i < numPatrolPoints; i++)
				{
					int x, y;
					fin >> x >> y;
					patrolPoints.push_back(Vec2(rX * 1280, rY * 768) + Vec2(x * 64 + 32, y * 64 + 32));
				}

				enemies->addComponent<CPatrol>(patrolPoints, Speed);
			}
		}
	}

    //Spawn player
    spawnPlayer();
}

void GameState_Play::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("StandDown"), true);
    m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), true, true);
	m_player->addComponent<CState>("Standing");
	m_player->addComponent<CDraggable>(true);
	m_player->addComponent<Health>(m_playerConfig.HitPoints);
	m_player->addComponent<CInventory>();
	m_player->getComponent<CInventory>()->weapons.push_back("sword");
    
    // New element to CTransform: 'facing', to keep track of where the player is facing
    m_player->getComponent<CTransform>()->facing = Vec2(0, 1);
}


    // conditions on which how and when a sword is spawned using apropriate inputs, animantions, state and facing
	// weapons are readded every time you shoot by m_entityManager.addEntity(weapons[sel])
	// weapons is a vector of strings within the CInventory Component.
	// The strings in the weapons vector are then used as keys to both of the animation maps to ensure the right direction animation is called.
	// If the strings don't match, then it will work.
	// Whichever animation name is used in the level file is the one that will be stored in the weapons vector and be used as the key for animations.
	// The 'values' in the map will need to be hardcoded with the names of all the item animation assets in the game
	// The player will only have access to these through the weapons vector, which only gets added to when a collision is detected with the item in the level.
	// The sword is left as a defualt weapon.
void GameState_Play::spawnSword(std::shared_ptr<Entity> entity)
{
    auto eTransform = entity->getComponent<CTransform>();

    auto sword = m_entityManager.addEntity (m_player->getComponent<CInventory>()->weapons[m_player->getComponent<CInventory>()->sel]);
    sword->addComponent<CTransform>(entity->getComponent<CTransform>()->pos + eTransform->facing * 64);
	sword->addComponent<CDraggable>(false);

	//Horizontal
	if (eTransform->facing.x != 0)
	{
		sword->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_player->getComponent<CInventory>()->wepMapRight[m_player->getComponent<CInventory>()->weapons[m_player->getComponent<CInventory>()->sel]]), true);

		//Invert animation on X axis
		if (eTransform->facing.x == -1)
		{
			sword->getComponent<CTransform>()->scale.x *= -1;
		}
	}
	//Vertical
	else
	{
		sword->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_player->getComponent<CInventory>()->wepMapUp[m_player->getComponent<CInventory>()->weapons[m_player->getComponent<CInventory>()->sel]]), true);

		//Invert animation on Y axis
		if (eTransform->facing.y == 1)
		{
			sword->getComponent<CTransform>()->scale.y *= -1;
		}
	}

	sword->addComponent<CBoundingBox>(sword->getComponent<CAnimation>()->animation.getSize(), false, false);
	sword->addComponent<CLifeSpan>(150);
}

void GameState_Play::spawnBow(std::shared_ptr<Entity> entity)
{

}


// for projectiles?
//if (special->hasComponent<CItem>())
//{
//	if (special->getComponent<CItem>()->hasProjectile != 0)
//	{
//       use hasProjectile value as speed or range
//		//spawnBullet()
//	}
//}
const int SPECIAL_DURATION = 2000;
const int SPECIAL_SPEED = 10;
void GameState_Play::spawnSpecial(std::shared_ptr<Entity> entity)
{
	auto eTransform = entity->getComponent<CTransform>();

	auto special = m_entityManager.addEntity("special");
	special->addComponent<CTransform>(entity->getComponent<CTransform>()->pos + eTransform->facing * 64);
	special->addComponent<CDraggable>(false);

	//Horizontal
	if (eTransform->facing.x != 0)
	{
		special->addComponent<CAnimation>(m_game.getAssets().getAnimation("SwordRight"), true);
		special->getComponent<CTransform>()->speed = Vec2(SPECIAL_SPEED, 0);

		//Invert animation on X axis
		if (eTransform->facing.x == -1)
		{
			special->getComponent<CTransform>()->scale.x *= -1;
			special->getComponent<CTransform>()->speed = Vec2(-SPECIAL_SPEED, 0);
		}
	}
	//Vertical
	else
	{
		special->addComponent<CAnimation>(m_game.getAssets().getAnimation("SwordUp"), true);
		special->getComponent<CTransform>()->speed = Vec2(0, -SPECIAL_SPEED);

		//Invert animation on Y axis
		if (eTransform->facing.y == 1)
		{
			special->getComponent<CTransform>()->scale.y *= -1;
			special->getComponent<CTransform>()->speed = Vec2(0, SPECIAL_SPEED);
		}
	}

	special->addComponent<CBoundingBox>(special->getComponent<CAnimation>()->animation.getSize(), false, false);
	special->addComponent<CLifeSpan>(SPECIAL_DURATION);
}

void GameState_Play::toggleWeaponLeft()
{
	if (m_player->getComponent<CInventory>()->canToggle)
	{
		if (m_player->getComponent<CInventory>()->sel - 1 >= 0)
		{
			m_player->getComponent<CInventory>()->sel--;
		}
		else
		{
			m_player->getComponent<CInventory>()->sel++;
		}
	}
}

void GameState_Play::toggleWeaponRight()
{
	if (m_player->getComponent<CInventory>()->canToggle)
	{
		if (m_player->getComponent<CInventory>()->sel + 1 < m_player->getComponent<CInventory>()->weapons.size())
		{
			m_player->getComponent<CInventory>()->sel++;
		}
		else
		{
			m_player->getComponent<CInventory>()->sel--;
		}
	}
}
void GameState_Play::update()
{
    m_entityManager.update();

    if (!m_paused)
    {
        sAI();
        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
		//changeAnimation();
		//deleteEntity();
		//insertTile();
    }

    sUserInput();
    sRender();
}

void GameState_Play::sMovement()
{
	//Reset speed every frame
	m_player->getComponent<CTransform>()->speed.x = 0;
	m_player->getComponent<CTransform>()->speed.y = 0;

	if (m_player->hasComponent<CTransform>())
	{
		//Vertical movement
		if (m_player->getComponent<CInput>()->up && !m_player->getComponent<CInput>()->down)
		{
			m_player->getComponent<CTransform>()->speed.x = 0;
			m_player->getComponent<CState>()->state = "RunUp";
			m_player->getComponent<CTransform>()->facing = Vec2(0, -1);
			m_player->getComponent<CTransform>()->speed.y -= m_playerConfig.SPEED; 
		}
		else if (m_player->getComponent<CInput>()->down && !m_player->getComponent<CInput>()->up)
		{
			m_player->getComponent<CTransform>()->speed.x = 0;
			m_player->getComponent<CState>()->state = "RunDown";
			m_player->getComponent<CTransform>()->facing = Vec2(0, 1);
			m_player->getComponent<CTransform>()->speed.y += m_playerConfig.SPEED; 
		}

		//Horizontal movement
		if (m_player->getComponent<CInput>()->left && !m_player->getComponent<CInput>()->right)
		{
			m_player->getComponent<CTransform>()->speed.y = 0;
			m_player->getComponent<CState>()->state = "RunLeft";
			m_player->getComponent<CTransform>()->facing = Vec2(-1, 0);
			m_player->getComponent<CTransform>()->speed.x -= m_playerConfig.SPEED; 
		}
		else if (m_player->getComponent<CInput>()->right && !m_player->getComponent<CInput>()->left)
		{
			m_player->getComponent<CTransform>()->speed.y = 0;
			m_player->getComponent<CState>()->state = "RunRight";
			m_player->getComponent<CTransform>()->facing = Vec2(1, 0);
			m_player->getComponent<CTransform>()->speed.x += m_playerConfig.SPEED; 
		}

		//No movement (Standing)
		if (!m_player->getComponent<CInput>()->up && !m_player->getComponent<CInput>()->down && !m_player->getComponent<CInput>()->left && !m_player->getComponent<CInput>()->right)
		{
			//Vertical
			if (m_player->getComponent<CTransform>()->facing.x == 0 && m_player->getComponent<CTransform>()->facing.y == -1)
			{
				m_player->getComponent<CTransform>()->speed.x = 0;
				m_player->getComponent<CState>()->state = "StandUp";
			}
			else if (m_player->getComponent<CTransform>()->facing.x == 0 && m_player->getComponent<CTransform>()->facing.y == 1)
			{
				m_player->getComponent<CTransform>()->speed.x = 0;
				m_player->getComponent<CState>()->state = "StandDown";
			}
			//Horizontal
			else if (m_player->getComponent<CTransform>()->facing.y == 0 && m_player->getComponent<CTransform>()->facing.x == -1)
			{
				m_player->getComponent<CTransform>()->speed.y = 0;
				m_player->getComponent<CState>()->state = "StandLeft";
			}
			else if (m_player->getComponent<CTransform>()->facing.y == 0 && m_player->getComponent<CTransform>()->facing.x == 1)
			{
				m_player->getComponent<CTransform>()->speed.y = 0;
				m_player->getComponent<CState>()->state = "StandRight";
			}
		}

		//Limit player speed
		m_player->getComponent<CTransform>()->speed.x = fmin(m_playerConfig.SPEED, fmax(m_player->getComponent<CTransform>()->speed.x, -m_playerConfig.SPEED));
		m_player->getComponent<CTransform>()->speed.y = fmin(m_playerConfig.SPEED, fmax(m_player->getComponent<CTransform>()->speed.y, -m_playerConfig.SPEED));
	}

	//All other entities
	for (auto entity : m_entityManager.getEntities())
	{
		if (entity->hasComponent<CTransform>())
		{
			entity->getComponent<CTransform>()->prevPos = entity->getComponent<CTransform>()->pos;
			entity->getComponent<CTransform>()->pos += entity->getComponent<CTransform>()->speed;

			//Update the sword's position as it follows with the player
			//Updates for the next frame
			if (entity->tag() == "sword")
			{
				m_player->getComponent<CInput>()->shoot = false;
				entity->getComponent<CTransform>()->facing = m_player->getComponent<CTransform>()->facing;
				entity->getComponent<CTransform>()->pos = m_player->getComponent<CTransform>()->pos + m_player->getComponent<CTransform>()->facing * 64;
				m_player->getComponent<CState>()->state = "Attack";
			}
		}
	}

	
}

const int PATROL_DISTANCE_MARGIN = 5;
void GameState_Play::sAI()
{
	for (auto entity : m_entityManager.getEntities("npc"))
	{
		Vec2 destination(0, 0);
		float speed;
		
		entity->getComponent<CTransform>()->speed = Vec2(0, 0);

		//Patrol NPCs
		if (entity->hasComponent<CPatrol>())
		{
			destination = entity->getComponent<CPatrol>()->positions[entity->getComponent<CPatrol>()->currentPosition];
			speed = entity->getComponent<CPatrol>()->speed;

			//If enemy is within a margin of PATROL DISTANCE, move to player
			if (entity->getComponent<CTransform>()->pos.dist(destination) <= PATROL_DISTANCE_MARGIN)
			{
				entity->getComponent<CPatrol>()->currentPosition += 1;

				if (entity->getComponent<CPatrol>()->positions.size() <= entity->getComponent<CPatrol>()->currentPosition)
				{
					entity->getComponent<CPatrol>()->currentPosition = 0;
				}

				destination = entity->getComponent<CPatrol>()->positions[entity->getComponent<CPatrol>()->currentPosition];
			}
		}
		//Follow NPCs
		else if (entity->hasComponent<CFollowPlayer>() && entity->hasComponent<CBoundingBox>())
		{
			bool inSight = true;
			speed = entity->getComponent<CFollowPlayer>()->speed;

			for (auto e : m_entityManager.getEntities())
			{
				//Skip player and itself
				if (e->tag() == "player" || entity == e) continue;

				if (e->hasComponent<CBoundingBox>() && e->getComponent<CBoundingBox>()->blockVision)
				{
					//If vision is blocked then cancel movement towards player
					if (Physics::EntityIntersect(m_player->getComponent<CTransform>()->pos, entity->getComponent<CTransform>()->pos, e) || entity->getComponent<CTransform>()->pos.dist(m_player->getComponent<CTransform>()->pos) > 10 * 64) // Check if sight is blocked by entity OR out of range
					{
						inSight = false;
						break;
					}
				}
			}

			//Move towards player if in sight
			if (inSight == true)
			{
				destination = m_player->getComponent<CTransform>()->pos;
			}
			else
			{
				destination = entity->getComponent<CFollowPlayer>()->home;
			}		
		}

		//Handle NPC speed
		if (entity->getComponent<CTransform>()->pos.dist(destination) <= PATROL_DISTANCE_MARGIN)
		{
			entity->getComponent<CTransform>()->speed = Vec2(0, 0);
		}
		else
		{
			float deltaX, deltaY, tanAngle;
			
			deltaX = destination.x - entity->getComponent<CTransform>()->pos.x;
			deltaY = destination.y - entity->getComponent<CTransform>()->pos.y;
			tanAngle = atan2(deltaY, deltaX);

			entity->getComponent<CTransform>()->speed = Vec2(speed * cos(tanAngle), speed * sin(tanAngle));
		}
	}
}

void GameState_Play::sLifespan()
{
	for (auto & entity : m_entityManager.getEntities()) {
		if (entity->hasComponent<CLifeSpan>() && !m_paused)
		{
			const float ratio = entity->getComponent<CLifeSpan>()->clock.getElapsedTime().asMilliseconds() / (float)entity->getComponent<CLifeSpan>()->lifespan;

			if (ratio >= 1.0)
			{
				entity->destroy();
			}
		}
	}
}

void GameState_Play::sCollision()
{
	for (auto & tile : m_entityManager.getEntities("tile")) {
		Vec2 overlap = Physics::GetOverlap(m_player, tile);
		Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, tile);

		const auto tileType = tile->getComponent<CAnimation>()->animation.getName();
		const auto tileTransform = tile->getComponent<CTransform>();
		const auto playerTransform = m_player->getComponent<CTransform>();
		auto playerState = m_player->getComponent<CState>();

		//Collide against all tiles besides 'black'
		if (!Animation::isTileFloor(tileType))
		{
			//Collision happens if theres an overlap in both axis
			if (overlap.x > 0 && overlap.y > 0) {
				//Player vs tile top
				if (prevOverlap.x > 0 && tileTransform->prevPos.y < playerTransform->prevPos.y)
				{
					playerTransform->pos.y += overlap.y;
					playerTransform->speed.y = 0;

					if (playerTransform->speed.x == 0)
					{
						playerState->state = "standing";
					}
					else
					{
						playerState->state = "running";
					}
				}
				//Player vs tile bottom
				else if (prevOverlap.x > 0 && tileTransform->prevPos.y > playerTransform->prevPos.y)
				{
					playerTransform->pos.y -= overlap.y;
					playerTransform->speed.y = 0;
				}

				//Player vs tile left
				else if (prevOverlap.y > 0 && tileTransform->prevPos.x < playerTransform->prevPos.x)
				{
					playerTransform->pos.x += overlap.x;
				}

				//Player vs tile right
				else if (prevOverlap.y > 0 && tileTransform->prevPos.x > playerTransform->prevPos.x)
				{
					playerTransform->pos.x -= overlap.x;
				}
			}
		}

		//Special vs tile
		for (auto & special : m_entityManager.getEntities("special"))
		{
			Vec2 overlap = Physics::GetOverlap(special, tile);

			if (overlap.x > 0 && overlap.y > 0)
			{
				//Special dies if hits tile
				special->destroy();
			}
		}
	}

	//Player vs enemy NPCs
	for (auto & npc : m_entityManager.getEntities("npc"))
	{
		Vec2 overlap = Physics::GetOverlap(m_player, npc);
		

		// Player dies if they collide with enemy NPC
		if (overlap.x > 0 && overlap.y > 0 && m_player->getComponent<Health>()->HP > 0)
		{
			m_player->getComponent<Health>()->HP--;
			std::cout << m_player->getComponent<Health>()->HP;

			if (m_player->getComponent<Health>()->HP == 0)
			{
				m_player->destroy();
				spawnPlayer();
			}
		}

		//NPC vs sword
		for (auto & sword : m_entityManager.getEntities("sword"))
		{
			Vec2 overlap = Physics::GetOverlap(sword, npc);

			if (overlap.x > 0 && overlap.y > 0)
			{
				if (npc->getComponent<CAnimation>()->animation.getName() != "Explosion" && npc != NULL)
				{
					auto boom = m_entityManager.addEntity("dec");

					boom->addComponent<CTransform>()->pos = npc->getComponent<CTransform>()->pos;
					boom->addComponent<CAnimation>(m_game.getAssets().getAnimation("Explosion"), false);
				}

				// NPC dies if hit with sword
				npc->removeComponent<CDraggable>();
				npc->destroy();
			}
		}

		for (auto & special : m_entityManager.getEntities("special"))
		{
			Vec2 overlap = Physics::GetOverlap(special, npc);

			if (overlap.x > 0 && overlap.y > 0)
			{
				if (npc->getComponent<CAnimation>()->animation.getName() != "Explosion")
				{
					auto boom = m_entityManager.addEntity("dec");

					boom->addComponent<CTransform>()->pos = npc->getComponent<CTransform>()->pos;
					boom->addComponent<CAnimation>(m_game.getAssets().getAnimation("Explosion"), false);
				}

				// NPC dies if hit with sword
				npc->destroy();
				special->destroy();
			}
		}

		//NPC vs tiles
		for (auto & tile : m_entityManager.getEntities("tile"))
		{
			const auto npcTransform = npc->getComponent<CTransform>();
			Vec2 overlap = Physics::GetOverlap(npc, tile);
			Vec2 prevOverlap = Physics::GetPreviousOverlap(npc, tile);
			const auto tileTransform = tile->getComponent<CTransform>();

			if (overlap.x > 0 & overlap.y > 0)
			{
				if (overlap.x > 0 && overlap.y > 0) {
					//NPC vs tile top
					if (prevOverlap.x > 0 && tileTransform->prevPos.y < npcTransform->prevPos.y)
					{
						npcTransform->pos.y += overlap.y;
						npcTransform->speed.y = 0;
					}
					//NPC vs tile bottom
					else if (prevOverlap.x > 0 && tileTransform->prevPos.y > npcTransform->prevPos.y)
					{
						npcTransform->pos.y -= overlap.y;
						npcTransform->speed.y = 0;
					}

					//Player vs tile left
					else if (prevOverlap.y > 0 && tileTransform->prevPos.x < npcTransform->prevPos.x)
					{
						npcTransform->pos.x += overlap.x;
					}

					//NPC vs tile right
					else if (prevOverlap.y > 0 && tileTransform->prevPos.x > npcTransform->prevPos.x)
					{
						npcTransform->pos.x -= overlap.x;
					}
				}
			}
		}
	}

	// player vs items
	Vec2 iOverlap;
	for (auto x : m_entityManager.getEntities("Item"))
	{
		if (x->hasComponent<CItem>())
		{
			if (x->getComponent<CItem>()->isWeapon == 1)
			{
				iOverlap = Physics::GetOverlap(m_player, x);
				if (iOverlap.x > 0 && iOverlap.y > 0)
				{
					// adds asset string to the weapons vector
					m_player->getComponent<CInventory>()->weapons.push_back(x->getComponent<CAnimation>()->animation.getName());
					// swtiches the current weapon to the one just picked up
					m_player->getComponent<CInventory>()->sel = m_player->getComponent<CInventory>()->weapons.size()-1;
					x->destroy();


				}
			}
		}
	}
}

void GameState_Play::sAnimation()
{
	for (auto entity : m_entityManager.getEntities()) 
	{
		//Skip entity if it doesn't have an animation
		if (!entity->hasComponent<CAnimation>()) { continue; }
		auto animation = entity->getComponent<CAnimation>();

		//Handle player animation
		if (entity == m_player)
		{
			//Vertical running
			if (m_player->getComponent<CState>()->state == "RunUp" && animation->animation.getName() != "RunUp")
			{
				m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("RunUp"), true);
			}
			else if (m_player->getComponent<CState>()->state == "RunDown" && animation->animation.getName() != "RunDown")
			{
				m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("RunDown"), true);
			}
			//Horizontal running
			else if (m_player->getComponent<CState>()->state == "RunLeft" || m_player->getComponent<CState>()->state == "RunRight")
			{
				if (animation->animation.getName() != "RunRight")
				{
					m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("RunRight"), true);
				}
				m_player->getComponent<CTransform>()->scale.x = m_player->getComponent<CTransform>()->facing.x;
			}
			//Vertical standing
			else if (m_player->getComponent<CState>()->state == "StandUp" && animation->animation.getName() != "StandUp")
			{
				m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("StandUp"), true);
			}
			else if (m_player->getComponent<CState>()->state == "StandDown" && animation->animation.getName() != "StandDown")
			{
				m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("StandDown"), true);
			}
			//Horizontal standing
			else if (m_player->getComponent<CState>()->state == "StandLeft" || m_player->getComponent<CState>()->state == "StandRight")
			{
				if (animation->animation.getName() != "StandRight")
				{
					m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("StandRight"), true);
				}
				m_player->getComponent<CTransform>()->scale.x = m_player->getComponent<CTransform>()->facing.x;
			}
		}

		//Attack down
		if (m_player->getComponent<CState>()->state == "Attack" && m_player->getComponent<CTransform>()->facing == Vec2(0, 1))
		{
			auto attack_down = m_game.getAssets().getAnimation("AtkDown");
			m_player->getComponent<CAnimation>()->animation = attack_down;
		}

		//Attack up
		if (m_player->getComponent<CState>()->state == "Attack" && m_player->getComponent<CTransform>()->facing == Vec2(0, -1))
		{
			auto attack_up = m_game.getAssets().getAnimation("AtkUp");
			m_player->getComponent<CAnimation>()->animation = attack_up;
		}

		//Attack right
		if (m_player->getComponent<CState>()->state == "Attack" && m_player->getComponent<CTransform>()->facing == Vec2(-1, 0))
		{
			auto attack_right = m_game.getAssets().getAnimation("AtkRight");
			m_player->getComponent<CAnimation>()->animation = attack_right;
		}

		//Attack left
		if (m_player->getComponent<CState>()->state == "Attack" && m_player->getComponent<CTransform>()->facing == Vec2(1, 0))
		{
			auto attack_right = m_game.getAssets().getAnimation("AtkRight");
			m_player->getComponent<CAnimation>()->animation = attack_right;
			
		}

		//Sword
		if (entity->tag() == "sword")
		{
			entity->getComponent<CTransform>()->scale = Vec2(1, 1);
			auto swordFace = entity->getComponent<CTransform>()->facing.y;

			//Handle facing
			// updated with weapons[sel] instead of string
			if (abs(swordFace) == 1)
			{
				entity->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_player->getComponent<CInventory>()->wepMapUp[m_player->getComponent<CInventory>()->weapons[m_player->getComponent<CInventory>()->sel]]), true);
				entity->getComponent<CTransform>()->scale.y = entity->getComponent<CTransform>()->facing.y * -1;
			}
			else
			{
				entity->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_player->getComponent<CInventory>()->wepMapRight[m_player->getComponent<CInventory>()->weapons[m_player->getComponent<CInventory>()->sel]]), true);
				entity->getComponent<CTransform>()->scale.x = entity->getComponent<CTransform>()->facing.x;
			}
		}

		//Update
		animation->animation.update();

		//Destroy entity if animation does not repeat and has finish
		if (animation->animation.hasEnded() && !animation->repeat) {
			entity->destroy();
		}
	}
}

void GameState_Play::sDrag()
{
	for (auto e : m_entityManager.getEntities())
	{
		auto drag = e->getComponent<CTransform>();
		
		drag->prevPos = drag->pos;
		
		drag->pos += drag->speed;
	
		float x_difference1 = 0;
		float x_diference2 = 0;
		
		float y_difference1 = 0;
		float y_difference2 = 0;
		
		for (auto e2 : m_entityManager.getEntities())
		{
			if (e2->hasComponent<CDraggable>()) // Check if entity has required component 
			{
				if (e2->getComponent<CDraggable>()->canDrag)
				{
					if (e2->hasComponent<CBoundingBox>())
					{
						// Check if user is clicking on an entity

						x_difference1 = abs(mouse_x - e2->getComponent<CTransform>()->pos.x);
						y_difference1 = abs(mouse_y - e2->getComponent<CTransform>()->pos.y);


						if ((x_difference1 >= 0 && x_difference1 < e2->getComponent<CBoundingBox>()->halfSize.x) && (y_difference1 >= 0 && y_difference1 < e2->getComponent<CBoundingBox>()->halfSize.y))
						{
							e2->getComponent<CDraggable>()->clicked = clicked;
						}
					}
				}
			}
		}
	}
}


void GameState_Play::insertTile()
{
	
	sf::Vector2i mousePos = sf::Mouse::getPosition(m_game.window());
	
	sf::Vector2f worldpos = m_game.window().mapPixelToCoords(mousePos);
	
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CDraggable>())
		{

			if (e->getComponent<CDraggable>()->canDrag && e->getComponent<CDraggable>()->clicked != NULL)
			{
				e->getComponent<CTransform>()->pos.x = worldpos.x;
				e->getComponent<CTransform>()->pos.y = worldpos.y;

				if (insert && e->tag() == "tile") // Check if user presssed INSERT key, and only able to insert new "Tile" entities 
				{
					auto new_e = m_entityManager.addEntity(e->tag());

					new_e->addComponent<CTransform>(Vec2(worldpos.x, worldpos.y));

					new_e->addComponent<CBoundingBox>(e->getComponent<CBoundingBox>()->size, e->getComponent<CBoundingBox>()->blockMove, e->getComponent<CBoundingBox>()->blockVision);
					new_e->addComponent<CAnimation>(e->getComponent<CAnimation>()->animation, e->getComponent<CAnimation>()->repeat);
					new_e->addComponent<CDraggable>(true);

					clicked = !clicked;

				}
			}
		}
	} 
}

void GameState_Play::changeAnimation() {
	

} 


void GameState_Play::makeGrid()
{
	// TODO
}


void GameState_Play::deleteEntity()
{
	for (auto e : m_entityManager.getEntities())
	{

		if (toDelete && e->getComponent<CDraggable>()->clicked && e->tag() != "player") // Check if user pressed DEL key and if an entity besides the player is selected 
		{
			e->destroy(); // Destroy entity current on cursor 

				clicked = !clicked; // Entity is destroyed, therefore isn't being clicked anymore 

		}
	}
}

void GameState_Play::saveLevel()
{
	std::cout << "Enter file you wish to save your level too: ";

	std::ofstream fOut;

	std::string fileName;

	std::cin >> fileName;

	fileName = fileName + ".txt";

	fOut.open(fileName);

	int w = m_game.window().getDefaultView().getSize().x;
	int h = m_game.window().getDefaultView().getSize().y;

	for (auto e : m_entityManager.getEntities("tile")) // Saving tiles 
	{
		int rX = e->getComponent<CTransform>()->pos.x / 1268;   // Calculate opposite of what we did when we read in to save level
		int rY = e->getComponent<CTransform>()->pos.y / 768;

		int bV = e->getComponent<CBoundingBox>()->blockVision;
		int bM = e->getComponent<CBoundingBox>()->blockMove;

		int tY = (e->getComponent<CTransform>()->pos.x - rX * 1268 - 32) / 64;
		int tX = (e->getComponent<CTransform>()->pos.y - rY * 768 - 32) / 64;

		std::string tileName = e->getComponent<CAnimation>()->animation.getName();

		fOut << "Tile" << " " << tileName << " " << rX << " " << rY << " " << tX << " " << tY << " " << bM << " " << bV << " " << std::endl;
	}

	for (auto NPC : m_entityManager.getEntities("npc")) // Saving NPCs
	{
		std::string enemyName = NPC->getComponent<CAnimation>()->animation.getName();

		int rX = NPC->getComponent<CTransform>()->pos.x / 1268;
		int rY = NPC->getComponent<CTransform>()->pos.y / 768;

		int tX = (NPC->getComponent<CTransform>()->pos.x - rX * 1268 - 32) / 64;
		int tY = (NPC->getComponent<CTransform>()->pos.y - rY * 768 - 32) / 64;

		int bV = NPC->getComponent<CBoundingBox>()->blockVision;
		int bM = NPC->getComponent<CBoundingBox>()->blockMove;

		std::string AI;

		if (NPC->hasComponent<CFollowPlayer>()) // Follow NPCs
		{
			AI = "Follow";
			int S = NPC->getComponent<CFollowPlayer>()->speed;
			
			fOut << "NPC" << " " << enemyName << " " << " " << rX << " " << rY << " " << tX << " " << tY << " " << bM << " " << bV << " " << AI << " " << S << std::endl;
		}

		if (NPC->hasComponent<CPatrol>()) // Patrolling NPCs
		{
			AI = "Patrol";
			std::vector<Vec2>patrolPoints;
			std::vector<Vec2>realPoints;

			int realX;
			int realY;
			
			int S = NPC->getComponent<CPatrol>()->speed;
			patrolPoints = NPC->getComponent<CPatrol>()->positions;
			Vec2 pos;

			fOut << "NPC" << " " << enemyName << " " << " " << rX << " " << rY << " " << tX << " " << tY << " " << bM << " " << bV << " " << AI << " " << S << " " << patrolPoints.size() << " ";

			for (int i = 0; i <= patrolPoints.size() - 1; i++)
			{
				realX = (patrolPoints[i].x - rX * w - 32) / (64);
				realY = (patrolPoints[i].y - rY * h - 32) / (64);
				
				pos = Vec2(realX, realY);

				realPoints.push_back(pos);
			}

			for (int i = 0; i <= realPoints.size() - 1; i++)
			{
				pos = realPoints[i];
				fOut << pos.x << " " << pos.y << " ";
			}

			fOut << std::endl;

		}


	}

	for (auto e : m_entityManager.getEntities("player")) // Saving player
	{
		Vec2 playerPos = e->getComponent<CTransform>()->pos;
		Vec2 boundingSize = e->getComponent<CBoundingBox>()->size;
		Vec2 playerSpeed = e->getComponent<CTransform>()->speed;


		fOut << "Player" << " " << playerPos.x << " " << playerPos.y << " " << boundingSize.x << " " << boundingSize.y << " " << m_playerConfig.SPEED << std::endl;
	}
	
	fOut.close();
	std::cout << "Level saved!" << std::endl;
}

void GameState_Play::sUserInput()
{
    auto pInput = m_player->getComponent<CInput>();

    sf::Event event;
    while (m_game.window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_game.quit();
        }
        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
			switch (event.key.code)
			{
			case sf::Keyboard::Escape: { m_game.popState(); break; }
			case sf::Keyboard::W: { pInput->up = true; break; }
			case sf::Keyboard::A: { pInput->left = true; break; }
			case sf::Keyboard::S: { pInput->down = true; break; }
			case sf::Keyboard::D: { pInput->right = true; break; }
			case sf::Keyboard::Z: { init(m_levelPath); break; }
			case sf::Keyboard::R: { m_drawTextures = !m_drawTextures; break; }
			case sf::Keyboard::F: { m_drawCollision = !m_drawCollision; break; }
			case sf::Keyboard::Y: { m_follow = !m_follow; break; }
			case sf::Keyboard::P: { setPaused(!m_paused); break; }
			case sf::Keyboard::Tilde: { saveLevel(); break; }
			case sf::Keyboard::Space: { spawnSword(m_player); break; }
			case sf::Keyboard::Enter: { spawnSpecial(m_player); break; }
			case sf::Keyboard::Delete: {toDelete = true; break; }
			case sf::Keyboard::Insert: {insert = true; break; }
			case sf::Keyboard::Add: { changeAsset = true; break; }
			case sf::Keyboard::Dash:  { toggleWeaponLeft(); m_player->getComponent<CInventory>()->canToggle = false; break; }
			case sf::Keyboard::Equal: { toggleWeaponRight(); m_player->getComponent<CInventory>()->canToggle = false; break; }
			}
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:       { pInput->up = false; break; }
                case sf::Keyboard::A:       { pInput->left = false; break; }
                case sf::Keyboard::S:       { pInput->down = false; break; }
                case sf::Keyboard::D:       { pInput->right = false; break; }
                case sf::Keyboard::Space:   { pInput->shoot = false; pInput->canShoot = true; break; }
				case sf::Keyboard::Delete:  {toDelete = false; break; }
				case sf::Keyboard::Insert:  {insert = false; break; }
				case sf::Keyboard::Add:		{ changeAsset = false; break; }
				case sf::Keyboard::Equal:   { m_player->getComponent<CInventory>()->canToggle = true; break; }
				case sf::Keyboard::Dash:    { m_player->getComponent<CInventory>()->canToggle = true; break; }
            }
        }

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				mouse_x = event.mouseButton.x;
				mouse_y = event.mouseButton.y;
				clicked = !clicked;
				//sDrag();

			}
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{

		}
    }
}

void GameState_Play::sRender()
{
    m_game.window().clear(sf::Color(255, 192, 122));
	sf::View playerView;

	playerView.setSize(1280, 768);

	float playerPos_X = m_player->getComponent<CTransform>()->pos.x;
	float playerPos_Y = m_player->getComponent<CTransform>()->pos.y;

	//Set the camera to follow player's position 
	if (m_follow)
	{
		playerView.setCenter(sf::Vector2f(playerPos_X, playerPos_Y));
	}
	//Set camera to overview the room 
	else if (!m_follow)
	{
		int rx = floorf(m_player->getComponent<CTransform>()->pos.x / 1280) * 1280;
		int ry = floorf(m_player->getComponent<CTransform>()->pos.y / 768) * 768;

		playerView.reset(sf::FloatRect(rx, ry, 1280, 768));
	}

	m_game.window().setView(playerView);
        
    //Draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            auto transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto animation = e->getComponent<CAnimation>()->animation;
                animation.getSprite().setRotation(transform->angle);
                animation.getSprite().setPosition(transform->pos.x, transform->pos.y);
                animation.getSprite().setScale(transform->scale.x, transform->scale.y);
                m_game.window().draw(animation.getSprite());
            }
        }
    }

    //Draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto box = e->getComponent<CBoundingBox>();
                auto transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box->size.x-1, box->size.y-1));
                rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
                rect.setPosition(transform->pos.x, transform->pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));

                if (box->blockMove && box->blockVision)  { rect.setOutlineColor(sf::Color::Black); }
                if (box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::Blue); }
                if (!box->blockMove && box->blockVision) { rect.setOutlineColor(sf::Color::Red); }
                if (!box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::White); }
                rect.setOutlineThickness(1);
                m_game.window().draw(rect);
            }

            if (e->hasComponent<CPatrol>())
            {
                auto & patrol = e->getComponent<CPatrol>()->positions;
                for (size_t p = 0; p < patrol.size(); p++)
                {
                    dot.setPosition(patrol[p].x, patrol[p].y);
                    m_game.window().draw(dot);
                }
            }

            if (e->hasComponent<CFollowPlayer>())
            {
                sf::VertexArray lines(sf::LinesStrip, 2);
                lines[0].position.x = e->getComponent<CTransform>()->pos.x;
                lines[0].position.y = e->getComponent<CTransform>()->pos.y;
                lines[0].color = sf::Color::Black;
                lines[1].position.x = m_player->getComponent<CTransform>()->pos.x;
                lines[1].position.y = m_player->getComponent<CTransform>()->pos.y;
                lines[1].color = sf::Color::Black;
                m_game.window().draw(lines);
                dot.setPosition(e->getComponent<CFollowPlayer>()->home.x, e->getComponent<CFollowPlayer>()->home.y);
                m_game.window().draw(dot);
            }
        }
    }



    m_game.window().display();
}