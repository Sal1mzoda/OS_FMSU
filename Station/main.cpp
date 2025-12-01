#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <ctime>

std::string now_time() {
    using namespace std::chrono;
    auto t = system_clock::now();
    std::time_t tt = system_clock::to_time_t(t);
    auto ms = duration_cast<milliseconds>(t.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&tt), "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::mutex cout_mtx;

void log_msg(const std::string& s) {
    std::lock_guard<std::mutex> lock(cout_mtx);
    std::cout << "[" << now_time() << "] " << s << std::endl;
}

struct TrainConfig {
    int id;
    int start;
};

int main() {
    std::vector<std::string> stations = {
        "Dushanbe", "Vahdat", "Obigarm", "Nurobod",
        "Tavildara", "Dangara", "Farkhor", "Kulob"
    };

    int ST  = static_cast<int>(stations.size());
    int NUM = 8;

    std::vector<std::mutex> st_mtx(ST);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dwell(200, 600);
    std::uniform_int_distribution<int> travel(300, 800);

    std::vector<TrainConfig> cfg;
    for (int i = 0; i < NUM; ++i) {
        cfg.push_back({ i + 1, i % ST });
    }

    auto train = [&](TrainConfig c) {
        int id  = c.id;
        int pos = c.start;
        int dir = 1;

        std::mt19937 lrng(id + static_cast<int>(
            std::chrono::system_clock::now().time_since_epoch().count()
        ));

        log_msg("Train " + std::to_string(id) +
                " started at station " + stations[pos]);

        while (true) {
            {
                std::unique_lock<std::mutex> lock(st_mtx[pos]);
                int d = dwell(lrng);
                log_msg("Train " + std::to_string(id) +
                        " is dwelling at " + stations[pos] +
                        " for " + std::to_string(d) + " ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(d));

                int next = pos + dir;

                if (next >= ST) {
                    dir = -1;
                    next = ST - 2;
                } else if (next < 0) {
                    dir = 1;
                    next = 1;
                }

                log_msg("Train " + std::to_string(id) +
                        " departed " + stations[pos] +
                        " -> " + stations[next]);
            }

            int t = travel(lrng);
            std::this_thread::sleep_for(std::chrono::milliseconds(t));

            {
                std::unique_lock<std::mutex> lock(st_mtx[pos + dir]);
                int next = pos + dir;
                if (next >= ST) {
                    next = ST - 1;
                } else if (next < 0) {
                    next = 0;
                }

                pos = next;

                log_msg("Train " + std::to_string(id) +
                        " arrived at " + stations[pos] +
                        " (" + std::to_string(t) + " ms)");
            }
        }
    };

    std::vector<std::thread> th;
    for (auto &c : cfg) {
        th.emplace_back(train, c);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    for (auto &t : th) t.join();

    return 0;
}

