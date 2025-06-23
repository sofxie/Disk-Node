#include "StorageMonitor.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include "Include/tinyxml2.h"
#include "Log.h"

namespace fs = std::filesystem;
using namespace tinyxml2;

const std::uintmax_t MAX_TAMANO_MB = 100;
const std::uintmax_t MAX_TAMANO_BYTES = MAX_TAMANO_MB * 1024 * 1024;

std::uintmax_t calcularTamanoTotal(const fs::path& carpeta) {
    std::uintmax_t tamanoTotal = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(carpeta)) {
            if (fs::is_regular_file(entry.status())) {
                tamanoTotal += fs::file_size(entry.path());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error al calcular tamaño en: " << carpeta << " -> " << e.what() << std::endl;
    }
    return tamanoTotal;
}

void verificarTamanosPeriodicamente(const std::string& carpetaXMLs) {
    while (true) {
        std::cout << "[Verificación de carpetas] Iniciando análisis...\n";

        for (const auto& entry : fs::directory_iterator(carpetaXMLs)) {
            if (entry.path().extension() == ".xml") {
                XMLDocument doc;
                if (doc.LoadFile(entry.path().string().c_str()) == XML_SUCCESS) {
                    XMLElement* root = doc.FirstChildElement("config");
                    if (root) {
                        const char* path = root->FirstChildElement("path")->GetText();
                        if (path && fs::exists(path)) {
                            std::uintmax_t tamano = calcularTamanoTotal(path);
                            std::string nombreNodo = fs::path(path).filename().string();

                            if (tamano > MAX_TAMANO_BYTES) {
                                std::cerr << "[ALERTA] Nodo " << nombreNodo << " ha superado los "
                                    << MAX_TAMANO_MB << " MB (" << tamano / (1024 * 1024) << " MB).\n";

                                Log logger(std::string(path) + "/" + nombreNodo);
                                logger.addEntry("ALERTA: Se superó el límite de espacio en " + nombreNodo);
                            }
                            else {
                                std::cout << "[OK] Nodo " << nombreNodo << ": "
                                    << tamano / (1024 * 1024) << " MB usados.\n";
                            }
                        }
                    }
                }
            }
        }

        std::cout << "[Verificación finalizada] Esperando próximo ciclo...\n";
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

