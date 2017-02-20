#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP

#include <type_traits>
#include "traits.hpp"

namespace kgr {
namespace detail {

template<typename T>
using lazy_stored_type = typename std::conditional<
	std::is_lvalue_reference<T>::value,
	typename std::add_pointer<typename std::remove_reference<T>::type>::type,
	typename std::decay<T>::type
>::type;

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
		if (o) {
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
		
		if (o) {
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
		if (o) {
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
		
		if (o) {
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
		destructor();
	}
	
protected:
	void destructor() noexcept(std::is_nothrow_destructible<T>::value) {
		auto&& self = static_cast<CRTP&>(*this);
		
		using DestructingType = typename CRTP::type;
		
		if (self) {
			self.data().~DestructingType();
		}
	}
};

template<typename CRTP, typename T>
struct LazyDestruction<CRTP, T, enable_if_t<std::is_trivially_destructible<T>::value>> {
protected:
	void destructor() noexcept {}
};

template<typename CRTP, typename T>
struct LazyStorageBase :
	LazyCrtpHelper<
		CRTP, lazy_stored_type<T>,
		LazyCopyConstruct, LazyCopyAssign, LazyMoveAssign, LazyMoveConstruct, LazyDestruction> {
	
	using type = lazy_stored_type<T>;
	
	LazyStorageBase() = default;
	
	LazyStorageBase& operator=(LazyStorageBase&&) = default;
	LazyStorageBase& operator=(const LazyStorageBase&) = default;
	LazyStorageBase(LazyStorageBase&&) = default;
	LazyStorageBase(const LazyStorageBase&) = default;
	
	~LazyStorageBase() = default;
	
	friend struct LazyMoveAssign<CRTP, lazy_stored_type<T>>;
	
	template<typename Arg>
	void assign(Arg&& arg) noexcept(std::is_nothrow_constructible<type, Arg>::value && std::is_nothrow_assignable<type, Arg>::value) {
		auto&& self = static_cast<CRTP&>(*this);
		
		if (self) {
			data() = std::forward<Arg>(arg);
		} else {
			emplace(std::forward<Arg>(arg));
		}
	}
	
	template<typename... Args>
	void emplace(Args&&... args) noexcept(std::is_nothrow_constructible<type, Args...>::value) {
		static_cast<CRTP&>(*this).reset();
		new (&_data) type(std::forward<Args>(args)...);
	}
	
protected:
	using LazyDestruction<CRTP, lazy_stored_type<T>>::destructor;
	
	type& data() noexcept {
		return *reinterpret_cast<type*>(&_data);
	}
	
	const type& data() const noexcept {
		return *reinterpret_cast<const type*>(&_data);
	}
	
	typename std::aligned_storage<sizeof(type), alignof(type)>::type _data;
};

template<typename T>
struct LazyStorage : LazyStorageBase<LazyStorage<T>, T> {
private:
	using Base = LazyStorageBase<LazyStorage<T>, T>;
	using Base::_data;
	
	friend struct LazyStorageBase<LazyStorage<T>, T>;
	
public:
	using Base::data;
	using Base::emplace;
	using typename Base::type;
	
	LazyStorage() = default;
	
	LazyStorage& operator=(LazyStorage&&) = default;
	LazyStorage& operator=(const LazyStorage&) = default;
	LazyStorage(LazyStorage&&) = default;
	LazyStorage(const LazyStorage&) = default;
	
	~LazyStorage() = default;
	
	explicit operator bool() const noexcept {
		return _initialized;
	}
	
	void construct(T&& value) noexcept(std::is_nothrow_constructible<type, T&&>::value) {
		emplace(std::move(value));
	}
	
	T& value() noexcept {
		return data();
	}
	
	void destroy() noexcept(std::is_nothrow_destructible<type>::value) {
		_initialized = false;
		Base::destructor();
	}
	
private:
	void reset() noexcept(std::is_nothrow_destructible<type>::value) {
		Base::destructor();
		
		_initialized = true;
	}
	
	bool _initialized = false;
};

template<typename T>
struct LazyStorage<T&> : LazyStorageBase<LazyStorage<T&>, T&> {
private:
	using Base = LazyStorageBase<LazyStorage<T&>, T&>;
	using Base::_data;
	
	friend struct LazyStorageBase<LazyStorage<T&>, T&>;
	
public:
	using Base::data;
	using Base::emplace;
	using typename Base::type;
	
	LazyStorage() {
		emplace(nullptr);
	}
	
	LazyStorage& operator=(LazyStorage&&) = default;
	LazyStorage& operator=(const LazyStorage&) = default;
	LazyStorage(LazyStorage&&) = default;
	LazyStorage(const LazyStorage&) = default;
	
	~LazyStorage() = default;
	
	explicit operator bool() const noexcept {
		return data();
	}
	
	void construct(T& value) noexcept(std::is_nothrow_constructible<type, type*>::value) {
		emplace(&value);
	}
	
	T& value() noexcept {
		return *data();
	}
	
	void destroy() noexcept {
		data() = nullptr;
	}
	
private:
	void reset() noexcept {}
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP
