#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 50;
    int PROJECTILE_COUNT = 5;
    
    ~LevelA();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void reset() override;
};
