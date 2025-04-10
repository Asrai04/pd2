//
// Created by sofia on 2025/3/30.
//

#ifndef BLOCKMEMORY_H
#define BLOCKMEMORY_H
#include <iostream>
#include <memory_resource>

class BlockMemory {
public:
    int id;
    std::string type;
    std::string value;
    int size;
    void * ptr;
    int refCount;

    BlockMemory(int id, int size, std::string type, std::string value, void* ptr = nullptr, int refCount = 1);

};



#endif //BLOCKMEMORY_H
