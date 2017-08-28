#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP

#include <type_traits>
#include "traits.hpp"

namespace kgr {
namespace detail {

/*
 * This is is the contained type the lazy should hold.
 * If T is a reference, we must hold a pointer instead.
 * If T is a rvalue reference, we must hold an object.
 */
template<typename T>
using lazy_stored_type = typename std::conditional<
	std::is_lvalue_reference<T>::value,
	typename std::add_pointer<typename std::remove_reference<T>::type>::type,
	typename std::decay<T>::type
>::type;

/*
 * This is used to implement all of the CRTP classes at once, without copy-pasting the CRTP parameters.
 */
template<typename CRTP, typename T, template<typename, typename, typename = void> class... Bases>
struct LazyCrtpHelper : Bases<CRTP, T>... {};

/*
 * This class implements the copy constructor of LazyStorage.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct LazyCopyConstruct {
	LazyCopyConstruct() = default;
	LazyCopyConstruct(const LazyCopyConstruct&) = delete;
	LazyCopyConstruct(LazyCopyConstruct&&) = default;
	LazyCopyConstruct& operator=(const LazyCopyConstruct&) = default;
	LazyCopyConstruct& operator=(LazyCopyConstruct&&) = default;
};

/*
 * This class is a specialization of LazyCopyConstruct when T is copy constructible.
 * In that case, the copy constructor is implemented using emplacement.
 */
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

/*
 * This is a specialization of LazyCopyConstruct when T is trivially copy constructible.
 * In that case, we become trivially copy constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct LazyCopyConstruct<CRTP, T, enable_if_t<std::is_trivially_copy_assignable<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct LazyCopyAssign {
	LazyCopyAssign() = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(const LazyCopyAssign&) = delete;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
};

/*
 * This class is a specialization of LazyCopyAssign when T is copy assignable, and copy constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to copy contructing too.
 */
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
		
		// If as a value, assign it by copy.
		if (o) {
			self.assign(o.data());
		} else {
			// If we did not have any value, sinply destroy the value of this one.
			self.destroy();
		}
		
		return *this;
	}
	
	LazyCopyAssign() = default;
	LazyCopyAssign(const LazyCopyAssign&) = default;
	LazyCopyAssign(LazyCopyAssign&&) = default;
	LazyCopyAssign& operator=(LazyCopyAssign&&) = default;
};

/*
 * This class is a specialization of LazyCopyAssign, when T is trivially copy assignable, and trivially copy constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct LazyCopyAssign<CRTP, T, enable_if_t<
	std::is_trivially_copy_assignable<T>::value &&
	std::is_trivially_copy_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the move constructor of LazyStorage.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct LazyMoveConstruct {
	LazyMoveConstruct() = default;
	LazyMoveConstruct(const LazyMoveConstruct&) = default;
	LazyMoveConstruct(LazyMoveConstruct&&) = delete;
	LazyMoveConstruct& operator=(const LazyMoveConstruct&) = default;
	LazyMoveConstruct& operator=(LazyMoveConstruct&&) = default;
};

/*
 * This class is a specialization of LazyMoveConstruct when T is move constructible.
 * In that case, the move constructor is implemented using emplacement.
 */
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

/*
 * This is a specialization of LazyMoveConstruct when T is trivially move constructible.
 * In that case, we become trivially move constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct LazyMoveConstruct<CRTP, T, enable_if_t<std::is_trivially_move_constructible<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct LazyMoveAssign {
	LazyMoveAssign() = default;
	LazyMoveAssign(const LazyMoveAssign&) = default;
	LazyMoveAssign(LazyMoveAssign&&) = default;
	LazyMoveAssign& operator=(const LazyMoveAssign&) = default;
	LazyMoveAssign& operator=(LazyMoveAssign&&) = delete;
};

/*
 * This class is a specialization of LazyMoveAssign when T is move assignable, and move constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to move contructing too.
 */
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

/*
 * This class is a specialization of LazyCopyAssign, when T is trivially move assignable, and trivially move constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct LazyMoveAssign<CRTP, T, enable_if_t<
	std::is_trivially_move_assignable<T>::value &&
	std::is_trivially_move_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the destructor for LazyStorage.
 * By default, we call the destructor of the lazy object.
 */
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

/*
 * This class is a specialization of LazyDestruction when T is trivially destructible.
 * In that case, the destructor function does nothing and we don't implement the destructor.
 */
template<typename CRTP, typename T>
struct LazyDestruction<CRTP, T, enable_if_t<std::is_trivially_destructible<T>::value>> {
protected:
	void destructor() noexcept {}
};

/*
 * This class implements all the common things the LazyStorage must implement.
 */
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

/*
 * This class is the default storage class for services that returns an
 * object or a rvalue reference as it's forwarded service type.
 */
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

/*
 * This class is a specialization of LazyStorage when T is a lvalue reference type.
 * In that case, we simply holds a pointer, and set it to null if no object is held.
 */
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
