#include <debug.h>
#include <intrinsics.h>
#include <utils/utils.h>


VOID DbgPrintBuffer(IN PCHAR buffer, IN QWORD length)
{
    for(QWORD i = 0; i < length; i++)
        __outbyte(DEBUG_PORT, buffer[i]);
}

VOID DbgPrintNullTerminatedBuffer(IN PCHAR buffer)
{
    for(; *buffer; buffer++)
        __outbyte(DEBUG_PORT, *buffer);
}

VOID DbgPrintVaArg(IN PCHAR fmt, va_list args)
{
    CHAR buffer[BUFF_MAX_SIZE] = { 0 }, currq;
    QWORD bufferPosition, fmtLength, num, digits, delimiter, length, currQword, mask;
    BYTE_PTR byteArr;
    QWORD_PTR qwordArr;
    BYTE currByte, numberOfBytes;
    
    bufferPosition = 0;
    fmtLength = StringLength(fmt);
    for(QWORD i = 0; i < fmtLength; i++)
    {
        if(fmt[i] == '%')
        {
            switch(fmt[i + 1])
            {
                case 'd': // %d
                {
                    num = va_arg(args, QWORD);
                    digits = NumberOfDigits(num);
                    delimiter = pow(10, digits - 1);

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
                            length = va_arg(args, QWORD);
                            byteArr = (BYTE_PTR)va_arg(args, QWORD);
                            for(QWORD j = 0; j < length; j++)
                            {
                                currByte = byteArr[j];

                                buffer[bufferPosition++] = ConvertHalfByteToHexChar(currByte >> 4);
                                buffer[bufferPosition++] = ConvertHalfByteToHexChar(currByte & 0xf);
                                buffer[bufferPosition++] = ' ';
                            }
                            break;
                        }
                        case 'q':
                        {
                            length = va_arg(args, QWORD);
                            qwordArr = (QWORD_PTR)va_arg(args, QWORD);
                            for(QWORD j = 0; j < length; j++)
                            {
                                currq = 15;
                                currQword = qwordArr[j];
                                mask = 0xf << (currq * 4);
                                for(; currq >= 0; currq--, mask >>= 4)
                                    buffer[bufferPosition++] = ConvertHalfByteToHexChar((currQword & mask) >> (currq * 4));
                                buffer[bufferPosition++] = ' ';
                            }
                        }
                    }
                    i += 2;
                    break;
                }
                default: // %<NUMBER>
                {
                    numberOfBytes = fmt[i + 1] - '0';
                    currq = 2 * numberOfBytes - 1;
                    num = va_arg(args, QWORD), mask = 0xf << (currq * 4);
                    for(; currq >= 0; currq--, mask >>= 4)
                        buffer[bufferPosition++] = ConvertHalfByteToHexChar((num & mask) >> (currq * 4));
                    i++;
                }
            }
        }
        else
            buffer[bufferPosition++] = fmt[i];
    }
    DbgPrintBuffer(buffer, bufferPosition);
}


VOID DbgPrint(IN PCHAR fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    DbgPrintVaArg(fmt, args);
    va_end(args);
}

VOID DbgPrintDebugLevelDebug(IN PCHAR fmt, ...)
{
#if DEBUG_LVL <= 3
    va_list args;
    va_start(args, fmt);
    DbgPrintVaArg(fmt, args);
    va_end(args);
#endif
}

VOID DbgPrintDebugLevelWarning(IN PCHAR fmt, ...)
{
#if DEBUG_LVL <= 2
    va_list args;
    va_start(args, fmt);
    DbgPrintVaArg(fmt, args);
    va_end(args);
#endif
}

VOID DbgPrintDebugLevelInfo(IN PCHAR fmt, ...)
{
#if DEBUG_LVL <= 1
    va_list args;
    va_start(args, fmt);
    DbgPrintVaArg(fmt, args);
    va_end(args);
#endif
}