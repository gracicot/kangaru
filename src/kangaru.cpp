module;

#include <array>
#include <exception>
#include <type_traits>
#include <cstddef>
#include <cstdlib>
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
#include <new>
#include <cstdint>
#include <functional>
#include <compare>
#include <initializer_list>
#include <algorithm>
#include <tuple>
#include <typeindex>
#include <version>


#include "kangaru/detail/define.hpp"

#if KANGARU5_OPTIONAL_IMPLEMENTATION() == 1
	#include <optional>
#elif KANGARU5_OPTIONAL_IMPLEMENTATION() == 2
	#include <tl/optional.hpp>
#elif KANGARU5_OPTIONAL_IMPLEMENTATION() == 3
	// No includes from the outside when using embedded optional
#else
	#error "Unknown optional implementation"
#endif

#include "kangaru/detail/undef.hpp"

export module kangaru;

#define KANGARU5_MODULES

extern "C++" {
	#include "kangaru/kangaru.hpp"
}
