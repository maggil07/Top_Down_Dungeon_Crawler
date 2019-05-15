#pragma once

#include "Common.h"
#include <vector>

class Animation
{
    sf::Sprite  m_sprite;
    size_t      m_frameCount    = 1;    // total number of frames of animation
    size_t      m_currentFrame  = 0; // the current frame of animation being played
    size_t      m_speed         = 0; // the speed to play this animation
    Vec2        m_size          = { 1, 1 }; // size of the animation frame
    std::string m_name = "none";

public:

	static const std::string TILE_DUNGEON_FLOOR_1;
	static const std::string TILE_DUNGEON_FLOOR_2;
	static const std::string TILE_DUNGEON_FLOOR_2_BRICK;

    Animation();
    Animation(const std::string & name, const sf::Texture & t);
    Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed);
        
    void update();
    bool hasEnded() const;
    const std::string & getName() const;
    const Vec2 & getSize() const;
    sf::Sprite & getSprite();

	static bool isTileFloor(std::string tile);
};