#include "BlockMemory.h"
#include <stdexcept>

BlockMemory::BlockMemory(int id, int size, std::string type, std::string value, void *ptr, int refCount) { // clase bloque para memory map
    this->id = id; // Id del bloque especifico
    this->size = size; // Tamaño del tipo de variable a guardar
    this->value = value; // no se empleo
    this->type = type; // Tipo de dato almacenado
    this->ptr = ptr; // ptr a la direccion de la memoria donde esta almacenado el dato
    this->refCount = refCount; // conteo de refertencias al ptr
}