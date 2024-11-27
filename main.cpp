#define GL_SILENCE_DEPRECATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 1
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Menu.h"

enum AppStatus { RUNNING, TERMINATED };

bool can_win = false;

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "Hello, Maps!";

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char IDLE_FILEPATH[] = "assets/images/yoshi_idle.png",
WALKING_FILEPATH[] = "assets/images/yoshi_walk.png",
PADAWON_IDLE_FILEPATH[] = "assets/images/padawon_idle.png",
PADAWON_WALKING_FILEPATH[] = "assets/images/padawon_walking.png",
PADAWON_DEATH_FILEPATH[] = "assets/images/padawon_death.png",
MAP_TILESET_FILEPATH[] = "assets/images/redtileset.png",
BGM_FILEPATH[] = "assets/audio/yoshi_island.mp3",
JUMP_SFX_FILEPATH[] = "assets/audio/bounce.wav",
BACKGROUND_FILEPATH[] = "assets/images/scifi wallpaper.png";

constexpr int CD_QUAL_FREQ = 44100,
AUDIO_CHAN_AMT = 2,
AUDIO_BUFF_SIZE = 4096;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

Scene* g_curr_scene = nullptr;
LevelA* g_levelA = nullptr;
LevelB* g_levelB = nullptr;
LevelC* g_levelC = nullptr;
Menu* g_menu = nullptr;

Scene* g_levels[4];


SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

GLint fontsheet_texture_id;

Mix_Music* g_music;

float g_previous_ticks = 0.0f,
g_accumulator = 0.0f;

void switch_to_scene(Scene* scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

void switch_to_scene(Scene* scene) {
    g_curr_scene = scene;
    g_curr_scene->initialise();
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // BGM

    Mix_OpenAudio(CD_QUAL_FREQ, MIX_DEFAULT_FORMAT, AUDIO_CHAN_AMT, AUDIO_BUFF_SIZE);

    g_music = Mix_LoadMUS(BGM_FILEPATH);

    Mix_PlayMusic(g_music, -1);

    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);

    // Scene Setup

    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    g_menu = new Menu();

    g_levels[0] = g_levelA;
    g_levels[1] = g_levelB;
    g_levels[2] = g_levelC;
    g_levels[3] = g_menu;

    switch_to_scene(g_levels[3]);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Font
    fontsheet_texture_id = Utility::load_texture("assets/fonts/font1.png");
}

void process_input()
{
    g_curr_scene->get_state().player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_SPACE:
                // Jump
                if (g_curr_scene->get_state().player->get_collided_bottom())
                {
                    g_curr_scene->get_state().player->jump();
                    //Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            case SDLK_RETURN:
                if (g_curr_scene == g_levels[3]) {
                    switch_to_scene(g_levels[0]);
                }

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    g_curr_scene->get_state().player->set_player_state(STAND);
    if (key_state[SDL_SCANCODE_A])       g_curr_scene->get_state().player->move_left(), g_curr_scene->get_state().player->set_player_state(WALK), g_curr_scene->get_state().player->set_scale(glm::vec3(1.0f, 1.25f, 0.0f));
    else if (key_state[SDL_SCANCODE_D]) g_curr_scene->get_state().player->move_right(), g_curr_scene->get_state().player->set_player_state(WALK), g_curr_scene->get_state().player->set_scale(glm::vec3(-1.0f, 1.25f, 0.0f));

    if (key_state[SDL_SCANCODE_K]) { g_curr_scene->get_state().enemies[0].set_ai_state(DYING), g_curr_scene->get_state().enemies[0].set_scale(glm::vec3(1.4f, 1.0f, 0.0f)); }

    if (glm::length(g_curr_scene->get_state().player->get_movement()) > 1.0f)
        g_curr_scene->get_state().player->normalise_movement();
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_curr_scene->update(FIXED_TIMESTEP);

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    g_view_matrix = glm::mat4(1.0f);

    // Camera Follows the player
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_curr_scene->get_state().player->get_position().x, -g_curr_scene->get_state().player->get_position().y, 0.0f));

    if (g_curr_scene == g_levels[0] || g_curr_scene == g_levels[3] || g_curr_scene == g_levels[2]) {
        g_curr_scene->get_state().backgrounds->set_position(glm::vec3(g_curr_scene->get_state().player->get_position().x + (3.5 - g_curr_scene->get_state().player->get_position().x) / 7, -2.0f, 0.0f));
    }

    g_curr_scene->get_state().backgrounds->update(0.0f, g_curr_scene->get_state().player, NULL, 0, g_curr_scene->get_state().map);
    

    if (g_curr_scene->get_state().next_scene_id >= 0) {
        if (g_curr_scene->get_state().next_scene_id == 2) {
            can_win = true;
        }
        int lives = g_curr_scene->get_state().player->get_lives();
        switch_to_scene(g_levels[g_curr_scene->get_state().next_scene_id]);
        g_curr_scene->get_state().player->set_lives(lives);
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_curr_scene->render(&g_shader_program);

    if (g_curr_scene == g_levels[3]) {
        Utility::draw_text(&g_shader_program, fontsheet_texture_id, "Press Enter", 0.7f, 0.0001f, glm::vec3(g_curr_scene->get_state().player->get_position().x - 3.0f, g_curr_scene->get_state().player->get_position().y, 0.0f));
        Utility::draw_text(&g_shader_program, fontsheet_texture_id, "to start", 0.7f, 0.0001f, glm::vec3(g_curr_scene->get_state().player->get_position().x - 2.0f, g_curr_scene->get_state().player->get_position().y - 1.0f, 0.0f));
    }

    if (g_curr_scene->get_state().player->get_lose()) Utility::draw_text(&g_shader_program, fontsheet_texture_id, "You Lose", 0.7f, 0.01f, glm::vec3(g_curr_scene->get_state().player->get_position().x - 2.0f, g_curr_scene->get_state().player->get_position().y + 3.0f, 0.0f));
    if (can_win) {
        if (g_curr_scene->get_state().player->get_win()) Utility::draw_text(&g_shader_program, fontsheet_texture_id, "You Win", 0.7f, 0.01f, glm::vec3(g_curr_scene->get_state().player->get_position().x - 2.0f, g_curr_scene->get_state().player->get_position().y + 3.0f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_levelA;

    //Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_music);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}