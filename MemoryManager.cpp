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
    std::thread garbageThread(&MemoryManager::CollectGarbage, this);
    garbageThread.detach(); // Hace que corra en segundo plano sin bloquear el servidor
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

            std::cout << "Funcion: " << Funcion << " " << std::endl;

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
            if (Funcion == "Set") { // Definir valor para un puntero
                int id;
                std::string value;

                // Extraer el ID
                if (!(iss >> id)) {
                    std::cerr << "Error: Formato inválido para Set (falta ID)\n";
                    continue;
                }

                // Extraer el valor (el resto de la línea después del ID)
                if (!std::getline(iss >> std::ws, value)) { // std::ws elimina espacios iniciales
                    std::cerr << "Error: Formato inválido para Set (falta valor)\n";
                    continue;
                }

                // Eliminar espacios adicionales al inicio/final del valor (si los hay)
                value.erase(value.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                std::cout << "Set solicitado - ID: " << id << ", Valor: '" << value << "'" << std::endl;

                Set(id, value);
                std::string response = "1"; // Retornar ID del espacio creado
                send(new_socket, response.c_str(), response.size(), 0); // Enviarlo al Cliente
            }
            if (Funcion == "Get") {
                int id; // Obtener tipo de Dato a Crear
                iss >> id;
                std::string response = Get(id); // Retornar ID del espacio creado
                send(new_socket, response.c_str(), response.size(), 0); // Enviarlo al Cliente
            }
            if (Funcion == "IncreaseRefCount") {
                int id; // Obtener tipo de Dato a Crear
                iss >> id;
                IncreaseRefCount(id);
                AddDump();
                std::string response = "None"; // Generar respuesta
                send(new_socket, response.c_str(), response.size(), 0);
            }
            if (Funcion == "DecreaseRefCount") {
                int id; // Obtener tipo de Dato a Crear
                iss >> id;
                DecreaseRefCount(id);
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
    if (!reservedMem) {
        std::cerr << "Error: memoria no ha sido reservada. Llama a AssignMem() antes de Create.\n";
        return -1;
    }

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
    try {
        bool blockFound = false;

        for (auto& block : listBlock) {
            if (block.id == id) {
                blockFound = true;

                std::cout << "Asignando valor '" << value << "' a bloque ID: " << id
                          << " (Tipo: " << block.type << ")" << std::endl;

                // Asignación directa según el tipo
                if (block.type == "int") {
                    int val = std::stoi(value);
                    *reinterpret_cast<int*>(block.ptr) = val;
                } else if (block.type == "long") {
                    long val = std::stol(value);
                    *reinterpret_cast<long*>(block.ptr) = val;
                } else if (block.type == "float") {
                    float val = std::stof(value);
                    *reinterpret_cast<float*>(block.ptr) = val;
                } else if (block.type == "double") {
                    double val = std::stod(value);
                    *reinterpret_cast<double*>(block.ptr) = val;
                } else if (block.type == "char") {
                    if (value.length() != 1) {
                        std::cerr << "Error: el valor para char debe ser un solo carácter." << std::endl;
                        return;
                    }
                    *reinterpret_cast<char*>(block.ptr) = value[0];
                } else {
                    std::cerr << "Tipo de dato no soportado: " << block.type << std::endl;
                }

                block.value = value; // Solo por registro interno (opcional)
                break;
            }
        }

        if (!blockFound) {
            std::cerr << "Error: No se encontró bloque con ID " << id << std::endl;
        }

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error de argumento inválido: " << e.what() << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error de valor fuera de rango: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error inesperado en Set: " << e.what() << std::endl;
    }
}

std::string MemoryManager::Get(int id) {
    for (auto& block : listBlock) {
        if (block.id == id) {
            std::cout << block.type <<  block.id << std::endl;
            if (block.type == "int") {
                return std::to_string(*reinterpret_cast<int*>(block.ptr));
            } else if (block.type == "long") {
                return std::to_string(*reinterpret_cast<long*>(block.ptr));
            } else if (block.type == "float") {
                return std::to_string(*reinterpret_cast<float*>(block.ptr));
            } else if (block.type == "double") {
                return std::to_string(*reinterpret_cast<double*>(block.ptr));
            } else if (block.type == "char") {
                return std::string(1, *reinterpret_cast<char*>(block.ptr));
            } else {
                std::cerr << "Tipo de dato no soportado: " << block.type << std::endl;
                return "Tipo no soportado";
            }
        }
    }
    return "None";
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
        for (size_t i = 0; i < listBlock.size(); ++i)  { // Recorre la lista de block
            if (listBlock[i].refCount == 0 && listBlock[i].ptr != nullptr) { // si un bloque no tiene referencias
                int size = listBlock[i].size;//Guardar tamaño de Dato antes de borrar
                listBlock[i].size = 0;
                listBlock[i].ptr = nullptr; // Eliminar el bloque de la lista
                actualMemory -= size; // Incrementa Memoria disponible
                AddDump(); // Escribe dentro del Dump
            }
        }
        std::this_thread::sleep_for(5s); // Revisar cada 5secs
    }
}

void MemoryManager::Stop() {
    serverRunning = false;
    if (garbageThread.joinable())
        garbageThread.join();

    listBlock.clear(); // Vaciar lista de bloques
    actualMemory = 0;

    if (reservedMem != nullptr) {
        std::cout << "[INFO] Liberando memoria reservada principal.\n";
        free(reservedMem); // Liberar el bloque grande
        reservedMem = nullptr;
    }

    AddDump(); // Si llevas registro de limpieza
    free(reservedMem); // ← aquí sí puedes
    reservedMem = nullptr;
}