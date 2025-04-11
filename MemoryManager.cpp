//
// Created by sofia on 2025/3/27.
//

#include "MemoryManager.h"
#include "BlockMemory.h"
#include <iostream>
#include <sstream> // para mensajes del cliente
#include <filesystem>
#include <fstream>
#include <thread>
#include <iomanip>
#include <ctime>
#include <chrono>
volatile bool serverRunning = true;
namespace fs = std::filesystem;
using namespace std;

MemoryManager::MemoryManager(int port, int memsize, const std::string& dumpFolder) {  // Inicializar
    this->port = port;
    this->memsize = memsize;
    this->dumpFolder = dumpFolder;
    server_fd = INVALID_SOCKET;
    new_socket = INVALID_SOCKET;
    std::vector<BlockMemory> listBlock; // Es el Memory Map, nose porque le puse ese nombre
}

// Funcion para Asignar Memoria
void MemoryManager::AssignMem() {
    // Convertir de MB a BYTES
    size_t byte = memsize*1024*1024;

    // Asignar memoria
    reservedMem = malloc(byte);

    // Revisar si hay memoria disponible
    if (reservedMem == nullptr) {
        printf("Malloc Error");
        exit(1);
    }
}

// Funcion para Iniciar Servidor
void MemoryManager::InitServer() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    // Crear un socket para el servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones desde cualquier IP
    address.sin_port = htons(port);  // Puerto del servidor

    // Asociar el socket con la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
    }

    // Escuchar por conexiones entrantes
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }
    std::cout << "Servidor escuchando en el puerto " << port << "...\n";
}

// Fucion para escuchar al cliente
void MemoryManager::Listen() {
    std::thread consola_thread([]() { // Cerrar el Servidor
        std::string command;
        while (serverRunning) {
            std::getline(std::cin, command);
            if (command == "exit") {
                std::cout << "Server terminated\n";
                serverRunning = false;
                break;
            }
        }
    });

    // Bucle para aceptar clientes
    while (serverRunning) {
        // Aceptar la conexión del cliente
        new_socket = accept(server_fd, NULL, NULL);
        if (new_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;  // Seguir esperando conexiones
        }

        std::cout << "Cliente conectado.\n";

        char buffer[1024];

        while (serverRunning) {
            memset(buffer, 0, sizeof(buffer));  // Limpiar buffer
            int valread = recv(new_socket, buffer, sizeof(buffer) - 1, 0);

            if (valread <= 0) {  // Cliente se desconectó
                std::cout << "Cliente desconectado.\n";
                break;
            }

            buffer[valread] = '\0';  // Asegurarse de que el string esté bien terminado

            std::string receivedMessage(buffer);
            std::cout << "Mensaje recibido: " << buffer << std::endl;

            // Para descomponer el mensaje recibido por el cliente
            std::istringstream iss(receivedMessage);
            std::string Funcion;
            iss >> Funcion;
            std::cout << actualMemory << std::endl;
            if (Funcion == "Create") { // Verificar que funcion ejecutar
                std::string type; // Obtener tipo de Dato a Crear
                iss >> type;
                std::cout << "Create solicitado con tipo: " << type << std::endl;

                // Difinir tamano de Dato
                int size = 0;
                if (type == "int") size = sizeof(int);
                else if (type == "long")size =  sizeof(long);
                else if (type == "float")size =  sizeof(float);
                else if (type == "double") size = sizeof(double);
                else if (type == "float") size = sizeof(float);
                else if (type == "char") size = sizeof(char);
                else {
                    std::cerr << "Tipo desconocido.\n";
                }
                int id = Create(size,type); // Retornar ID del espacio creado
                AddDump();
                std::string response = std::to_string(id); // Generar respuesta
                send(new_socket, response.c_str(), response.size(), 0); // Enviarlo al Cliente
            }
            else if (Funcion == "Set") { // Definir valor para un puntero
                int id;
                std::string value;
                iss >> id;// Obtener ID
                std::getline(iss, value);
                value = value.substr(value.find_first_not_of(" ")); // Quitar espacios iniciales
                Set(id,value);
                AddDump();
                std::string response = "None"; // Generar respuesta
                send(new_socket, response.c_str(), response.size(), 0);
            }
            else if (Funcion == "Get") {
                int id; // Obtener tipo de Dato a Crear
                iss >> id;
                int value = Get(id); // Retornar ID del espacio creado
                std::string response = std::to_string(value); // Generar respuesta
                send(new_socket, response.c_str(), response.size(), 0); // Enviarlo al Cliente
            }
            else if (Funcion == "IncreaseRef") {
                int id; // Obtener tipo de Dato a Crear
                iss >> id;
                IncreaseRefCount(id);
                AddDump();
                std::string response = "None"; // Generar respuesta
                send(new_socket, response.c_str(), response.size(), 0);
            }
        }

        // Cerrar conexión con este cliente, pero el servidor sigue funcionando
        closesocket(new_socket);
        std::cout << "Esperando nuevo cliente...\n";
    }

    closesocket(server_fd);
    WSACleanup();
    consola_thread.join();
}

// Funcion crea archivo dumpFolder
void MemoryManager::DumpFolder() {
    try { // Intenta crear el Folder
        if (std::filesystem::create_directory(dumpFolder)) { // Revisa si existe o No
            std::cout << "Se creo el archivo dump folder: " << dumpFolder << std::endl;
        } else {
            std::cout << " Dump folder ya existente" << std::endl;
        }
    } catch (std::filesystem::filesystem_error& e) { // Si fallo el intento, error
        std::cout << e.what() << std::endl;
    }
}

// Funcion para agregar Archivos(txt) en DumpFolder
void MemoryManager::AddDump() {
    try {
        // Verificar que se haya creado la carpeta
        if (!fs::exists(dumpFolder)) {
            fs::create_directories(dumpFolder); // Si no existe, lo crea
        }

        // Obtener fecha y hora para el nombre del archivo
        auto now = std::chrono::system_clock::now(); // Obtiene el tiempo actual del sistema
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; // Convierte en milisegundos
        std::time_t t = std::chrono::system_clock::to_time_t(now); // Convierte al tipo de dato para manejar fechas
        std::tm tm = *std::localtime(&t);

        // Crear Nombre del Archivo
        std::ostringstream oss;
        oss << dumpFolder << "/memory_dump_"
            << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S")
            << "-" << std::setfill('0') << std::setw(3) << now_ms.count()  // Agregar milisegundos
            << ".txt";

        // Construir ruta del Archivo
        std::string dumpFilePath = oss.str();

        // Crear y escribir en el archivo
        std::ofstream file(dumpFilePath);
        if (file.is_open()) {
            // Formato de Tabla
            file << "__________________ DUMP FILE __________________ \n";
            file << std::left; // Alineado
            file << std::setw(10) << "ID"
                 << std::setw(12) << "TYPE"
                 << std::setw(10) << "SIZE"
                 << std::setw(18) << "PTR"
                 << std::setw(12) << "REF COUNT" << "\n";
            file << std::string(60, '-') << "\n";

            for (const auto& block : listBlock) { // Escribir el contenido de la memoria (listBock)
                file.width(10); file << block.id;
                file.width(12); file << block.type;
                file.width(10); file << block.size;
                file.width(18); file << block.ptr;
                file.width(12); file << block.refCount;
                file << "\n";
            }
            file.close();
        } else {
            std::cerr << "Error al crear el archivo dump." << std::endl;
        }
    } catch (std::filesystem::filesystem_error& e) { // Sino notificar Error
        std::cout << e.what() << std::endl;
    }
}

// Funcion para inicializar(Crear) un bloque de Memoria
int MemoryManager::Create(int size, const std::string& type) {
    // Ve si hay espacio disponible
    for (const auto& block : listBlock) {
        actualMemory += block.size;
    }
    if (actualMemory + size > memsize * 1024 * 1024) {
        std::cerr << "No hay suficiente memoria disponible\n";
        return -1; // Indica error
    }

    // Calcular la dirección del nuevo bloque dentro
    void* newPtr = static_cast<char*>(reservedMem) + actualMemory;

    // Crear un nuevo bloque de memoria
    BlockMemory newBlock(listBlock.size() + 1, size, type, "---",newPtr, 1);
    listBlock.push_back(newBlock); // Agregar a la lista
    actualMemory += size;
        return newBlock.id;
}

// Funcion para guardar un valor en el bloque
void MemoryManager::Set(int id, std::string value) {
    for (auto& block : listBlock) {
        if (block.id == id) {
            actualMemory += block.size;
            block.value = value;

            // Guarda el tipo de dato, dependiendo de su valor
            if (block.type == "int") {
                *reinterpret_cast<int*>(block.ptr) = std::stoi(value);
            } else if (block.type == "long") {
                *reinterpret_cast<long*>(block.ptr) = std::stol(value);
            } else if (block.type == "float") {
                *reinterpret_cast<float*>(block.ptr) = std::stof(value);
            } else if (block.type == "double") {
                *reinterpret_cast<double*>(block.ptr) = std::stod(value);
            } else if (block.type == "char") {
                *reinterpret_cast<char*>(block.ptr) = value[0];
            }
        }
    }
}

// Funcion para retornar el valor guardado
int MemoryManager::Get(int id) {
    for (auto& block : listBlock) {
        if (block.id == id) {
            if (block.type == "int") {
                *reinterpret_cast<int*>(block.ptr) = std::stoi(block.value);
            } else if (block.type == "long") {
                *reinterpret_cast<long*>(block.ptr) = std::stol(block.value);
            } else if (block.type == "float") {
                *reinterpret_cast<float*>(block.ptr) = std::stof(block.value);
            } else if (block.type == "double") {
                *reinterpret_cast<double*>(block.ptr) = std::stod(block.value);
            } else if (block.type == "char") {
                *reinterpret_cast<char*>(block.ptr) = block.value[0];
            }
        }
    }
}

// Funcion incrementa o decrementa las referencias de un puntero
void MemoryManager::IncreaseRefCount(int id) {
    for (auto& block : listBlock) {
        if (block.id == id) {
            block.refCount++;
        }
    }
}
void MemoryManager::DecreaseRefCount(int id) {
    for (auto& block : listBlock) {
        if (block.id == id) {
            block.refCount--;
        }
    }
}
// Funcion que ejecuta en segundo plano(hilo) para liberar memoria con 0=refcount
void MemoryManager::CollectGarbage() {
    while (serverRunning) {// Bucle mientras el servidor se ejecute
        for (auto block = listBlock.begin(); block != listBlock.end();)  { // Recorre la lista de block
            if (block->refCount == 0) { // si un bloque no tiene referencias
                int size = block->size;//Guardar tamaño de Dato antes de borrar
                free(block->ptr); // libera su puntero
                listBlock.erase(block); // Eliminar el bloque de la lista
                actualMemory -= size; // Incrementa Memoria disponible
                AddDump(); // Escribe dentro del Dump
                break;
            }
        }
        std::this_thread::sleep_for(5s); // Revisar cada 5secs
    }
}