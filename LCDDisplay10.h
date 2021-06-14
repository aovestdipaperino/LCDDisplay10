#ifndef TEN_DIGIT_LCD_H
#define TEN_DIGIT_LCD_H
#include <stdint.h>

class LCDDisplay10
{
private:
     void fill(unsigned char ch);
     void fillDigits(unsigned char ch);
     uint8_t sendCommand(uint8_t command, uint8_t val);

public:
     void begin(uint8_t sda = 0, uint8_t scl = 0);
     uint8_t writeMemory(uint8_t addr, uint8_t val);
     void clear();
     bool writeToBuffer(const char *number);
     uint8_t sendBuffer();
     void reset();
     bool print(const char *number);
     void setError(bool hasError);
     void setMemory(bool hasMemory);
     void setNegative(bool hasNegative);
     void setDigit(uint8_t pos, int8_t value);
     void setPointPos(uint8_t pos);
     void setThousands(uint8_t num, uint8_t shift);
};

#endif