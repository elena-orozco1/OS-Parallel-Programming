#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <vector>
#include <pthread.h>
#include <cstdlib>

using namespace std;

struct ThreadArg {
    char symbol;
    string input_message;
    string prev_symbols;
    string hostname;
    int port;

    string filtered;
    vector<int> positions;
    vector<int> differences;
    int pos_gamma;
    int diff_gamma;
    int pos_delta;
    int diff_delta;
};

// helper from my PA1
string vector_to_string(vector<int> vals) {
    string result = "";
    for (int val : vals) {
        result += to_string(val);
        result += " ";
    }
    return result;
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

void* threadwork(void* arg_ptr) {
    ThreadArg* data = (ThreadArg*) arg_ptr;

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return nullptr;
    }
    
    // resolve hostname
    struct hostent* server = gethostbyname(data->hostname.c_str());
    if (server == NULL) {
        close(sock);
        return nullptr;
    }
    
    // setup address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&address.sin_addr.s_addr, server->h_length);
    address.sin_port = htons(data->port);

    // connect to server
    if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(sock);
        return nullptr;
    }

    // send data to server
    sendString(sock, data->input_message);
    sendChar(sock, data->symbol);
    sendString(sock, data->prev_symbols);

    // receive from server
    data->filtered = receiveString(sock);
    data->positions = receiveVector(sock);
    data->differences = receiveVector(sock);
    data->pos_gamma = receiveInt(sock);
    data->diff_gamma = receiveInt(sock);
    data->pos_delta = receiveInt(sock);
    data->diff_delta = receiveInt(sock);
    
    close(sock);
    return nullptr;
}

int main(int argc, char* argv[]) {
    string hostname = argv[1];
    int port = atoi(argv[2]);

    string message;
    getline(cin, message);

    vector<char>symbols;
    string line;
    while(getline(cin, line)) {
        if(!line.empty()) {
            symbols.push_back(line[0]);
        }
    }

    int nThreads = symbols.size();
    vector<pthread_t> tid(nThreads);
    vector<ThreadArg> data(nThreads);

    for(int i=0; i<nThreads; i++) {
        data[i].symbol = symbols[i];
        data[i].input_message = message;
        data[i].hostname = hostname;
        data[i].port = port;

        data[i].prev_symbols = "";
        for (int j=0; j<i; j++) {
            data[i].prev_symbols += symbols[j];
        }

        if(pthread_create(&tid[i], NULL, threadwork, &data[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;    
        }    
    }
     // Initialize totals
    int total_gamma_pos = 0;
    int total_gamma_diff = 0;
    int total_delta_pos = 0;
    int total_delta_diff = 0;
    
    // Join threads and print results
    for(int i = 0; i < nThreads; i++) {
        pthread_join(tid[i], NULL);
        
        // Print thread info (same as your original)
        cout << "Symbol: " << data[i].symbol << ", Frequency: " << data[i].positions.size() << endl;
        cout << "Message: " << data[i].filtered << endl;
        cout << "Position(s): " << vector_to_string(data[i].positions) << endl;
        cout << "Difference between position(s): " << vector_to_string(data[i].differences) << endl;
        cout << "Number of bits to represent the position(s) (Elias Gamma): " << data[i].pos_gamma << endl;
        cout << "Number of bits to represent the difference between position(s) (Elias Gamma): " << data[i].diff_gamma << endl;
        cout << "Number of bits to represent the position(s) (Elias Delta): " << data[i].pos_delta << endl;
        cout << "Number of bits to represent the difference between position(s) (Elias Delta): " << data[i].diff_delta << endl;
        cout << endl;
                // Calculate totals
        total_gamma_pos += data[i].pos_gamma;
        total_gamma_diff += data[i].diff_gamma;
        total_delta_pos += data[i].pos_delta;
        total_delta_diff += data[i].diff_delta;
    }
    
    // Print totals (same as your original)
    cout << "Bits used for the positions (Elias Gamma encoding): " << total_gamma_pos << endl;
    cout << "Bits used for the difference between positions (Elias Gamma encoding): " << total_gamma_diff << endl;
    cout << endl;
    cout << "Bits used for the positions (Elias Delta encoding): " << total_delta_pos << endl;
    cout << "Bits used for the difference between positions (Elias Delta encoding): " << total_delta_diff << endl;
    
    return 0;
}
