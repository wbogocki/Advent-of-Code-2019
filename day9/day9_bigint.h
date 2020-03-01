/*
    ** Advent of Code 2019 **
    ** Day 9 Part 1 & 2 **
    ** Wojciech Bogócki **
    ** 9-10 Dec 2019 Taipei **
*/

#ifndef BIGINT_H
#define BIGINT_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct bigint
{
    int Digits[128];
    int Length;
    bool IsNegative;
};

typedef struct bigint bigint;

bigint BigIntMake(long long Value);
bigint BigIntParse(const char* Value);
long long BigIntValue(bigint BigInt);
void BigIntPrint(bigint BigInt);
void BigIntPrintln(bigint BigInt);
int BigIntSprint(char* Buffer, bigint BigInt);
bigint BigIntAdd(bigint A, bigint B);
bigint BigIntMul(bigint A, bigint B);
bool BigIntLessThan(bigint A, bigint B);
bool BigIntLessThanUnsigned(bigint A, bigint B);
bool BigIntEqual(bigint A, bigint B);
bigint BigIntNormalize(bigint BigInt); // internal

#endif

#ifdef BIGINT_IMPL

bigint BigIntMake(long long Value)
{
    bigint Result = {0};
    long long AbsoluteValue = Value < 0 ? -Value : Value;
    long long Power = 1;
    while(Power > 0 && Power <= AbsoluteValue)
    {
        Result.Digits[Result.Length++] = AbsoluteValue / Power % 10;
        Power *= 10;
    }
    Result.IsNegative = Value < 0;
    Result = BigIntNormalize(Result);
    return Result;
}

bigint BigIntParse(const char* Value)
{
    bigint Result = {0};
    for(int Index = strlen(Value)-1; Index >= 0; --Index)
    {
        char Digit = Value[Index];
        if(Digit == '-')
        {
            Result.IsNegative = true;
            break;
        }
        switch(Digit)
        {
            case '0': Result.Digits[Result.Length++] = 0; break;
            case '1': Result.Digits[Result.Length++] = 1; break;
            case '2': Result.Digits[Result.Length++] = 2; break;
            case '3': Result.Digits[Result.Length++] = 3; break;
            case '4': Result.Digits[Result.Length++] = 4; break;
            case '5': Result.Digits[Result.Length++] = 5; break;
            case '6': Result.Digits[Result.Length++] = 6; break;
            case '7': Result.Digits[Result.Length++] = 7; break;
            case '8': Result.Digits[Result.Length++] = 8; break;
            case '9': Result.Digits[Result.Length++] = 9; break;
        }
    }
    Result = BigIntNormalize(Result);
    return Result;
}

long long BigIntValue(bigint BigInt)
{
    long long Value = 0;
    long long Power = 1;
    for(int DecimalPlace = 0; DecimalPlace < BigInt.Length; ++DecimalPlace)
    {
        if(Power <= 0)
        {
            printf("Big integer does not fit in a long long: ");
            BigIntPrint(BigInt);
            printf("\n");
            exit(1);
        }
        Value += BigInt.Digits[DecimalPlace] * Power;
        Power *= 10;
    }
    if(BigInt.IsNegative)
    {
        Value *= -1;
    }
    return Value;
}

void BigIntPrint(bigint BigInt)
{
    if(BigInt.IsNegative) putchar('-');
    if(BigInt.Length == 0) putchar('0');
    else for(int Index = BigInt.Length-1; Index >= 0; --Index)
    {
        switch(BigInt.Digits[Index])
        {
            case 0: putchar('0'); break;
            case 1: putchar('1'); break;
            case 2: putchar('2'); break;
            case 3: putchar('3'); break;
            case 4: putchar('4'); break;
            case 5: putchar('5'); break;
            case 6: putchar('6'); break;
            case 7: putchar('7'); break;
            case 8: putchar('8'); break;
            case 9: putchar('9'); break;
        }
    }
}

void BigIntPrintln(bigint BigInt)
{
    BigIntPrint(BigInt);
    putchar('\n');
}

int BigIntSprint(char* Buffer, bigint BigInt)
{
    int Count = 0;
    if(BigInt.IsNegative) Buffer[Count++] = '-';
    if(BigInt.Length == 0) Buffer[Count++] = '0';
    else for(int Index = BigInt.Length-1; Index >= 0; --Index)
    {
        switch(BigInt.Digits[Index])
        {
            case 0: Buffer[Count++] = '0'; break;
            case 1: Buffer[Count++] = '1'; break;
            case 2: Buffer[Count++] = '2'; break;
            case 3: Buffer[Count++] = '3'; break;
            case 4: Buffer[Count++] = '4'; break;
            case 5: Buffer[Count++] = '5'; break;
            case 6: Buffer[Count++] = '6'; break;
            case 7: Buffer[Count++] = '7'; break;
            case 8: Buffer[Count++] = '8'; break;
            case 9: Buffer[Count++] = '9'; break;
        }
    }
    Buffer[Count++] = '\0';
    return Count;
}

bigint BigIntAdd(bigint A, bigint B)
{
    bool DifferentSign = A.IsNegative != B.IsNegative;
    if(DifferentSign && BigIntLessThanUnsigned(A, B))
    {
        bigint T;
        memcpy(&T, &A, sizeof(bigint));
        memcpy(&A, &B, sizeof(bigint));
        memcpy(&B, &T, sizeof(bigint));
    }
    bigint Result = {0};
    if(!DifferentSign)
    {
        int Carry = 0;
        int DecimalPlace = 0;
        while(Carry != 0 || DecimalPlace < A.Length || DecimalPlace < B.Length)
        {
            int Sum = A.Digits[DecimalPlace] + B.Digits[DecimalPlace] + Carry;
            int Digit = Sum % 10;
            Carry = Sum / 10;
            Result.Digits[DecimalPlace] = Digit;
            ++DecimalPlace;
        }
        Result.Length = DecimalPlace;
        Result.IsNegative = A.IsNegative;
    }
    else
    {
        int Borrow = 0;
        int DecimalPlace = 0;
        while(Borrow != 0 || DecimalPlace < A.Length || DecimalPlace < B.Length)
        {
            int Difference = A.Digits[DecimalPlace] - B.Digits[DecimalPlace] - Borrow;
            int Digit;
            if(Difference < 0)
            {
                Borrow = 1;
                Digit = Difference + 10;
            }
            else
            {
                Borrow = 0;
                Digit = Difference;
            }
            Result.Digits[DecimalPlace] = Digit;
            ++DecimalPlace;
        }
        Result.Length = DecimalPlace;
        Result.IsNegative = A.IsNegative;
    }
    Result = BigIntNormalize(Result);
    return Result;
}

bigint BigIntMul(bigint A, bigint B)
{
    bigint Result = {0};
    for(int DecimalPlaceB = 0; DecimalPlaceB < B.Length; ++DecimalPlaceB)
    {
        bigint Subresult = {0};
        int Carry = 0;
        int DecimalPlaceA = 0;
        while(Carry != 0 || DecimalPlaceA < A.Length)
        {
            int Product = A.Digits[DecimalPlaceA] * B.Digits[DecimalPlaceB] + Carry;
            int Digit = Product % 10;
            Carry = Product / 10;
            Subresult.Digits[DecimalPlaceA + DecimalPlaceB] = Digit;
            ++DecimalPlaceA;
        }
        Subresult.Length = DecimalPlaceA + DecimalPlaceB;
        Result = BigIntAdd(Result, Subresult);
    }
    Result.IsNegative = A.IsNegative != B.IsNegative;
    Result = BigIntNormalize(Result);
    return Result;
}

bool BigIntLessThan(bigint A, bigint B)
{
    if(A.IsNegative)
    {
        if(B.IsNegative) // A negative and B negative
        {
            return BigIntLessThanUnsigned(B, A);
        }
        else // A negative and B positive
        {
            return true;
        }
    }
    else
    {
        if(B.IsNegative) // A positive and B negative
        {
            return false;
        }
        else // A positive and B positive
        {
            return BigIntLessThanUnsigned(A, B);
        }
    }
}

bool BigIntLessThanUnsigned(bigint A, bigint B)
{
    if(A.Length == B.Length)
    {
        for(int DecimalPlace = A.Length-1; DecimalPlace >= 0; --DecimalPlace)
        {
            if(A.Digits[DecimalPlace] < B.Digits[DecimalPlace]) return true;
            if(A.Digits[DecimalPlace] > B.Digits[DecimalPlace]) return false;
        }
        return false;
    }
    else
    {
        return A.Length < B.Length;
    }
}

bool BigIntEqual(bigint A, bigint B)
{
    if(A.Length != B.Length || A.IsNegative != B.IsNegative)
    {
        return false;
    }
    else
    {
        for(int DecimalPlace = 0; DecimalPlace < A.Length; ++DecimalPlace)
        {
            if(A.Digits[DecimalPlace] != B.Digits[DecimalPlace])
            {
                return false;
            }
        }
        return true;
    }
}

bigint BigIntNormalize(bigint BigInt)
{
    bigint Result = {0};
    memcpy(&Result, &BigInt, sizeof(bigint));
    while(Result.Length > 0 && Result.Digits[Result.Length-1] == 0)
    {
        --Result.Length;
    }
    if(Result.Length == 0)
    {
        Result.IsNegative = false;
    }
    return Result;
}

#endif
