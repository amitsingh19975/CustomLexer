#ifndef DARK_UTF8_HPP
#define DARK_UTF8_HPP

#include <array>
#include <cstddef>
#include <string_view>

namespace dark::detail::utf8 {
    static constexpr std::array<std::uint8_t, 16> lookup {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 2, 2, 3, 4
    };

    constexpr auto get_length(char c) -> std::uint8_t {
        auto const byte = static_cast<std::uint8_t>(c);
        return lookup[byte >> 4];
    }

	// INFO: We assume the user will pass the correct utf8 string
	constexpr auto to_utf32(std::string_view s) -> std::pair<char32_t, std::size_t> {
		if (s.empty()) return {};

		auto size = static_cast<std::size_t>(get_length(s[0]));
		char32_t codepoint = static_cast<char32_t>(s[0]);
		
		switch (size) {
			case 1: break;
			case 2: codepoint &= 0x1f; break;
			case 3: codepoint &= 0x0f; break;
			case 4: codepoint &= 0x07; break;
		}

	
		for (auto i = 1zu; i < size; ++i) {
			auto byte = static_cast<char32_t>(s[i]);
			codepoint = (codepoint << 6) | (byte & 0x3f);
		}


		return {codepoint, size};
	}

} // namespace dark::detail::utf8

#endif // DARK_UTF8_HPP

