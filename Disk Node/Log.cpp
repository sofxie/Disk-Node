#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "Include/tinyxml2.h"
#include "Log.h"

namespace fs = std::filesystem;
using namespace tinyxml2;

void procesarXMLsYCrearLogs(const std::string& carpetaXMLs) {
    for (const auto& entry : fs::directory_iterator(carpetaXMLs)) {
        if (entry.path().extension() == ".xml") {
            XMLDocument doc;
            if (doc.LoadFile(entry.path().string().c_str()) == XML_SUCCESS) {
                XMLElement* root = doc.FirstChildElement("config");
                if (root) {
                    const char* path = root->FirstChildElement("path")->GetText();
                    if (path) {
                        std::string ruta(path);

                        // Crear carpeta si no existe
                        if (!fs::exists(ruta)) {
                            try {
                                fs::create_directories(ruta);
                                std::cout << "Carpeta creada: " << ruta << std::endl;
                            }
                            catch (const std::exception& e) {
                                std::cerr << "Error al crear carpeta: " << ruta << " -> " << e.what() << std::endl;
                                continue;
                            }
                        }

                        // Extraer nombre de nodo de la ruta
                        std::string nombreNodo = fs::path(ruta).filename().string();

                        // Crear log y agregar entrada de prueba
                        Log logger(ruta + "/" + nombreNodo);
                        logger.addEntry("Log inicial para " + nombreNodo);
                    }
                }
            }
            else {
                std::cerr << "Error al cargar XML: " << entry.path() << std::endl;
            }
        }
    }
}

