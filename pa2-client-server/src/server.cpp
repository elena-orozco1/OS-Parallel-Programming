// port num available as command-line arg
// receives request from client using sockets
// creates a child process to handle the request
// fireman function to handle zombie processes

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <cmath>
#include <cstdlib>

using namespace std;

// function from Dr. Rincon's fireman.cpp file
void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0);
}

// functions from PA1
int EliasGamma(vector<int> vals) {
    int total_bits = 0;
    for(int i=0; i<vals.size(); i++) {
        if (vals[i] > 0) {
            total_bits += 2*floor(log2(vals[i])) + 1;
        }
    }
    return total_bits;
}

int EliasDelta(vector<int> vals) {
    int total_bits = 0;
    int temp_val;
    for (int i=0; i<vals.size(); i++) {
        if (vals[i] > 0) {
            temp_val = floor(log2(vals[i])) + 1;
            total_bits += (temp_val - 1) + (2 * floor(log2(temp_val)) + 1);
        }
    }
    return total_bits;
}

// send functions
void sendString(int sock, const std::string& str) {
    int len = str.length();
    send(sock, &len, sizeof(len), 0);
    send(sock,str.c_str(), len, 0);
}

void sendInt(int sock, int value) {
    send(sock, &value, sizeof(value), 0);
}

void sendChar(int sock, char c) {
    send(sock, &c, sizeof(c), 0);
}

void sendVector(int sock, const std::vector<int>& vec) {
    int size = vec.size();
    send(sock, &size, sizeof(size), 0);
    if (size > 0) {
        send(sock, vec.data(), size * sizeof(int), 0);
    }
}

// receive functions 
std::string receiveString(int sock) {
    int len;
    int n = recv(sock, &len, sizeof(len), 0);
    if (n <= 0) return "";
    char* buffer = new char[len+1];
    n = recv(sock, buffer, len, 0);
    if (n <= 0) {
        delete[] buffer;
        return "";
    }
    buffer[len] = '\0';
    std::string result(buffer);
    delete[] buffer;
    return result;
}

int receiveInt(int sock) {
    int val;
    int n = recv(sock, &val, sizeof(val), 0);
    if (n <= 0) return 0;
    return val;
}

char receiveChar(int sock) {
    char c;
    int n = recv(sock, &c, sizeof(c), 0);
    if (n <= 0) return '\0';
    return c;
}

std::vector<int> receiveVector(int sock) {
    int size;
    int n = recv(sock, &size, sizeof(size), 0);
    if (n <= 0) return std::vector<int>();
    std::vector<int> vec(size);
    if(size > 0) {
        n = recv(sock, vec.data(), size * sizeof(int), 0);
        if (n <= 0) return std::vector<int>();
    }
    return vec;
}

void processRequest(int client_socket) {
    // receive data from client
    string original_message = receiveString(client_socket);
    char symbol = receiveChar(client_socket);
    string prev_symbols = receiveString(client_socket);

    // remove previous symbols
    string filtered = "";
    for (char c : original_message) {
        bool remove = false;
        for (char prev : prev_symbols) {
            if (c == prev) {
                remove = true;
                break;
            }
        }
        if (!remove) {
            filtered.push_back(c);
        }
    }

    // find positions
    vector<int> positions;
    for (int i=0; i<filtered.size(); i++) {
        if(filtered[i] == symbol) {
            positions.push_back(i+1);
        }
    }

    // calculate differences
    vector<int> differences;
    if (!positions.empty()) {
        differences.push_back(positions[0]);
        for (int i=0; i<positions.size()-1; i++) {
            differences.push_back(positions[i+1] - positions[i]);
        }
    }

    // calculate bits
    int pos_gamma = EliasGamma(positions);
    int diff_gamma = EliasGamma(differences);
    int pos_delta = EliasDelta(positions);
    int diff_delta = EliasDelta(differences);

    // send results back to client
    sendString(client_socket, filtered);
    sendVector(client_socket, positions);
    sendVector(client_socket, differences);
    sendInt(client_socket, pos_gamma);
    sendInt(client_socket, diff_gamma);
    sendInt(client_socket, pos_delta);
    sendInt(client_socket, diff_delta);
}

int main (int argc, char *argv[]) {
    int port, server_socket, client_socket;
    struct sockaddr_in address;

    if (argc != 2) {
        return 1;
    }
    
    port = atoi(argv[1]);

    // signal handler
    signal(SIGCHLD, fireman);

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        return 1;
    }

    // allow socket reuse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return 1;
    }

    // setup address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // bind socket
    if (::bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        return 1;
    }

    // listen for connections
    if (listen(server_socket, 5) < 0) {
        return 1;
    }

    while(true) {
        client_socket = accept(server_socket, NULL, NULL);
        if(fork() == 0) {
            close(server_socket);
            processRequest(client_socket);
            close(client_socket);
            exit(0);
        } else {
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}
