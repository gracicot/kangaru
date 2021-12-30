#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_STRING_VIEW_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_STRING_VIEW_HPP

#include <utility>

namespace kgr {
namespace detail {

/**
 * A function that returns the size of a string at compile time
 */
constexpr auto strlen(char const* string) -> std::size_t {
	return *string == 0 ? std::size_t{0} : std::size_t{1} + strlen(string + 1);
}

/**
 * A function that returns if two strings are equal
 */
constexpr auto strncmp(char const* str1, char const* str2, std::size_t count) -> bool {
	return count == 0 || (*str1 == *str2 && strncmp(str1 + 1, str2 + 1, count - 1));
}

/**
 * This class is similar to a C++17 string view.
 * 
 * However, in C++11, you can't really have mutating member functions
 * This implementation skip such member functions.
 */
struct string_view {
	static constexpr auto npos = std::size_t(-1);
	
	constexpr string_view(char const* literal) noexcept : _data{literal}, _size{strlen(literal)} {}
	explicit constexpr string_view(char const* string, std::size_t size) noexcept : _data{string}, _size{size} {}
	
	constexpr auto operator==(string_view const& rhs) const noexcept -> bool {
		return _size == rhs._size && strncmp(_data, rhs._data, _size);
	}
	
	constexpr auto operator[](std::size_t index) const noexcept -> char const& {
		return _data[index];
	}
	
	constexpr auto begin() const noexcept -> char const* {
		return _data;
	}
	
	constexpr auto end() const noexcept -> char const* {
		return _data + _size;
	}
	
	constexpr auto cbegin() const noexcept -> char const* {
		return _data;
	}
	
	constexpr auto cend() const noexcept -> char const* {
		return _data + _size;
	}
	
	constexpr auto data() const noexcept -> char const* {
		return _data;
	}
	
	constexpr auto size() const noexcept -> std::size_t {
		return _size;
	}
	
	constexpr auto substr(std::size_t pos, std::size_t count = npos) const noexcept -> string_view {
		return string_view{_data + pos, count == npos ? _size - pos : count};
	}
	
	constexpr auto find(string_view str, std::size_t pos = 0) const noexcept -> std::size_t {
		return pos > _size || pos + str._size > _size ? npos : find_impl(_data + pos, str, _size - pos - str._size - 1);
	}
	
	constexpr auto starts_with(string_view str) const noexcept -> bool {
		return _size >= str._size && strncmp(_data, str._data, str._size);
	}
	
private:
	static constexpr auto find_impl(char const* haystack, string_view needle, std::size_t max_iter) -> std::size_t {
		return max_iter == 0 ? 0 : (
			strncmp(haystack, needle._data, needle._size) ? 0 : 1 + find_impl(haystack + 1, needle, max_iter - 1)
		);
	}
	
	char const* _data;
	std::size_t _size;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_STRING_VIEW_HPP
