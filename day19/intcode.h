#ifndef __INTCODE_H__
#define __INTCODE_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE 1024

// Intcode value
typedef long long icv;

// OP codes
typedef enum
{
    OP_ADD = 1,  // OK
    OP_MUL = 2,  // OK
    OP_IN = 3,   // OK
    OP_OUT = 4,  // OK
    OP_JT = 5,   // OK; Jump if true
    OP_JF = 6,   // OK; Jump if false
    OP_TLT = 7,  // OK; Less than
    OP_TEQ = 8,  // OK; Equals
    OP_ARB = 9,  // OK; Adjust relative base
    OP_HLT = 99, // OK
} opcode;

const char *OPCODES[] =
    {
        [OP_ADD] = "ADD",
        [OP_MUL] = "MUL",
        [OP_IN] = "IN",
        [OP_OUT] = "OUT",
        [OP_JT] = "JT",
        [OP_JF] = "JF",
        [OP_TLT] = "TLT",
        [OP_TEQ] = "TEQ",
        [OP_ARB] = "ARB",
        [OP_HLT] = "HLT",
};

// Parameter modes (encoded in opcodes)
typedef enum
{
    PMODE_POS = 0,
    PMODE_IMM = 1,
    PMODE_REL = 2,
} pmode;

// Interrupts (bit flags)
typedef enum
{
    INT_HLT = 1,
    INT_IN = 2,
    INT_OUT = 4,
} interrupt;

// Intcode computer
typedef struct
{
    icv Memory[MEMORY_SIZE];
    icv IP;    // Instruction pointer
    icv Rbase; // Relative base
    icv Int;   // Interrupt bits
    icv In;
    icv Out;
} computer;

icv LoadMemory(const char *Filename, icv Memory[MEMORY_SIZE]);
interrupt Run(computer *Computer);

// Implementation

#ifdef INTCODE_IMPL

icv LoadMemory(const char *Filename, icv Memory[MEMORY_SIZE])
{
    FILE *File = fopen(Filename, "r");
    if (!File)
    {
        puts("No program file\n");
        exit(1);
    }
    icv Size = 0;
    while (fscanf(File, "%lld,", &Memory[Size]) != EOF)
    {
        Size++;
    }
    fclose(File);
    return Size;
}

// Read value from memory
icv Read(computer *Computer, icv Address)
{
    assert(Address < MEMORY_SIZE - 1);
    return Computer->Memory[Address];
}

// Write value to memory
void Write(computer *Computer, icv Address, icv Value)
{
    assert(Address < MEMORY_SIZE - 1);
    Computer->Memory[Address] = Value;
}

// Get the next value from the program
icv Advance(computer *Computer)
{
    return Read(Computer, Computer->IP++);
}

void SetInterrupt(computer *Computer, interrupt Interrupt)
{
    Computer->Int |= Interrupt;
};

bool GetInterrupt(computer *Computer, interrupt Interrupt)
{
    return Computer->Int & Interrupt;
}

void ClearInterrupts(computer *Computer)
{
    Computer->Int = 0;
}

typedef struct
{
    pmode Mode;
    icv Value;
} param;

// Advance and read an operation parameter
param AdvanceParam(computer *Computer, icv Pmodes, icv Ordinal)
{
    assert(Ordinal < 3); // There are never more than 3 parameters
    int Pow10[] = {1, 10, 100};

    // Parameter mode
    pmode Mode = (Pmodes / Pow10[Ordinal]) % 10;

    // Parameter value
    int Value = Advance(Computer);

    return (param){
        .Mode = Mode,
        .Value = Value,
    };
}

// Read based on parameter value
icv ReadParam(computer *Computer, param Parameter)
{
    switch (Parameter.Mode)
    {
    case PMODE_POS:
    {
        return Read(Computer, Parameter.Value);
    }
    case PMODE_IMM:
    {
        return Parameter.Value;
    }
    case PMODE_REL:
    {
        return Read(Computer, Computer->Rbase + Parameter.Value);
    }
    default:
    {
        printf("Invalid parameter mode: %d\n", Parameter.Mode);
        exit(1);
    }
    }
}

// Write based on parameter value
void WriteParam(computer *Computer, param Parameter, icv Value)
{
    switch (Parameter.Mode)
    {
    case PMODE_POS:
    {
        Write(Computer, Parameter.Value, Value);
        break;
    }
    case PMODE_IMM:
    {
        printf("Writing on a parameter in immediate mode\n");
        exit(1);
    }
    case PMODE_REL:
    {
        Write(Computer, Computer->Rbase + Parameter.Value, Value);
        break;
    }
    default:
    {
        printf("Invalid parameter mode: %d\n", Parameter.Mode);
        exit(1);
    }
    }
}

interrupt Run(computer *Computer)
{
    if (GetInterrupt(Computer, INT_HLT))
    {
        printf("Not running");
        exit(1);
    }

    if (GetInterrupt(Computer, INT_IN))
    {
        // Retroactively execute OP_IN
        icv Op = Computer->Memory[Computer->IP - 1];
        icv Opcode = Op % 100;
        icv Pmodes = Op / 100;
        assert(Opcode == OP_IN);

        param P1 = AdvanceParam(Computer, Pmodes, 0);
        WriteParam(Computer, P1, Computer->In);
    }

    ClearInterrupts(Computer);

    for (;;)
    {
        icv Op = Advance(Computer);

        icv Opcode = Op % 100;
        icv Pmodes = Op / 100;

        switch (Opcode)
        {
        case OP_ADD:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            param P3 = AdvanceParam(Computer, Pmodes, 2);
            WriteParam(Computer, P3, ReadParam(Computer, P1) + ReadParam(Computer, P2));
            break;
        }
        case OP_MUL:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            param P3 = AdvanceParam(Computer, Pmodes, 2);
            WriteParam(Computer, P3, ReadParam(Computer, P1) * ReadParam(Computer, P2));
            break;
        }
        case OP_IN:
        {
            // We finish the operation after the interrupt is handled
            SetInterrupt(Computer, INT_IN);
            return INT_IN;
        }
        case OP_OUT:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            Computer->Out = ReadParam(Computer, P1);
            SetInterrupt(Computer, INT_OUT);
            return INT_OUT;
        }
        case OP_JT:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            if (ReadParam(Computer, P1) != 0)
            {
                Computer->IP = ReadParam(Computer, P2);
            }
            break;
        }
        case OP_JF:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            if (ReadParam(Computer, P1) == 0)
            {
                Computer->IP = ReadParam(Computer, P2);
            }
            break;
        }
        case OP_TLT:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            param P3 = AdvanceParam(Computer, Pmodes, 2);
            if (ReadParam(Computer, P1) < ReadParam(Computer, P2))
            {
                WriteParam(Computer, P3, 1);
            }
            else
            {
                WriteParam(Computer, P3, 0);
            }
            break;
        }
        case OP_TEQ:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            param P2 = AdvanceParam(Computer, Pmodes, 1);
            param P3 = AdvanceParam(Computer, Pmodes, 2);
            if (ReadParam(Computer, P1) == ReadParam(Computer, P2))
            {
                WriteParam(Computer, P3, 1);
            }
            else
            {
                WriteParam(Computer, P3, 0);
            }
            break;
        }
        case OP_ARB:
        {
            param P1 = AdvanceParam(Computer, Pmodes, 0);
            Computer->Rbase += ReadParam(Computer, P1);
            break;
        }
        case OP_HLT:
        {
            SetInterrupt(Computer, INT_HLT);
            return INT_HLT;
        }
        default:
        {
            printf("Invalid opcode at address %lld: %lld\n", Computer->IP - 1, Opcode);
            exit(1);
        }
        }
    }
}

#endif

#endif