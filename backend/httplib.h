#ifndef HTTLIB_H
#define HTTLIB_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <fstream>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::string body;
};

struct Response {
    int status = 200;
    std::string body;
    std::string content_type = "text/plain";

    void set_content(const std::string& content, const std::string& type) {
        body = content;
        content_type = type;
    }

    void set_redirect(const std::string&, int status_code = 302) {
        status = status_code;
        body = "";
    }
};

class Server {
private:
    std::string mount_dir;
    std::map<std::string, std::function<void(const Request&, Response&)>> get_routes;
    std::map<std::string, std::function<void(const Request&, Response&)>> post_routes;

    std::string get_file_content(const std::string& filepath, std::string& ctype) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) return "";
        std::ostringstream ss;
        ss << file.rdbuf();
        if (filepath.find(".html") != std::string::npos) ctype = "text/html";
        else if (filepath.find(".css") != std::string::npos) ctype = "text/css";
        else if (filepath.find(".js") != std::string::npos) ctype = "application/javascript";
        else ctype = "text/plain";
        return ss.str();
    }

public:
    void set_mount_point(const std::string&, const std::string& dir) {
        mount_dir = dir;
    }

    void Get(const std::string& pattern, std::function<void(const Request&, Response&)> handler) {
        get_routes[pattern] = handler;
    }

    void Post(const std::string& pattern, std::function<void(const Request&, Response&)> handler) {
        post_routes[pattern] = handler;
    }

    void listen(const std::string&, int port) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        bind(server_fd, (struct sockaddr*)&address, sizeof(address));
        ::listen(server_fd, SOMAXCONN);

        std::cout << "Voting Management System Server started at http://localhost:" << port << "\n";

        while (true) {
            SOCKET client_socket = accept(server_fd, nullptr, nullptr);
            if (client_socket == INVALID_SOCKET) continue;

            char buffer[4096] = {0};
            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                closesocket(client_socket);
                continue;
            }

            std::string req_str(buffer, bytes_received);
            Request req;
            Response res;

            size_t first_space = req_str.find(' ');
            size_t second_space = req_str.find(' ', first_space + 1);
            if (first_space != std::string::npos && second_space != std::string::npos) {
                req.method = req_str.substr(0, first_space);
                req.path = req_str.substr(first_space + 1, second_space - first_space - 1);
            }

            size_t body_pos = req_str.find("\r\n\r\n");
            if (body_pos != std::string::npos) {
                req.body = req_str.substr(body_pos + 4);
            }

            if (req.method == "GET") {
                if (get_routes.find(req.path) != get_routes.end()) {
                    get_routes[req.path](req, res);
                } else if (req.path.rfind("/api/", 0) == 0) {
                    res.status = 404;
                    res.set_content("{\"error\": \"API endpoint not found\"}", "application/json");
                } else {
                    std::string filepath = mount_dir;
                    if (req.path == "/") filepath += "/index.html";
                    else filepath += req.path;

                    std::string ctype;
                    std::string content = get_file_content(filepath, ctype);
                    if (!content.empty()) {
                        res.status = 200;
                        res.set_content(content, ctype);
                    } else {
                        res.status = 404;
                        res.set_content("404 Not Found", "text/plain");
                    }
                }
            } else if (req.method == "POST") {
                if (post_routes.find(req.path) != post_routes.end()) {
                    post_routes[req.path](req, res);
                } else {
                    res.status = 404;
                    res.set_content("{\"error\": \"Not found\"}", "application/json");
                }
            }

            std::string resp_str;
            if (res.status == 302 || res.status == 301) {
                resp_str = "HTTP/1.1 302 Found\r\nLocation: /index.html\r\nContent-Length: 0\r\n\r\n";
            } else {
                resp_str = "HTTP/1.1 " + std::to_string(res.status) + " OK\r\n" +
                           "Content-Type: " + res.content_type + "\r\n" +
                           "Content-Length: " + std::to_string(res.body.length()) + "\r\n" +
                           "Connection: close\r\n\r\n" + res.body;
            }

            send(client_socket, resp_str.c_str(), (int)resp_str.length(), 0);
            closesocket(client_socket);
        }

        WSACleanup();
    }
};

} // namespace httplib

#endif