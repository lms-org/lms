#include <core/framework.h>
#include <cstdlib>
/**
 * @brief framework
 * @return
 * you can have a custom framework if you extend it and set it here
 */
int main (int argc, char* const*argv) {
    Framework framework(argc, argv);
    return EXIT_SUCCESS;
}
