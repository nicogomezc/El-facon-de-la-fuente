#include "includes.h"

int main(void)
{
    echo_app_init();

    while (true)
    {
        echo_app_step();
    }
}
