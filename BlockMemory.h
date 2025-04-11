#ifndef BLOCKMEMORY_H
#define BLOCKMEMORY_H

#include <string>

class BlockMemory {
public:
    int id;
    int size;
    std::string type;
    std::string value;
    void* ptr;
    int refCount;

    BlockMemory(int id, int size, std::string type, std::string value, void* ptr, int refCount);
    void assignValueFromString(const std::string& newValue);

    std::string getValueAsString() const;
};

#endif // BLOCKMEMORY_H
