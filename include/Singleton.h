/*
 * @Author: Ricardo
 * @Date: 2024-05-07 14:34:18
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-07 14:59:36
 * @Title: 单例
 */
#pragma once
#include "boost/numeric/conversion/converter_policies.hpp"
#include <iostream>
#include <memory>
#include <mutex>

namespace ICEY {
template <class T> class Singleton {
protected:
  /*
   * note:
   * 设置为保护是因为，派生类在创建对象时会先调用基类的构造函数
   * 即要让基类的构造函数仅对派生类可见
   */
  /**
   * @brief 构造函数（使用默认构造函数）
   */
  Singleton() = default;
  /**
   * @brief 拷贝构造函数（不允许拷贝构造）
   */
  Singleton(const Singleton<T> &) = delete;
  /**
   * @brief 对象赋值运算符重载（不允许重载赋值）
   */
  Singleton &operator=(const Singleton<T> &) = delete;

public:
  /**
   * @brief 析构函数
   */
  ~Singleton() { std::cout << "this is singleton destruct" << std::endl; }
  /**
   * @brief 获取单例对象
   */
  static std::shared_ptr<T> GetInstance() {
    static std::once_flag s_flag;
    std::call_once(s_flag, [&]() { m_instance = std::shared_ptr<T>(new T); });
    return m_instance;
  }
  /**
   * @brief 打印此对象地址
   */
  static void PrintAddress() { std::cout << m_instance->get() << std::endl; }

private:
  // 需要创建的单例对象指针
  static std::shared_ptr<T> m_instance;
};

// 初始化为nullptr
template <class T> std::shared_ptr<T> Singleton<T>::m_instance = nullptr;

} // namespace ICEY
