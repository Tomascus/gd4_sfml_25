#pragma once
#include "entity.hpp"
#include "pickup_type.hpp"
#include "resource_identifiers.hpp"
#include "aircraft.hpp"

class Pickup : public Entity
{
	Pickup(PickupType type, const TextureHolder& textures);
	virtual unsigned int GetCategory() const override;
	void Apply(Aircraft& player);
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	PickupType m_type;
	sf::Sprite m_sprite;
};

