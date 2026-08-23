#pragma once

#include <charconv>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {

// здесь ваш код
template <typename T>
using BaseType = std::remove_cvref_t<T>;

template <typename T>
concept IntegralType =
    (std::same_as<BaseType<T>, int8_t> || std::same_as<BaseType<T>, int16_t> || std::same_as<BaseType<T>, int32_t> ||
     std::same_as<BaseType<T>, int64_t> || std::same_as<BaseType<T>, uint8_t> || std::same_as<BaseType<T>, uint16_t> ||
     std::same_as<BaseType<T>, uint32_t> || std::same_as<BaseType<T>, uint64_t>) &&
    !std::is_reference_v<T>;
;

template <typename T>
concept FloatType = (std::same_as<BaseType<T>, float> || std::same_as<BaseType<T>, double>) && !std::is_reference_v<T>;
;

template <typename T>
concept StringType =
    (std::same_as<BaseType<T>, std::string> || std::same_as<BaseType<T>, std::string_view>) && !std::is_reference_v<T>;
;

template <typename T>
std::expected<T, scan_error> parse_value_impl(const std::string_view &input) {
    T value{};
    auto [ptr, err] = std::from_chars(input.data(), input.data() + input.size(), value);
    if (err != std::errc()) {
        return std::unexpected(details::scan_error("parse failed"));
    }

    return value;
}

template <typename T>
    requires IntegralType<T>
std::expected<T, scan_error> parse_value_int(const std::string_view &input) {
    return parse_value_impl<T>(input);
}

template <typename T>
    requires FloatType<T>
std::expected<T, scan_error> parse_value_double(const std::string_view &input) {
    return parse_value_impl<T>(input);
}

template <typename T>
    requires StringType<T>
std::expected<T, scan_error> parse_value_string(const std::string_view &input) {
    return T{input};
}

template <typename T>
std::expected<T, scan_error> parse_value(const std::string_view &) {
    return std::unexpected(details::scan_error("unexpected type"));
}

// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    if (fmt == "%d" || fmt == "%u") {
        if constexpr (!IntegralType<T>)
            return std::unexpected(scan_error("Format specifier %d or %u requires an integral type"));
    } else if (fmt == "%f") {
        if constexpr (!FloatType<T>)
            return std::unexpected(scan_error("Format specifier %f requires an floating point type"));
    } else if (fmt == "%s") {
        if constexpr (!StringType<T>)
            return std::unexpected(scan_error("Format specifier %s requires an string or string_view type"));
    } else {
        return std::unexpected(scan_error("Unsupported format"));
    }

    if constexpr (IntegralType<T>)
        return parse_value_int<T>(input);
    if constexpr (FloatType<T>)
        return parse_value_double<T>(input);
    if constexpr (StringType<T>)
        return parse_value_string<T>(input);

    return std::unexpected(scan_error("Unsupported format"));
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

}  // namespace stdx::details