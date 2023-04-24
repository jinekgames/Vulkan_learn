#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <app.h>
#include <logs.h>

#include <vector>

int main() {

    auto result = App::Inst().Run();

    if (!CHECK_RESULT(result)) {
        PRINT_E("Failed to run app. Code: %d", result);
        return result;
    }

    PRINT("Finished");
    return result;
}
