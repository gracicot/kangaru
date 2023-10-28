#ifndef KANGARU5_DETAIL_MURMUR_HPP
#define KANGARU5_DETAIL_MURMUR_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <concepts>
#include <string_view>
#include <vector>

namespace kangaru::detail::murmur {
	using hash_t = std::uint64_t;
	
	inline constexpr auto murmur_default_seed = hash_t{0};
	
	template<typename T>
	concept byte_like =
		   std::same_as<char, T>
		or std::same_as<char8_t, T>
		or std::same_as<std::int8_t, T>
		or std::same_as<std::uint8_t, T>
		or std::same_as<std::byte, T>;
	
	/**
	 * Constexpr compatible implementation of murmur hash 64a.
	 * Theorically slightly slower since it only does aligned reads.
	 */
	template<byte_like T>
	constexpr auto murmur64a(std::span<T const> const buf, hash_t const seed = murmur_default_seed) noexcept -> hash_t {
		auto constexpr m = std::uint64_t{0xc6a4a7935bd1e995ull};
		auto constexpr r = int{47};
		
		auto const length = buf.size();
		
		auto hash = static_cast<std::size_t>(seed) ^ (length * m);
		auto data = buf.data();
		auto const end = buf.data() + (length & ~0b111ull); // floor to closest multiple of 8
		
		while (data != end) {
			auto k = std::uint64_t{0}
				| static_cast<std::uint64_t>(data[0]) << 0
				| static_cast<std::uint64_t>(data[1]) << 8
				| static_cast<std::uint64_t>(data[2]) << 16
				| static_cast<std::uint64_t>(data[3]) << 24
				| static_cast<std::uint64_t>(data[4]) << 32
				| static_cast<std::uint64_t>(data[5]) << 40
				| static_cast<std::uint64_t>(data[6]) << 48
				| static_cast<std::uint64_t>(data[7]) << 56;
			
			k *= m;
			k ^= k >> r;
			k *= m;
			
			hash ^= k;
			hash *= m;
			
			data += sizeof(std::uint64_t);
		}
		
		switch (length & 0b111ull) {
			case 7: hash ^= static_cast<std::uint64_t>(data[6]) << 48; [[fallthrough]];
			case 6: hash ^= static_cast<std::uint64_t>(data[5]) << 40; [[fallthrough]];
			case 5: hash ^= static_cast<std::uint64_t>(data[4]) << 32; [[fallthrough]];
			case 4: hash ^= static_cast<std::uint64_t>(data[3]) << 24; [[fallthrough]];
			case 3: hash ^= static_cast<std::uint64_t>(data[2]) << 16; [[fallthrough]];
			case 2: hash ^= static_cast<std::uint64_t>(data[1]) << 8;  [[fallthrough]];
			case 1: hash ^= static_cast<std::uint64_t>(data[0]) << 0;
			hash *= m;
		}
		
		hash ^= hash >> r;
		hash *= m;
		hash ^= hash >> r;
		
		return static_cast<hash_t>(hash);
	}
	
	constexpr auto murmur64a(std::string_view const buf, hash_t const seed = murmur_default_seed) noexcept -> hash_t {
		return murmur64a(std::span<char const>{buf.data(), buf.size()}, seed);
	}
	
	constexpr auto murmur64a(std::u8string_view const buf, hash_t const seed = murmur_default_seed) noexcept -> hash_t {
		return murmur64a(std::span<char8_t const>{buf.data(), buf.size()}, seed);
	}
	
	template<byte_like T>
	constexpr auto murmur64a(std::vector<T> const& buf, hash_t const seed = murmur_default_seed) noexcept -> hash_t {
		return murmur64a(std::span{buf}, seed);
	}
	
	template<byte_like T, std::size_t n>
	constexpr auto murmur64a(std::array<T, n> const& buf, hash_t const seed = murmur_default_seed) noexcept -> hash_t {
		return murmur64a(std::span<T const>{buf.data(), n}, seed);
	}
}

#endif // KANGARU5_DETAIL_MURMUR_HPP
