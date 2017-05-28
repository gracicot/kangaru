#ifndef KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP

#include "container.hpp"
#include "detail/injected.hpp"
#include "detail/traits.hpp"

namespace kgr {
namespace detail {

template<typename Generic, typename Type, typename = void>
struct GenericServiceDestruction {
	~GenericServiceDestruction() {
		static_cast<Generic&>(*this).instance().~Type();
	}
};

template<typename Generic, typename Type>
struct GenericServiceDestruction<Generic, Type, enable_if_t<std::is_trivially_destructible<Type>::value>> {};

} // namespace detail

template<typename Type>
struct GenericService : detail::GenericServiceDestruction<GenericService<Type>, Type> {
	friend struct Container;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) noexcept {
		emplace(std::move(other.instance()));
	}
	
	GenericService& operator=(GenericService&& other) noexcept {
		emplace(std::move(other.instance()));
		return *this;
	}
	
	GenericService(const GenericService& other) = delete;
	GenericService& operator=(const GenericService& other) = delete;
	
	template<typename... Args, detail::enable_if_t<detail::is_someway_constructible<Type, Args...>::value, int> = 0>
	GenericService(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
protected:
	Type& instance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& instance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	friend struct detail::GenericServiceDestruction<GenericService<Type>, Type>;
	template<typename, typename...> friend struct detail::has_emplace_helper;
	
	template<typename... Args, detail::enable_if_t<std::is_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<typename... Args, detail::enable_if_t<detail::is_only_brace_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type{std::forward<Args>(args)...};
	}
	
	detail::aligned_storage_t<sizeof(Type), alignof(Type)> _instance;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
