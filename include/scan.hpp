#pragma once

#include "parse.hpp"
#include "types.hpp"
#include <algorithm>
#include <expected>
#include <tuple>
#include <type_traits>

namespace stdx {

template <typename... Ts, std::size_t... Is>
auto create_tuple(std::index_sequence<Is...>, const std::vector<std::string_view> &formats,
                  const std::vector<std::string_view> &inputs)
    -> std::expected<std::tuple<Ts...>, details::scan_error> {

    std::tuple<std::expected<std::decay_t<Ts>, details::scan_error>...> parsed = {
        (formats[Is].empty() ? std::expected<std::decay_t<Ts>, details::scan_error>(std::decay_t<Ts>{})
                             : details::parse_value_with_format<std::decay_t<Ts>>(inputs[Is], formats[Is]))...};

    details::scan_error err{""};
    bool has_error = ((!std::get<Is>(parsed).has_value() ? (err = std::get<Is>(parsed).error(), true) : false) || ...);

    if (has_error) {
        return std::unexpected(err);
    }

    return std::tuple<Ts...>{std::move(*std::get<Is>(parsed))...};
}

// замените болванку функции scan на рабочую версию
template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {
    const auto sources = stdx::details::parse_sources(input, format);
    if (!sources.has_value()) {
        return std::unexpected(sources.error());
    }

    if (sources->first.size() != sources->second.size()) {
        return std::unexpected(details::scan_error{"Unformatted text in input and format string are different"});
    }

    if (sizeof...(Ts) != sources->first.size()) {
        return std::unexpected(details::scan_error{"Unformatted text in input and format string are different"});
    }

    auto status = create_tuple<Ts...>(std::make_index_sequence<sizeof...(Ts)>{}, sources->first, sources->second);

    if (!status.has_value())
        return std::unexpected(status.error());

    return details::scan_result<Ts...>{std::move(status.value())};
}

}  // namespace stdx
