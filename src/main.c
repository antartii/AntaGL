#include <unistd.h>

int main(const int argc, const char **argv, const char **env)
{
    write(1, "Hello world\n", 13);
    return 0;
}