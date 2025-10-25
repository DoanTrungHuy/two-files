#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

using namespace std;

template <typename T>
class BoundedQueue {
    queue<T> q;
    mutex mtx;
    condition_variable cv_not_full, cv_not_empty;
    size_t capacity;
    bool finished = false;

public:
    explicit BoundedQueue(size_t cap = 64) : capacity(cap) {}

    void push(T item) {
        unique_lock<mutex> lock(mtx);
        cv_not_full.wait(lock, [&] { return q.size() < capacity || finished; });
        if (finished) return;
        q.push(move(item));
        cv_not_empty.notify_one();
    }

    bool pop(T& out) {
        unique_lock<mutex> lock(mtx);
        cv_not_empty.wait(lock, [&] { return !q.empty() || finished; });
        if (q.empty()) return false;
        out = move(q.front());
        q.pop();
        cv_not_full.notify_one();
        return true;
    }

    void set_finished() {
        lock_guard<mutex> lock(mtx);
        finished = true;
        cv_not_empty.notify_all();
        cv_not_full.notify_all();
    }
};

class FileReader {
    string filename;
    size_t numThreads;

public:
    FileReader(string fname, size_t threads = 4)
        : filename(move(fname)), numThreads(threads) {}

    void readLines(function<void(const string&)> callback) {
        BoundedQueue<string> queue(256);

        thread producer([&] {
            ifstream file(filename);
            if (!file.is_open()) {
                cerr << "Cannot open file: " << filename << endl;
                queue.set_finished();
                return;
            }

            string line;
            while (getline(file, line)) {
                queue.push(move(line));
             }

            queue.set_finished();
        });

        vector<thread> workers;
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([&] {
                string line;
                while (queue.pop(line)) {
                    callback(line);
                }
            });
        }

        producer.join();
        for (auto& t : workers) t.join();
    }
};

int main() {
    unordered_map<string, unordered_set<string>> visits_day1;
    unordered_map<string, unordered_set<string>> visits_day2;
    mutex map_mtx;

    auto parseLog = [&](const string& line, auto& storage) {
        stringstream ss(line);
        string timestamp, page, customer;
        if (getline(ss, timestamp, ',') &&
            getline(ss, page, ',') &&
            getline(ss, customer)) {
            lock_guard<mutex> lock(map_mtx);
            storage[customer].insert(page);
        }
    };

    thread t1([&] {
        cout << "Reading day 1 logs...\n";
        FileReader reader1("log_day1.txt", 4);
        reader1.readLines([&](const string& line) {
            parseLog(line, visits_day1);
        });
    });

    thread t2([&] {
        cout << "Reading day 2 logs...\n";
        FileReader reader2("log_day2.txt", 4);
        reader2.readLines([&](const string& line) {
            parseLog(line, visits_day2);
        });
    });

    t1.join();
    t2.join();

    cout << "Finished reading files. Searching for loyal customers...\n";

    vector<string> loyal_customers;
    for (const auto& [cust, pages1] : visits_day1) {
        if (pages1.size() < 2) continue;
        auto it = visits_day2.find(cust);
        if (it != visits_day2.end() && it->second.size() >= 2) {
            loyal_customers.push_back(cust);
        }
    }

    cout << "Loyal customers found: " << loyal_customers.size() << endl;
    for (size_t i = 0; i < min<size_t>(loyal_customers.size(), 10); ++i) {
        cout << "- Customer " << loyal_customers[i] << endl;
    }

    return 0;
}