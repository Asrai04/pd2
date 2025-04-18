//
// Created by sofia on 2025/3/27.
//
#pragma once
#include <string>
#include <vector>
#include <winsock2.h>  // Incluye Winsock

#include "BlockMemory.h"

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

class MemoryManager {
private:
    int port; // Numero de puerto para el servidor
    int memsize; // Tamaño de Memoria
    std::string dumpFolder;
    void *reservedMem;
    std::vector<BlockMemory> listBlock; // Lista de bloques de memoria
    SOCKET server_fd; // Socket para el servidor
    SOCKET new_socket; // Socket del cliente
public:
    MemoryManager(int port, int memsize, const std::string& dumpFolder); // Declarar Constructor
    void AssignMem(); // Declarar Funcion Asigar Memoria
    void DumpFolder(); // Declarar Funcion para crear DumpFolder
    void InitServer(); // Declarar Funcion Iniciar Servidor
    void Listen(); // Declarar Funcion Escuchar Cliente
    int Create(int size, const std::string& type);
    void Set(int id, int value);
    int Get(int id);
    void IncreaseRefCount(int id);
    void DecreaseRefCount(int id);
};



#endif //MEMORYMANAGER_H
