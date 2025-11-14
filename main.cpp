#include <iostream>
#include <vector>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <locale>

using std::cout;
using std::endl;
using std::thread;
using std::vector;


long long partialSum(const vector<int>& data, int start, int end) {
    long long sum = 0;
    for (int i = start; i < end; ++i) {
        sum += data[i];
    }
    return sum;
}

int main() {
    std::setlocale(LC_ALL, "");
    const int SIZE = 10000000;
    vector<int> vec(SIZE);


    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < SIZE; ++i) {
        vec[i] = std::rand() % 100;
    }

    // ---- 1. Îäíîïîòî÷íàÿ ñóììà ----
    std::clock_t startSingle = std::clock();

    long long sumSingle = 0;
    for (int i = 0; i < SIZE; ++i) {
        sumSingle += vec[i];
    }

    double timeSingle = static_cast<double>(std::clock() - startSingle) / CLOCKS_PER_SEC;

    cout << "Однопоточная сумма: " << sumSingle << endl;
    cout << "Время (1 поток): " << timeSingle << " сек" << endl << endl;
    // ---- 2. Ìíîãîïîòî÷íàÿ ñóììà ----
    int numThreads = 4;
    std::vector<long long> partialSums(numThreads, 0);
    std::vector<thread> threads;
    int step = SIZE / numThreads;

    std::clock_t startMulti = std::clock();

    for (int t = 0; t < numThreads; ++t) {
        int start = t * step;
        int end = (t == numThreads - 1) ? SIZE : start + step;
        threads.emplace_back([&vec, &partialSums, t, start, end]() {
            partialSums[t] = partialSum(vec, start, end);
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    long long sumMulti = 0;
    for (long long s : partialSums) {
        sumMulti += s;
    }

    double timeMulti = static_cast<double>(std::clock() - startMulti) / CLOCKS_PER_SEC;

    cout << "Многопоточная сумма: " << sumMulti << endl;
    cout << "Время (" << numThreads << " потоков): " << timeMulti << " сек" << endl;
    return 0;
}

