#pragma once
#ifndef __STRING__
#define __STRING__

#include "array.hpp"
namespace sjtu {
    template <int length>
    class string {
        array<char,length,char> str;
    public:
        string(const char* source){
            int k=strlen(source);
            if (k > length) {
                throw InvalidArgumentException();
            }
            for (int i = 0; i < k; i++) {
                str.push_back(source[i]);
            }
        }

        string(const std::string &source) {
            int k = source.size();
            if (k > length) {
                throw InvalidArgumentException();
            }
            for (int i = 0; i < k; ++i) {
                str.push_back(source[i]);
            }
        }
//        string(std::string other){
//            for (int i = 0; i < other.size(); i++) {
//                str.push_back(other[i]);
//            }
//        }
        string()= default;
        template <int l2>
        string(const string<l2>& other){
            if (other.size() > length) {
                throw InvalidArgumentException();
            }
            for (int i = 0; i < other.size(); i++) {
                str.push_back(other[i]);
            }
        }
        template <int l2>
        string&operator=(const string<l2>& other){
            new(this) string(other);
            return *this;
        }
        char* c_str() const{
            char* result=new char[size()+1];
            for (int i = 0; i < size(); i ++) {
                result[i]=(*this)[i];
            }
            result[size()]=0;
            return result;
        }

        inline int size()const {
            return str.size();
        }
        inline char& operator[](int pos){
            return str[pos];
        }
        inline const char&operator[](int pos)const{
            return str[pos];
        }

        string<length> &operator+= (char i){
            str.push_back(i);
            return *this;
        }
        template <int l2>
        bool operator==(const string<l2>& other){
            if (size() != other.size()) {
                return false;
            }
            for (int i = 0; i < size(); i++) {
                if (this->operator[](i) != other[i]) {
                    return false;
                }
            }
            return true;
        }
        string& operator++(){
            if (str.size() != length) {
                str.push_back(0x80);
                return *this;
            }
            bool tmp=true;
            int i=size()-1;
            while(tmp && i>=0) {
                str[i]++;
                tmp=(str[i]==0x80);
                i--;
            }
            return *this;
        }
        template<int l2>
        bool operator!= (const string<l2> &other){
            return !this->operator==(other);
        }
        template <int l1,int l2>
        friend string<l1+l2> operator+(const string<l1>& str1,const string<l2>& str2);
        template <int l>
        friend std::istream&operator>>(std::istream& is,string<l>&);
        template <int l>
        friend std::ostream& operator<<(std::ostream& os,const string<l>& str);
        operator std::string()const {
            std::string a;
            for (int i = 0; i < size(); i++) {
                a.push_back(str[i]);
            }
            return a;
        }
        template <int l2>
        int compareTo(const string<l2>& str2) const{
            for (int i = 0; i < std::min(size() , str2.size()); i++) {
                if ((*this)[i] < str2[i]) {
                    return -1;
                } else if ((*this)[i] > str2[i]) {
                    return 1;
                }
            }
            if ((*this).size() < str2.size()) {
                return -1;
            } else if ((*this).size() > str2.size()) {
                return 1;
            } else {
                return 0;
            }
        }

    };
    template <int l1,int l2>
    string<l1+l2> operator+(const string<l1>& str1,const string<l2>& str2){
        string<l1+l2> result;
        for (int i = 0; i < str1.size(); i++) {
            result.str.push_back(str1[i]);
        }
        for (int i = 0; i <  str2.size(); i++) {
            result.str.push_back(str2[i]);
        }
        return result;
    }

    template <int length>
    std::ostream& operator<<(std::ostream& os,const string<length>& str){
        for (int i = 0; i < str.size(); i++) {
            os<<str[i];
        }
        return os;
    }

    //max 40 bytes
    template<int length>
    std::istream &operator>> (std::istream &is , string<length> &string1){
        char buf[41]{};
        is>>buf;
        new(&string1) string<length>(buf);
        return is;
    }

}
#endif