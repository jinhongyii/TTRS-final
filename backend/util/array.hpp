#pragma once
#ifndef SJTU_ARRAY_HPP
#define SJTU_ARRAY_HPP

#include "exceptions.hpp"
#include <new>
#include <climits>
#include <cstddef>
#include <vector>

namespace sjtu {
/**
 * a data container like std::array
 * store data in a successive memory and support random access.
 * string最大127个元素
 *
 * lengthtype 改成小类型是没用的，因为会有空隙
 */
    template <int length>
    class string;
    template<typename T , int sz,class lengthtype=short>
    class array {
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
            friend class array;

        private:
            array *thisvector;
            int currentPos;
        public:
            iterator (array *thisvector , int pos) : thisvector(thisvector) , currentPos(pos) {}

            iterator (const iterator &other) : thisvector(other.thisvector) , currentPos(other.currentPos) {}

            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */
            inline iterator operator+ (const int &n) const {
                auto tmp = *this;
                tmp.currentPos += n;
                return tmp;
            }

            inline iterator operator- (const int &n) const {
                auto tmp = *this;
                tmp.currentPos -= n;
                return tmp;
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            inline int operator- (const iterator &rhs) const {
                return currentPos - rhs.currentPos;
            }

            inline iterator operator+= (const int &n) {
                currentPos += n;
                return *this;
            }

            inline iterator operator-= (const int &n) {
                currentPos -= n;
                return *this;
            }

            inline iterator operator++ (int) {
                auto tmp = *this;
                operator++();
                return tmp;
            }

            inline iterator &operator++ () {
                currentPos++;
                return *this;
            }

            inline iterator operator-- (int) {
                auto tmp = *this;
                operator--();
                return tmp;
            }

            inline iterator &operator-- () {
                currentPos--;
                return *this;
            }

            inline T &operator* () const {
                return thisvector->data[currentPos];
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            inline  bool operator== (const iterator &rhs) const {
                return thisvector == rhs.thisvector && currentPos == rhs.currentPos;
            }

            inline bool operator== (const const_iterator &rhs) const {
                return thisvector == rhs.thisvector && currentPos == rhs.currentPos;
            }

            /**
             * some other operator for iterator.
             */
            inline bool operator!= (const iterator &rhs) const {
                return !operator==(rhs);
            }

            inline bool operator!= (const const_iterator &rhs) const {
                return !operator==(rhs);
            }
        };

        class const_iterator : public iterator {
        public:
            const_iterator (int pos) : iterator(pos) {}

            const_iterator (const const_iterator &other) : iterator(other) {}

            const_iterator (const iterator &other) : iterator(other) {}
        };

    private:
        lengthtype currentLength;
        T data[sz];



    public:
        /**
         *
         * Atleast three: default constructor, copy constructor and a constructor for std::vector
         */
        array () {
            currentLength = 0;
        }

        array (const array &other) {

            currentLength = other.currentLength;

            for (int i = 0; i < currentLength; i++) {
                data[i] = other.data[i];
            }
        }

        array (const std::vector<T> &other) {

            currentLength = other.size();
            for (int i = 0; i < currentLength; i++) {
                data[i] = other[i];
            }
        }

        /**
         *
         */
        ~array () {

        }

        /**
         *
         */
        array &operator= (const array &other) {
            currentLength = other.currentLength;
            for (int i = 0; i < currentLength; i++) {
                data[i] = other.data[i];
            }
            return *this;
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        inline T &at (const size_t &pos) {
//            if (pos < 0 || pos > size()) {
//                throw index_out_of_bound();
//            }
            return data[pos];
        }

        inline const T &at (const size_t &pos) const {
//            if (pos < 0 || pos > size()) {
//                throw index_out_of_bound();
//            }
            return data[pos];
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        inline T &operator[] (const size_t &pos) {
            return at(pos);
        }

        inline const T &operator[] (const size_t &pos) const {
            return at(pos);
        }

        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        inline const T &front () const {
//            if (size() == 0) {
//                throw container_is_empty();
//            }
            return data[0];
        }

        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        inline const T &back () const {
            return data[size() - 1];
        }

        /**
         * returns an iterator to the beginning.
         */
        inline iterator begin () {
            return iterator(this , 0);
        }

        inline const_iterator cbegin () const {
            return iterator(const_cast<array *>(this) , 0);
        }

        /**
         * returns an iterator to the end.
         */
        inline iterator end () {
            return iterator(this , size());
        }

        inline const_iterator cend () const {
            return iterator(const_cast<array *>(this) , size());
        }

        /**
         * checks whether the container is empty
         */
        inline bool empty () const { return size() == 0; }

        /**
         * returns the number of elements
         */
        inline size_t size () const { return currentLength; }

        /**
         * returns the number of elements that can be held in currently allocated storage.
         */
        inline size_t capacity () const { return sz; }

        /**
         * clears the contents
         */
        inline void clear () {
            currentLength = 0;
        }

        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert (iterator pos , const T &value) {
//            if (currentLength >= sz) {
//                throw ContainerIsFullException();
//            }
            for (int i = size(); i != pos.currentPos; i--) {
                data[i]=data[i-1];
            }
            data[pos.currentPos] = value;
            currentLength++;
            return iterator(this , pos.currentPos);
        }

        iterator insert (iterator pos , char *data_insert) {
//            if (currentLength >= sz) {
//                throw ContainerIsFullException();
//            }
            for (int i = size(); i != pos.currentPos; i--) {
                data[i]=data[i-1];
            }
            data[pos.currentPos] = *reinterpret_cast<T *>(data_insert);
            currentLength++;
            return iterator(this , pos.currentPos);
        }

        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value is true
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert (const size_t &ind , const T &value) {
//            if (ind > size()) {
//                throw index_out_of_bound();
//            }
//            if (currentLength >= sz) {
//                throw ContainerIsFullException();
//            }

            for (int i = size(); i != ind; i--) {
                data[i]=data[i-1];
            }
            data[ind] = value;
            currentLength++;
            return iterator(this , ind);
        }

        iterator insert (const size_t &ind , char *data_insert) {
            if (ind > size()) {
                return insert(size(),data_insert);

            }
//            if (currentLength >= sz) {
//                throw ContainerIsFullException();
//            }

            for (int i = size(); i != ind ; i--) {
                data[i]=data[i-1];
            }
            data[ind] = *reinterpret_cast<T *>(data_insert);
            currentLength++;
            return iterator(this , ind);
        }

        void update(const size_t &ind, char *data_update) {
            data[ind] = *reinterpret_cast<T *>(data_update);
        }

        void update(const size_t &ind, const T &value) {
            data[ind] = value;
        }

        void move(const size_t &ind, const size_t &size, array &other, const size_t &ind2) {
            other.currentLength += size;
            for (int i = other.currentLength -1 ; i > ind2 + size - 1; --i) {
                other.data[i] = other.data[i - size];
            }
            for (int i = ind2; i <= ind2 + size - 1; ++i)
                other.data[i] = data[ind + i - ind2];
            currentLength -= size;
            for (int i = ind; i < currentLength; ++i)
                data[i] = data[i + size];
        }
        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase (iterator pos) {
            for (int i = pos.currentPos; i < size() - 1; i++) {
                data[i] = data[i + 1];
            }
            currentLength--;
            return iterator(this , pos.currentPos);

        }

        /**
         * removes the element with index ind.
         * return deleted element
         * throw index_out_of_bound if ind >= size
         */
        T erase (const size_t &ind) {
//            if (ind >= size()) {
//                throw index_out_of_bound();
//            }
            auto tmp=data[ind];
            for (int i = ind; i < size() - 1; i++) {
                data[i] = data[i + 1];
            }
            currentLength--;
            return tmp;
        }

        /**
         * adds an element to the end.
         */
        inline void push_back (const T &value) {
//            if (sz == currentLength) {
//                throw ContainerIsFullException();
//            }
           data[currentLength]=value;
            currentLength++;
        }

        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        inline T pop_back () {
//            if (size() == 0) {
//                throw container_is_empty();
//            }
            currentLength--;
            return data[currentLength];
        }

        void setsize(int size){
//            if (size > sz) {
//                throw InvalidArgumentException();
//            }
            currentLength=size;
        }


        array& mergeright (const array &other){
            memcpy(data+currentLength,other.data,other.size()*sizeof(T));
            currentLength+=other.size();
            return *this;
        }

        array &mergeleft (const array &other){
            memmove(data+other.currentLength,data,sizeof(T)*currentLength);
            memcpy(data,other.data,sizeof(T)*other.currentLength);
            currentLength+=other.currentLength;
            return *this;
        }
        template <class E,int length>
        friend std::istream&operator>>(std::istream&,array<E,length>&);
        template <int length>
        friend std::istream &operator>> (std::istream &is , string<length> &string1);
    };


}

#endif