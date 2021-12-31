#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPTIONAL_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPTIONAL_HPP

#include "detail/traits.hpp"
#include "detail/define.hpp"

#include <type_traits>

namespace kgr {
namespace detail {

template<typename T>
using is_trivially_copy_constructible =
#if __GNUC__ < 5 && !defined(__clang__) && !defined(_MSC_VER)
	bool_constant<
		(__has_trivial_copy(T) && std::is_copy_constructible<T>::value) ||
		std::is_pointer<T>::value || std::is_arithmetic<T>::value>
#else
	std::is_trivially_copy_constructible<T>
#endif
;

template<typename T>
using is_trivially_copy_assignable =
#if __GNUC__ < 5 && !defined(__clang__) && !defined(_MSC_VER)
	bool_constant<
		(__has_trivial_assign(T) && std::is_copy_assignable<T>::value) ||
		std::is_pointer<T>::value || std::is_arithmetic<T>::value>
#else
	std::is_trivially_copy_assignable<T>
#endif
;

template<typename T>
using is_trivially_move_constructible =
#if __GNUC__ < 5 && !defined(__clang__) && !defined(_MSC_VER)
	bool_constant<std::is_pointer<T>::value || std::is_arithmetic<T>::value>
#else
	std::is_trivially_move_constructible<T>
#endif
;

template<typename T>
using is_trivially_move_assignable =
#if __GNUC__ < 5 && !defined(__clang__) && !defined(_MSC_VER)
	bool_constant<std::is_pointer<T>::value || std::is_arithmetic<T>::value>
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
using optional_stored_type = typename std::conditional<
	std::is_lvalue_reference<T>::value,
	typename std::add_pointer<typename std::remove_reference<T>::type>::type,
	typename std::decay<T>::type
>::type;

/*
 * This is used to implement all of the CRTP classes at once, without copy-pasting the CRTP parameters.
 */
template<typename CRTP, typename T, template<typename, typename, typename = void> class... Bases>
struct KGR_KANGARU_EMPTY_BASES optional_crtp_helper : Bases<CRTP, T>... {};

/*
 * This class implements the copy constructor of optional.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct optional_copy_construct {
	optional_copy_construct() = default;
	optional_copy_construct(const optional_copy_construct&) = delete;
	optional_copy_construct(optional_copy_construct&&) = default;
	optional_copy_construct& operator=(const optional_copy_construct&) = default;
	optional_copy_construct& operator=(optional_copy_construct&&) = default;
};

/*
 * This class is a specialization of optional_copy_construct when T is copy constructible.
 * In that case, the copy constructor is implemented using emplacement.
 */
template<typename CRTP, typename T>
struct optional_copy_construct<CRTP, T, enable_if_t<std::is_copy_constructible<T>::value && !is_trivially_copy_constructible<T>::value>> {
	optional_copy_construct(const optional_copy_construct& other) noexcept(std::is_nothrow_copy_constructible<T>::value){
		auto&& o = static_cast<const CRTP&>(other);
		if (o) {
			static_cast<CRTP&>(*this).emplace(o.data());
		}
	}
	
	optional_copy_construct() = default;
	optional_copy_construct& operator=(const optional_copy_construct&) = default;
	optional_copy_construct(optional_copy_construct&&) = default;
	optional_copy_construct& operator=(optional_copy_construct&&) = default;
};

/*
 * This is a specialization of optional_copy_construct when T is trivially copy constructible.
 * In that case, we become trivially copy constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct optional_copy_construct<CRTP, T, enable_if_t<is_trivially_copy_assignable<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct optional_copy_assign {
	optional_copy_assign() = default;
	optional_copy_assign(const optional_copy_assign&) = default;
	optional_copy_assign(optional_copy_assign&&) = default;
	optional_copy_assign& operator=(const optional_copy_assign&) = delete;
	optional_copy_assign& operator=(optional_copy_assign&&) = default;
};

/*
 * This class is a specialization of optional_copy_assign when T is copy assignable, and copy constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to copy contructing too.
 */
template<typename CRTP, typename T>
struct optional_copy_assign<CRTP, T, enable_if_t<
	std::is_copy_assignable<T>::value && std::is_copy_constructible<T>::value &&
	!(is_trivially_copy_assignable<T>::value && is_trivially_copy_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	optional_copy_assign& operator=(const optional_copy_assign& other)
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
	
	optional_copy_assign() = default;
	optional_copy_assign(const optional_copy_assign&) = default;
	optional_copy_assign(optional_copy_assign&&) = default;
	optional_copy_assign& operator=(optional_copy_assign&&) = default;
};

/*
 * This class is a specialization of optional_copy_assign, when T is trivially copy assignable, and trivially copy constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct optional_copy_assign<CRTP, T, enable_if_t<
	is_trivially_copy_assignable<T>::value &&
	is_trivially_copy_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the move constructor of optional.
 * By default, it is deleted.
 */
template<typename CRTP, typename T, typename = void>
struct optional_move_construct {
	optional_move_construct() = default;
	optional_move_construct(const optional_move_construct&) = default;
	optional_move_construct(optional_move_construct&&) = delete;
	optional_move_construct& operator=(const optional_move_construct&) = default;
	optional_move_construct& operator=(optional_move_construct&&) = default;
};

/*
 * This class is a specialization of optional_move_construct when T is move constructible.
 * In that case, the move constructor is implemented using emplacement.
 */
template<typename CRTP, typename T>
struct optional_move_construct<CRTP, T, enable_if_t<std::is_move_constructible<T>::value && !is_trivially_move_constructible<T>::value>> {
	optional_move_construct(optional_move_construct&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
		auto&& o = static_cast<CRTP&>(other);
		if (o) {
			static_cast<CRTP&>(*this).emplace(std::move(o.data()));
		}
	}
	
	optional_move_construct() = default;
	optional_move_construct(const optional_move_construct&) = default;
	optional_move_construct& operator=(optional_move_construct&&) = default;
	optional_move_construct& operator=(const optional_move_construct&) = default;
};

/*
 * This is a specialization of optional_move_construct when T is trivially move constructible.
 * In that case, we become trivially move constructible too, and simply copying the contained buffer.
 */
template<typename CRTP, typename T>
struct optional_move_construct<CRTP, T, enable_if_t<is_trivially_move_constructible<T>::value>> {};

/*
 * This class implements the copy assignation operator.
 * By default, it's deleted.
 */
template<typename CRTP, typename T, typename = void>
struct optional_move_assign {
	optional_move_assign() = default;
	optional_move_assign(const optional_move_assign&) = default;
	optional_move_assign(optional_move_assign&&) = default;
	optional_move_assign& operator=(const optional_move_assign&) = default;
	optional_move_assign& operator=(optional_move_assign&&) = delete;
};

/*
 * This class is a specialization of optional_move_assign when T is move assignable, and move constructible.
 * Since the assignation may be a construction if the lazy did not contained any value, we must be able to move contructing too.
 */
template<typename CRTP, typename T>
struct optional_move_assign<CRTP, T, enable_if_t<
	std::is_move_assignable<T>::value && std::is_move_constructible<T>::value &&
	!(is_trivially_move_assignable<T>::value && is_trivially_move_constructible<T>::value && std::is_trivially_destructible<T>::value)
>> {
	optional_move_assign& operator=(optional_move_assign&& other)
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
	
	optional_move_assign() = default;
	optional_move_assign(const optional_move_assign&) = default;
	optional_move_assign(optional_move_assign&&) = default;
	optional_move_assign& operator=(const optional_move_assign&) = default;
};

/*
 * This class is a specialization of optional_move_assign, when T is trivially move assignable, and trivially move constructible.
 * In this case, the buffer is simply copied into this one without any other operations.
 */
template<typename CRTP, typename T>
struct optional_move_assign<CRTP, T, enable_if_t<
	is_trivially_move_assignable<T>::value &&
	is_trivially_move_constructible<T>::value &&
	std::is_trivially_destructible<T>::value
>> {};

/*
 * This class implements the destructor for optional.
 * By default, we call the destructor of the lazy object.
 */
template<typename CRTP, typename T, typename = void>
struct optional_destruction {
	~optional_destruction() noexcept(std::is_nothrow_destructible<T>::value) {
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
 * This class is a specialization of optional_destruction when T is trivially destructible.
 * In that case, the destructor function does nothing and we don't implement the destructor.
 */
template<typename CRTP, typename T>
struct optional_destruction<CRTP, T, enable_if_t<std::is_trivially_destructible<T>::value>> {
protected:
	void destructor() noexcept {}
};

/*
 * This class implements all the common things the optional must implement.
 */
template<typename CRTP, typename T>
struct optional_base :
	optional_crtp_helper<
		CRTP, optional_stored_type<T>,
		optional_copy_construct, optional_copy_assign, optional_move_assign, optional_move_construct, optional_destruction> {
	
	using type = optional_stored_type<T>;
	
	optional_base() = default;
	
	optional_base& operator=(optional_base&&) = default;
	optional_base& operator=(optional_base const&) = default;
	optional_base(optional_base&&) = default;
	optional_base(optional_base const&) = default;
	
	~optional_base() = default;
	
	friend optional_move_assign<CRTP, optional_stored_type<T>>;
	
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
	using optional_destruction<CRTP, optional_stored_type<T>>::destructor;
	
	type& data() noexcept {
		return *static_cast<type*>(static_cast<void*>(&_data));
	}
	
	const type& data() const noexcept {
		return *static_cast<type const*>(static_cast<void const*>(&_data));
	}
	
	aligned_storage_t<sizeof(type), alignof(type)> _data;
};

} // namespace detail

/*
 * This class is the default storage class for services that returns an
 * object or a rvalue reference as it's forwarded service type.
 */
template<typename T>
struct optional : detail::optional_base<optional<T>, T> {
private:
	using base = detail::optional_base<optional<T>, T>;
	using base::_data;
	
	friend detail::optional_base<optional<T>, T>;
	
public:
	using base::data;
	using base::emplace;
	using typename base::type;
	using reference = T&;
	using const_reference = T const&;
	using value_type = T;
	using pointer = T*;
	using const_pointer = T const*;
	
	auto has_value() const noexcept -> bool {
		return _initialized;
	}
	
	explicit operator bool() const noexcept {
		return has_value();
	}
	
	void construct(T&& value) noexcept(std::is_nothrow_constructible<type, T&&>::value) {
		emplace(std::move(value));
	}
	
	auto value() noexcept -> reference {
		return data();
	}
	
	void destroy() noexcept(std::is_nothrow_destructible<type>::value) {
		_initialized = false;
		base::destructor();
	}
	
	auto operator*() const noexcept -> const_reference {
		return data();
	}
	
	auto operator*() noexcept -> reference {
		return data();
	}
	
	auto operator->() const noexcept -> const_pointer {
		return &data();
	}
	
	auto operator->() noexcept -> pointer {
		return &data();
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
struct optional<T&> : detail::optional_base<optional<T&>, T&> {
private:
	using base = detail::optional_base<optional<T&>, T&>;
	using base::_data;
	
	friend struct detail::optional_base<optional<T&>, T&>;
	
public:
	using base::data;
	using base::emplace;
	using typename base::type;
	using reference = T&;
	using const_reference = T const&;
	using value_type = T;
	using pointer = T*;
	using const_pointer = T const*;
	
	optional() {
		emplace(nullptr);
	}

	auto has_value() const noexcept -> bool {
		return data() != nullptr;
	}
	
	explicit operator bool() const noexcept {
		return has_value();
	}
	
	void construct(T& value) noexcept(std::is_nothrow_constructible<type, type*>::value) {
		emplace(&value);
	}
	
	auto value() noexcept -> reference {
		return *data();
	}
	
	void destroy() noexcept {
		data() = nullptr;
	}
	
	auto operator*() const noexcept -> const_reference {
		return *data();
	}
	
	auto operator*() noexcept -> reference {
		return *data();
	}
	
	auto operator->() const noexcept -> const_pointer {
		return data();
	}
	
	auto operator->() noexcept -> pointer {
		return data();
	}
	
private:
	void reset() noexcept {}
};

} // namespace kgr

#include "detail/undef.hpp"

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPTIONAL_HPP
