#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP

#include "../container.hpp"

namespace kgr {
namespace detail {

template<typename T>
struct LazyHelper {
	using type = T;
protected:
	using ref = T&;
	using rref = T&&;
	using ptr = T*;
	using assign_value_type = T&&;
	
	T&& assign_value(T&& service) noexcept {
		return std::move(service);
	}
	
	T& value(T& service) noexcept {
		return service;
	}
};

template<typename T>
struct LazyHelper<T&> {
	using type = T*;
protected:
	using ref = T&;
	using rref = T&&;
	using ptr = T*;
	using assign_value_type = T*;
	
	T* assign_value(T& service) noexcept {
		return &service;
	}
	
	T& value(T* service) noexcept {
		return *service;
	}
};

template<typename CRTP, typename T, template<typename, typename, typename = void> class... Bases>
struct LazyCrtpHelper : Bases<CRTP, T>... {};

template<typename CRTP, typename T, typename = void>
struct LazyCopyConstruct {
	LazyCopyConstruct() = default;
	LazyCopyConstruct(const LazyCopyConstruct&) = delete;
	LazyCopyConstruct(LazyCopyConstruct&&) = default;
	LazyCopyConstruct& operator=(const LazyCopyConstruct&) = default;
	LazyCopyConstruct& operator=(LazyCopyConstruct&&) = default;
};

template<typename CRTP, typename T, typename = void>
struct LazyMoveAssign {
	LazyMoveAssign() = default;
	LazyMoveAssign(const LazyMoveAssign&) = default;
	LazyMoveAssign(LazyMoveAssign&&) = default;
	LazyMoveAssign& operator=(const LazyMoveAssign&) = default;
	LazyMoveAssign& operator=(LazyMoveAssign&&) = delete;
};

template<typename CRTP, typename T, typename = void>
struct LazyCopyAssign {
	LazyCopyAssign() = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(const LazyCopyAssign&) = delete;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
};

template<typename CRTP, typename T, typename = void>
struct LazyMoveConstruct {
	LazyMoveConstruct() = default;
	LazyMoveConstruct(const LazyMoveConstruct&) = default;
	LazyMoveConstruct(LazyMoveConstruct&&) = delete;
	LazyMoveConstruct& operator=(const LazyMoveConstruct&) = default;
	LazyMoveConstruct& operator=(LazyMoveConstruct&&) = default;
};

template<typename CRTP, typename T>
struct LazyCopyConstruct<CRTP, T, enable_if_t<std::is_copy_constructible<T>::value && !std::is_trivially_copy_constructible<T>::value>> {
	LazyCopyConstruct(const LazyCopyConstruct& other) noexcept(std::is_nothrow_copy_constructible<T>::value){
		auto&& o = static_cast<const CRTP&>(other);
		if (o._initialized) {
			static_cast<CRTP&>(*this).emplace(o.data());
		}
	}
	
	LazyCopyConstruct() = default;
	LazyCopyConstruct& operator=(const LazyCopyConstruct&) = default;
	LazyCopyConstruct(LazyCopyConstruct&&) = default;
	LazyCopyConstruct& operator=(LazyCopyConstruct&&) = default;
};

template<typename CRTP, typename T>
struct LazyCopyConstruct<CRTP, T, enable_if_t<std::is_trivially_copy_assignable<T>::value>> {};

template<typename CRTP, typename T>
struct LazyCopyAssign<CRTP, T, enable_if_t<
	std::is_copy_assignable<T>::value && std::is_copy_constructible<T>::value &&
	!(std::is_trivially_copy_assignable<T>::value && std::is_trivially_copy_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	LazyCopyAssign& operator=(const LazyCopyAssign& other)
	noexcept(
		std::is_nothrow_copy_assignable<T>::value &&
		std::is_nothrow_copy_constructible<T>::value &&
		std::is_nothrow_destructible<T>::value
	) {
		auto&& o = static_cast<const CRTP&>(other);
		auto&& self = static_cast<CRTP&>(*this);
		
		if (o._initialized) {
			self.assign(o.data());
		} else {
			self.destroy();
		}
		
		return *this;
	}
	
	LazyCopyAssign() = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
};

template<typename CRTP, typename T>
struct LazyCopyAssign<CRTP, T, enable_if_t<
	std::is_trivially_copy_assignable<T>::value &&
	std::is_trivially_copy_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

template<typename CRTP, typename T>
struct LazyMoveConstruct<CRTP, T, enable_if_t<std::is_move_constructible<T>::value && !std::is_trivially_move_constructible<T>::value>> {
	LazyMoveConstruct(LazyMoveConstruct&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
		auto&& o = static_cast<CRTP&>(other);
		if (o._initialized) {
			static_cast<CRTP&>(*this).emplace(std::move(o.data()));
		}
	}
	
	LazyMoveConstruct() = default;
	LazyMoveConstruct(const LazyMoveConstruct&) = default;
	LazyMoveConstruct& operator=(LazyMoveConstruct&&) = default;
	LazyMoveConstruct& operator=(const LazyMoveConstruct&) = default;
};

template<typename CRTP, typename T>
struct LazyMoveConstruct<CRTP, T, enable_if_t<std::is_trivially_move_constructible<T>::value>> {};

template<typename CRTP, typename T>
struct LazyMoveAssign<CRTP, T, enable_if_t<
	std::is_move_assignable<T>::value && std::is_move_constructible<T>::value &&
	!(std::is_trivially_move_assignable<T>::value && std::is_trivially_move_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	LazyMoveAssign& operator=(LazyMoveAssign&& other)
	noexcept(
		std::is_nothrow_move_assignable<T>::value &&
		std::is_nothrow_move_constructible<T>::value && 
		std::is_nothrow_destructible<T>::value
	) {
		auto&& o = static_cast<CRTP&>(other);
		auto&& self = static_cast<CRTP&>(*this);
		
		if (o._initialized) {
			self.assign(std::move(o.data()));
		} else {
			self.destroy();
		}
		
		return *this;
	}
	
	LazyMoveAssign() = default;
	LazyMoveAssign(const LazyMoveAssign&) = default;
	LazyMoveAssign(LazyMoveAssign&&) = default;
	LazyMoveAssign& operator=(const LazyMoveAssign&) = default;
};

template<typename CRTP, typename T>
struct LazyMoveAssign<CRTP, T, enable_if_t<
	std::is_trivially_move_assignable<T>::value &&
	std::is_trivially_move_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

template<typename CRTP, typename T, typename = void>
struct LazyDestruction {
	~LazyDestruction() noexcept(std::is_nothrow_destructible<T>::value) {
		destroy();
	}
	
protected:
	void destroy() noexcept(std::is_nothrow_destructible<T>::value) {
		auto&& self = static_cast<CRTP&>(*this);
		
		using DestructingType = typename CRTP::type;
		
		if (self._initialized) {
			self._initialized = false;
			self.data().~DestructingType();
		}
	}
};

template<typename CRTP, typename T>
struct LazyDestruction<CRTP, T, enable_if_t<std::is_trivially_destructible<T>::value>> {
protected:
	void destroy() noexcept {}
};

template<typename CRTP, typename T>
struct LazyBase :
	LazyHelper<ServiceType<T>>,
	LazyCrtpHelper<
		LazyBase<CRTP, T>, typename LazyHelper<ServiceType<T>>::type,
		LazyCopyConstruct, LazyCopyAssign, LazyMoveAssign, LazyMoveConstruct, LazyDestruction
	> {
private:
	using Helper = LazyHelper<ServiceType<T>>;
	
	using typename Helper::type;
	using typename Helper::ref;
	using typename Helper::rref;
	using typename Helper::ptr;
	using typename Helper::assign_value_type;
	
	template<typename, typename, typename> friend struct LazyCopyConstruct;
	template<typename, typename, typename> friend struct LazyMoveConstruct;
	template<typename, typename, typename> friend struct LazyCopyAssign;
	template<typename, typename, typename> friend struct LazyMoveAssign;
	template<typename, typename, typename> friend struct LazyDestruction;
	
	using Helper::assign_value;
	using Helper::value;
	using LazyDestruction<LazyBase<CRTP, T>, typename LazyHelper<ServiceType<T>>::type>::destroy;
	
	static constexpr bool nothrow_get = 
		noexcept(std::declval<Container>().service<T>()) &&
		std::is_nothrow_constructible<type, assign_value_type>::value;
	
public:
	LazyBase() = default;
	
	LazyBase& operator=(LazyBase&&) = default;
	LazyBase& operator=(const LazyBase&) = default;
	LazyBase(LazyBase&&) = default;
	LazyBase(const LazyBase&) = default;
	
	~LazyBase() = default;
	
	ref get() noexcept(nothrow_get) {
		if (!_initialized) {
			emplace(assign_value(static_cast<CRTP*>(this)->container().template service<T>()));
		}
		
		return value(data());
	}
	
	ref operator*() & noexcept(nothrow_get) {
		return get();
	}
	
	ptr operator->() noexcept(nothrow_get) {
		return &get();
	}
	
	rref operator*() && noexcept(nothrow_get) {
		return std::move(get());
	}
	
private:
	type& data() noexcept {
		return *reinterpret_cast<type*>(&_service);
	}
	
	const type& data() const noexcept {
		return *reinterpret_cast<const type*>(&_service);
	}
	
	template<typename... Args>
	void emplace(Args&&... args) noexcept(std::is_nothrow_constructible<type, Args...>::value) {
		destroy();
		
		_initialized = true;
		new (&_service) type(std::forward<Args>(args)...);
	}
	
	template<typename Arg>
	void assign(Arg&& arg) noexcept(std::is_nothrow_constructible<type, Arg>::value && std::is_nothrow_assignable<type, Arg>::value) {
		if (_initialized) {
			data() = std::forward<Arg>(arg);
		} else {
			emplace(std::forward<Arg>(arg));
		}
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(type), alignof(type)>::type _service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
