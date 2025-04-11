#include "BlockMemory.h"
#include <stdexcept>

BlockMemory::BlockMemory(int id, int size, std::string type, std::string value, void *ptr, int refCount) {
    this->id = id;
    this->size = size;
    this->value = value;
    this->type = type;
    this->ptr = ptr;
    this->refCount = refCount;
}

void BlockMemory::assignValueFromString(const std::string& newValue) {
    if (!ptr)
        throw std::runtime_error("Null pointer in assignValueFromString");

    if (type == "int") {
        int val = std::stoi(newValue);
        *static_cast<int*>(ptr) = val;
    } else if (type == "long") {
        long val = std::stol(newValue);
        *static_cast<long*>(ptr) = val;
    } else if (type == "float") {
        float val = std::stof(newValue);
        *static_cast<float*>(ptr) = val;
    } else if (type == "double") {
        double val = std::stod(newValue);
        *static_cast<double*>(ptr) = val;
    } else if (type == "char") {
        if (newValue.length() != 1)
            throw std::invalid_argument("Invalid char input: must be a single character");
        *static_cast<char*>(ptr) = newValue[0];
    } else {
        throw std::invalid_argument("Unsupported type: " + type);
    }

    this->value = newValue;

}
std::string BlockMemory::getValueAsString() const {
    if (!ptr) return "NULL";

    if (type == "int") {
        return std::to_string(*static_cast<int*>(ptr));
    } else if (type == "long") {
        return std::to_string(*static_cast<long*>(ptr));
    } else if (type == "float") {
        return std::to_string(*static_cast<float*>(ptr));
    } else if (type == "double") {
        return std::to_string(*static_cast<double*>(ptr));
    } else if (type == "char") {
        return std::string(1, *static_cast<char*>(ptr));
    }

    return "Tipo no soportado";
}

