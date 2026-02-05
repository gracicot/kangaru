module;

#include <exception>
#include <type_traits>
#include <cstddef>
#include <memory>
#include <limits>
#include <concepts>
#include <utility>
#include <any>
#include <unordered_map>
#include <iterator>
#include <string_view>
#include <vector>
#include <span>
#include <cstdint>
#include <functional>
#include <compare>
#include <initializer_list>
#include <algorithm>
#include <tuple>
#include <version>

export module kangaru;

#define KANGARU5_MODULES

extern "C++" {
	#include "kangaru/kangaru.hpp"
}
