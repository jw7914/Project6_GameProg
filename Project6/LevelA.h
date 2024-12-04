#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 10;
    int PROJECTILE_COUNT = 3;
    
    ~LevelA();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void reset() override;
};
