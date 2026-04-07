#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <cmath>
#include <cstring>
#include <algorithm>

using namespace std;

struct ThreadArg {
    int thread_id; 
    char symbol;
    string input_message; // original message
    string prev_symbols;

    // RESULTS
    string filtered;  // message without prev chars
    vector<int> positions; // list of positions
    vector<int> differences;
    int pos_gamma; // bits to represent the positions (Elias Gamma)
    int diff_gamma; // bits to represent diff of positions (Elias Gamma)
    
    int pos_delta; // bits to represent the positions (Elias Delta)
    int diff_delta; // bits to represent the diff of positions (Elias Gamma)

    // FOR SYNCHRONIZATION
    pthread_mutex_t* print_mutex; // control printing order
    pthread_cond_t* finished_cond; // signal when thread finishes
    int* threads_completed; // counter of finished threads
    int* total_threads; // total number of threads
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

void* threadwork(void* arg) {
    ThreadArg* data = (ThreadArg*)arg;

    // removing previous symbols
    string filtering = "";
    for (char c:data->input_message) {
        bool should_remove = false;
        for (char prev : data->prev_symbols) {
            if (c == prev) {
                should_remove = true;
                break;
            }
        }
        if (!should_remove) {
            filtering += c;
        }
    }
    data->filtered = filtering;

    // finding positions of symbols
    for (int i=0; i < filtering.size(); i++) {
        if(filtering[i] == data->symbol) {
            data->positions.push_back(i+1); // positions start at 1
        }
    }

    // finding differences between positions
    if(!data->positions.empty()) {
        data->differences.push_back(data->positions[0]);
        for (int i=0; i<data->positions.size() -1; i++) {
            data->differences.push_back(data->positions[i+1] - data->positions[i]);
        }
    }

    // calculating Elias encoding bits using helpers
    data->pos_gamma = EliasGamma(data->positions);
    data->diff_gamma = EliasGamma(data->differences);
    data->pos_delta = EliasDelta(data->positions);
    data->diff_delta = EliasDelta(data->differences);

    // print results using mutex to ensure only one thread prints at a time
    // FIRST CRITICAL SECTION
    pthread_mutex_lock(data->print_mutex);
    while(*data->threads_completed != data->thread_id) {
        pthread_cond_wait(data->finished_cond, data->print_mutex);
    }
    pthread_mutex_unlock(data->print_mutex);
    
    cout << "Symbol: " << data->symbol << ", Frequency: " << data->positions.size() << endl;
    cout << "Message: " << data->filtered << endl;
    cout << "Position(s): " << vector_to_string(data->positions) << endl;
    cout << "Difference between position(s): " << vector_to_string(data->differences) << endl;
    cout << "Number of bits to represent the position(s) (Elias Gamma): " << data->pos_gamma << endl;
    cout << "Number of bits to represent the difference between position(s) (Elias Gamma): " << data->diff_gamma << endl;
    cout << "Number of bits to represent the position(s) (Elias Delta): " << data->pos_delta << endl;
    cout << "Number of bits to represent the difference between position(s) (Elias Delta): " << data->diff_delta << endl;
    cout << endl;

    // SECOND CRITICAL SECTION
    pthread_mutex_lock(data->print_mutex);
    // increment counter and signal other threads
    (*data->threads_completed)++;
    pthread_cond_broadcast(data->finished_cond);
    pthread_mutex_unlock(data->print_mutex);
    
    return nullptr;
}

int main() {
    string message;
    getline(cin, message);

    vector<char> symbols;
    string line;
    while(getline(cin, line)) {
        if (!line.empty()) {
            symbols.push_back(line[0]);
        }
    }

    int num_threads = symbols.size();
    vector<pthread_t> threads(num_threads);
    vector<ThreadArg> thread_data(num_threads);

    // shared synchronization tools
    pthread_mutex_t print_mutex;
    pthread_cond_t finished_cond;
    int threads_completed = 0;

    pthread_mutex_init(&print_mutex, NULL);
    pthread_cond_init(&finished_cond, NULL);

    // create threads
    for (int i=0; i<num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].symbol = symbols[i];
        thread_data[i].input_message = message;
        thread_data[i].print_mutex = &print_mutex;
        thread_data[i].finished_cond = &finished_cond;
        thread_data[i].threads_completed = &threads_completed;
        thread_data[i].total_threads = &num_threads;

        thread_data[i].prev_symbols = "";
        for(int j=0; j<i; j++) {
            thread_data[i].prev_symbols += symbols[j];
        }
        pthread_create(&threads[i], NULL, threadwork, &thread_data[i]);
    }

    for (int i=0; i<num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // calculate totals
    int total_pos_gamma = 0;
    int total_diff_gamma = 0;
    int total_pos_delta = 0;
    int total_diff_delta = 0;
    
    for(int i=0; i<num_threads; i++) {
        total_pos_gamma += thread_data[i].pos_gamma;
        total_diff_gamma += thread_data[i].diff_gamma;
        total_pos_delta += thread_data[i].pos_delta;
        total_diff_delta += thread_data[i].diff_delta;
    }

    // printing totals
    cout << "Bits used for the positions (Elias Gamma encoding): " << total_pos_gamma << endl;
    cout << "Bits used for the difference between positions (Elias Gamma encoding): " << total_diff_gamma << endl;
    cout << endl;
    cout << "Bits used for the positions (Elias Delta encoding): " << total_pos_delta << endl;
    cout << "Bits used for the difference between positions (Elias Delta encoding): " << total_diff_delta << endl;

    // cleanup
    pthread_mutex_destroy(&print_mutex);
    pthread_cond_destroy(&finished_cond);

    return 0;
}
