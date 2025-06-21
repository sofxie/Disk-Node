#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include "ConeccionHTTP.h"
#include "LogManager.h"
using namespace std;
namespace fs = std::filesystem;

void GetXML(const std::string& carpetaPath);
void InitServer(const std::string& archivoXML);

int main()
{
	// Nombre de carpeta donde se almacena los XML
	std::string carpeta = "C:\\XML";

	// Ejecutar funcion
	GetXML(carpeta);
	procesarXMLsYCrearLogs(carpeta);

	std::cout << "Todos los servidores han sido lanzados desde XML.";
	std::cin.get();
	return 0;
}

// Lee los XML y lanza hilo
void GetXML(const std::string& carpetaPath) {
	// Revisar ruta de la carpeta
	if (!fs::exists(carpetaPath)) {
		std::cerr << "[ERROR] Carpeta no encontrada: " << carpetaPath << std::endl;
		return;
	}

	// Revisar cada archivo dentro de la carpeta que sea xml
	for (const auto& entry : fs::directory_iterator(carpetaPath)) {
		if (entry.is_regular_file() && entry.path().extension() == ".xml") {
			std::thread servidorThread(InitServer, entry.path().string());
			servidorThread.detach(); // Corre cada servidor en segundo plano
		}
	}
}

// Lanzar un servidor con parametros del XML
void InitServer(const std::string& archivoXML) {
	std::string ip, path;
	int port;

	ConeccionHTTP* conn = new ConeccionHTTP();

	if (conn->ReadXML(archivoXML, ip, port, path)) {
		std::cout << "[OK] Configuración cargada desde: " << archivoXML << "\n";
		std::cout << "IP: " << ip << ", Puerto: " << port << ", Carpeta: " << path << "\n";

		conn->run(port, ip); // ¡Esto ya ejecuta el servidor!
	}
	else {
		std::cerr << "[ERROR] No se pudo leer el archivo: " << archivoXML << "\n";
	}
}