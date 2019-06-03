#pragma once
#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"
#include <new>
#include <climits>
#include <cstddef>
#include <vector>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector {
public:
	/**
	 *
	 * a type for actions of the elements of a vector, and you should write
	 *   a class named const_iterator with same interfaces.
	 */
	/**
	 * you can see RandomAccessIterator at CppReference for help.
	 */
	class const_iterator;
	class iterator {
		friend class vector;
	private:
		vector* thisvector;
		int currentPos;
	public:
		iterator(vector*thisvector,int pos):thisvector(thisvector),currentPos(pos){}
		iterator(const iterator& other):thisvector(other.thisvector),currentPos(other.currentPos){}
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, just return the answer.
		 * as well as operator-
		 */
		inline iterator operator+(const int &n) const {
			auto tmp=*this;
			tmp.currentPos+=n;
			return tmp;
		}
		inline iterator operator-(const int &n) const {
			auto tmp=*this;
			tmp.currentPos-=n;
			return tmp;
		}
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		inline int operator-(const iterator &rhs) const {
			return currentPos-rhs.currentPos;
		}
		iterator operator+=(const int &n) {
			currentPos+=n;
			return *this;
		}
		inline iterator operator-=(const int &n) {
			currentPos-=n;
			return *this;
		}

		inline iterator operator++(int) {
			auto tmp=*this;
			operator++();
			return tmp;
		}

		inline iterator& operator++() {
			currentPos++;
			return *this;
		}

		inline iterator operator--(int) {
			auto tmp=*this;
			operator--();
			return tmp;
		}

		inline iterator& operator--() {
			currentPos--;
			return *this;
		}

		inline T& operator*() const{
			return thisvector->data[currentPos];
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		inline bool operator==(const iterator &rhs) const {
			return thisvector==rhs.thisvector&&currentPos==rhs.currentPos;
		}
		inline bool operator==(const const_iterator &rhs) const {
			return thisvector==rhs.thisvector&&currentPos==rhs.currentPos;
		}
		/**
		 * some other operator for iterator.
		 */
		inline bool operator!=(const iterator &rhs) const {
			return !operator==(rhs);
		}
		inline bool operator!=(const const_iterator &rhs) const {
			return !operator==(rhs);
		}

		inline T*operator->(){
            return &thisvector->data[currentPos];
		}
	};

	class const_iterator :public iterator{
    public:
		const_iterator(int pos):iterator(pos){}
		const_iterator(const const_iterator& other):iterator(other){}
		const_iterator(const iterator& other):iterator(other){}
	};

private:
	T *data;
	int maxSize;
	int currentLength;

	void doubleSpace(){
		maxSize*=2;
		T* tmp=(T*)malloc(sizeof(T)*maxSize);
		for (int i = 0; i < currentLength; i++) {
			tmp[i]=data[i];
		}
		free(data);
		data=tmp;
	}
public:
	/**
	 *
	 * Atleast three: default constructor, copy constructor and a constructor for std::vector
	 */
	vector(int initSize=16) {
		data=(T*)malloc(sizeof(T)*initSize);

		maxSize=initSize;
		currentLength=0;
	}
	vector(const vector &other) {
		maxSize=other.maxSize;
		currentLength=other.currentLength;
        data=(T*)malloc(sizeof(T)*maxSize);
		for (int i = 0; i < currentLength; i++) {
			data[i]=other.data[i];
		}
	}

	vector (const std::vector<T> &other){
		maxSize=other.capacity();
		currentLength=other.size();
		data=malloc(sizeof(T)*maxSize);
		for (int i = 0; i < currentLength; i++) {
			data[i]=other[i];
		}
	}
	/**
	 *
	 */
	~vector() {
        for (int i = 0; i < currentLength; i++) {
            data[i].~T();
        }
		free(data);
	}
	/**
	 *
	 */
	vector &operator=(const vector &other) {
        for (int i = 0; i < currentLength; i++) {
            data[i].~T();
        }
		free(data);
		maxSize=other.maxSize;
		currentLength=other.currentLength;
        data=(T*)malloc(sizeof(T)*maxSize);
		for (int i = 0; i < currentLength; i++) {
			data[i]=other.data[i];
		}
		return *this;
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	inline T & at(const size_t &pos) {
//		if (pos < 0 || pos > size()) {
//			throw index_out_of_bound();
//		}
		return data[pos];
	}
	inline const T & at(const size_t &pos) const {
//		if (pos < 0 || pos > size()) {
//			throw index_out_of_bound();
//		}
		return data[pos];
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	inline T & operator[](const size_t &pos) {
		return at(pos);
	}
	inline const T & operator[](const size_t &pos) const {
		return at(pos);
	}
	/**
	 * access the first element.
	 * throw container_is_empty if size == 0
	 */
	const T & front() const {
		if (size() == 0) {
			throw container_is_empty();
		}
		return data[0];
	}
	/**
	 * access the last element.
	 * throw container_is_empty if size == 0
	 */
	const T & back() const {
		if (size() == 0) {
			throw container_is_empty();
		}
		return data[size()-1];
	}
	/**
	 * returns an iterator to the beginning.
	 */
	inline iterator begin() {
		return iterator(this,0);
	}
	const_iterator cbegin() const {
		return iterator(const_cast<vector*>(this),0);
	}
	/**
	 * returns an iterator to the end.
	 */
	inline iterator end() {
		return iterator(this,size());
	}
	const_iterator cend() const {
		return iterator(const_cast<vector*>(this),size());
	}
	/**
	 * checks whether the container is empty
	 */
	inline bool empty() const { return size()==0;}
	/**
	 * returns the number of elements
	 */
	inline size_t size() const { return currentLength;}
	/**
	 * returns the number of elements that can be held in currently allocated storage.
	 */
	inline size_t capacity() const { return maxSize;}
	/**
	 * clears the contents
	 */
	inline void clear() {
		currentLength=0;
	}
	/**
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value.
	 */
	iterator insert(iterator pos, const T &value) {
		if (currentLength >= maxSize ) {
			doubleSpace();
		}
		for (int i = size(); i != pos.currentPos; i--) {
            new(&data[i])T(data[i-1]);
		}
		data[pos.currentPos]=value;
		currentLength++;
		return iterator(this,pos.currentPos);
	}
	/**
	 * inserts value at index ind.
	 * after inserting, this->at(ind) == value is true
	 * returns an iterator pointing to the inserted value.
	 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
	 */
	iterator insert(const size_t &ind, const T &value) {
//		if (ind > size()) {
//			throw index_out_of_bound();
//		}
		if (currentLength >= maxSize ) {
			doubleSpace();
		}

		for (int i = size(); i != ind; i--) {
			new(&data[i])T(data[i-1]);
		}
		data[ind]=value;
		currentLength++;
		return iterator(this,ind);
	}
	/**
	 * removes the element at pos.
	 * return an iterator pointing to the following element.
	 * If the iterator pos refers the last element, the end() iterator is returned.
	 */
	iterator erase(iterator pos) {
		for (int i = pos.currentPos; i < size()-1; i++) {
			data[i]=data[i+1];
		}
		currentLength--;
		return iterator(this,pos.currentPos);

	}
	/**
	 * removes the element with index ind.
	 * return an iterator pointing to the following element.
	 * throw index_out_of_bound if ind >= size
	 */
	iterator erase(const size_t &ind) {
//		if (ind >= size()) {
//			throw index_out_of_bound();
//		}
		for (int i = ind; i < size()-1; i++) {
			data[i]=data[i+1];
		}
		currentLength--;
		return iterator(this,ind);
	}
	/**
	 * adds an element to the end.
	 */
	inline void push_back(const T &value) {
		if (maxSize == currentLength) {
			doubleSpace();
		}
		new (&data[currentLength])T(value);
		currentLength++;
	}
	/**
	 * remove the last element from the end.
	 * throw container_is_empty if size() == 0
	 */
	inline void pop_back() {
//		if (size() == 0) {
//			throw container_is_empty();
//		}
		currentLength--;
	}
};


}

#endif