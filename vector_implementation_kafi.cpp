#include <iostream>
#include <set>
#include <string>


/******************************************************************************************************/
#include <cstddef>
#include <memory>
#include <iterator>
#include <type_traits>
#include <stdexcept>
#include <initializer_list>

/*
 *Written by Kafi Ul Shabbir kafiulshabbir@gmail.com
 *Date of last modification: 2020-11-26 06:10
 *Project based on: https://github.com/Costello1329/cpp-interns-2019/tree/master/sem3/hw3_vector
 *Description: Almost mimicry of std::vector
 *Implementation Philoshopy:
 *  All private methods are static, to avoid confusion of which data it had moified
 *  Similar methods are created form a smallest-possible collection of static methods
 *  Use of iterators in the methods inside is avoided to maintain uniformity of simplest
 */

template <class T, class Alloc = std::allocator<T>>
class vector {
/// This private section contains members which are used mostly by the "Member types"
private:
	typedef std::allocator_traits<Alloc> _trait;
	
	template <class P, class R>
	class _itr_base {
	private:
		P _ptr;

	public:
		/// Constructors
		typedef std::random_access_iterator_tag iterator_category;
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef P pointer;
		typedef R reference;
		
		_itr_base () = delete;
		_itr_base (const _itr_base&) = default;
		_itr_base& operator= (const _itr_base&) = default;
		~_itr_base () = default;
		explicit _itr_base (pointer ptr): _ptr(ptr) {}
		
		/// Element/pointer access
		reference operator* () {
			return *_ptr;
		}
		
		pointer operator-> () {
			return _ptr;
		}
		
		reference operator[] (const difference_type& n) {
			return *(_ptr + n);
		}
		
		/// Changement operators
		_itr_base& operator++ () {
			++ _ptr;
			return *this;
		}
		
		_itr_base& operator-- () {
			-- _ptr;
			return *this;
		}
		
		_itr_base operator++ (int) {
			_itr_base temp(*this);
			++ _ptr;
			return temp;
		}
		
		_itr_base operator-- (int) {
			_itr_base temp(*this);
			-- _ptr;
			return temp;
		}
		
		_itr_base& operator+= (const difference_type& n) {
			_ptr += n;
			return *this;
		}
		
		_itr_base& operator-= (const difference_type& n) {
			_ptr -= n;
			return *this;
		}
		
		_itr_base operator+ (const difference_type& n) const {
			return _itr_base(*this) += n;
		}
		
		_itr_base operator- (const difference_type& n) const {
			return _itr_base(*this) -= n;
		}
		
		difference_type operator- (const _itr_base& other) const {
			return static_cast<difference_type>(_ptr - other._ptr);
		}
		
		/// Equality Operators
		bool operator== (const _itr_base& other) {
			return _ptr == other._ptr;
		}
		
		bool operator!= (const _itr_base& other) {
			return _ptr != other._ptr;
		}
		
		bool operator< (const _itr_base& other) {
			return _ptr < other._ptr;
		}
		
		bool operator<= (const _itr_base& other) {
			return _ptr <= other._ptr;
		}
		
		bool operator> (const _itr_base& other) {
			return _ptr > other._ptr;
		}
		
		bool operator>= (const _itr_base& other) {
			return _ptr >= other._ptr;
		}
	};
	
/// This Public section contains the "Member types"
public:
	typedef T									value_type;
	typedef Alloc								allocator_type;
	typedef std::size_t							size_type;
	typedef std::ptrdiff_t						difference_type;
	typedef value_type&							reference;
	typedef const value_type& 					const_reference;
	
	typedef typename _trait::pointer				pointer;
	typedef typename _trait::const_pointer			const_pointer;
	
	typedef _itr_base<pointer, reference> 				iterator;
	typedef _itr_base<const_pointer, const_reference> 	const_iterator;
	typedef std::reverse_iterator<iterator> 			reverse_iterator;
	typedef std::reverse_iterator<const_iterator>		const_reverse_iterator;
	
/// This Private section contains the minimum necessary members to maintain the data structure
private:
	allocator_type _alloc;
	pointer _begin;
	pointer _end;
	pointer _cap;
	
	
	static size_type __sz_ptr_dif (const pointer& begin, const pointer& end) {
		return static_cast<size_type>(end - begin);
	}
	
	static size_type __find_new_cap (
		const pointer& begin,
		const pointer& end,
		const pointer& cap,
		size_type amount
		)
	{
		size_type twice = size_type(2) * __sz_ptr_dif(begin, cap);
		size_type added = amount + __sz_ptr_dif(begin, end);
		if (twice > added) {
			return twice;
		}
		return added;
	}

	template <class InputIt>
	static size_type __sz_fwd_iter (InputIt first, InputIt last) {
		size_type count = 0;
		while (first ++ != last) {
			++ count;
		}
		return count;
	}

	
	static void __alloc_buffer (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap,
		size_type amount)
	{
		begin = end = cap = _trait::allocate(alloc, amount);
		cap += static_cast<difference_type>(amount);
	}
	
	
	template <typename ...Args>
	static void __construct_on_buffer (
		allocator_type& alloc,
		pointer& end,
		size_type amount,
		Args&& ...args)
	{
		for (size_type i(0); i < amount; ++ i) {
			_trait::construct(alloc, end ++, std::forward<Args>(args)...);
		}
	}
	
	
	template <class InputIt>
	static void __populate_from_iterator (
		allocator_type& alloc,
		pointer& end,
		InputIt first,
		InputIt last) 
	{
		while (first != last) {
			_trait::construct(alloc, end ++, *first);
			++ first;
		}
	}
	
	
	static void __populate_from_other (
		allocator_type& alloc,
		pointer& end,
		pointer other_begin,
		size_type count)
	{
		for (size_type i(0); i < count; ++ i) {
			_trait::construct(alloc, end++, *(other_begin++));
		}
	}
	
	
	static void __populate_by_move (
		allocator_type& alloc,
		pointer& end,
		pointer other_begin,
		size_type count)
	{
		for (size_type i(0); i < count; ++ i) {
			_trait::construct(alloc, end++, std::move(*(other_begin++)));
		}
	}
	
	static void __steal_contents (
		pointer& begin,
		pointer& end,
		pointer& cap,
		pointer& other_begin,
		pointer& other_end,
		pointer& other_cap)
	{
		begin = other_begin;
		end = other_end;
		cap = other_cap;
		other_begin = nullptr;
		other_end = nullptr;
		other_cap = nullptr;
	}
	
	
	static void __destroy_buffer (allocator_type& alloc, pointer& begin, pointer& end) {
		while (end != begin) {
			_trait::destroy(alloc, -- end);
		}
	}
	
	
	static void __deallocate_buffer (allocator_type& alloc, pointer& begin, size_type amount) {
		_trait::deallocate(alloc, begin, amount);
	}
	
	
	static void __clear_buffer (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap)
	{
		__destroy_buffer(alloc, begin, end);
		__deallocate_buffer(alloc, begin, __sz_ptr_dif(begin, cap));
		cap = begin;
	}


	static void __realloc_to_bigger_buffer (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap,
		size_type new_cap)
	{
		pointer old_begin = begin;
		pointer old_end = end;
		pointer old_cap = cap;
		__alloc_buffer(alloc, begin, end, cap, new_cap);
		__populate_by_move(alloc, end, old_begin, __sz_ptr_dif(old_begin, old_end));
		__clear_buffer(alloc, old_begin, old_end, old_cap);
	}
	
	
	static void __shrink_if_excess_free_space (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap)	
	{
		if(size_type(2) * __sz_ptr_dif(begin, end) <= __sz_ptr_dif(begin, cap)) {
			__clear_buffer(alloc, end, end, cap);
		}
	}
	
	
	static pointer __master_erase (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap,
		pointer where,
		difference_type count)
	{
		const difference_type ofset = where - begin;
		pointer shift_from = where + count;
		while (shift_from != end) {
			_trait::destroy(alloc, where);
			_trait::construct(alloc, where, std::move(*shift_from));
			++ where;
			++ shift_from;
		}
		__destroy_buffer(alloc, where, end);
		__shrink_if_excess_free_space(alloc, begin, end, cap);
		return begin + ofset;
	}
	
	template <typename ...Args>
	static pointer __master_insert (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap,
		pointer where,
		const difference_type amount,
		Args&& ...args)
	{
		const size_type quantitity_shift = __sz_ptr_dif(where, end);
		if (end + amount > cap) {
			const size_type new_cap = __find_new_cap(begin, end, cap, amount);
			difference_type relative_where = where - begin;
			__realloc_to_bigger_buffer(alloc, begin, end, cap, new_cap);
			where = begin + relative_where;
		}
		
		pointer from_where = end;
		for (size_type i(0); i < quantitity_shift; ++ i) {
			-- from_where;
			_trait::construct(alloc, from_where + amount, std::move(*(from_where)));
			_trait::destroy(alloc, from_where);
		}
		end += amount;
		pointer pseudo_end = where;
		__construct_on_buffer(alloc, pseudo_end, amount, std::forward<Args>(args)...);
		return where;
	}
	
	
	template <typename ...Args>
	static void __master_resize (
		allocator_type& alloc,
		pointer& begin,
		pointer& end,
		pointer& cap,
		size_type count,
		Args&& ...args)
	{
		size_type current_size = __sz_ptr_dif(begin, end);
		if (count < current_size)
		{
			pointer where = begin + static_cast<difference_type>(count);
			__master_erase(alloc, begin, end, cap, where, current_size - count);
		}
		else if (count > current_size) {
			__master_insert(alloc, begin, end, cap, end, count - current_size, std::forward<Args>(args)...);
		}
	}
	
/// This Public section contains all the methods the user needs to access	
public:
	
/// MEMBER FUNCTIONS
	//(1)
	vector () noexcept (noexcept(Alloc())): _alloc(Alloc()) {
		__alloc_buffer(_alloc, _begin, _end, _cap, 0);
	}
	
	//(2)
	explicit vector (const Alloc& alloc) noexcept: _alloc(alloc) {
		__alloc_buffer(_alloc, _begin, _end, _cap, 0);
	}
	
	//(3)
	vector (size_type count, const T& value, const Alloc& alloc = Alloc()): _alloc(alloc) {
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__construct_on_buffer(_alloc, _end, count, value);
	}
	
	//(4)
	explicit vector (size_type count, const Alloc& alloc = Alloc()): _alloc(alloc) {
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__construct_on_buffer(_alloc, _end, count);
	}
	
	
	//(5)
	/* Important NOTE:
	 * Optimization for random access is not done
	 * Constructor overload resolution ambiguity
	 * std::enable_if_t<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, int> = 0>
	 * Copied from:
	 * https://stackoverflow.com/questions/58646533/class-template-constructor-overload-resolution-ambiguity
	 */
	 
	template<class InputIt, std::enable_if_t<std::is_base_of<std::input_iterator_tag,
		typename std::iterator_traits<InputIt>::iterator_category>::value, int> = 0>
	vector (InputIt first, InputIt last, const Alloc& alloc = Alloc()): _alloc(alloc) {
		size_type count = __sz_fwd_iter(first, last);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__populate_from_iterator(_alloc, _end, first, last);
	}
	
	//(6)
	vector (const vector& other):
		_alloc(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other._alloc))
	{
		size_type count = __sz_ptr_dif(other._begin, other._end);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__populate_from_other(_alloc, _end, other._begin, count);
	}
	
	//(7)
	vector (const vector& other, const Alloc& alloc): _alloc(alloc) {
		size_type count = __sz_ptr_dif(other._begin, other._end);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__populate_from_other(_alloc, _end, other._begin, count);
	}
	
	//(8)
	vector (vector&& other) noexcept: _alloc(other._alloc) {
		__steal_contents(_begin, _end, _cap, other._begin, other._end, other._cap);
	}
	
	//(9)
	vector (vector&& other, const Alloc& alloc): _alloc(alloc) {
		__steal_contents(_begin, _end, _cap, other._begin, other._end, other._cap);
	}
	
	
	// (xx) OUT OF REQUIREMENT ADDED FOR FUN
	vector (std::initializer_list<T> init, const Alloc& alloc = Alloc()): _alloc(alloc) {
		__alloc_buffer(_alloc, _begin, _end, _cap, init.size());
		__populate_from_iterator(_alloc, _end, init.begin(), init.end());
	}
	
	//(10)
	~vector () {
		if (_begin != nullptr) {
			__clear_buffer(_alloc, _begin, _end, _cap);
		}
	}
	
	//(11)
	vector& operator= (const vector& other) {
		if (this == &other) {
			return *this;
		}
		_alloc = other._alloc;
		size_type count = __sz_ptr_dif(other._begin, other._end);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__populate_from_other(_alloc, _end, other._begin, count);
	}
		
	//(12)	
	vector& operator= (vector&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		_alloc = other._alloc;
		__steal_contents(_begin, _end, _cap, other._begin, other._end, other._cap);
	}
	
	//(13)
	void assign (size_type count, const T& value) {
		__clear_buffer(_alloc, _begin, _end, _cap);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__construct_on_buffer(_alloc, _end, count, value);
	}
	
	//(14)
	template<class InputIt, std::enable_if_t<std::is_base_of<std::input_iterator_tag,
		typename std::iterator_traits<InputIt>::iterator_category>::value, int> = 0>
	void assign (InputIt first, InputIt last) {
		__clear_buffer(_alloc, _begin, _end, _cap);
		size_type count = __sz_fwd_iter(first, last);
		__alloc_buffer(_alloc, _begin, _end, _cap, count);
		__populate_from_iterator(_alloc, _end, first, last);
	}
	
	//(15)
	Alloc get_allocator () const noexcept {
		return _alloc;
	}
	
	
/// ELEMENT ACCESS
	//(16)
	reference at (size_type pos) {
		if (pos < __sz_ptr_dif(_begin, _end)) {
			return *(_begin  + static_cast<difference_type>(pos));
		}
		
		throw std::out_of_range("Illegal Access of out of range element!!");
		return *_begin;
	}
	
	//(17)
	const_reference at (size_type pos) const {
		if (pos < __sz_ptr_dif(_begin, _end)) {
			return *(_begin + static_cast<difference_type>(pos));
		}
		
		throw std::out_of_range("Illegal Access of out of range element!!");
		return *_begin;
	}
	
	//(18)
	reference operator[] (size_type pos) {
		return *(_begin  + static_cast<difference_type>(pos));
	}
	
	//(19)
	const_reference operator[] (size_type pos) const {
		return *(_begin  + static_cast<difference_type>(pos));
	}
	
	//(20)
	reference front () {
		return *_begin;
	}
	
	//(21)
	const_reference front () const {
		return *_begin;
	}
	
	//(22)
	reference back () {
		return *(_end - 1);
	}
	
	//(23)
	const_reference back () const {
		return *(_end - 1);
	}
	
	//(24)
	T* data () noexcept {
		return _begin;
	}
	
	//(25)
	const T* data () const noexcept {
		return _begin;
	}
	
	
/// Capacity:
	//(26)
	bool empty () const noexcept {
		return _begin == _end;
	}
	
	//(27)
	size_type size () const noexcept {
		return __sz_ptr_dif(_begin, _end);
	}
	
	//(28)
	void reserve (size_type new_cap) {
		if (new_cap <= capacity()) {
			return;
		}
		__realloc_to_bigger_buffer(_alloc, _begin, _end, _cap, new_cap);
	}
	
	//(29)
	size_type capacity () const noexcept {
		return __sz_ptr_dif(_begin, _cap);
	}
	
	//(30)
	void shrink_to_fit () {
		__clear_buffer(_alloc, _end, _end, _cap);
	}
	
	
/// Modifiers:
	//(31)
	void clear() noexcept {
		__clear_buffer(_alloc, _begin, _end, _cap);
	}
	
	//(32)
	iterator insert (const_iterator pos, const T& value) {
		pointer where = _begin + (pos - cbegin());
		return iterator(__master_insert(_alloc, _begin, _end, _cap, where, 1, value));
	}
	
	//(33)
	iterator insert (const_iterator pos, T&& value) {
		pointer where = _begin + (pos - cbegin());
		return iterator(__master_insert(_alloc, _begin, _end, _cap, where, 1, std::move(value)));
	}
	
	//(34)
	iterator insert (const_iterator pos, size_type count, const T& value) {
		pointer where = _begin + (pos - cbegin());
		return iterator(__master_insert(_alloc, _begin, _end, _cap, where, count, value));
	}
	
	//(35)
	template <class InputIt, std::enable_if_t<std::is_base_of<std::input_iterator_tag,
		typename std::iterator_traits<InputIt>::iterator_category>::value, int> = 0>
	iterator insert (const_iterator pos, InputIt first, InputIt last) {
		pointer where = _begin + (pos - cbegin());
		const difference_type amount = __sz_fwd_iter(first, last);
		const size_type quantitity_shift = __sz_ptr_dif(where, _end);
		
		if (_end + amount > _cap) {
			const size_type new_cap = __find_new_cap(_begin, _end, _cap, amount);
			difference_type relative_where = where - _begin;
			__realloc_to_bigger_buffer(_alloc, _begin, _end, _cap, new_cap);
			where = _begin + relative_where;
		}
		
		pointer from_where = _end;
		for (size_type i(0); i < quantitity_shift; ++ i) {
			-- from_where;
			_trait::construct(_alloc, from_where + amount, std::move(*(from_where)));
			_trait::destroy(_alloc, from_where);
		}
		
		_end += amount;
		pointer pseudo_end = where;
		while (first != last) {
			_trait::construct(_alloc, pseudo_end ++, *first);
		}
		return iterator(where);
	}
	
	
	//(36)
	template <class... Args>
	iterator emplace (const_iterator pos, Args&&... args) {
		pointer where = _begin + (pos - cbegin());
		return iterator(__master_insert(_alloc, _begin, _end, _cap, where, 1, std::forward<Args>(args)...));
	}
	
	
	//(37)
	iterator erase (const_iterator pos) {
		pointer where = _begin + (pos - cbegin());
		return iterator(__master_erase(_alloc, _begin, _end, _cap, where, 1));
	}
	
	//(38)
	iterator erase (const_iterator first, const_iterator last) {
		pointer where = _begin + (first - cbegin());
		return iterator(__master_erase(_alloc, _begin, _end, _cap, where, last - first));
	}
	
	//(39)
	void push_back (const T& value) {
		__master_insert(_alloc, _begin, _end, _cap, _end, 1, value);
	}
	
	//(40)
	void push_back (T&& value) {
		__master_insert(_alloc, _begin, _end, _cap, _end, 1, std::move(value));
	}
	
	//(41)
	template <class... Args>
	reference emplace_back (Args&&... args) {
		return *__master_insert(_alloc, _begin, _end, _cap, _end, 1, std::forward<Args>(args)...);
	}
	
	
	//(42)
	void pop_back () {
		__master_erase(_alloc, _begin, _end, _cap, _end - 1, 1);
	}
	
	//(43)
	void resize (size_type count) {
		__master_resize(_alloc, _begin, _end, _cap, count);
	}
	
	//(44)
	void resize (size_type count, const value_type& value) {
		__master_resize(_alloc, _begin, _end, _cap, count, value);
	}
	
	//(45)
	void swap (vector& other) noexcept {
		allocator_type temp_alloc = _alloc;
		pointer temp_begin = _begin;
		pointer temp_end = _end;
		pointer temp_cap = _cap;
		
		_alloc = other._alloc;
		_begin = other._begin;
		_end = other._end;
		_cap = other._cap;
		
		other._alloc = temp_alloc;
		other._begin = temp_begin;
		other._end = temp_end;
		other._cap = temp_cap;
	}
	
	
/// Exclusive iterator acesses
	//(46)
	iterator begin () {
		return iterator(_begin);
	}
	
	//(48)
	const_iterator begin () const {
		return const_iterator(_begin);
	}
	
	//(49)
	const_iterator cbegin () const {
		return const_iterator(_begin);
	}
	

	//(50)
	iterator end () {
		return iterator(_end);
	}
	
	//(51)
	const_iterator end () const {
		return const_iterator(_end);
	}
	
	//(52)
	const_iterator cend () const {
		return const_iterator(_end);
	}
	

	//(53)
	reverse_iterator rbegin () {
		return reverse_iterator(_begin);
	}
	
	//(54)
	const_reverse_iterator rbegin () const {
		return const_reverse_iterator(_begin);
	}
	
	//(55)
	const_reverse_iterator crbegin () const {
		return const_reverse_iterator(_begin);
	}
	

	//(56)
	reverse_iterator rend () {
		return reverse_iterator(_end);
	}
	
	//(57)
	const_reverse_iterator rend () const {
		return const_reverse_iterator(_end);
	}
	
	//(58)
	const_reverse_iterator crend () const {
		return const_reverse_iterator(_end);
	}
	
};


/// The comparison, and swap operator Overloads for vector
template <class T, class Alloc>
bool operator== (const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (size_t i = 0; i < lhs.size(); i ++) {
		if (lhs[i] != rhs[i]) {
			return false;
		}
	}
	return true;
}

template <class T, class Alloc>
bool operator!= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs) {
	return !(lhs == rhs);
}

template <class T, class Alloc>
bool operator< (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs) {
	for (size_t i = 0; i < lhs.size() && i < rhs.size(); i ++) {
		if (lhs[i] < rhs[i]) {
			return true;
		}
		else if(lhs[i] > rhs[i]) {
			return false;
		}
	}
	return lhs.size() < rhs.size();
}

template <class T, class Alloc>
bool operator<= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs) {
	return lhs == rhs || lhs < rhs;
}

template <class T, class Alloc>
bool operator> (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs) {
	return !(lhs <= rhs);
}

template <class T, class Alloc>
bool operator>= (const vector<T,Alloc>& lhs, const vector<T,Alloc>& rhs) {
	return !(lhs < rhs);
}

template <class T, class Alloc>		 
void std::swap (vector<T,Alloc>& lhs, vector<T,Alloc>& rhs) {
	lhs.swap(rhs);
}
/******************************************************************************************************/

template <class T>
std::ostream& operator<< (std::ostream& os, const vector<T>& v) {
	os << "-vec: [size = " << v.size() << "] + [capacity = " << v.capacity() << "] + [elem = {";
	for (int i = 0; i < v.size(); i++) {
		if(i) os << ", ";
		os << v[i];
	}
	os << "}]\n";
	return os;
}


class Dog {
private:
	int _x;
	double y;
	char p[10];

public:
	Dog () {
		std::cout << "->Dog () " << this << " is called" << std::endl;
	}
	
	Dog (const Dog& other): _x(other._x) {
		std::cout << "->Dog (const Dog& other): " << this << " is called" << std::endl;
	}
	
	Dog (Dog&& other): _x(other._x) {
		std::cout << "->Dog (Dog&& other) " << this << " is called" << std::endl;
	}
	
	Dog (int x): _x(x) {
		std::cout << "->Dog (int x) " << this << " is called x = " << _x << std::endl;
	}
		
	Dog& operator= (const Dog& other) {
		_x = other._x;
		std::cout << "->operator= (const Dog& other) " << this << " is called" << std::endl;
		return *this;
	}
	
	Dog& operator= (Dog&& other) {
		_x = other._x;
		std::cout << "->operator= (Dog&& other) " << this << " is called" << std::endl;
		return *this;
	}
	
	~Dog () {
		std::cout << "->~Dog () " << this << " is called" << std::endl;
	}
	
	friend std::ostream& operator<< (std::ostream&, const Dog&);
};

std::ostream& operator<< (std::ostream& os, const Dog& dog) {
	os << "d:" << dog._x;
	return os;
}

void test(const std::string& s) {
	static int count = 0;
	std::cout << std::endl << "Test-" << ++ count << ": " << s << ':' << std::endl;
}


void section(const std::string& s) {
	static int count = 0;
	std::cout << std::endl << std::endl << "---------------------------------------------------------------------------------" << std::endl;
	std::cout << "*****" << "Section-" << ++ count << ": " << s << "*****" << std::endl;
}

int main () {
	section("vector(), Constructors");
	{
		test("Default cons of size 5");
		vector<int> a(5);
		std::cout << a;
		
		test("Construct with count, value 5 x -1");
		vector<int> b(5, -1);
		std::cout << b;
		
		test("Construct with count, 3");
		vector<int> c(3);
		std::cout << c;
		
		test("Construct from a strange iterator, set<int> = {2, 4, 6, 9, 100}, proff that my constructor work good with non-random iterators");
		std::set<int> d{100, 9, 6, 4, 2};
		vector<int> e(d.begin(), d.end());
		std::cout << e;
	
		test("Copy constructor, copy previous");
		vector<int> f(e);
		std::cout << f;
		
		test("Move constructor");
		std::cout << "initial f" << f;
		vector<int> g(std::move(f));
		std::cout << "g(std::move(f))" << g;
		std::cout << "final f" << f;
		
		test("List Initializer, 4, 5 ... 8");
		vector<int> h{4, 5, 6, 7, 8};
		std::cout << h;
	}
	
	section("Destructor, operator=, assign");
	{
		vector<int> a{1, 2, 3, 4, 5};
		std::cout << "a" << a << std::endl;
		
		test("operator = copy");
		vector<int> b = a;
		std::cout << "b = a" << b;
		
		test("operator = move");
		vector<int> c = std::move(b);
		std::cout << "c = std::move(b)" << c;
		std::cout << "b" << b;
		
		test("assign b, count = 6, val = 3");
		b.assign(6, 3);
		std::cout << "b" << b;
		
		test("assign b, c.begin(), c.end() : Convincing proof they my custom_iterator tag is good");
		b.assign(c.begin(), c.end());
		std::cout << "b" << b;
		
		test("vector<Dog> d(3, -1), 3 destructors ->~Dog()");
		vector<Dog> d(3, -1);
		std::cout << "d" << d;
	}
	
	section("Element access at, [], back(), front()");
	{
		vector<int> a{1, 3, 8, 9, -3};
		std::cout << "a" << a;
		
		test("Access");
		std::cout << "front() = " << a.front() << std::endl;
		std::cout << "at(1) = " << a.at(1) << std::endl;
		std::cout << "[2] = " << a[2] << std::endl;
		std::cout << "data()[3] = " << a.data()[3]<< std::endl;
		std::cout << "back() = " << a.back() << std::endl;
	}

	section("Capacity");
	{
		vector<int> a;
		test("Is it empty?");
		std::cout << "a" << a;
		std::cout << "? empty = " << a.empty() << std::endl;
		
		test("assign(5, -1)");
		a.assign(5, -1);
		std::cout << "a" << a << std::endl;
		
		test("Size");
		std::cout << a.size() << std::endl;
		
		test("Is it empty?");
		std::cout << "? empty = " << a.empty() << std::endl;
		
		test("After reserve 100");
		a.reserve(100);
		std::cout << "a" << a;
		
		test("Shrink to fit");
		a.shrink_to_fit();
		std::cout << "a" << a;

		test("Proof that move happens on elements[->Dog&&] when capacity is enlarged");
		vector<Dog> b(5, 3);
		std::cout << "b" << b;
		std::cout << "b.reserve(100)" << std::endl;
		b.reserve(100);
		std::cout << "b" << b;
	}
	
	section("push_back()");
	{
		test("Push_back(), capacity(), comparison");
		vector<int> a;
		for (int i = 0; i < 12; ++ i) {
			a.push_back(i);
			std::cout << a;
		}
		
		test("Move assignment is working:");
		vector<Dog> b(3, 1);
		std::cout << b;
		
		Dog dog(3);
		
		std::cout << "after b.push_back(std::move(dog)):" << std::endl;
		b.push_back(std::move(dog));
		std::cout << b;
	}
	
	section("emplace_back()");
	{
		vector<Dog> b;
		test("Prove that construction, then move constructor is called for push_back()");
		for (int i = 0; i < 3; ++ i) {
			b.push_back(Dog(i));
			std::cout << "b" << b;
		}
		
		vector<Dog> c;
		test("Prove that construction is called for emplace_back()");
		for (int i = 0; i < 3; ++ i) {
			c.emplace_back(Dog(i));
			std::cout << "c" << c;
		}
	}
	
	section("pop_back()");
	{
		vector<int> a(20);
		test("pop_back(), capacity(), comparison");
		while (a.size() > 0) {
			a.pop_back();
			std::cout << a;
		}
	}
	
	section("insert");
	{
		vector<int> a(3, 1);
		std::cout << "a" << a << std::endl;
		auto it = a.cbegin();
		
		test("a.insert(it, 7, 2)");
		auto it1 = a.insert(it, 7, 2);
		std::cout << "a" << a;
		
		it1 += 3;
		it = a.cbegin() + (it1 - a.begin());
		test("a.insert(it, 2, 3)");
		a.insert(it, 2, 3);
		std::cout << "a" << a;
		
		test("a.insert(it, 2, 3)");
		a.insert(a.cend(), 5);
		std::cout << "a" << a;
		
	}
	
	
	
	section("erase");
	{
		vector<int> a;
		for(int i = 0; i < 20; ++ i) {
			a.push_back(i);
		}
		std::cout << "a" << a << std::endl;
		
		test("auto it = a.cbegin() + 1;	a.erase(it);");
		auto it = a.cbegin() + 1;
		a.erase(it);
		std::cout << "a" << a;
		
		test("a.erase(fr, to) fr = [1], to = size() - 2");
		auto fr = a.cbegin() + 1;
		auto to = a.cend() -2 ;
		a.erase(fr, to);
		std::cout << "a" << a;
	}
	
	section("Comparison, swap");
	{
		vector<int> a(3, 1);
		vector<int> b(3, 1);
		std::cout << "a" << a;
		std::cout << "b" << b;
		
		test("a == b");
		std::cout << (a == b) << std::endl;
		
		test("a != b");
		std::cout << (a != b) << std::endl;
		
		test("a < b");
		std::cout << (a < b) << std::endl;
		
		test("a <= b");
		std::cout << (a <= b) << std::endl;
		
		test("a > b");
		std::cout << (a > b) << std::endl;
		
		test("a >= b");
		std::cout << (a >= b) << std::endl;
		
		
		b.push_back(1);
		std::cout << std::endl;
		std::cout << "a" << a;
		std::cout << "b" << b;
		
		test("a == b");
		std::cout << (a == b) << std::endl;
		
		test("a != b");
		std::cout << (a != b) << std::endl;
		
		test("a < b");
		std::cout << (a < b) << std::endl;
		
		test("a <= b");
		std::cout << (a <= b) << std::endl;
		
		test("a > b");
		std::cout << (a > b) << std::endl;
		
		test("a >= b");
		std::cout << (a >= b) << std::endl;
		
		
		a.front() = 2;
		std::cout << std::endl;
		std::cout << "a" << a;
		std::cout << "b" << b;
		
		test("a == b");
		std::cout << (a == b) << std::endl;
		
		test("a != b");
		std::cout << (a != b) << std::endl;
		
		test("a < b");
		std::cout << (a < b) << std::endl;
		
		test("a <= b");
		std::cout << (a <= b) << std::endl;
		
		test("a > b");
		std::cout << (a > b) << std::endl;
		
		test("a >= b");
		std::cout << (a >= b) << std::endl;
		
		std::swap(a, b);
		std::cout << std::endl;
		std::cout << "a" << a;
		std::cout << "b" << b;
	}
	
	std::cout << "Program execution success!" <<std::endl;
	return 0;
}
