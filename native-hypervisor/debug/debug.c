#include <debug.h>
#include <intrinsics.h>
#include <utils.h>


VOID PrintBuffer(IN PCHAR buffer, IN QWORD length)
{
    for(QWORD i = 0; i < length; i++)
        __outbyte(DEBUG_PORT, buffer[i]);
}

VOID PrintNullTerminatedBuffer(IN PCHAR buffer)
{
    for(; *buffer; buffer++)
        __outbyte(DEBUG_PORT, *buffer);
}

VOID Print(IN PCHAR fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    CHAR buffer[BUFF_MAX_SIZE] = { 0 };
    QWORD bufferPosition = 0, fmtLength = StringLength(fmt);

    for(QWORD i = 0; i < fmtLength; i++)
    {
        if(fmt[i] == '%')
        {
            switch(fmt[i + 1])
            {
                case 'd': // %d
                {
                    QWORD num = va_arg(args, QWORD), digits = NumberOfDigits(num), delimiter = pow(10, digits - 1);
                    while(delimiter)
                    {
                        buffer[bufferPosition++] = (num / delimiter) % 10 + '0';
                        delimiter /= 10;
                    }
                    i++;
                    break;
                }
                case '.':
                {
                    switch(fmt[i + 2])
                    {
                        case 'b': // %.b
                        {
                            QWORD length = va_arg(args, QWORD);
                            BYTE_PTR byteArr = (BYTE_PTR)va_arg(args, QWORD);
                            for(QWORD j = 0; j < length; j++)
                            {
                                BYTE currByte = *(byteArr + j);

                                buffer[bufferPosition++] = ConvertHalfByteToHexChar(currByte >> 4);
                                buffer[bufferPosition++] = ConvertHalfByteToHexChar(currByte & 0xf);
                                buffer[bufferPosition++] = ' ';
                            }
                            break;
                        }
                    }
                    i += 2;
                    break;
                }
                default: // %<NUMBER>
                {
                    BYTE numberOfBytes = fmt[i + 1] - '0';
                    CHAR currq = 2 * numberOfBytes - 1;
                    QWORD num = va_arg(args, QWORD), mask = 0xf << (currq * 4);
                    for(; currq >= 0; currq--, mask >>= 4)
                    {
                        buffer[bufferPosition++] = ConvertHalfByteToHexChar((num & mask) >> (currq * 4));
                    }
                    i++;
                }
            }
        }
        else
            buffer[bufferPosition++] = fmt[i];
    }
    va_end(args);
    PrintBuffer(buffer, bufferPosition);
}