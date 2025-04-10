//
// Created by sofia on 2025/3/30.
//

#include "BlockMemory.h"
BlockMemory::BlockMemory(int id, int size, std::string type, std::string value, void *ptr, int refCount) {
    this->id = id;
    this->size = size;
    this->value = value;
    this->type = type;
    this->ptr = ptr;
    this->refCount = refCount;
}