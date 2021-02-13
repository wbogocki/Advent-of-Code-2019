#define INTCODE_IMPL

#include "intcode.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    icv Program[MEMORY_SIZE] = {0};
    LoadMemory("input.txt", Program);

    computer Computer = {0};
    memcpy(Computer.Memory, Program, sizeof(icv) * MEMORY_SIZE);

    /**
     * Strategy:
     * 1. If there is a hole at any of ABC; and
     * 2. If there is terrain at D (our would-be landing spot); and
     * 3. If there is terrain on either E or H (H is where we would have to jump next turn if E had a hole); then
     * 3. Jump; else, don't.
     **/

    size_t Head = 0;
    char Script[] =
        "NOT J J\n" // J = 1
        "AND A J\n" // J &&= A
        "AND B J\n" // J &&= B
        "AND C J\n" // J &&= C

        // J is 0 if there is a hole in A, B, or C
        "NOT J J\n" // J = !J
        "AND D J\n" // J &&= D

        "OR E T\n" // T <- ground on E
        "OR H T\n" // T <- ground on E or H (where we would be forced to jump next turn)

        "AND T J\n"
        "RUN\n";

    bool Done = false;
    while (!Done)
    {
        interrupt Interrupt = Run(&Computer);
        switch (Interrupt)
        {
        case INT_HLT:
        {
            Done = true;
            break;
        }
        case INT_IN:
        {
            assert(Head < sizeof(Script));
            Computer.In = Script[Head++];
            break;
        }
        case INT_OUT:
        {
            icv Out = Computer.Out;
            if (Out <= 127)
            {
                putchar(Computer.Out);
            }
            else
            {
                printf("Amount of damage to the hull: %lld\n", Out);
            }
            break;
        }
        default:
        {
            printf("Unhandled interrupt: %d\n", Interrupt);
            exit(1);
        }
        }
    }
}
