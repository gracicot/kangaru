#ifndef KGR_INCLUDE_KANGARU_DETAIL_LAZY_BASE
#define KGR_INCLUDE_KANGARU_DETAIL_LAZY_BASE

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
	
	T&& assign_value(T&& service) {
		return std::move(service);
	}
	
	T& value(T& service) {
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
	
	T* assign_value(T& service) {
		return &service;
	}
	
	T& value(T* service) {
		return *service;
	}
};

template<typename CRTP, typename T, template<typename, typename, typename = void> class... Bases>
struct LazyCrtpHelper : Bases<CRTP, T>... {};

template<typename CRTP, typename T, typename = void>
struct LazyCopyConstruct {
	LazyCopyConstruct(const LazyCopyConstruct&) = delete;
	
	LazyCopyConstruct() = default;
	LazyCopyConstruct& operator=(const LazyCopyConstruct&) = default;
	LazyCopyConstruct(LazyCopyConstruct&&) = default;
	LazyCopyConstruct& operator=(LazyCopyConstruct&&) = default;
};

template<typename CRTP, typename T>
struct LazyCopyConstruct<CRTP, T, detail::enable_if_t<std::is_copy_constructible<T>::value>> {
	LazyCopyConstruct(const LazyCopyConstruct& other) {
		auto& o = static_cast<const CRTP&>(other);
		if (o._initialized) {
			static_cast<CRTP&>(*this).emplace(o.data());
		}
	}
	
	LazyCopyConstruct() = default;
	LazyCopyConstruct& operator=(const LazyCopyConstruct&) = default;
	LazyCopyConstruct(LazyCopyConstruct&&) = default;
	LazyCopyConstruct& operator=(LazyCopyConstruct&&) = default;
};

template<typename CRTP, typename T, typename = void>
struct LazyCopyAssign {
	LazyCopyAssign& operator=(const LazyCopyAssign&) = delete;
	
	LazyCopyAssign() = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
};

template<typename CRTP, typename T>
struct LazyCopyAssign<CRTP, T, detail::enable_if_t<std::is_copy_assignable<T>::value && std::is_copy_constructible<T>::value>> {
	LazyCopyAssign& operator=(const LazyCopyAssign& other) {
		auto& o = static_cast<const CRTP&>(other);
		auto& self = static_cast<CRTP&>(*this);
		
		if (o._initialized) {
			self.assign(o.data());
		} else {
			self.destroy();
		}
		
		return *this;
	}
	
	LazyCopyAssign() = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
};

template<typename CRTP, typename T, typename = void>
struct LazyMoveConstruct {
	LazyMoveConstruct(LazyMoveConstruct&&) = delete;
	
	LazyMoveConstruct() = default;
	LazyMoveConstruct& operator=(const LazyMoveConstruct&) = default;
	LazyMoveConstruct& operator=(LazyMoveConstruct&&) = default;
	LazyMoveConstruct(const LazyMoveConstruct&) = default;
};

template<typename CRTP, typename T>
struct LazyMoveConstruct<CRTP, T, detail::enable_if_t<std::is_move_constructible<T>::value>> {
	LazyMoveConstruct(LazyMoveConstruct&& other) {
		auto& o = static_cast<CRTP&>(other);
		if (o._initialized) {
			static_cast<CRTP&>(*this).emplace(std::move(o.data()));
		}
	}
	
	LazyMoveConstruct() = default;
	LazyMoveConstruct& operator=(const LazyMoveConstruct&) = default;
	LazyMoveConstruct& operator=(LazyMoveConstruct&&) = default;
	LazyMoveConstruct(const LazyMoveConstruct&) = default;
};

template<typename CRTP, typename T, typename = void>
struct LazyMoveAssign {
	LazyMoveAssign& operator=(LazyMoveAssign&&) = delete;
	
	LazyMoveAssign() = default;
	LazyMoveAssign& operator=(const LazyMoveAssign&) = default;
	LazyMoveAssign(LazyMoveAssign&&) = default;
	LazyMoveAssign(const LazyMoveAssign&) = default;
};

template<typename CRTP, typename T>
struct LazyMoveAssign<CRTP, T, detail::enable_if_t<std::is_move_assignable<T>::value && std::is_move_constructible<T>::value>> {
	LazyMoveAssign& operator=(LazyMoveAssign&& other) {
		auto& o = static_cast<CRTP&>(other);
		auto& self = static_cast<CRTP&>(*this);
		
		if (o._initialized) {
			self.assign(std::move(o.data()));
		} else {
			self.destroy();
		}
		
		return *this;
	}
	
	LazyMoveAssign() = default;
	LazyMoveAssign& operator=(const LazyMoveAssign&) = default;
	LazyMoveAssign(LazyMoveAssign&&) = default;
	LazyMoveAssign(const LazyMoveAssign&) = default;
};

template<typename CRTP, typename T>
struct LazyBase :
	LazyHelper<ServiceType<T>>,
	LazyCrtpHelper<LazyBase<CRTP, T>, typename detail::LazyHelper<ServiceType<T>>::type, LazyCopyConstruct, LazyCopyAssign, LazyMoveAssign, LazyMoveConstruct> {
private:
	using typename detail::LazyHelper<ServiceType<T>>::type;
	using typename detail::LazyHelper<ServiceType<T>>::ref;
	using typename detail::LazyHelper<ServiceType<T>>::rref;
	using typename detail::LazyHelper<ServiceType<T>>::ptr;
	
	template<typename, typename, typename> friend struct LazyCopyConstruct;
	template<typename, typename, typename> friend struct LazyCopyAssign;
	template<typename, typename, typename> friend struct LazyMoveAssign;
	template<typename, typename, typename> friend struct LazyMoveConstruct;
	
public:
	LazyBase() = default;
	
	LazyBase& operator=(LazyBase&&) = default;
	LazyBase& operator=(const LazyBase&) = default;
	LazyBase(LazyBase&&) = default;
	LazyBase(const LazyBase&) = default;
	
	~LazyBase() {
		destroy();
	}
	
	ref operator*() & {
		return get();
	}
	
	ptr operator->() {
		return &get();
	}
	
	rref operator*() && {
		return std::move(get());
	}
	
	ref get() {
		if (!_initialized) {
			emplace(this->assign_value(static_cast<CRTP*>(this)->_container.template service<T>()));
		}
		
		return this->value(data());
	}
	
private:
	type& data() {
		return *reinterpret_cast<type*>(&_service);
	}
	
	const type& data() const {
		return *reinterpret_cast<const type*>(&_service);
	}
	
	template<typename... Args>
	void emplace(Args&&... args) {
		destroy();
		
		_initialized = true;
		new (&_service) type(std::forward<Args>(args)...);
	}
	
	void destroy() {
		if (_initialized) {
			_initialized = false;
			data().~type();
		}
	}
	
	template<typename Arg>
	void assign(Arg&& arg) {
		if (!_initialized) {
			emplace(std::forward<Arg>(arg));
		} else {
			data() = std::forward<Arg>(arg);
		}
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(type), alignof(type)>::type _service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_LAZY_BASE
