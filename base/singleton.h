// Copyright (C) 2012 by wubenqi
//
// By: wubenqi<wubenqi@gmail.com>
//
// 单件设计模式实现
// 非线程安全
//

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_
#include<utility>
template<typename T>
class Singleton  {
public:
    template<typename...Args>
    static T& Instance(Args...args) {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance(std::forward<Args>(args)...);
        }
        return *(Singleton::s_instance);
    }
    template<typename...Args>
    static T* GetInstance(Args...args) {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance(std::forward<Args>(args)...);
        }
        return Singleton::s_instance;
    }
    
    template<typename...Args>
    static T* getInstance(Args...args) {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance(std::forward<Args>(args)...);
        }
        return Singleton::s_instance;
    }
    
    static void Destroy() {
        if(Singleton::s_instance!=0) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
    }
    
protected:
    Singleton()	{
        Singleton::s_instance = static_cast<T*>(this);
    }
    
    ~Singleton() {
        Singleton::s_instance = 0;
    }
    
private:
    template<typename...Args>
    static T* CreateInstance(Args...args){
        return new T(std::forward<Args>(args)...);
    }
    
    static void DestroyInstance(T* p) {
        delete p;
    }
    
private:
    static T *s_instance;
    
private:
    explicit Singleton(Singleton const &) { }
    Singleton& operator=(Singleton const&) { return *this; }
};

template<typename T>
T* Singleton<T>::s_instance=0;


#endif // BASE_SINGLETON_H_
