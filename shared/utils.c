#include <utils.h>
#include <intrinsics.h>
#include <debug.h>


VOID CopyMemory(OUT QWORD_PTR dest, IN QWORD_PTR src, IN QWORD count)
{
    __movsb((BYTE_PTR)dest, (BYTE_PTR)src, count);
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
    QWORD res = 1;
    for(; power; res *= base, --power);
    return res;
}

QWORD NumberOfDigits(IN QWORD number)
{
    QWORD digits = 0;
    do
    {
        number /= 10;
        digits++;
    } while(number);
    return digits;
}

QWORD StringLength(IN PCHAR str)
{
    QWORD length = 0;
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