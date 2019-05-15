#include "GameState_Menu.h"
#include "GameState_Play.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"

GameState_Menu::GameState_Menu(GameEngine & game)
    : GameState(game)
{
    init("");
}

sf::Sprite background;
sf::Texture backgroundTex;
void GameState_Menu::init(const std::string & menuConfig)
{
	if (!backgroundTex.loadFromFile("images/main_menu_bg.png"))
	{
		std::cout << "Error loading background";
	}
	background.setTexture(backgroundTex);

    m_menuStrings.push_back("Level 1");

    m_levelPaths.push_back("level1.txt");

    m_menuText.setFont(m_game.getAssets().getFont("Mana"));
    m_menuText.setCharacterSize(64);

	//Entity to handle background music and fade out transition
	m_transition = m_entityManager.addEntity("transition");
	m_transition->addComponent<CMusic>(m_game.getAssets().getMusic("MainMenu"), true, true);
	auto size = m_game.window().getSize();
	m_transition->addComponent<CRectangleShape>(Vec2(size.x, size.y), Vec2(0, 0), sf::Color(0, 0, 0, 0));
}

void GameState_Menu::update()
{
	//Play music when state is in focus
	if (m_transition->getComponent<CMusic>()->m_music->getStatus() == sf::SoundSource::Status::Stopped)
	{
		m_transition->getComponent<CMusic>()->m_music->setVolume(100.0);
		m_transition->getComponent<CMusic>()->m_music->play();
	}
    m_entityManager.update();

    sUserInput();
    sRender();
	sTransition();
}

bool startTransition = false;
void GameState_Menu::sUserInput()
{
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
                case sf::Keyboard::Escape: 
                { 
                    m_game.quit(); 
                    break; 
                }
                case sf::Keyboard::W: 
                {
                    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
                    else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
                    break;
                }
                case sf::Keyboard::S: 
                { 
                    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size(); 
                    break; 
                }
                case sf::Keyboard::D: 
                { 
					//Begin transitioning to next game state
					startTransition = true;
                    break; 
                }
            }
        }
    }

}

void GameState_Menu::sRender()
{
    // clear the window to a blue
    m_game.window().setView(m_game.window().getDefaultView());
    m_game.window().clear(sf::Color(0, 0, 0));

	m_game.window().draw(background);

    m_menuText.setCharacterSize(48);

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color(100, 100, 100));
        m_menuText.setPosition(sf::Vector2f(10, 110 + i * 72));
        m_game.window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color(100, 100, 100));
    m_menuText.setString("up: w     down: s    play: d      back: esc");
    m_menuText.setPosition(sf::Vector2f(10, 730));
    m_game.window().draw(m_menuText);

	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CRectangleShape>())
		{
			m_game.window().draw(e->getComponent<CRectangleShape>()->shape);
		}
	}

    m_game.window().display();
}

void GameState_Menu::sTransition()
{
	if (startTransition)
	{
		//Begin fade out
		if (!m_transition->hasComponent<CLifeSpan>())
		{
			m_transition->addComponent<CLifeSpan>(2000);
		}

		auto entityClock = m_transition->getComponent<CLifeSpan>()->clock;
		auto entityLifespan = m_transition->getComponent<CLifeSpan>()->lifespan;

		float ratio = entityClock.getElapsedTime().asMilliseconds() / (float)entityLifespan;

		//Check fade out progress
		if (ratio >= 1.0)
		{
			//Push next state, reset transition entity
			m_game.pushState(std::make_shared<GameState_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
			m_transition->getComponent<CMusic>()->m_music->stop();
			m_transition->getComponent<CRectangleShape>()->shape.setFillColor(sf::Color(0, 0, 0, 0));
			m_transition->removeComponent<CLifeSpan>();
			startTransition = false;
		}
		else
		{
			//Incrementally turn down volume and turn up black screen alpha
			m_transition->getComponent<CMusic>()->m_music->setVolume(100.0 - 100.0 * ratio);
			m_transition->getComponent<CRectangleShape>()->shape.setFillColor(sf::Color(0, 0, 0, 255 * ratio));
		}
	}
}