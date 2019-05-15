#pragma once

#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

struct PlayerConfig 
{ 
    float X, Y, CX, CY, SPEED, HitPoints;
};

class GameState_Play : public GameState
{

protected:

    EntityManager           m_entityManager;
    std::shared_ptr<Entity> m_player;
	std::shared_ptr<Entity> m_bg_music;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_follow = false;
	bool					isMoving = false;
	bool					isDeleting = false;
	bool					drawGrid = false;
	bool					clicked = false;
	bool					toDelete = false;
	bool					insert = false;
	bool					changeAsset = false;
	int						mouse_y = 0;
	int                     mouse_x = 0;
	std::vector<std::string> loadedAnimations;
	void toggleWeaponLeft();
	void toggleWeaponRight();
    
    void init(const std::string & levelPath);

    void loadLevel(const std::string & filename);

    void update();
    void spawnPlayer();
    void spawnSword(std::shared_ptr<Entity> entity);
	void spawnSpecial(std::shared_ptr<Entity> entity);
	void spawnBow(std::shared_ptr<Entity> entity);
    
    void sMovement();
    void sAI();
    void sLifespan();
    void sUserInput();
    void sAnimation();
    void sCollision();
	void sDrag();
	void insertTile();
	void makeGrid();
	void saveLevel();
    void sRender();
	void deleteEntity();
	void changeAnimation();
	

public:

    GameState_Play(GameEngine & game, const std::string & levelPath);

};