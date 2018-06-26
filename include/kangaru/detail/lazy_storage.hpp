#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_STORAGE_HPP

#include <type_traits>
#include "traits.hpp"

namespace kgr {
namespace detail {

template<typename T>
using is_trivially_copy_constructible =
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
	bool_constant<__has_trivial_copy(T) && std::is_copy_constructible<T>::value>
#else
	std::is_trivially_copy_constructible<T>
#endif
;

template<typename T>
using is_trivially_copy_assignable =
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
	bool_constant<__has_trivial_assign(T) && std::is_copy_assignable<T>::value>
#else
	std::is_trivially_copy_assignable<T>
#endif
;

template<typename T>
using is_trivially_move_constructible =
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
	bool_constant<false_t<T>::value>
#else
	std::is_trivially_move_constructible<T>
#endif
;

template<typename T>
using is_trivially_move_assignable =
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
	bool_constant<false_t<T>::value>
#else
	std::is_trivially_move_assignable<T>
#endif
;

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
struct lazy_crtp_helper : Bases<CRTP, T>... {};

/*
 * This class implements the copy constructor of lazy_storage.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct lazy_copy_construct {
	lazy_copy_construct() = default;
	lazy_copy_construct(const lazy_copy_construct&) = delete;
	lazy_copy_construct(lazy_copy_construct&&) = default;
	lazy_copy_construct& operator=(const lazy_copy_construct&) = default;
	lazy_copy_construct& operator=(lazy_copy_construct&&) = default;
};

/*
 * This class is a specialization of lazy_copy_construct when T is copy constructible.
 * In that case, the copy constructor is implemented using emplacement.
 */
template<typename CRTP, typename T>
struct lazy_copy_construct<CRTP, T, enable_if_t<std::is_copy_constructible<T>::value && !is_trivially_copy_constructible<T>::value>> {
	lazy_copy_construct(const lazy_copy_construct& other) noexcept(std::is_nothrow_copy_constructible<T>::value){
		auto&& o = static_cast<const CRTP&>(other);
		if (o) {
			static_cast<CRTP&>(*this).emplace(o.data());
		}
	}
	
	lazy_copy_construct() = default;
	lazy_copy_construct& operator=(const lazy_copy_construct&) = default;
	lazy_copy_construct(lazy_copy_construct&&) = default;
	lazy_copy_construct& operator=(lazy_copy_construct&&) = default;
};

/*
 * This is a specialization of lazy_copy_construct when T is trivially copy constructible.
 * In that case, we become trivially copy constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct lazy_copy_construct<CRTP, T, enable_if_t<is_trivially_copy_assignable<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct lazy_copy_assign {
	lazy_copy_assign() = default;
	lazy_copy_assign(const lazy_copy_assign&) = default;
	lazy_copy_assign(lazy_copy_assign&&) = default;
	lazy_copy_assign& operator=(const lazy_copy_assign&) = delete;
	lazy_copy_assign& operator=(lazy_copy_assign&&) = default;
};

/*
 * This class is a specialization of lazy_copy_assign when T is copy assignable, and copy constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to copy contructing too.
 */
template<typename CRTP, typename T>
struct lazy_copy_assign<CRTP, T, enable_if_t<
	std::is_copy_assignable<T>::value && std::is_copy_constructible<T>::value &&
	!(is_trivially_copy_assignable<T>::value && is_trivially_copy_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	lazy_copy_assign& operator=(const lazy_copy_assign& other)
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
	
	lazy_copy_assign() = default;
	lazy_copy_assign(const lazy_copy_assign&) = default;
	lazy_copy_assign(lazy_copy_assign&&) = default;
	lazy_copy_assign& operator=(lazy_copy_assign&&) = default;
};

/*
 * This class is a specialization of lazy_copy_assign, when T is trivially copy assignable, and trivially copy constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct lazy_copy_assign<CRTP, T, enable_if_t<
	is_trivially_copy_assignable<T>::value &&
	is_trivially_copy_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the move constructor of lazy_storage.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct lazy_move_construct {
	lazy_move_construct() = default;
	lazy_move_construct(const lazy_move_construct&) = default;
	lazy_move_construct(lazy_move_construct&&) = delete;
	lazy_move_construct& operator=(const lazy_move_construct&) = default;
	lazy_move_construct& operator=(lazy_move_construct&&) = default;
};

/*
 * This class is a specialization of lazy_move_construct when T is move constructible.
 * In that case, the move constructor is implemented using emplacement.
 */
template<typename CRTP, typename T>
struct lazy_move_construct<CRTP, T, enable_if_t<std::is_move_constructible<T>::value && !is_trivially_move_constructible<T>::value>> {
	lazy_move_construct(lazy_move_construct&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
		auto&& o = static_cast<CRTP&>(other);
		if (o) {
			static_cast<CRTP&>(*this).emplace(std::move(o.data()));
		}
	}
	
	lazy_move_construct() = default;
	lazy_move_construct(const lazy_move_construct&) = default;
	lazy_move_construct& operator=(lazy_move_construct&&) = default;
	lazy_move_construct& operator=(const lazy_move_construct&) = default;
};

/*
 * This is a specialization of lazy_move_construct when T is trivially move constructible.
 * In that case, we become trivially move constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct lazy_move_construct<CRTP, T, enable_if_t<std::is_trivially_move_constructible<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct lazy_move_assign {
	lazy_move_assign() = default;
	lazy_move_assign(const lazy_move_assign&) = default;
	lazy_move_assign(lazy_move_assign&&) = default;
	lazy_move_assign& operator=(const lazy_move_assign&) = default;
	lazy_move_assign& operator=(lazy_move_assign&&) = delete;
};

/*
 * This class is a specialization of lazy_move_assign when T is move assignable, and move constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to move contructing too.
 */
template<typename CRTP, typename T>
struct lazy_move_assign<CRTP, T, enable_if_t<
	std::is_move_assignable<T>::value && std::is_move_constructible<T>::value &&
	!(is_trivially_move_assignable<T>::value && is_trivially_move_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	lazy_move_assign& operator=(lazy_move_assign&& other)
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
	
	lazy_move_assign() = default;
	lazy_move_assign(const lazy_move_assign&) = default;
	lazy_move_assign(lazy_move_assign&&) = default;
	lazy_move_assign& operator=(const lazy_move_assign&) = default;
};

/*
 * This class is a specialization of lazy_move_assign, when T is trivially move assignable, and trivially move constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct lazy_move_assign<CRTP, T, enable_if_t<
	is_trivially_move_assignable<T>::value &&
	is_trivially_move_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the destructor for lazy_storage.
 * By default, we call the destructor of the lazy object.
 */
template<typename CRTP, typename T, typename = void>
struct lazy_destruction {
	~lazy_destruction() noexcept(std::is_nothrow_destructible<T>::value) {
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
 * This class is a specialization of lazy_destruction when T is trivially destructible.
 * In that case, the destructor function does nothing and we don't implement the destructor.
 */
template<typename CRTP, typename T>
struct lazy_destruction<CRTP, T, enable_if_t<std::is_trivially_destructible<T>::value>> {
protected:
	void destructor() noexcept {}
};

/*
 * This class implements all the common things the lazy_storage must implement.
 */
template<typename CRTP, typename T>
struct lazy_storage_base :
	lazy_crtp_helper<
		CRTP, lazy_stored_type<T>,
		lazy_copy_construct, lazy_copy_assign, lazy_move_assign, lazy_move_construct, lazy_destruction> {
	
	using type = lazy_stored_type<T>;
	
	lazy_storage_base() = default;
	
	lazy_storage_base& operator=(lazy_storage_base&&) = default;
	lazy_storage_base& operator=(const lazy_storage_base&) = default;
	lazy_storage_base(lazy_storage_base&&) = default;
	lazy_storage_base(const lazy_storage_base&) = default;
	
	~lazy_storage_base() = default;
	
	friend lazy_move_assign<CRTP, lazy_stored_type<T>>;
	
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
	using lazy_destruction<CRTP, lazy_stored_type<T>>::destructor;
	
	type& data() noexcept {
		return *reinterpret_cast<type*>(&_data);
	}
	
	const type& data() const noexcept {
		return *reinterpret_cast<const type*>(&_data);
	}
	
	aligned_storage_t<sizeof(type), alignof(type)> _data;
};

/*
 * This class is the default storage class for services that returns an
 * object or a rvalue reference as it's forwarded service type.
 */
template<typename T>
struct lazy_storage : lazy_storage_base<lazy_storage<T>, T> {
private:
	using base = lazy_storage_base<lazy_storage<T>, T>;
	using base::_data;
	
	friend lazy_storage_base<lazy_storage<T>, T>;
	
public:
	using base::data;
	using base::emplace;
	using typename base::type;
	
	lazy_storage() = default;
	
	lazy_storage& operator=(lazy_storage&&) = default;
	lazy_storage& operator=(const lazy_storage&) = default;
	lazy_storage(lazy_storage&&) = default;
	lazy_storage(const lazy_storage&) = default;
	
	~lazy_storage() = default;
	
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
		base::destructor();
	}
	
private:
	void reset() noexcept(std::is_nothrow_destructible<type>::value) {
		base::destructor();
		
		_initialized = true;
	}
	
	bool _initialized = false;
};

/*
 * This class is a specialization of LazyStorage when T is a lvalue reference type.
 * In that case, we simply holds a pointer, and set it to null if no object is held.
 */
template<typename T>
struct lazy_storage<T&> : lazy_storage_base<lazy_storage<T&>, T&> {
private:
	using base = lazy_storage_base<lazy_storage<T&>, T&>;
	using base::_data;
	
	friend struct lazy_storage_base<lazy_storage<T&>, T&>;
	
public:
	using base::data;
	using base::emplace;
	using typename base::type;
	
	lazy_storage() {
		emplace(nullptr);
	}
	
	lazy_storage& operator=(lazy_storage&&) = default;
	lazy_storage& operator=(const lazy_storage&) = default;
	lazy_storage(lazy_storage&&) = default;
	lazy_storage(const lazy_storage&) = default;
	
	~lazy_storage() = default;
	
	explicit operator bool() const noexcept {
		return data() != nullptr;
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
