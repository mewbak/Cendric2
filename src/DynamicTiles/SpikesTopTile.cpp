#include "DynamicTiles/SpikesTopTile.h"
#include "Spell.h"

void SpikesTopTile::init() {
	setPositionOffset(sf::Vector2f(-5.f, -20.f));
	setSpriteOffset(sf::Vector2f(-5.f, -20.f));
	setBoundingBox(sf::FloatRect(0.f, 0.f, 40.f, 65.f));
}

void SpikesTopTile::load(int skinNr) {

	Animation idleAnimation(sf::seconds(10.0f));
	idleAnimation.setSpriteSheet(g_resourceManager->getTexture(ResourceID::Texture_tile_spikestop));
	idleAnimation.addFrame(sf::IntRect(0, (skinNr - 1) * m_tileSize.y * 2, m_tileSize.x, m_tileSize.y * 2));

	addAnimation(GameObjectState::Idle, idleAnimation);

	// initial values
	setState(GameObjectState::Idle);
	playCurrentAnimation(false);
}

void SpikesTopTile::update(const sf::Time& frameTime) {
	DynamicTile::update(frameTime);
	if (m_damageCooldown > sf::Time::Zero) {
		m_damageCooldown = m_damageCooldown - frameTime;
		if (m_damageCooldown < sf::Time::Zero) {
			m_damageCooldown = sf::Time::Zero;
		}
	}
}

void SpikesTopTile::onHit(LevelMovableGameObject* mob) {
	if (m_damageCooldown == sf::Time::Zero) {
		mob->addDamage(DMG_PER_S);
		m_damageCooldown = sf::seconds(1);
	}
}
