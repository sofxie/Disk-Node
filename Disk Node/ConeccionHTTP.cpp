
#include "ConeccionHTTP.h"
#include "Include/json.hpp" // Librería para mensajes en JSON
#include "Include/tinyxml2.h" // Librería para leer parámetros desde XML
#include "Log.h" // Librería para registrar logs

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <thread>

using json = nlohmann::json;
using namespace std;

// Leer archivo XML
bool ConeccionHTTP::ReadXML(const std::string& xmlPath, std::string& ip, int& port, std::string& path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.LoadFile(xmlPath.c_str());
    if (eResult != tinyxml2::XML_SUCCESS) {
        return false;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("config");
    if (!root) return false;

    // Buscar elementos en el documento XML
    tinyxml2::XMLElement* ipElem = root->FirstChildElement("ip");
    tinyxml2::XMLElement* portElem = root->FirstChildElement("port");
    tinyxml2::XMLElement* pathElem = root->FirstChildElement("path");

    // Verificar estructura
    if (!ipElem || !portElem || !pathElem) return false;

    // Extraer contenido de texto
    const char* ipText = ipElem->GetText();
    const char* portText = portElem->GetText();
    const char* pathText = pathElem->GetText();

    if (!ipText || !portText || !pathText) return false;

    ip = ipText;
    port = std::stoi(portText);
    path = pathText;

    return true;
}

// Establecer conexión con Controlador de Disco
void ConeccionHTTP::run(int port, std::string addr) {
    SOCKET wsocket;
    SOCKET new_wsocket;
    WSADATA wsaData;
    struct sockaddr_in server;
    int server_len;
    const int BUFFER_SIZE = 30720;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Error de inicialización" << std::endl;
        return;
    }

    wsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (wsocket == INVALID_SOCKET) {
        std::cout << "Socket no creado" << std::endl;
        WSACleanup();
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(addr.c_str());
    server.sin_port = htons(port);
    server_len = sizeof(server);

    if (::bind(wsocket, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cout << "No se puede asociar el socket con la dirección." << std::endl;
        closesocket(wsocket);
        WSACleanup();
        return;
    }

    if (listen(wsocket, 20) != 0) {
        std::cout << "No se puede escuchar" << std::endl;
        closesocket(wsocket);
        WSACleanup();
        return;
    }

    char buff[BUFFER_SIZE] = { 0 };

    while (true) {
        new_wsocket = accept(wsocket, (SOCKADDR*)&server, &server_len);
        if (new_wsocket == INVALID_SOCKET) {
            std::cout << "No se pudo conectar" << std::endl;
            continue;
        }

        int bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
        if (bytes < 0) {
            std::cout << "No se pudo leer request" << std::endl;
            closesocket(new_wsocket);
            continue;
        }

        std::string request(buff, bytes);
        size_t pos = request.find("\r\n\r\n");
        std::string body = (pos != std::string::npos) ? request.substr(pos + 4) : "";

        json j;
        json responseJson;

        if (body.empty()) {
            responseJson["status"] = "activo";
            responseJson["mensaje"] = "Servidor HTTP";
        }
        else {
            try {
                j = json::parse(body);
                std::string logEntry;

                if (!j["comando"].is_null()) {
                    std::string comando = j["comando"];
                    std::cout << "Comando: " << comando << std::endl;
                    logEntry += "Comando: " + comando;
                }

                if (!j["contenido"].is_null()) {
                    std::string contenido = j["contenido"];
                    std::cout << "Contenido: " << contenido << std::endl;
                    logEntry += " | Contenido: " + contenido;
                }

                if (!logEntry.empty() && logger) {
                    logger->addEntry(logEntry);
                }

                std::cout << "JSON recibido: " << j.dump() << std::endl;
            }
            catch (const json::parse_error& e) {
                std::cout << "Error al parsear JSON: " << e.what() << std::endl;
                responseJson["status"] = "error";
                responseJson["mensaje"] = "Error al procesar el JSON";

                if (logger) {
                    logger->addEntry("Error al parsear JSON: " + std::string(e.what()));
                }
            }
        }

        std::string responseBody = responseJson.dump();

        std::string serverMessage =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            responseBody;

        int totalBytesSent = 0;
        int messageSize = serverMessage.size();

        while (totalBytesSent < messageSize) {
            int bytesSent = send(new_wsocket, serverMessage.c_str() + totalBytesSent, messageSize - totalBytesSent, 0);
            if (bytesSent < 0) {
                std::cout << "No se envió respuesta" << std::endl;
                break;
            }
            totalBytesSent += bytesSent;
        }
        closesocket(new_wsocket);
    }

    WSACleanup();
}
