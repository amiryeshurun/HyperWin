#include <utils/utils.h>
#include <intrinsics.h>
#include <debug.h>
#include <vmm/msr.h>

VOID CopyMemory(OUT BYTE_PTR dest, IN BYTE_PTR src, IN QWORD count)
{
    __movsb(dest, src, count);
}

CHAR ConvertHalfByteToHexChar(IN BYTE halfByte)
{
    CHAR ch;

    switch(halfByte)
    {
        case 0:
            ch = '0';
            break;
        case 1:
            ch = '1';
            break;
        case 2:
            ch = '2';
            break;
        case 3:
            ch = '3';
            break;
        case 4:
            ch = '4';
            break;
        case 5:
            ch = '5';
            break;
        case 6:
            ch = '6';
            break;
        case 7:
            ch = '7';
            break;
        case 8:
            ch = '8';
            break;
        case 9:
            ch = '9';
            break;
        case 10:
            ch = 'A';
            break;
        case 11:
            ch = 'B';
            break;
        case 12:
            ch = 'C';
            break;
        case 13:
            ch = 'D';
            break;
        case 14:
            ch = 'E';
            break;
        case 15:
            ch = 'F';
            break;
        default:
            ASSERT(FALSE);
    }

    return ch;
}

QWORD pow(IN QWORD base, IN QWORD power)
{
    QWORD res;

    for(res = 1; power; res *= base, --power);
    return res;
}

QWORD NumberOfDigits(IN QWORD number)
{
    QWORD digits;
    
    digits = 0;
    do
    {
        number /= 10;
        digits++;
    } while(number);
    return digits;
}

QWORD StringLength(IN PCHAR str)
{
    QWORD length;
    
    length = 0;
    while(*(str++)) 
        length++;
    return length;
}

INT CompareMemory(IN BYTE_PTR buff1, IN BYTE_PTR buff2, IN QWORD length)
{
    for(QWORD i = 0; i < length; i++)
    {
        if(buff1[i] == buff2[i])
            continue;
        if(buff1[i] < buff2[i])
            return -1;
        else
            return 1;
    }
    return 0;
}

VOID SetMemory(IN BYTE_PTR base, IN BYTE value, IN QWORD length)
{
    for(; length; length--, *base++ = value);
}

VOID DumpHostStack(IN QWORD_PTR stackAddress)
{
    for(QWORD offset = 0; offset < 200; offset++)
    {
        if(offset != 0 && !(offset % 8))
            Print("\n");
        Print("%8 ", *(stackAddress + offset));
    }
    Print("\n");
}

BOOL IsMsrValid(IN QWORD msrNumber, IN BYTE_PTR msrRange)
{
    BOOL result;
    
    result = (msrNumber >= 0 && msrNumber <= 0x1fff) || (msrNumber >= 0xc0000000 && msrNumber <= 0xc0001fff);
    if(result)
        *msrRange = (msrNumber >= 0 && msrNumber <= 0x1fff) ? MSR_RANGE_FIRST : MSR_RANGE_SECOND;
    return result;
}

QWORD SumDigits(IN QWORD num)
{
    QWORD sum;
    
    sum = 0;
    while(num)
    {
        sum += num % 10;
        num /= 10;
    }
    return sum;
}

QWORD MemoryContains(IN BYTE_PTR buff1, IN QWORD size1, IN BYTE_PTR buff2, IN QWORD size2)
{
    for(QWORD i = 0; i < size1 - size2; i++)
        if(!CompareMemory(buff1 + i, buff2, size2))
            return i;
    return IDX_NOT_FOUND;
}