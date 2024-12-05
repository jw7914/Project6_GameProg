#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 7;
    int PROJECTILE_COUNT = 2;

    
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void reset() override;

};
