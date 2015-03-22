#include "stdafx.h"

MenuScreen::MenuScreen()
{
	cout << "Init MenuScreen...";
	if (!m_screenTexture.loadFromFile("res/screens/screen_menu.png"))
	{
		printf("SplashScreen: Error at opening file res/screens/screen_menu.png \n");
		// TODO: what to do if resource loading fails in constructor?
	}
	m_screenSprite = sf::Sprite(m_screenTexture);
	cout << "...done." << endl;
}

Screen* MenuScreen::update(sf::Time frameTime)
{
	if (Keyboard::isKeyPressed(Keyboard::Unknown) || Mouse::isButtonPressed(Mouse::Button::Left))
	{
		return new GameScreen();
	}
	return this;
}

void MenuScreen::render(sf::RenderTarget &renderTarget) const
{
	renderTarget.draw(m_screenSprite);
}