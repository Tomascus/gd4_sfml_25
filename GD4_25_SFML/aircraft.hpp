#pragma once
#include "entity.hpp"
#include "aircraft_type.hpp"
#include "resource_identifiers.hpp"
#include "text_node.hpp"

class Aircraft : public Entity
{
public:
	Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts);
	unsigned int GetCategory() const override;

	void UpdateTexts();
	void UpdateMovementPattern(sf::Time dt);

	float GetMaxSpeed() const;

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	AircraftType m_type;
	sf::Sprite m_sprite;

	TextNode* m_health_display;
	float m_distance_travelled;
	int m_directions_index;

};

