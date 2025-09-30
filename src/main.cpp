#include "Application.hpp"

#include <cstdlib>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Application app;

    if (!app.Initialize()) {
        return EXIT_FAILURE;
    }

    app.Run();

    return EXIT_SUCCESS;
}
