#include <iostream>
#include <winsock2.h>  // Incluye Winsock

#pragma comment(lib, "ws2_32.lib")  // Enlaza con la librería Winsock

#define PORT 8080  // Puerto del servidor

int main() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return -1;
    }

    // Crear un socket para el servidor
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones desde cualquier IP
    address.sin_port = htons(PORT);  // Puerto del servidor

    // Asociar el socket con la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // Escuchar por conexiones entrantes
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    std::cout << "Servidor escuchando en el puerto " << PORT << "...\n";

    // Aceptar la conexión del cliente
    SOCKET new_socket = accept(server_fd, NULL, NULL);
    if (new_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // Enviar un mensaje de bienvenida al cliente
    const char *message = "Hola desde el servidor\n";
    send(new_socket, message, strlen(message), 0);

    // Recibir datos del cliente
    char buffer[1024] = {0};
    recv(new_socket, buffer, sizeof(buffer), 0);
    std::cout << "Mensaje recibido del cliente: " << buffer << std::endl;

    // Cerrar los sockets
    closesocket(new_socket);
    closesocket(server_fd);

    // Limpiar Winsock
    WSACleanup();

    return 0;
}
