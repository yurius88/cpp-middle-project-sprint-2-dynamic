#pragma once
#include <string>
#include <tuple>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
struct scan_result {
public:
    scan_result() = default;
    explicit scan_result(std::tuple<Ts...> data) : data_(std::move(data)) {}
    const std::tuple<Ts...> &values() const noexcept { return data_; }

private:
    std::tuple<Ts...> data_;
};

}  // namespace stdx::details
