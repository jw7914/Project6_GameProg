#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 20;
    int PROJECTILE_COUNT = 2;
    
    ~LevelC();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void reset() override;

};
