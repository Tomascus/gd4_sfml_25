#include "pickup.hpp"
#include "data_tables.hpp"

namespace
{
    const std::vector<PickupData> Table = InitializePickupData();
}

Pickup::Pickup(PickupType type, const TextureHolder& textures)
{
}

unsigned int Pickup::GetCategory() const
{
    return 0;
}

void Pickup::Apply(Aircraft& player)
{
}

void Pickup::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
}
