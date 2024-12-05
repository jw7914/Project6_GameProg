#include "Title.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char ENEMY_FILEPATH[]       = "devil.png";


unsigned int TITLE_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


// All methods included due to functionality required for main for other levels, all entities will be deactiavted and not updated to reduce resource usage while on title screen


Title::~Title()
{
    delete [] m_game_state.enemies;
    delete [] m_game_state.hearts;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.background;
    Mix_FreeChunk(m_game_state.levelclear_sfx);
    Mix_FreeChunk(m_game_state.enemydeath_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Title::reset() {
    
}

void Title::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("new_tilemap.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, TITLE_DATA, map_texture_id, 1.0f, 20, 9);
    
    // Code from main.cpp's initialise()
    std::vector<GLuint> player_texture_ids = {
        Utility::load_texture("player.png"),   // IDLE spritesheet
    };

    std::vector<std::vector<int>> player_animations = {
        {1, 1, 1},       // IDLE animation frames
        {1, 1, 1}     // ATTACK animations
    };
    
//    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

        
    m_game_state.player =  new Entity(
                                      player_texture_ids,
                                      5.0f,
                                      acceleration,
                                      3.0f,
                                      player_animations,
                                      0.0f,
                                      1,
                                      0,
                                      3,
                                      3,
                                      0.75f,
                                      1.0f,
                                      PLAYER,
                                      DEFAULT
                                  );
        
    m_game_state.player->set_position(glm::vec3(1.0f, -3.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(3.0f);
    m_game_state.player->deactivate();
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 0.0f, 1.0f, 1.0f, ENEMY, EASY);
        m_game_state.enemies[i].set_scale(glm::vec3(1.0f,1.0f,0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        m_game_state.enemies[i].activate();
        m_game_state.enemies[i].set_entity_type(ENEMY);
        m_game_state.enemies[i].set_speed(1.0f);
        m_game_state.enemies[i].set_ai_type(EASY);
        m_game_state.enemies[i].set_position(glm::vec3(6.0f, -3.0f, 0.0f));
        m_game_state.enemies[i].deactivate();
    }
    
    GLuint heart_texture_id = Utility::load_texture("heart.png");
    m_game_state.hearts = new Entity[3];
    for (int i = 0; i < 3; i++) {
        m_game_state.hearts[i] = Entity();
        m_game_state.hearts[i].set_texture_id(heart_texture_id);
        m_game_state.hearts[i].update(0.0f, NULL, NULL, 0, NULL);
        m_game_state.hearts[i].set_scale(glm::vec3(0.5f,0.5f,0.0f));
        m_game_state.hearts[i].deactivate();
    }
    
    GLuint background_texture_id = Utility::load_texture("Background_0.png");
    m_game_state.background = new Entity;
    m_game_state.background->set_texture_id(background_texture_id);
    m_game_state.background->set_scale(glm::vec3(10.0f, 8.0f, 0.0f));
    m_game_state.background->set_position(glm::vec3(5.0f, -4.0f, 0.0f));
    m_game_state.background->update(0.0f, NULL, NULL, 0, NULL);

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("galactic.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);
    
    m_game_state.enemydeath_sfx = Mix_LoadWAV("Users/jasonwu/Desktop/Coding/CompSciClasses/Game_Programming/Project6_GameProg/Project6/victory-96688.wav");
}

void Title::update(float delta_time)
{
    int player_collision = m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    //Death
    if (player_collision == 1) {
        m_game_state.death = true;
        if (m_game_state.death) {
            m_game_state.lives -= 1;
            m_game_state.death = false;
            m_game_state.reset = true;
        }
        if (m_game_state.lives == 0) {
            m_game_state.lose = true;
        }
    }
    if (m_game_state.reset) {
        initialise();
        m_game_state.reset = false;
    }
    
    
    glm::vec3 player_pos = m_game_state.player->get_position();
    for (int i = 0; i < 3; i++) {
        float spacing = 0.5f;
        m_game_state.hearts[i].set_position(glm::vec3((player_pos.x + i * spacing) - 0.5f, player_pos.y + 1.0f, 0.0f));
        m_game_state.hearts[i].update(0.0f, NULL, NULL, 0, NULL);
    }


    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    
    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
}

void Title::render(ShaderProgram *program)
{
    m_game_state.background->render(program);
    GLuint g_font_texture_id = Utility::load_texture("font1.png");
    Utility::draw_text(program, g_font_texture_id, "Press return", 0.5f, 0.05f,
          glm::vec3(2.0f,-3.0f,0.0f));
    Utility::draw_text(program, g_font_texture_id, "to start", 0.5f, 0.05f,
          glm::vec3(2.0f,-4.0f,0.0f));
    Utility::draw_text(program, g_font_texture_id, "Halloween", 0.5f, 0.05f,
          glm::vec3(2.0f,-5.0f,0.0f));
    Utility::draw_text(program, g_font_texture_id, "Defense", 0.5f, 0.05f,
          glm::vec3(2.0f,-6.0f,0.0f));
    
}
