
template <class T>
CircularBuffer<T>::CircularBuffer() {
    readPosition = 0;
    writePosition = 0;
}

template <class T>
bool CircularBuffer<T>::available() {
   return readPosition != writePosition;
}

template <class T>
bool CircularBuffer<T>::writable() {
   return ((writePosition + 1) % BUFFER_LENGTH) != readPosition;
}

template <class T>
T CircularBuffer<T>::read() {
    T val = buffer[readPosition];
    readPosition = (readPosition + 1) % BUFFER_LENGTH;
    return val;
}

template <class T>
void CircularBuffer<T>::write(T value) {
   buffer[writePosition] = value;
   writePosition = (writePosition + 1) % BUFFER_LENGTH;
}

template <class T>
void CircularBuffer<T>::reset() {
    readPosition = 0;
    writePosition = 0;
}
