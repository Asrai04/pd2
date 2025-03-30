//
// Created by sofia on 2025/3/27.
//

#include "MemoryManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
namespace fs = std::filesystem;

MemoryManager::MemoryManager(int port, int memsize, const std::string& dumpFolder) {  // Inicializar
    this->port = port;
    this->memsize = memsize;
    this->dumpFolder = dumpFolder;
    server_fd = INVALID_SOCKET;
    new_socket = INVALID_SOCKET;

    // Crear la carpeta DumpFolder si no existe
    if (!fs::exists(dumpFolder)) {
        fs::create_directories(dumpFolder);
        std::cout << "Se creo: " << dumpFolder << std::endl;
    }
}

// Funcion para Asignar Memoria
void MemoryManager::AssignMem() {
    // Convertir de MB a BYTES
    size_t byte = memsize*1024*1024;
    // Asignar memoria
    void *ptr = malloc(byte);

    // Revisar si hay memoria disponible
    if (ptr == NULL) {
        printf("Malloc Error");
        exit(0);
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
    // Bucle para aceptar clientes
    while (true) {
        // Aceptar la conexión del cliente
        new_socket = accept(server_fd, NULL, NULL);
        if (new_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;  // Seguir esperando conexiones
        }

        std::cout << "Cliente conectado.\n";

        // Enviar un mensaje de bienvenida al cliente
        const char *message = "Hola desde el servidor\n";
        send(new_socket, message, strlen(message), 0);

        char buffer[1024] = {0};

        while (true) {
            memset(buffer, 0, sizeof(buffer));  // Limpiar buffer
            int valread = recv(new_socket, buffer, sizeof(buffer) - 1, 0);

            if (valread <= 0) {  // Cliente se desconectó
                std::cout << "Cliente desconectado.\n";
                break;
            }

            buffer[valread] = '\0';  // Agregar fin de string
            std::cout << "Mensaje recibido: " << buffer << std::endl;

            // Enviar respuesta al cliente
            std::string respuesta = "Servidor recibió: " + std::string(buffer);
            send(new_socket, respuesta.c_str(), respuesta.size(), 0);
        }

        // Cerrar conexión con este cliente, pero el servidor sigue funcionando
        closesocket(new_socket);
        std::cout << "Esperando nuevo cliente...\n";
    }

    closesocket(server_fd);
    WSACleanup();
}

// Funcion crea archivo dumpFolder
void MemoryManager::DumpFolder() {
    // Obtener fecha y hora para el archivo
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << dumpFolder << "/memory_dump_"
        << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S")
        << "-" << std::setfill('0') << std::setw(3) << now_ms.count()  // Agregar milisegundos
        << ".txt";

    std::string dumpFilePath = oss.str();

    // Crear y escribir en el archivo
    std::ofstream file(dumpFilePath);
    if (file.is_open()) {
        file << "Este es un archivo dump con marca de tiempo precisa.\n";
        file.close();
        std::cout << "Archivo dump creado: " << dumpFilePath << std::endl;
    } else {
        std::cerr << "Error al crear el archivo dump." << std::endl;
    }
}