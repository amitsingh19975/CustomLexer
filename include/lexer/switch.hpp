#ifndef DARK_LEXER_SWITCH_HPP
#define DARK_LEXER_SWITCH_HPP

#include "static_string.hpp"
#include <algorithm>
#include <array>
#include <numeric>
#include <type_traits>

namespace dark::detail {

  // ["-", "->", "=", "=>", "int", "int8", "int32"]
  //                  |
  //                  V
  // [
  //   '-' => [
  //      '>': Token::Arrow,
  //       0 : Token::Sub
  //   ],
  //  ...
  // ]

  template <std::size_t N, typename Tag>
  struct Case: StaticString<N> {
    using tag_t = Tag;

    Tag tag;

    using StaticString<N>::StaticString;

    template<typename... Ts>
    constexpr Case(Tag t, Ts&&... ts)
      : StaticString<N>{std::forward<Ts>(ts)...}
      , tag(t)
    {}


    constexpr Case(Case const&) noexcept = default;
    constexpr Case& operator=(Case const&) noexcept = default;
    constexpr Case(Case &&) noexcept = default;
    constexpr Case& operator=(Case &&) noexcept = default;
    constexpr ~Case() noexcept = default;

    constexpr auto empty() const noexcept -> std::size_t { return StaticString<N>::size() == 0; }

    constexpr operator std::pair<std::string_view, Tag>() const noexcept {
      return { *this, tag };
    }
  };


  template <std::size_t N, typename T>
  Case(T,const char (&)[N]) -> Case<N - 1, T>;

  template <Case L0, Case... Ls>
    requires (std::same_as<typename decltype(L0)::tag_t, typename decltype(Ls)::tag_t> &&...)
  class Switch {
    static constexpr bool is_utf8_enabled = false;
    static constexpr std::array<std::pair<std::string_view, typename decltype(L0)::tag_t>, sizeof...(Ls) + 1> lexems = {L0, Ls...};
    static constexpr auto index_mapping = [] {
      std::array<unsigned char, 256> temp;
      std::fill(temp.begin(), temp.end(), 0);

      auto index = std::size_t{1};

      for (auto const [el, _] : lexems) {
        for (auto c : el) {
          auto idx = static_cast<size_t>(c);
          if (temp[idx] == 0)
            temp[idx] = static_cast<unsigned char>(index++);
        }
      }

      return temp;
    }();

    static constexpr auto max_len = std::max({L0.size(), (Ls.size())...});
    static constexpr auto max_extent = [] {
      auto sum = std::accumulate(
          index_mapping.begin(), index_mapping.end(), 0zu,
          [](auto acc, auto i) { return acc + static_cast<std::size_t>(i > 0); });
      return std::max(0zu, sum) + 1zu;
    }();

    static constexpr auto get_char_index(char index) noexcept {
      return index_mapping[static_cast<std::size_t>(index)];
    }

    template <std::size_t N> struct ArrayWrapper {
      std::array<std::size_t, N> data;
      std::array<std::size_t, max_len> stride{1};
    };

  public:
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

  private:
    static constexpr auto make_nd_map() noexcept {
      std::array<std::size_t, max_len> stride{1};
      constexpr auto size = [] {
        auto size = 1zu;
        for (auto i = 0zu; i < max_len; ++i) {
          size *= max_extent;
        }
        return size;
      }();

      for (auto i = 1zu; i < max_len; ++i) {
        stride[i] = stride[i - 1] * max_extent;
      }

      auto res = ArrayWrapper<size>{.stride = stride};

      std::fill(res.data.begin(), res.data.end(), npos);

      for (auto k = 0zu; auto const &[el, _] : lexems) {
        auto idx = 0zu;
        for (auto i = 0zu; auto c : el) {
          auto j = get_char_index(c);
          idx += stride[i++] * j;
        }

        res.data[idx] = k++;
      }

      return res;
    }

    static constexpr auto map = Switch::make_nd_map();

    /*static auto print(std::size_t const* a, std::size_t const*const s,*/
    /*  std::size_t i = 0, std::array<std::size_t, max_len> ids = {0}) {*/
    /*    if (i >= max_len) {*/
    /*        if (*a == npos) return;*/
    /*        std::cout << "Arr";*/
    /*        for (auto j = 0zu; j < max_len; ++j) {*/
    /*            std::cout << "[" << ids[j] << "]";*/
    /*        }*/
    /*        std::cout << ": " << *a << '\n';*/
    /*        return;*/
    /*    }*/
    /**/
    /*    for (auto e = 0zu; e < max_extent; ++e) {*/
    /*        ids[i] = e;*/
    /*        print(a + s[i] * e, s, i + 1, ids);*/
    /**/
    /*    }*/
    /**/
    /*}*/
  public:
       auto match(std::string_view s) const noexcept -> std::size_t {
        auto size = std::min(max_len, s.size());

        auto idx = 0zu;

        auto found_index = npos;

        for (auto i = 0zu; i < size; ++i) {
            auto c = get_char_index(s[i]);
            idx += map.stride[i] * c;
            auto temp = map.data[idx];
            if (temp != npos) found_index = temp;
        }

        return found_index;
      }

      constexpr auto match(char c) const noexcept -> std::size_t {
        auto idx = map.stride[0] * get_char_index(c);
        return map.data[idx];
      }
    
      constexpr auto str_from_index(std::size_t index) const noexcept -> std::string_view { return lexems[index].first; }
      constexpr auto token_from_index(std::size_t index) const noexcept -> typename decltype(L0)::tag_t { return lexems[index].second; }
  };

  template <typename T>
  struct is_switch: std::false_type{};
  
  template <Case... Cs>
  struct is_switch<Switch<Cs...>>: std::true_type{};

} // namespace dark::detail

#endif // DARK_LEXER_SWITCH_HPP
