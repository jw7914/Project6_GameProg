#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Utility.h"
#include <random>


int Entity::projectile_activate(Entity *collideable_entities, int collidable_entity_count) {
    int flag = 0;
    int collsion_x = check_collision_x(collideable_entities, collidable_entity_count);
    int collsion_y = check_collision_y(collideable_entities, collidable_entity_count);

    if (get_position().x > 10.0f) {
        deactivate();
    }
    if (collsion_x != collidable_entity_count + 1 || collsion_y != collidable_entity_count + 1) {
        collideable_entities[collsion_x].deactivate();
        collideable_entities[collsion_y].deactivate();
        deactivate();
        flag = 1;
    }
    m_movement = glm::vec3(5.0f,0.0f,0.0f);
    return flag;
}

void Entity::ai_activate(Entity *player, float delta_time)
{
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 30);

    // Track scale state and timer
    static float scale_timer = 0.0f;
    static bool is_scaled_down = false;
    static const glm::vec3 original_scale(1.0f, 1.0f, 1.0f);

    if (m_ai_type == EASY) {
        float movement = -3.0f;
        m_movement = glm::vec3(movement, 0.0f, 0.0f);
        
        if (!is_scaled_down && dist(gen) == 10) {
            // Scale down
            m_scale = glm::vec3(0.5f, 0.5f, 0.0f);
            m_height *= 0.5f;
            m_width *= 0.5f;
            is_scaled_down = true;
            scale_timer = 0.0f;
        }

        if (is_scaled_down) {
            scale_timer += delta_time;
            if (scale_timer >= 2.0f) {
                m_scale = original_scale;
                m_height *= 2.0f;
                m_width *= 2.0f;
                is_scaled_down = false;
            }
        }
        if (m_collided_bottom) {
            set_jumping_power(5.0f);
        }
    }

    if (m_ai_type == MEDIUM) {
        m_theta += 1.0f * delta_time;
        float movement = glm::sin(m_theta) * 1.25f;
        m_movement = glm::vec3(-4.0f, movement, 0.0f);
        if (m_position.y <= -7.0f) {
            m_movement = glm::vec3(-4.0f, 1, 0.0f);
        }
        if (m_position.y >= -0.75) {
            m_movement = glm::vec3(-4.0f, -1, 0.0f);
        }
    }

    if (m_ai_type == HARD) {
        m_theta += 1.0f * delta_time;
        float movement = glm::sin(m_theta) * 1.25f;
        m_movement = glm::vec3(-4.0f, movement, 0.0f);

        if (m_position.y <= -7.0f) {
            m_movement = glm::vec3(-4.0f, 1, 0.0f);
        }
        if (m_position.y >= -0.75) {
            m_movement = glm::vec3(-4.0f, -1, 0.0f);
        }

        if (!is_scaled_down && dist(gen) == 10) {
            // Scale down
            m_scale = glm::vec3(0.5f, 0.5f, 0.0f);
            m_height *= 0.5f;
            m_width *= 0.5f;
            is_scaled_down = true;
            scale_timer = 0.0f;
        }

        if (is_scaled_down) {
            scale_timer += delta_time;
            if (scale_timer >= 2.0f) {
                m_scale = original_scale;
                m_height *= 2.0f;
                m_width *= 2.0f;
                is_scaled_down = false;
            }
        }
    }

    if (m_ai_type == IDLE) {
        float movement = -3.0f;
        m_movement = glm::vec3(movement, 0.0f, 0.0f);
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

// Parameterized constructor
Entity::Entity(std::vector<GLuint> texture_ids, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> animations, float animation_time, int animation_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType, Animation animation)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed),m_acceleration(acceleration), m_jumping_power(jump_power), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_texture_ids(texture_ids),  m_animations(animations), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType), m_current_animation(animation)
{
    set_animation_state(m_current_animation);
}


// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType): m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType), m_ai_type(AIType)
{
// Initialize m_walking with zeros or any default value
for (int i = 0; i < SECONDS_PER_FRAME; ++i)
    for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::~Entity() { }

void Entity::set_animation_state(Animation new_animation)
{
  
   // Update the current animation state
   m_current_animation = new_animation;

   // Update the texture and animation indices based on the current animation
   m_animation_indices = m_animations[m_current_animation].data();
   
   // Update the number of rows to match the new texture spritesheet
   m_animation_rows = static_cast<int>(m_animations[m_current_animation].size());
   m_animation_frames = static_cast<int>(m_animations[m_current_animation].size());
   
   // Scale and adjust position based on the new animation state
   switch (m_current_animation) {
       case DEFAULT:
           if (m_scale.x != m_scale.y) {
               m_scale.x = m_scale.y;
           }
           break;

       case ATTACK:
           if (m_scale.x < m_scale.y * 2.0) {
           }
           break;

       case DEATH:
           if (m_scale.x < m_scale.y * 1.5) {
               m_scale.x *= 1.5;
           }
           break;

       case RUN:
           if (m_scale.x < m_scale.y * 1.75) {
               m_scale.x *= 1.75;
           }
           break;

       default:
           break;
   }

}


void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program)
{
    m_texture_ids.clear();
    m_texture_ids = {
        Utility::load_texture("player.png"),   // IDLE spritesheet
        Utility::load_texture("player.png"),
        Utility::load_texture("player.png"),
    };
   GLuint current_texture = m_texture_ids[m_current_animation];  // Get the right texture

   float u_coord = (float) (m_animation_index % m_animation_cols) / (float) m_animation_cols;
   float v_coord = (float) (m_animation_index / m_animation_cols) / (float) m_animation_rows;

   float width = 1.0f / (float) m_animation_cols;
   float height = 1.0f / (float) m_animation_rows;

   float tex_coords[] =
   {
       u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
       u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
   };

   float vertices[] =
   {
       -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
       -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
   };

   glBindTexture(GL_TEXTURE_2D, current_texture);

   glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
   glEnableVertexAttribArray(program->get_position_attribute());

   glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
   glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDisableVertexAttribArray(program->get_position_attribute());
   glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

int const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity) && collidable_entity->isActive())
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y   -= y_overlap;
                m_velocity.y    = 0;
                
                // Collision!
                m_collided_top  = true;
                return i;
            } else if (m_velocity.y < 0)
            {
                m_position.y      += y_overlap;
                m_velocity.y       = 0;
                
                // Collision!
                m_collided_bottom  = true;
                return i;
            }
        }
    }
    return collidable_entity_count + 1;
}

int const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity) && collidable_entity->isActive())
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x > 0)
            {
                m_position.x     -= x_overlap;
                m_velocity.x      = 0;

                // Collision!
                m_collided_right  = true;
                return i;
                
            } else if (m_velocity.x < 0)
            {
                m_position.x    += x_overlap;
                m_velocity.x     = 0;
 
                // Collision!
                m_collided_left  = true;
                return i;
            }
        }
    }
    return collidable_entity_count + 1;

}

void const Entity::check_collision_y(Map *map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    
    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    
    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        
    }
}

void const Entity::check_collision_x(Map *map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left  = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}
int Entity::update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map)
{
    if (!m_is_active) return 0;
    
    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
        
    if (m_animation_indices != NULL)
    {
        m_animation_time += delta_time;
        float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
        
        if (m_animation_time >= frames_per_second)
        {
            m_animation_time = 0.0f;
            m_animation_index++;
            
            if (m_animation_index >= m_animation_frames)
            {
                if (m_current_animation == DEATH) {
                    m_animation_index = m_animation_frames - 1; // Play once
                }
                else if (m_current_animation == DAMAGE) {
                    m_animation_index = m_animation_frames - 2;
                }
                else {
                    m_animation_index = 0;
                }
            }
        }
    }
    
    m_velocity.x = m_movement.x * m_speed;
    m_velocity.y = m_movement.y * m_speed;
    m_velocity += m_acceleration * delta_time;
    
    m_position.y += m_velocity.y * delta_time;
    
    
    if (map != NULL){
        check_collision_y(map);
    }
    m_position.x += m_velocity.x * delta_time;
    if (map != NULL){
        check_collision_x(map);
    }
    
   
    if (m_entity_type == ENEMY) ai_activate(player, delta_time);
    else if (m_entity_type ==  PROJECTILE){
        if (projectile_activate(collidable_entities, collidable_entity_count) == 1) {
            return 5;
            
        }
    }
    else if (m_entity_type == PLAYER) {
        int collidedObjectY = check_collision_y(collidable_entities, collidable_entity_count);
        int collidedObjectX = check_collision_x(collidable_entities, collidable_entity_count);
        if ((collidedObjectX != collidable_entity_count + 1 || collidedObjectY != collidable_entity_count + 1) && m_current_animation == ATTACK) {
            collidable_entities[collidedObjectX].deactivate();
            collidable_entities[collidedObjectY].deactivate();
        }
        else if((collidedObjectX != collidable_entity_count + 1 || collidedObjectY != collidable_entity_count + 1) && m_current_animation != ATTACK) {
            return 1;
        }
    }
    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    return 0;
}


void Entity::render(ShaderProgram* program)
{
   program->set_model_matrix(m_model_matrix);
   if (m_animation_indices != nullptr)
   {
       draw_sprite_from_texture_atlas(program);
       return;
   }

    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
