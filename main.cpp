#include <iostream>
#include "MemoryManager.h" // Importa la clase MemoryManager
#pragma comment(lib, "ws2_32.lib")  // Enlaza con la librería Winsock


int main(int argc, char* argv[]) { // Cuenta Cant de argumentos (argc) del tipo string

    int port = 0;
    int memsize = 0;
    std::string dumpFolder = "";

    // Leer argumentos
    for (int i = 1; i < argc; i++) {
        // Recorrer cada elemento del string
        std::string arg = argv[i];
        if (arg == "-port" && i + 1 < argc) { // Si el string es "-port"
            port = std::stoi(argv[++i]); // asigna valor de port
        } else if (arg == "-memsize" && i + 1 < argc) { // Si el string es "-memsize"
            memsize = std::stoi(argv[++i]); // asigna valor de memsize
        } else if (arg == "-dumpFolder" && i + 1 < argc) {
            // Si el string es "–dumpFolder"
            dumpFolder = argv[++i];; // asigna dumpFolder
        }
    }

    std::cout << "Puerto recibido: " << port << std::endl;
    std::cout << "Tamaño de memoria: " << memsize << " MB" << std::endl;

    MemoryManager manager(port,memsize,dumpFolder);
    manager.InitServer();
    manager.Listen();

    return 0;
}
