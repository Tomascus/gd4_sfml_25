#include "world.hpp"
#include "sprite_node.hpp"
#include <iostream>
#include "state.hpp"
#include <SFML/System/Angle.hpp>

World::World(sf::RenderWindow& window, FontHolder* font)
	: m_window(window)
	, m_camera(window.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_scene_graph()
	, m_scene_layers()
	, m_world_bounds(sf::Vector2f(0.f, 0.f), sf::Vector2f(m_camera.getSize().x, 1000.f))
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.size.y - m_camera.getSize().y/2.f)
	, m_scroll_speed(-100.f)
	, m_player_aircraft(nullptr)
{
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
}

void World::Update(sf::Time dt)
{
	//Scroll the world
	m_camera.move(sf::Vector2f(0, m_scroll_speed * dt.asSeconds()));

	m_player_aircraft->SetVelocity(0.f, 0.f);

	//Process commands from the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scene_graph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	SpawnEnemies();

	m_scene_graph.Update(dt);
	AdaptPlayerPosition();
}



void World::Draw()
{
	m_window.setView(m_camera);
	m_window.draw(m_scene_graph);
}



CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

void World::LoadTextures()
{
	m_textures.Load(TextureID::kEagle, "Media/Textures/Eagle.png");
	m_textures.Load(TextureID::kRaptor, "Media/Textures/Raptor.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Desert.png");
}

void World::BuildScene()
{
	//Initialise the different layers
	for (int i = 0; i < static_cast<int>(SceneLayers::kLayerCount); i++)
	{
		SceneNode::Ptr layer(new SceneNode());
		m_scene_layers[i] = layer.get();
		m_scene_graph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(TextureID::kLandscape);
	sf::IntRect textureRect(m_world_bounds);
	texture.setRepeated(true);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(sf::Vector2f(0, 0));
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//Homework add the player's aircraft
	//Add two Raptor escort planes that are 50 units behind the plane and 80 units either side of the player's plane
	std::unique_ptr<Aircraft> leader(new Aircraft(AircraftType::kEagle, m_textures, m_fonts));
	m_player_aircraft = leader.get();
	std::cout << m_spawn_position.x << m_spawn_position.y << std::endl;
	m_player_aircraft->setPosition(m_spawn_position);
	m_player_aircraft->SetVelocity(40.f, m_scroll_speed);
	m_scene_layers[static_cast<int>(SceneLayers::kAir)]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> left_escort(new Aircraft(AircraftType::kRaptor, m_textures, m_fonts));
	left_escort->setPosition(sf::Vector2f(- 80.f, 50.f));
	m_player_aircraft->AttachChild(std::move(left_escort));

	std::unique_ptr<Aircraft> right_escort(new Aircraft(AircraftType::kRaptor, m_textures, m_fonts));
	right_escort->setPosition(sf::Vector2f(80.f, 50.f));
	m_player_aircraft->AttachChild(std::move(right_escort));

	AddEnemies();
}

void World::AdaptPlayerVelocity()
{
	sf::Vector2f velocity = m_player_aircraft->GetVelocity();

	//If they are moving diagonally divide by sqrt 2
	if (velocity.x != 0.f && velocity.y != 0.f)
	{
		m_player_aircraft->SetVelocity(velocity / std::sqrt(2.f));
	}
	//Add scrolling velocity
	m_player_aircraft->Accelerate(0.f, m_scroll_speed);
}

void World::AdaptPlayerPosition()
{
	//keep player on the screen
	sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	const float border_distance = 40.f;

	sf::Vector2f position = m_player_aircraft->getPosition();
	position.x = std::min(position.x, view_bounds.size.x - border_distance);
	position.x = std::max(position.x, border_distance);
	position.y = std::min(position.y, view_bounds.position.y + view_bounds.size.y - border_distance);
	position.y = std::max(position.y, view_bounds.position.y + border_distance);
	m_player_aircraft->setPosition(position);

}

void World::SpawnEnemies()
{
	//Spawn an enemy when it is relevent i.e in BattlefieldBounds
	while (!m_enemy_spawn_points.empty() && m_enemy_spawn_points.back().m_y > GetBattleFieldBounds().position.y)
	{
		SpawnPoint spawn = m_enemy_spawn_points.back();
		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.m_type, m_textures, m_fonts));
		enemy->setPosition(sf::Vector2f(spawn.m_x, spawn.m_y));
		enemy->setRotation(sf::degrees(180.f));
		m_scene_layers[static_cast<int>(SceneLayers::kAir)]->AttachChild(std::move(enemy));
		m_enemy_spawn_points.pop_back();
	}
}

void World::AddEnemies()
{
	AddEnemy(AircraftType::kRaptor, 0.f, 500.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 1000.f);
	AddEnemy(AircraftType::kRaptor, 100.f, 1100.f);
	AddEnemy(AircraftType::kRaptor, -100.f, 1100.f);
	AddEnemy(AircraftType::kRaptor, -70.f, 1400.f);
	AddEnemy(AircraftType::kRaptor, 70.f, 1400.f);
	AddEnemy(AircraftType::kRaptor, 70.f, 1600.f);

	//Sort the enemies according to y-value
	std::sort(m_enemy_spawn_points.begin(), m_enemy_spawn_points.end(), [](SpawnPoint lhs, SpawnPoint rhs)
		{
			return lhs.m_y < rhs.m_y;
		});
}

void World::AddEnemy(AircraftType type, float relx, float rely)
{
	SpawnPoint spawn(type, m_spawn_position.x + relx, m_spawn_position.y - rely);
	m_enemy_spawn_points.emplace_back(spawn);
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());;
}

sf::FloatRect World::GetBattleFieldBounds() const
{
	//Return camera bounds + a small area off screen where the enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.position.y -= 100.f;
	bounds.size.y += 100.f;
	return bounds;
}
