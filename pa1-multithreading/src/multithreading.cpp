// one thread per symbol
// each thread constructs simplified message by removing symbols
// each thread computes positions, differences, # of bits for each

#include <iostream>
#include <pthread.h>
#include <cmath>
#include <vector>

using namespace std;

struct ThreadArg {
    char symbol;
    string input_message; // original message
    string filtered; // message without prev chars
    
    vector<char> list_of_chars; // list of input chars
    int index; // index of symbol in list_of_chars

    vector<int> positions; // list of positions
    vector<int> differences;
    
    int pos_gamma; // bits to represent the positions (Elias Gamma)
    int diff_gamma; // bits to represent diff of positions (Elias Gamma)
    
    int pos_delta; // bits to represent the positions (Elias Delta)
    int diff_delta; // bits to represent the diff of positions (Elias Gamma)
};

int EliasGamma(vector<int> vals) {
    int total_bits = 0;
    for(int i=0; i<vals.size(); i++) {
        total_bits += 2*floor(log2(vals[i])) + 1;
    }
    return total_bits;
}

int EliasDelta(vector<int> vals) {
    int total_bits = 0;
    int temp_val;
    for (int i=0; i<vals.size(); i++) {
        temp_val = floor(log2(vals[i])) + 1;
        total_bits += (temp_val - 1) + (2 * floor(log2(temp_val)) + 1);
    }
    return total_bits;
}

// used for printing purposes
string vector_to_string(vector<int> vals) {
    string result = "";
    for (int val : vals) {
        result += to_string(val);
        result += " ";
    }
    return result;
}

void* threadwork(void* arg_ptr) {
    ThreadArg* data = (struct ThreadArg*) arg_ptr;
    char symbol = data->list_of_chars[data->index];
    
    // removing earlier characters
    string filtered;
    for (char c: data->input_message) {
        bool remove = false;
        for (int j = 0; j < data->index; j++) {
            if(c == data->list_of_chars[j]) {
                remove = true;
                break;
            }
        }
        if (!remove) filtered.push_back(c);
    }
    data->filtered = filtered;

    // Finding positions
    for (int i=0; i<filtered.size(); i++) {
        if(filtered[i] == symbol) {
            data->positions.push_back(i+1);
        }
    }

    // Differences between positions
    data->differences.push_back(data->positions[0]); // first position stays as is
    for (int i=0; i< data->positions.size() - 1; i++) {
        data->differences.push_back(data->positions[i+1]-data->positions[i]);
    }

    // counting Elias Gamma
    data->pos_gamma = EliasGamma(data->positions);
    data->diff_gamma = EliasGamma(data->differences);

    // counting Elias Delta
    data->pos_delta = EliasDelta(data->positions);
    data->diff_delta = EliasDelta(data->differences);

    return nullptr;
}

int main() {
    string message;
    getline(cin, message);

    vector<char> symbols;
    string line;
    
    while(getline(cin, line)) {
        if(!line.empty()) {
            symbols.push_back(line[0]);
        }
    }

    int nthreads = symbols.size();
    vector<pthread_t> tid(nthreads);
    vector<ThreadArg> data(nthreads);

    // creating threads
    for(int i=0; i<nthreads; i++) {
        data[i].symbol = symbols[i];
        data[i].index = i;
        data[i].input_message = message;
        data[i].list_of_chars = symbols;
        if (pthread_create(&tid[i], NULL, threadwork, &data[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    // initializing totals
    int total_gamma_pos = 0;
    int total_gamma_diff = 0;
    int total_delta_pos = 0;
    int total_delta_diff = 0;

    // joining threads
    for(int i=0; i<nthreads; i++) {
        pthread_join(tid[i], NULL);

        // printing thread info
        cout << "Symbol: " << data[i].symbol << ", Frequency: " << data[i].positions.size() << endl;
        cout << "Message: " << data[i].filtered << endl;
        cout << "Position(s): " << vector_to_string(data[i].positions) << endl;
        cout << "Difference between position(s): " << vector_to_string(data[i].differences) << endl;
        cout << "Number of bits to represent the position(s) (Elias Gamma): " << data[i].pos_gamma << endl;
        cout << "Number of bits to represent the difference between position(s) (Elias Gamma): " << data[i].diff_gamma << endl;
        cout << "Number of bits to represent the position(s) (Elias Delta): " << data[i].pos_delta << endl;
        cout << "Number of bits to represent the difference between position(s) (Elias Delta): " << data[i].diff_delta << endl;
        cout << endl;  

        // finding totals
        total_gamma_pos += data[i].pos_gamma;
        total_gamma_diff += data[i].diff_gamma;
        total_delta_pos += data[i].pos_delta;
        total_delta_diff += data[i].diff_delta;
    }

    // printing totals
    cout << "Bits used for the positions (Elias Gamma encoding): " << total_gamma_pos << endl;
    cout << "Bits used for the difference between positions (Elias Gamma encoding): " << total_gamma_diff << endl;
    cout << endl;
    cout << "Bits used for the positions (Elias Delta encoding): " << total_delta_pos << endl;
    cout << "Bits used for the difference between positions (Elias Delta encoding): " << total_delta_diff << endl;

    return 0;
}
