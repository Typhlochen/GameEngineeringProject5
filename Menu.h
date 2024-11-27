#include "Scene.h"
class Menu : public Scene {
public:
    static const int LEVEL_WIDTH = 14;
    static const int LEVEL_HEIGHT = 5;
    ~Menu();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};