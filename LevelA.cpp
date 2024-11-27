#include "LevelA.h"
#include "Utility.h"
#include <vector>
#define LEVEL_HEIGHT 7
unsigned int LEVELA_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 6, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 14, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 8,
    2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};
LevelA::~LevelA()
{
    delete m_game_state.player;
    delete m_game_state.map;
    delete[] m_game_state.enemies;

    Mix_FreeChunk(m_game_state.kill_sfx);
}
void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;

    // MAP
    GLuint map_texture_id = Utility::load_texture("assets/images/levelA_tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 8, 3);

    // Player
    std::vector<std::vector<int>> animations =
    {
        {0, 1, 2, 3, 4, 5, 6}, // IDLE
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9} // WALKING
    };
    glm::vec3 acceleration = glm::vec3(0.0f, -9.81f, 0.0f);
    std::vector<GLuint> texture_ids =
    {
        Utility::load_texture("assets/images/yoshi_idle.png"),
        Utility::load_texture("assets/images/yoshi_walk.png")
    };
    m_game_state.player = new Entity(
        texture_ids,               // texture id
        2.5f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        animations,                // animation index sets
        0.0f,                      // animation time
        7,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        7,                         // animation row amount
        1.0f,                      // width
        1.25f,                      // height
        PLAYER,                    // entity type
        STAND                      // player state
    );

    m_game_state.player->set_scale(glm::vec3(-1.0f, 1.25f, 0.0f));
    m_game_state.player->set_position(glm::vec3(1.0f, -3.0f, 0.0f));

    // Background
    std::vector<GLuint> background_texture_ids = { Utility::load_texture("assets/images/wallpaper.png")};

    m_game_state.backgrounds = new Entity();
    m_game_state.backgrounds->set_texture_id(background_texture_ids);
    m_game_state.backgrounds->set_player_state(STAND);
    m_game_state.backgrounds->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.backgrounds->set_width(10.0f);
    m_game_state.backgrounds->set_height(6.0f);
    m_game_state.backgrounds->set_scale(glm::vec3(15.0f, 11.5f, 0.0f));
    m_game_state.backgrounds->set_entity_type(BACKGROUND);

    // Enemies
    std::vector<GLuint> enemy_texture_ids = {
        Utility::load_texture("assets/images/goomba_walk.png"),
        Utility::load_texture("assets/images/goomba_walk.png"),
        Utility::load_texture("assets/images/goomba_walk.png")
    };

    std::vector<std::vector<int>> padawon_animations = {
        {0, 1},
        {0, 1},
        {0, 1}
    };

    m_game_state.enemies = new Entity[3];

    m_game_state.enemies[0] = Entity(
        enemy_texture_ids,               // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
        padawon_animations,                // animation index sets
        0.0f,                      // animation time
        2,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        2,                         // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        ENEMY,                    // entity type
        STAND                      // player state
    );

    m_game_state.enemies[1] = Entity(
        enemy_texture_ids,               // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
        padawon_animations,                // animation index sets
        0.0f,                      // animation time
        2,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        2,                         // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        ENEMY,                    // entity type
        STAND                      // player state
    );

    m_game_state.enemies[2] = Entity(
        enemy_texture_ids,               // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        2.0f,                      // jumping power
        padawon_animations,                // animation index sets
        0.0f,                      // animation time
        2,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        2,                         // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        ENEMY,                    // entity type
        STAND                      // player state
    );

    for (int i = 0; i < 3; i++) {
        m_game_state.enemies[i].set_ai_type(GUARD);
        m_game_state.enemies[i].set_ai_state(IDLE);
        m_game_state.enemies[i].set_position(glm::vec3(i * 2 + 4, -2, 0));
    }

    m_game_state.enemies[0].set_ai_type(STARE);

    m_game_state.enemies[1].set_ai_type(STARE);
    m_game_state.enemies[1].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    m_game_state.enemies[1].set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_scale(glm::vec3(-1.0f, 1.0f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(12.0f, 0.0f, 0.0f));

    m_game_state.enemies[2].set_ai_state(WALKING);
    m_game_state.enemies[2].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    m_game_state.enemies[2].set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
    m_game_state.enemies[2].set_position(glm::vec3(10.0f, -3.0f, 0.0f));

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.kill_sfx = Mix_LoadWAV("/assets/audio/yoshi_name.mp3");
}
void LevelA::update(float delta_time)
{
    if (m_game_state.player->get_kill_sound()) {
        Mix_PlayChannel(-1, m_game_state.kill_sfx, 0);
        m_game_state.player->set_kill_sound(false);
    }

    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, 3,
        m_game_state.map);
    for (int i = 0; i < 3; i++) { m_game_state.enemies[i].update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map); }
    if (m_game_state.player->get_win()) m_game_state.next_scene_id = 1;
}
void LevelA::render(ShaderProgram* program)
{
    m_game_state.backgrounds->render(program);
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < 3; i++) { m_game_state.enemies[i].render(program); }
}