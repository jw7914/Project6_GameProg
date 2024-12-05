/**
* Author: Jason Wu
* Assignment: Halloween Defense
* Date due: December 6th, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEFT_EDGE 5.0f
#define RIGHT_EDGE 5.0f


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
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
#include "Title.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 800;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
                F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };
// ––––– GLOBAL VARIABLES ––––– //

Scene  *g_current_scene;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;
Title *title_screen;

Scene   *g_levels[4];

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Project 6",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    title_screen = new Title();

    
    g_levels[0] = g_levelA;
    g_levels[1] = g_levelB;
    g_levels[2] = g_levelC;
    g_levels[3] = title_screen;

    
    // Start at level A
    switch_to_scene(g_levels[3]);
    
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    if (g_current_scene != g_levels[3]){
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
        g_current_scene->get_state().player->set_animation_state(DEFAULT);
    }
    
    SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                    // End game
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
                            
                        case SDLK_RETURN:
                            if (g_current_scene == g_levels[3])
                                switch_to_scene(g_levels[0]);
                            break;
                        
                        case SDLK_a:
                            if (!g_current_scene->get_state().lose){
                                g_current_scene->get_state().player->set_animation_state(DEFAULT);
                                g_current_scene->get_state().player->set_animation_frames(0);
                                glm::vec3 player_pos = g_current_scene->get_state().player->get_position();
                                for(int i = 0; i < g_current_scene->get_state().num_player_projectiles; i++) {
                                    if (!g_current_scene->get_state().player_projectiles[i].isActive()){
                                        g_current_scene->get_state().player_projectiles[i].set_position(glm::vec3(player_pos.x + 0.5f, player_pos.y, 0.0f));
                                        g_current_scene->get_state().player_projectiles[i].activate();
                                        break;
                                    }
                                }
                            }
                            break;
                        case SDLK_r:
                            if (g_current_scene->get_state().lose) {
                                //Reset all variables in all levels
                                for (int i = 0; i < 3; i ++) {
                                    g_levels[i]->set_win(false);
                                    g_levels[i]->set_lose(false);
                                    
                                }
                                switch_to_scene(g_levelA);
                                g_current_scene->set_number_of_lives(3);
                            }
                            break;
                            
                            
                        default:
                            break;
                    }
                    
                default:
                    break;
            }
        }
        
    
        const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    for(int i = 0; i < g_current_scene->get_state().num_player_projectiles; i++) {
//        std::cout << i << ": " << g_current_scene->get_state().player_projectiles[i].isActive() << std::endl;

    }
    if (!g_current_scene->get_state().lose){
        if (g_current_scene->get_state().won) {
            if (key_state[SDL_SCANCODE_LEFT]) {
                if (g_current_scene->get_state().player->get_position().x > 0.0) {
                    g_current_scene->get_state().player->move_left();
                }
            }
            else if (key_state[SDL_SCANCODE_RIGHT])  g_current_scene->get_state().player->move_right();
        }
        
        if (key_state[SDL_SCANCODE_UP]) {
            if (g_current_scene->get_state().player->get_position().y < -0.75) {
                g_current_scene->get_state().player->move_up();
            }
        }
        else if (key_state[SDL_SCANCODE_DOWN]) {
            if (g_current_scene->get_state().player->get_position().y > -7.0) {
                g_current_scene->get_state().player->move_down();
            }
        }
        
        
        if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
    else {
        g_current_scene->get_state().player->set_animation_state(DEFAULT);
    }
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
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);

//        if (g_is_colliding_bottom == false && g_current_scene->get_state().player->get_collided_bottom()) g_effects->start(SHAKE, 1.0f);
        
        g_is_colliding_bottom = g_current_scene->get_state().player->get_collided_bottom();
        if (g_current_scene->get_state().player->get_position().y <= -7.0) {
            g_current_scene->get_state().player->set_acceleration(glm::vec3(0.0f,0.0f,0.f));
        }
        else {
            g_current_scene->get_state().player->set_acceleration(glm::vec3(0.0f, -20.0f, 0.0f));
        }
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    if (g_current_scene->get_state().player->get_position().x >= LEFT_EDGE && g_current_scene->get_state().player->get_position().x <= RIGHT_EDGE) {
        // Player is within bounds
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
    } else if (g_current_scene->get_state().player->get_position().x > RIGHT_EDGE) {
        // Player is beyond the RIGHT_EDGE
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-RIGHT_EDGE, 3.75, 0));
    } else if (g_current_scene->get_state().player->get_position().x < LEFT_EDGE) {
        // Player is beyond the LEFT_EDGE
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-LEFT_EDGE, 3.75, 0));
    }

   
    
    if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().x > 10.0f) {
        int currLives = g_current_scene->get_state().lives;
        switch_to_scene(g_levelB);
        g_current_scene->set_number_of_lives(currLives);
    }
    if (g_current_scene == g_levelB && g_current_scene->get_state().player->get_position().x > 10.0f){
        int currLives = g_current_scene->get_state().lives;
        switch_to_scene(g_levelC);
        g_current_scene->set_number_of_lives(currLives);
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
 
    glUseProgram(g_shader_program.get_program_id());
    g_current_scene->render(&g_shader_program);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete title_screen;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
        
        render();
    }
    
    
    shutdown();
    return 0;
}
