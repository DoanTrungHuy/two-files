#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>

// 🔹 Sinh timestamp ngẫu nhiên
std::string randomTimestamp(std::mt19937& rng) {
    std::uniform_int_distribution<int> hour(0, 23);
    std::uniform_int_distribution<int> minute(0, 59);
    std::uniform_int_distribution<int> second(0, 59);

    char buffer[32];
    sprintf(buffer, "2025-10-22T%02d:%02d:%02dZ", hour(rng), minute(rng), second(rng));
    return buffer;
}

// 🔹 Chọn trang web ngẫu nhiên
std::string randomPage(std::mt19937& rng) {
    static std::vector<std::string> pages = {
        "Home", "Products", "About", "Contact", "FAQ", "Cart", "Blog", "Support"
    };
    std::uniform_int_distribution<int> dist(0, pages.size() - 1);
    return pages[dist(rng)];
}

// 🔹 Sinh log file
void generateLogFile(const std::string& filename, int numCustomers, int avgVisitsPerUser) {
    std::ofstream out(filename, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "❌ Không thể tạo file: " << filename << std::endl;
        return;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> visitCountDist(1, avgVisitsPerUser * 2);

    std::cout << "🔹 Đang tạo file: " << filename << std::endl;

    for (int customerId = 1; customerId <= numCustomers; ++customerId) {
        int visitCount = visitCountDist(rng);
        for (int i = 0; i < visitCount; ++i) {
            out << randomTimestamp(rng) << ","
                << randomPage(rng) << ","
                << customerId << "\n";
        }

        // In tiến trình mỗi 10,000 khách
        if (customerId % 10000 == 0)
            std::cout << "   → Đã sinh log cho " << customerId << " khách hàng\n";
    }

    out.flush();
    out.close();
    std::cout << "✅ Hoàn thành tạo file: " << filename << std::endl;
}

int main() {
    std::cout << "🚀 Bắt đầu tạo dữ liệu log...\n";

    // Test nhỏ để chắc chắn chạy đúng — bạn có thể tăng lên 100000 sau
    generateLogFile("log_day1.txt", 10000, 5);
    generateLogFile("log_day2.txt", 10000, 5);

    std::cout << "🏁 Hoàn tất.\n";
    return 0;
}
