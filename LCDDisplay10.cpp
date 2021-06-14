#include <LCDDisplay10.h>
#include <Wire.h>

#define PCF_CONTINUE (1 << 7)

#define SEG_A 0b10000000
#define SEG_B 0b01000000
#define SEG_C 0b00100000
#define SEG_D 0b00000001
#define SEG_E 0b00000010
#define SEG_F 0b00001000
#define SEG_G 0b00000100
#define SEG_P 0b00010000
#define SEG_Err 0b00010000
#define SEG_Mem 0b01000000
#define SEG_Neg 0b00100000
#define BUFFER_SIZE 12
#define DIGITS 10
#define FLAGS 11

const uint8_t DIGIT_SEGMENTS[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
    SEG_B | SEG_C | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G};

uint8_t _buffer[BUFFER_SIZE];

void LCDDisplay10::begin(uint8_t sda, uint8_t scl)
{
    Wire.begin();
    reset();
}

void LCDDisplay10::clear()
{
    fill(0);
    sendBuffer();
}

uint8_t LCDDisplay10::sendCommand(uint8_t command, uint8_t val)
{
    Wire.beginTransmission(0x7c >> 1);
    Wire.write(command);
    Wire.write(val);
    return Wire.endTransmission();
}

uint8_t LCDDisplay10::writeMemory(uint8_t addr, uint8_t val)
{
    Wire.beginTransmission(0x7c >> 1);
    Wire.write(0xe0);
    Wire.write(addr);
    Wire.write(val);
    return Wire.endTransmission();
}

void LCDDisplay10::fill(uint8_t c)
{
    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        _buffer[i] = c;
    }
}

void LCDDisplay10::fillDigits(uint8_t c)
{
    for (uint8_t i = 0; i < DIGITS; i++)
    {
        _buffer[i] = c;
    }
}

uint8_t LCDDisplay10::sendBuffer()
{
    Wire.beginTransmission(0x7c >> 1);
    Wire.write(0xe0);
    Wire.write((uint8_t)0);
    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        Wire.write(_buffer[i]);
    }
    return Wire.endTransmission();
}

bool LCDDisplay10::writeToBuffer(const char *number)
{
    int cur_pos = 0;
    int len = 0;

    uint8_t write_buffer[DIGITS];
    bool isNegative = *number == '-';
    setNegative(isNegative);

    if (isNegative) {
        number++;
    }

    while (*number > 0)
    {
        char c = *number;
        if (c == '.')
        {
            if (!cur_pos)
            {
                // leading '.'
                write_buffer[cur_pos] = SEG_P;
            }
            else
            {
                write_buffer[cur_pos - 1] |= SEG_P;
            }
            number++;
            continue;
        }
        if (len >= 10)
        {
            break;
        }
        if (c == ' ')
        {
            write_buffer[cur_pos++] = 0;
            len++;
            number++;
            continue;
        }
        if ((c >= '0') && (c <= '9'))
        {
            write_buffer[cur_pos++] = DIGIT_SEGMENTS[c - '0'];
            len++;
            number++;
            continue;
        }
        return false; // for now
    }
    // clear the display buffer
    fillDigits(0);
    for (uint8_t i = 0; i < len; i++)
    {
        _buffer[10 - len + i] = write_buffer[len - i - 1];
    }
    return true;
}

void LCDDisplay10::reset()
{

    sendCommand(0xe0, 0x48);
    delay(3);
    sendCommand(0xe0, 0x70);
}

bool LCDDisplay10::print(const char *number)
{
    if (writeToBuffer(number))
    {
        sendBuffer();
        return true;
    };
    return false;
}

void LCDDisplay10::setError(bool hasError)
{
    _buffer[FLAGS] = hasError ? _buffer[FLAGS] | SEG_Err : _buffer[FLAGS] & ~SEG_Err;
}

void LCDDisplay10::setMemory(bool hasMemory)
{
    _buffer[FLAGS] = hasMemory ? _buffer[FLAGS] | SEG_Mem : _buffer[FLAGS] & ~SEG_Mem;
}

void LCDDisplay10::setNegative(bool hasNegative)
{
    _buffer[FLAGS] = hasNegative ? _buffer[FLAGS] | SEG_Neg : _buffer[FLAGS] & ~SEG_Neg;
}

void LCDDisplay10::setDigit(uint8_t pos, int8_t value)
{
    if (pos < DIGITS)
    {
        _buffer[pos] = value >= 0 && value <= 9
                           // preserve decimal pos
                           ? (_buffer[pos] & SEG_P) | DIGIT_SEGMENTS[value]
                           : 0;
    }
}

void LCDDisplay10::setPointPos(uint8_t pos)
{
    if (pos < DIGITS)
    {
        for (uint8_t dp = 0; dp < DIGITS; dp++)
        {
            // Clear all other decimal points.
            _buffer[dp] = _buffer[dp] & ~SEG_P;
        }
        _buffer[pos] |= SEG_P;
    }
}

void LCDDisplay10::setThousands(uint8_t n, uint8_t s)
{
    if (n > 3) {
        return;
    }
    uint8_t mask = 0b00000000;
    switch(n) {
        case 1:
            mask = 0b00000001 << s;
            break;
        case 2:
        case 3:
            mask = 0b00010001 << s;
            break;
    }
    _buffer[10] = mask;
    _buffer[FLAGS] = n == 3 ? _buffer[FLAGS] | 1 : _buffer[FLAGS] & 254;
}