#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 20;
    int PROJECTILE_COUNT = 3;

    
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void reset() override;

};
