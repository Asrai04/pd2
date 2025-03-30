//
// Created by sofia on 2025/3/27.
//
#pragma once
#include <string>
#include <winsock2.h>  // Incluye Winsock

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

class MemoryManager {
private:
    int port; // Numero de puerto para el servidor
    int memsize; // Tamaño de Memoria
    std::string dumpFolder;
    SOCKET server_fd; // Socket para el servidor
    SOCKET new_socket; // Socket del cliente
public:
    MemoryManager(int port, int memsize, const std::string& dumpFolder); // Declarar Constructor
    void AssignMem(); // Declarar Funcion Asigar Memoria
    void DumpFolder(); // Declarar Funcion para crear DumpFolder
    void InitServer(); // Declarar Funcion Iniciar Servidor
    void Listen(); // Declarar Funcion Escuchar Cliente


};



#endif //MEMORYMANAGER_H
