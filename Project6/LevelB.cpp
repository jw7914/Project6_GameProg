#include "LevelB.h"
#include "Utility.h"
#include <cstdlib>
#include <ctime>

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char ENEMY1_FILEPATH[]       = "vampire.png";
constexpr char ENEMY2_FILEPATH[]       = "skeleton.png";
constexpr char ENEMY3_FILEPATH[]       = "eyeball.png";
constexpr char PROJECTILE_FILEPATH[]       = "arrow.png";
bool levelB_played = false;



unsigned int LEVELB_DATA[] =
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

LevelB::~LevelB()
{
    delete [] m_game_state.enemies;
    delete [] m_game_state.hearts;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete [] m_game_state.background;
    delete [] m_game_state.player_projectiles;
    Mix_FreeChunk(m_game_state.levelclear_sfx);
    Mix_FreeChunk(m_game_state.enemydeath_sfx);
    Mix_FreeChunk(m_game_state.playerdeath_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::reset()
{
    m_game_state.player->set_position(glm::vec3(1.0f, -3.0f, 0.0f));
    
    for (int i = 0; i < PROJECTILE_COUNT; i++) {
        m_game_state.player_projectiles[i].set_position(m_game_state.player->get_position());
        m_game_state.player_projectiles[i].deactivate();
    }
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
   
    int multiplierX = 2;
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        float randomY = -2.0f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (1.0f - (-2.0f));
        m_game_state.enemies[i].set_position(glm::vec3(10.0f + (i * multiplierX), randomY - 4, 0.0f));
        m_game_state.enemies[i].activate();
    }
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("new_tilemap.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 20, 9);
    
    // Code from main.cpp's initialise()
    std::vector<GLuint> player_texture_ids = {
    };

    std::vector<std::vector<int>> player_animations = {
        {1, 1, 1},       // IDLE animation frames
        {2, 2, 2}     // ATTACK animations
    };
    
    glm::vec3 acceleration = glm::vec3(0.0f, -20.0f, 0.0f);
//    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

        
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
    GLuint arrow_texture_id = Utility::load_texture(PROJECTILE_FILEPATH);
    m_game_state.player->set_jumping_power(3.0f);
    m_game_state.num_player_projectiles = PROJECTILE_COUNT;
    m_game_state.player_projectiles = new Entity[PROJECTILE_COUNT];
    for (int i = 0; i < PROJECTILE_COUNT; i++) {
        m_game_state.player_projectiles[i] =  Entity();
        m_game_state.player_projectiles[i].set_texture_id(arrow_texture_id);
        m_game_state.player_projectiles[i].set_entity_type(PROJECTILE);
        m_game_state.player_projectiles[i].set_scale(glm::vec3(0.5f,0.5f,0.0f));
        m_game_state.player_projectiles[i].set_width(0.5f);
        m_game_state.player_projectiles[i].set_height(0.5f);
        m_game_state.player_projectiles[i].set_position(m_game_state.player->get_position());
        m_game_state.player_projectiles[i].deactivate();
        m_game_state.player_projectiles[i].set_speed(1.0f);
        m_game_state.player_projectiles[i].set_movement(glm::vec3(0.0f));
    }
    
    /**
    Enemies' stuff */
    std::vector<GLuint> enemy_texture_ids = {
        Utility::load_texture(ENEMY1_FILEPATH),
        Utility::load_texture(ENEMY2_FILEPATH),
        Utility::load_texture(ENEMY3_FILEPATH)};

    m_game_state.enemies = new Entity[ENEMY_COUNT];
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
   
    int multiplierX = 2;
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] =  Entity(enemy_texture_ids[i % 3], 0.0f, 1.0f, 1.0f, ENEMY, MEDIUM);
        m_game_state.enemies[i].set_scale(glm::vec3(1.0f,1.0f,0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].activate();
        m_game_state.enemies[i].set_entity_type(ENEMY);
        m_game_state.enemies[i].set_speed(1.0f);
        m_game_state.enemies[i].set_ai_type(MEDIUM);
        float randomY = -2.0f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (1.0f - (-2.0f));
        m_game_state.enemies[i].set_position(glm::vec3(10.0f + (i * multiplierX), randomY - 4, 0.0f));
    }
    
    GLuint heart_texture_id = Utility::load_texture("heart.png");
    m_game_state.hearts = new Entity[3];
    for (int i = 0; i < 3; i++) {
        m_game_state.hearts[i] = Entity();
        m_game_state.hearts[i].set_texture_id(heart_texture_id);
        m_game_state.hearts[i].update(0.0f, NULL, NULL, 0, NULL);
        m_game_state.hearts[i].set_scale(glm::vec3(0.5f,0.5f,0.0f));
    }
    
    m_game_state.background = new Entity[1];
    GLuint background1_texture_id = Utility::load_texture("castle_background.png");
    m_game_state.background[0] = Entity();
    m_game_state.background[0].set_texture_id(background1_texture_id);
    m_game_state.background[0].set_scale(glm::vec3(10.0f, 8.0f, 0.0f));
    m_game_state.background[0].set_position(glm::vec3(5.0f, -4.0f, 0.0f));
    m_game_state.background[0].update(0.0f, NULL, NULL, 0, NULL);

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("galactic.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);
    
    m_game_state.levelclear_sfx = Mix_LoadWAV("/Users/jasonwu/Desktop/Coding/CompSciClasses/Game_Programming/Project6_GameProg/Project6/level-completed-230568.wav");
    m_game_state.enemydeath_sfx = Mix_LoadWAV("/Users/jasonwu/Desktop/Coding/CompSciClasses/Game_Programming/Project6_GameProg/Project6/8-bit-power-down-2-shortened.wav");
    m_game_state.playerdeath_sfx = Mix_LoadWAV("/Users/jasonwu/Desktop/Coding/CompSciClasses/Game_Programming/Project6_GameProg/Project6/you-died.wav");
}

void LevelB::update(float delta_time)
{
    int player_collision = m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    //Death
    if (player_collision == 1) {
        m_game_state.death = true;
        if (m_game_state.death) {
            m_game_state.lives -= 1;
            m_game_state.death = false;
            m_game_state.reset = true;
            Mix_PlayChannel(-1, m_game_state.playerdeath_sfx, 0);
        }
        if (m_game_state.lives == 0) {
            m_game_state.lose = true;
        }
    }
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (m_game_state.enemies[i].get_position().x < 0.0) {
            m_game_state.death = true;
        }
        if (m_game_state.death) {
            m_game_state.lives -= 1;
            m_game_state.death = false;
            m_game_state.reset = true;
            Mix_PlayChannel(-1, m_game_state.playerdeath_sfx, 0);
        }
        if (m_game_state.lives == 0) {
            m_game_state.lose = true;
        }
    }
    
    if (m_game_state.reset) {
        reset();
        m_game_state.reset = false;
    }
    
    
    for (int i = 0; i < 3; i++) {
        float spacing = 0.5f;
        m_game_state.hearts[i].set_position(glm::vec3((8.5f + i * spacing), -0.25f, 0.0f));
        m_game_state.hearts[i].update(0.0f, NULL, NULL, 0, NULL);
    }
    
    
    for (int i = 0; i < PROJECTILE_COUNT; i++) {
        if (m_game_state.player_projectiles[i].isActive()){
            if (m_game_state.player_projectiles[i].update(delta_time, NULL, m_game_state.enemies, ENEMY_COUNT, NULL) == 5) {
                Mix_PlayChannel(-1, m_game_state.enemydeath_sfx, 0);
            }
        }
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (m_game_state.enemies[i].isActive() && !m_game_state.lose){
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, NULL);
        }
    }

    
    
    
    if (m_game_state.player->get_position().x > 10.0f) m_game_state.next_scene_id = 1;
}

void LevelB::render(ShaderProgram *program)
{
    for(int i = 0; i < 1; i++) {
        m_game_state.background[i].render(program);
    }
    int num_active = ENEMY_COUNT;
    m_game_state.map->render(program);
    for (int i = 0; i < m_game_state.lives; i++) {
        m_game_state.hearts[i].render(program);
    }
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (m_game_state.enemies[i].isActive()){
            if(!m_game_state.lose){
                m_game_state.enemies[i].render(program);
            }
        }
        else {
            num_active -= 1;
        }
    }
    
    for (int i = 0; i < PROJECTILE_COUNT; i++) {
        if (m_game_state.player_projectiles[i].isActive()){
            m_game_state.player_projectiles[i].render(program);
        }
    }

    
    if (m_game_state.lose) {
        GLuint g_font_texture_id = Utility::load_texture("font1.png");
        Utility::draw_text(program, g_font_texture_id, "Player Lose", 0.5f, 0.05f,
              glm::vec3(2.0f,-1.0f,0.0f));
        Utility::draw_text(program, g_font_texture_id, "Press r to retry", 0.5f, 0.05f,
              glm::vec3(1.0f,-2.0f,0.0f));

    }
    if (num_active == 0) {
        GLuint g_font_texture_id = Utility::load_texture("font1.png");
        Utility::draw_text(program, g_font_texture_id, "Level Clear", 0.5f, 0.05f,
              glm::vec3(2.0f,-2.0f,0.0f));
        m_game_state.won = true;
        if (!levelB_played){
            Mix_PlayChannel(-1, m_game_state.levelclear_sfx, 0);
            levelB_played = true;
        }
    }
    
    m_game_state.player->render(program);
}
