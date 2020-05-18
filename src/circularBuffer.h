#define BUFFER_LENGTH 64

template <class T> class CircularBuffer
{
    public:
        CircularBuffer();
        bool available();
        bool writable();
        T read();
        void write(T value);
        void reset();

    private:
        T buffer[BUFFER_LENGTH];
        uint8_t readPosition;
        uint8_t writePosition; 
};

#include "circularBufferImp.h"
