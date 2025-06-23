#include "Include/json.hpp" 
#include <fstream>
#include <iostream>
#include <filesystem>
#include "Log.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

void enviarABlockADiskNode(const std::string& mensajeJSON) {
    try {
        json data = json::parse(mensajeJSON);

        int nodo = data["nodo"];
        std::string archivo = data["archivo"];
        std::string contenido = data["contenido"];

        std::string rutaBase = "C:\\PDF\\Nodo" + std::to_string(nodo) + "\\";
        fs::create_directories(rutaBase);  // crea la carpeta si no existe

        std::string rutaArchivo = rutaBase + archivo;

        std::ofstream outFile(rutaArchivo);
        if (outFile.is_open()) {
            outFile << contenido;
            outFile.close();
            std::cout << "[Nodo " << nodo << "] Archivo guardado: " << rutaArchivo << std::endl;

            // Crear log para ese nodo
            Log logger(rutaBase + "Nodo" + std::to_string(nodo));
            logger.addEntry("Se guardó el bloque: " + archivo);
        }
        else {
            std::cerr << "[Nodo " << nodo << "] Error al guardar el archivo: " << archivo << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Fallo al procesar JSON en diskNode: " << e.what() << std::endl;
    }
}
