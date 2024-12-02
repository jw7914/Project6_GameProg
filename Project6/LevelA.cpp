#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char ENEMY_FILEPATH[]       = "enemy1.png";


unsigned int LEVELA_DATA[] =
{
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 0, 0, 0, 80, 80, 80, 80, 80, 80, 0, 0,
    4, 0, 0, 0, 0, 80, 120, 120, 120, 120, 120, 120, 80, 0,
    4, 0, 0, 0, 80, 120, 120, 120, 120, 120, 120, 120, 120, 80,
    4, 80, 80, 80, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete [] m_game_state.hearts;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;
    
    
    GLuint map_texture_id = Utility::load_texture("new_tilemap.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 20, 9);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    std::vector<GLuint> cat_texture_ids = {
        Utility::load_texture("Meow-Knight_Idle.png"),   // IDLE spritesheet
        Utility::load_texture("Meow-Knight_Attack_3.png"),  // ATTACK spritesheet
        Utility::load_texture("Meow-Knight_Death.png"), // DEATH spritesheet
        Utility::load_texture("Meow-Knight_Dodge.png"), // RUN spritesheet
        Utility::load_texture("Meow-Knight_Take_Damage.png") // DAMAGE spritesheet
    };

    std::vector<std::vector<int>> cat_animations = {
        {0, 1, 2, 3, 4, 5},       // IDLE animation frames
        {0, 1, 2, 3},  // ATTACK animation frames
        {0, 1, 2, 3, 4, 5},       // DEATH animation frames
        {2, 2, 3, 3, 5, 5, 6, 6}, //RUN animation frames
        {0, 1, 2} //DAMAGE animation frames
    };
    
    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
        
    m_game_state.player =  new Entity(
                                      cat_texture_ids,
                                      5.0f,
                                      acceleration,
                                      3.0f,
                                      cat_animations,
                                      0.0f,
                                      3,
                                      0,
                                      1,
                                      3,
                                      0.75f,
                                      1.0f,
                                      PLAYER,
                                      DEFAULT
                                  );
        
    m_game_state.player->set_position(glm::vec3(1.0f, -3.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(3.0f);
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 0.0f, 1.0f, 1.0f, ENEMY, PATROL);
        m_game_state.enemies[i].set_scale(glm::vec3(1.0f,1.0f,0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        m_game_state.enemies[i].activate();
        m_game_state.enemies[i].set_entity_type(ENEMY);
        m_game_state.enemies[i].set_speed(1.0f);
        m_game_state.enemies[i].set_ai_type(PATROL);
        m_game_state.enemies[i].set_position(glm::vec3(6.0f, -3.0f, 0.0f));
    }
    
    GLuint heart_texture_id = Utility::load_texture("heart.png");
    m_game_state.hearts = new Entity[3];
    for (int i = 0; i < 3; i++) {
        m_game_state.hearts[i] = Entity();
        m_game_state.hearts[i].set_texture_id(heart_texture_id);
        m_game_state.hearts[i].update(0.0f, NULL, NULL, 0, NULL);
        m_game_state.hearts[i].set_scale(glm::vec3(0.5f,0.5f,0.0f));
    }

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("galactic.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);
    
    m_game_state.jump_sfx = Mix_LoadWAV("jump-3-236683.wav");
}

void LevelA::update(float delta_time)
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

void LevelA::render(ShaderProgram *program)
{
    int num_active = 1;
    m_game_state.map->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (m_game_state.enemies[i].isActive()){
            m_game_state.enemies[i].render(program);
        }
        else {
            num_active -= 1;
        }
    }
    for (int i = 0; i < m_game_state.lives; i++) {
        m_game_state.hearts[i].render(program);
    }
    
    if (m_game_state.lose) {
        GLuint g_font_texture_id = Utility::load_texture("font1.png");
        Utility::draw_text(program, g_font_texture_id, "Player Lose", 0.5f, 0.05f,
              glm::vec3(2.0f,-1.0f,0.0f));
    }
    if (num_active == 0) {
        GLuint g_font_texture_id = Utility::load_texture("font1.png");
        Utility::draw_text(program, g_font_texture_id, "Level Clear", 0.5f, 0.05f,
              glm::vec3(5.0f,-2.0f,0.0f));
    }
    
    m_game_state.player->render(program);
}
