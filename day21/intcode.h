#ifndef __INTCODE_H__
#define __INTCODE_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE (16 * 1024)

// Intcode value
typedef long long icv;

// OP codes
typedef enum
{
    OP_ADD = 1,
    OP_MUL = 2,
    OP_IN = 3,
    OP_OUT = 4,
    OP_JT = 5,  // Jump if true
    OP_JF = 6,  // Jump if false
    OP_TLT = 7, // Less than
    OP_TEQ = 8, // Equals
    OP_ARB = 9, // Adjust relative base
    OP_HLT = 99,
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

// Operation
typedef struct
{
    icv Op;
    icv Opcode;
    icv Pmodes; // Paramter modes
    icv Pord;   // Paramter counter used for parsing
} iop;

// Intcode computer
typedef struct
{
    icv Memory[MEMORY_SIZE];
    icv IP;    // Instruction pointer
    icv Rbase; // Relative base
    icv Int;   // Interrupt bits
    iop Op;    // Last operation (useful during interrupts)
    icv In;
    icv Out;
} computer;

icv LoadMemory(const char *Filename, icv Memory[MEMORY_SIZE]);
interrupt Run(computer *Computer);

// Implementation

#ifdef INTCODE_IMPL
#undef INTCODE_IMPL

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

//
// Memory
//

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

//
// Interrupts
//

interrupt SetInterrupt(computer *Computer, interrupt Interrupt)
{
    Computer->Int |= Interrupt;
    return Interrupt;
};

bool GetInterrupt(computer *Computer, interrupt Interrupt)
{
    return Computer->Int & Interrupt;
}

void ClearInterrupts(computer *Computer)
{
    Computer->Int = 0;
}

//
// Operations
//

typedef struct
{
    pmode Mode;
    icv Value;
} param;

icv Advance(computer *Computer)
{
    return Read(Computer, Computer->IP++);
}

// Get the next value from the program
void AdvanceOp(computer *Computer)
{
    icv Op = Advance(Computer);

    icv Opcode = Op % 100;
    icv Pmodes = Op / 100;

    Computer->Op = (iop){
        .Op = Op,
        .Opcode = Opcode,
        .Pmodes = Pmodes,
        .Pord = 0,
    };
}

// Advance and read an operation parameter
param AdvanceParam(computer *Computer)
{
    icv Modes = Computer->Op.Pmodes;
    icv Ordinal = Computer->Op.Pord++;

    assert(Ordinal < 3); // There are never more than 3 parameters
    int Pow10[] = {1, 10, 100};

    // Parameter mode
    pmode Mode = (Modes / Pow10[Ordinal]) % 10;

    // Parameter value
    int Value = Advance(Computer);

    return (param){
        .Mode = Mode,
        .Value = Value,
    };
}

// Load the next parameter
icv Load(computer *Computer)
{
    param Parameter = AdvanceParam(Computer);

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

// Store a value at a parameter
void Store(computer *Computer, icv Value)
{
    param Parameter = AdvanceParam(Computer);

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

//
// CPU
//

interrupt Run(computer *Computer)
{
    if (GetInterrupt(Computer, INT_HLT))
    {
        printf("Not running");
        exit(1);
    }

    if (GetInterrupt(Computer, INT_IN))
    {
        assert(Computer->Op.Opcode == OP_IN);
        Store(Computer, Computer->In);
    }

    ClearInterrupts(Computer);

    for (;;)
    {
        AdvanceOp(Computer);

        switch (Computer->Op.Opcode)
        {
        case OP_ADD:
        {
            Store(Computer, Load(Computer) + Load(Computer));
            break;
        }
        case OP_MUL:
        {
            Store(Computer, Load(Computer) * Load(Computer));
            break;
        }
        case OP_IN:
        {
            // We finish the operation after the interrupt is handled
            return SetInterrupt(Computer, INT_IN);
        }
        case OP_OUT:
        {
            Computer->Out = Load(Computer);
            return SetInterrupt(Computer, INT_OUT);
        }
        case OP_JT:
        {
            icv P1 = Load(Computer);
            icv P2 = Load(Computer);
            if (P1 != 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_JF:
        {
            icv P1 = Load(Computer);
            icv P2 = Load(Computer);
            if (P1 == 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_TLT:
        {
            icv P1 = Load(Computer);
            icv P2 = Load(Computer);
            Store(Computer, P1 < P2);
            break;
        }
        case OP_TEQ:
        {
            icv P1 = Load(Computer);
            icv P2 = Load(Computer);
            Store(Computer, P1 == P2);
            break;
        }
        case OP_ARB:
        {
            Computer->Rbase += Load(Computer);
            break;
        }
        case OP_HLT:
        {
            return SetInterrupt(Computer, INT_HLT);
        }
        default:
        {
            printf("Invalid opcode at address %lld: %lld\n", Computer->IP - 1, Computer->Op.Opcode);
            exit(1);
        }
        }
    }
}

#endif

#endif