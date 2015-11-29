#include "GUI/Slider.h"

using namespace std;

const float Slider::WIDTH = 300.f;
const float Slider::HEIGHT = 20.f;
const float Slider::MARGIN = 2.f;

Slider::Slider(int minPos, int maxPos) : GameObject() {
	if (minPos < 0) minPos = 0;
	if (maxPos < minPos) maxPos = minPos;
	m_minPosition = minPos;
	m_maxPosition = maxPos;
	setSliderPosition(m_minPosition);

	m_background.setSize(sf::Vector2f(WIDTH, HEIGHT));
	m_background.setFillColor(m_backgroundColor);
	
	m_filler.setSize(sf::Vector2f(WIDTH, HEIGHT));
	m_filler.setFillColor(m_fillColor);

	m_margin.setSize(sf::Vector2f(WIDTH, HEIGHT));
	m_margin.setFillColor(sf::Color::Transparent);
	m_margin.setOutlineThickness(-MARGIN);
	m_margin.setOutlineColor(m_marginColor);

	m_minText.setCharacterSize(12);
	m_minText.setString(to_string(m_minPosition));
	m_maxText.setCharacterSize(12);
	m_maxText.setString(to_string(m_maxPosition));

	setBoundingBox(m_background.getGlobalBounds());
	setInputInDefaultView(true);
}

int Slider::getSliderPosition() const {
	return m_sliderPosition;
}

void Slider::onLeftClick() {
	if (m_isEnabled && m_isPressed) {
		m_isPressed = false;
		g_inputController->lockAction();

		// update the position of the slider if it was clicked.
		sf::Vector2f mousePos = g_inputController->getDefaultViewMousePosition();
		int length = m_maxPosition - m_minPosition;
		if (mousePos.x > m_knob.getPosition().x) {
			setSliderPosition(m_sliderPosition + static_cast<int>(length / 10.f));
		}
		else {
			setSliderPosition(m_sliderPosition - static_cast<int>(length / 10.f));
		}
	}
}

void Slider::setSliderPosition(int pos) {
	if (pos < m_minPosition) pos = m_minPosition;
	if (pos > m_maxPosition) pos = m_maxPosition;

	m_sliderPosition = pos;

	// update knob
	int length = m_maxPosition - m_minPosition;
	float xPos = (float)m_sliderPosition / (float)length * WIDTH;
	m_knob.setPosition(sf::Vector2f(getBoundingBox()->left + xPos, getBoundingBox()->top + HEIGHT / 2.f));

	// update filler
	m_filler.setSize(sf::Vector2f(xPos, HEIGHT));

	// update text
	setTextRaw(m_title);
}

void Slider::onLeftJustPressed() {
	if (m_isEnabled) {
		m_isPressed = true;
		g_inputController->lockAction();
	}
}

void Slider::render(sf::RenderTarget& renderTarget) {
	if (!m_isVisible) return;
	renderTarget.draw(m_background);
	renderTarget.draw(m_filler);
	renderTarget.draw(m_margin);
	m_knob.render(renderTarget);
	renderTarget.draw(m_titleText);
	renderTarget.draw(m_minText);
	renderTarget.draw(m_maxText);
}

void Slider::handleDragAndDrop() {
	sf::Vector2f mousePos = g_inputController->getDefaultViewMousePosition();
	setSliderPosition(calculateSliderPosition(mousePos.x));
}

int Slider::calculateSliderPosition(float mousePosX) const {
	float xOffset = mousePosX - getBoundingBox()->left;
	// map the offset onto the values
	int length = m_maxPosition - m_minPosition;
	return static_cast<int>(length * xOffset / WIDTH);
}

void Slider::setPosition(const sf::Vector2f& pos) {
	GameObject::setPosition(pos);
	m_background.setPosition(pos);
	m_filler.setPosition(pos);
	m_margin.setPosition(pos);
	m_minText.setPosition(sf::Vector2f(
		pos.x, 
		pos.y + HEIGHT + 5.f));
	m_maxText.setPosition(sf::Vector2f(
		pos.x + WIDTH - m_maxText.getLocalBounds().width, 
		pos.y + HEIGHT + 5.f));
	setSliderPosition(m_sliderPosition);
}

void Slider::update(const sf::Time& frameTime) {
	if (!m_isVisible || !m_isEnabled) return;

	m_knob.update(frameTime);
	if (m_knob.isPressed()) {
		handleDragAndDrop();
	}
	GameObject::update(frameTime);
}

void Slider::setKnobColor(const sf::Color& color) {
	m_knob.setColor(color);
}

void Slider::setBackgroundColor(const sf::Color& color) {
	m_background.setFillColor(color);
	m_backgroundColor = color;
}

void Slider::setMarginColor(const sf::Color& color) {
	m_margin.setOutlineColor(color);
	m_marginColor = color;
}

void Slider::setTextColor(const sf::Color& color) {
	m_titleText.setColor(color);
	m_maxText.setColor(color);
	m_minText.setColor(color);
}

void Slider::setText(const std::string& text, const sf::Color& color, int charSize) {
	m_title = g_textProvider->getText(text);
	m_titleText = BitmapText(m_title + ": " + to_string(m_sliderPosition));

	m_titleText.setColor(color);
	setCharacterSize(charSize);
}

void Slider::setText(const std::string& text) {
	setText(text, sf::Color::White, 12);
}

void Slider::setText(const std::string& text, int charSize) {
	setText(text, sf::Color::White, charSize);
}

void Slider::setTextRaw(const std::string& text, const sf::Color& color, int charSize) {
	m_title = text;
	m_titleText = BitmapText(m_title + ": " + to_string(m_sliderPosition));

	m_titleText.setColor(color);
	setCharacterSize(charSize);
}

void Slider::setTextRaw(const std::string& text, int charSize) {
	setTextRaw(text, sf::Color::White, charSize);
}

void Slider::setTextRaw(const std::string& text) {
	setTextRaw(text, sf::Color::White, 12);
}

void Slider::setCharacterSize(int size) {
	m_titleText.setCharacterSize(size);
	float xOffset = WIDTH / 2.f - m_titleText.getLocalBounds().width / 2.f;
	float yOffset = -(10.f + size);
	m_textOffset = sf::Vector2f(xOffset, yOffset);
	m_titleText.setPosition(getPosition() + m_textOffset);
}

void Slider::setEnabled(bool enabled) {
	m_isEnabled = enabled;
	m_margin.setOutlineColor(sf::Color(
		m_margin.getOutlineColor().r,
		m_margin.getOutlineColor().g,
		m_margin.getOutlineColor().b,
		m_isEnabled ? 255 : 100));
	m_titleText.setColor(sf::Color(
		m_titleText.getColor().r,
		m_titleText.getColor().g,
		m_titleText.getColor().b,
		m_isEnabled ? 255 : 100));
}

void Slider::setVisible(bool value) {
	m_isVisible = value;
}

bool Slider::isEnabled() const {
	return m_isEnabled;
}

bool Slider::isVisible() const {
	return m_isVisible;
}

GameObjectType Slider::getConfiguredType() const {
	return GameObjectType::_Button;
}

// SLIDER KNOB

const float SliderKnob::RADIUS = 10.f;
const float SliderKnob::MARGIN = 2.f;

SliderKnob::SliderKnob() : GameObject() {
	m_knob.setRadius(RADIUS);
	m_knob.setOutlineThickness(-MARGIN);
	m_knob.setFillColor(m_fillColor);
	m_knob.setOutlineColor(m_marginColor);

	setBoundingBox(m_knob.getLocalBounds());
}

void SliderKnob::onLeftJustPressed() {
	m_isPressed = true;
	g_inputController->lockAction();
}

void SliderKnob::onLeftClick() {
	m_isPressed = false;
	g_inputController->lockAction();
}

bool SliderKnob::isPressed() const {
	return m_isPressed;
}

void SliderKnob::onMouseOver() {

}

void SliderKnob::render(sf::RenderTarget& renderTarget) {
	renderTarget.draw(m_knob);
}

void SliderKnob::update(const sf::Time& frameTime) {
	GameObject::update(frameTime);
	if (m_isPressed && !g_inputController->isMousePressedLeft()) {
		m_isPressed = false;
	}
}

void SliderKnob::setPosition(const sf::Vector2f& pos) {
	GameObject::setPosition(pos - sf::Vector2f(RADIUS, RADIUS));
	m_knob.setPosition(pos - sf::Vector2f(RADIUS, RADIUS));
}

const sf::Vector2f& SliderKnob::getPosition() const {
	return GameObject::getPosition() + sf::Vector2f(RADIUS, RADIUS);
}

void SliderKnob::setColor(const sf::Color& color) {
	m_knob.setFillColor(color);
	m_fillColor = color;
}

GameObjectType SliderKnob::getConfiguredType() const {
	return GameObjectType::_Button;
}