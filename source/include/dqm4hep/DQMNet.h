/// \file DQMNet.h
/*
 *
 * DQMNet.h header template automatically generated by a class generator
 * Creation date : dim. d�c. 4 2016
 *
 * This file is part of DQM4HEP libraries.
 *
 * DQM4HEP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * DQM4HEP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DQM4HEP.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef DQMNET_H
#define DQMNET_H

#include <memory>
#include <string>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <cstring>

#include "json/json.h"

namespace dqm4hep {

  namespace net {

    /**
     * Buffer struct.
     *
     * Simple structure to handle a buffer
     */
    struct Buffer
    {
      Buffer() :
        m_bufferSize(0),
        m_pBuffer(nullptr),
        m_owner(false)
      {
        /* nop */
      }

      ~Buffer()
      {
        if(nullptr != m_pBuffer && m_owner)
          delete [] m_pBuffer;
      }

      void setBuffer(char *data, uint32_t size, bool owner = false)
      {
        if(nullptr == data || 0 == size)
          return;

        if(nullptr != m_pBuffer && m_owner)
        {
          delete [] m_pBuffer;
          m_pBuffer = nullptr;
        }

        if(owner)
        {
          m_pBuffer = new char[size];
          m_bufferSize = size;
          memcpy(m_pBuffer, data, size);
        }
        else
        {
          m_pBuffer = data;
          m_bufferSize = size;
        }

        m_owner = owner;
      }

      uint32_t        m_bufferSize;       ///< The buffer size
      char *          m_pBuffer;          ///< The raw buffer as a char array
      bool            m_owner;
    };

    inline std::string &tolower(std::string& str) {
      std::transform(str.begin(), str.end(), str.begin(), ::tolower);
      return str;
    }

    template <typename T>
    struct convert;

    namespace conversion {
      inline bool isInfinity(const std::string &input) {
        return input == ".inf" || input == ".Inf" || input == ".INF" ||
        input == "+.inf" || input == "+.Inf" || input == "+.INF";
      }

      inline bool isNegativeInfinity(const std::string &input) {
        return input == "-.inf" || input == "-.Inf" || input == "-.INF";
      }

      inline bool isNan(const std::string &input) {
        return input == ".nan" || input == ".NaN" || input == ".NAN";
      }
    }

    // Json::Value spec
    template <>
    struct convert<Json::Value>
    {
      static bool encode(std::string &lhs, const Json::Value &rhs)
      {
        Json::FastWriter writer;
        lhs = writer.write(rhs);
        return true;
      }

      static bool decode(const std::string &lhs, Json::Value &rhs)
      {
        Json::Reader reader;
        return reader.parse(lhs, rhs);
      }
    };

    // Buffer spec
    template <>
    struct convert<Buffer>
    {
      static bool encode(std::string &lhs, const Buffer &rhs)
      {
        lhs.clear();

        if(nullptr == rhs.m_pBuffer || 0 == rhs.m_bufferSize)
          return false;

        lhs.assign(rhs.m_pBuffer, rhs.m_bufferSize);
        return true;
      }

      static bool decode(const std::string &lhs, Buffer &rhs)
      {
        char *data = const_cast<char*>(lhs.c_str());
        size_t size = lhs.size();

        rhs.setBuffer(data, size, true);

        return true;
      }
    };

    // std::string spec
    template <>
    struct convert<std::string>
    {
      static bool encode(std::string &lhs, const std::string &rhs)
      {
        lhs = rhs;
        return true;
      }

      static bool decode(const std::string &lhs, std::string &rhs)
      {
        rhs = lhs;
        return true;
      }
    };

    // C-strings can only be encoded
    template <>
    struct convert<const char*> {
      static bool encode(std::string &lhs, const char*const& rhs) { lhs = rhs; return true; }
    };

    template <>
    struct convert<char*> {
      static bool encode(std::string &lhs, const char*& rhs) { lhs = rhs; return true; }
    };

    template <std::size_t N>
    struct convert<const char[N]> {
      static bool encode(std::string &lhs, const char (&rhs)[N]) { lhs = rhs; return true; }
    };

    template <std::size_t N>
    struct convert<char[N]> {
      static bool encode(std::string &lhs, const char (&rhs)[N]) { lhs = rhs; return true; }
    };

    // thank you yaml-cpp !
    #define DQMNET_DEFINE_CONVERT_STREAMING(Type, negative_op) \
    template <> \
    struct convert<Type> \
    { \
      static bool encode(std::string &lhs, const Type &rhs) \
      { \
        std::stringstream stream; \
        stream.precision(std::numeric_limits<Type>::digits10 + 1); \
        stream << rhs; \
        lhs = stream.str(); \
        return true; \
      } \
      static bool decode(const std::string &lhs, Type &rhs) \
      { \
        std::stringstream stream(lhs); \
        stream.unsetf(std::ios::dec); \
        if ((stream >> std::noskipws >> rhs) && (stream >> std::ws).eof()) \
          return true; \
        if (std::numeric_limits<Type>::has_infinity) { \
          if (conversion::isInfinity(lhs)) { \
            rhs = std::numeric_limits<Type>::infinity(); \
            return true; \
          } \
          else if (conversion::isNegativeInfinity(lhs)) { \
            rhs = negative_op std::numeric_limits<Type>::infinity(); \
            return true; \
          } \
        } \
        if (std::numeric_limits<Type>::has_quiet_NaN && conversion::isNan(lhs)) { \
          rhs = std::numeric_limits<Type>::quiet_NaN(); \
          return true; \
        } \
        return false; \
      } \
    }

    #define DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(type) DQMNET_DEFINE_CONVERT_STREAMING(type, -)
    #define DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(type) DQMNET_DEFINE_CONVERT_STREAMING(type, +)

    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(int);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(short);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(long);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(long long);
    DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(unsigned);
    DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(unsigned short);
    DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(unsigned long);
    DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(unsigned long long);

    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(char);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(signed char);
    DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED(unsigned char);

    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(float);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(double);
    DQMNET_DEFINE_CONVERT_STREAMING_SIGNED(long double);

    #undef DQMNET_DEFINE_CONVERT_STREAMING_SIGNED
    #undef DQMNET_DEFINE_CONVERT_STREAMING_UNSIGNED
    #undef DQMNET_DEFINE_CONVERT_STREAMING

    // bool spec
    template <>
    struct convert<bool>
    {
      static bool encode(std::string &lhs, bool rhs)
      {
        lhs = rhs ? "true" : "false";
        return true;
      }

      static bool decode(const std::string &lhs, bool &rhs)
      {
        static const struct {
            std::string truename, falsename;
        } names[] = {
              {"y", "n"}, {"yes", "no"}, {"true", "false"}, {"on", "off"}
        };

        std::string value(lhs);
        tolower(value);

        for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
          if (names[i].truename == value) {
            rhs = true;
            return true;
          }

          if (names[i].falsename == value) {
            rhs = false;
            return true;
          }
        }
        return false;
      }
    };



    /**
    *  @brief SignalBase class.
    *         Base class to store callback function
    */
    template <typename ...Args>
    class SignalBase
    {
    public:
      typedef std::vector<SignalBase<Args...> *>  Vector;

      /**
      *  @brief  Destructor
      */
      virtual ~SignalBase() {}

      /**
      *  @brief  Process the callback
      */
      virtual void process(Args ...args) = 0;
    };


    template <>
    class SignalBase<void>
    {
    public:
      typedef std::vector<SignalBase<void> *>  Vector;

      /**
      *  @brief  Destructor
      */
      virtual ~SignalBase() {}

      /**
      *  @brief  Process the callback
      */
      virtual void process() = 0;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
    *  @brief  SignalT class.
    */
    template <typename T, typename ...Args>
    class SignalT : public SignalBase<Args ...>
    {
    public:
      typedef void (T::*Function)(Args...args);

      /**
      *  @brief  Constructor with
      */
      SignalT(T *pClass, Function function);

      /**
      *  @brief  Process the callback
      */
      void process(Args ...args);

      /**
       *
       */
      const T *getClass() const;

      /**
       *
       */
      const Function getFunction() const;

    private:
      T                 *m_pClass;
      Function           m_function;
    };

    template <typename T>
    class SignalT<T, void> : public SignalBase<void>
    {
    public:
      typedef void (T::*Function)();

      /**
      *  @brief  Constructor with
      */
      SignalT(T *pClass, Function function);

      /**
      *  @brief  Process the callback
      */
      void process();

      /**
       *
       */
      const T *getClass() const;

      /**
       *
       */
      const Function getFunction() const;

    private:
      T                 *m_pClass;
      Function           m_function;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename ...Args>
    class Signal
    {
    public:
      /**
      *
      */
      ~Signal();

      /**
      *
      */
      void process(Args ...args);

      /**
      *
      */
      template <typename T, typename S>
      bool connect(T *pClass, S function);

      /**
      *
      */
      template <typename T>
      bool disconnect(T *pClass);

      /**
       *
       */
      template <typename T, typename S>
      bool disconnect(T *pClass, S function);

      /**
      *
      */
      void disconnectAll();

      /**
      *
      */
      template <typename T>
      bool isConnected(T *pClass) const;

      /**
       *
       */
      template <typename T, typename S>
      bool isConnected(T *pClass, S function) const;

      /**
      *
      */
      bool hasConnection() const;

    private:
      typename SignalBase<Args...>::Vector           m_callbacks;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <>
    class Signal<void>
    {
    public:
      /**
      *
      */
      ~Signal();

      /**
      *
      */
      void process();

      /**
      *
      */
      template <typename T, typename S>
      bool connect(T *pClass, S function);

      /**
      *
      */
      template <typename T>
      bool disconnect(T *pClass);

      /**
       *
       */
      template <typename T, typename S>
      bool disconnect(T *pClass, S function);

      /**
      *
      */
      void disconnectAll();

      /**
      *
      */
      template <typename T>
      bool isConnected(T *pClass) const;

      /**
       *
       */
      template <typename T, typename S>
      bool isConnected(T *pClass, S function) const;

      /**
      *
      */
      bool hasConnection() const;

    private:
      typename SignalBase<void>::Vector           m_callbacks;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline Signal<Args ...>::~Signal()
    {
      this->disconnectAll();
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline void Signal<Args ...>::process(Args ...args)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        (*iter)->process(args...);
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args ...>::connect(T *pClass, S function)
    {
      // check for existing connection
      if(this->isConnected(pClass))
        return false;

      // add the callback
      m_callbacks.push_back(new SignalT<T, Args...>(pClass, function));

      return true;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T>
    inline bool Signal<Args ...>::disconnect(T *pClass)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, Args...> *pCallBackT(dynamic_cast<SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args ...>::disconnect(T *pClass, S function)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, Args...> *pCallBackT(dynamic_cast<SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline void Signal<Args ...>::disconnectAll()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        delete *iter;

      m_callbacks.clear();
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T>
    inline bool Signal<Args ...>::isConnected(T *pClass) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, Args...> *pCallBackT(dynamic_cast<const SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
          return true;
      }

      return false;
    }

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args...>::isConnected(T *pClass, S function) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, Args...> *pCallBackT(dynamic_cast<const SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline bool Signal<Args ...>::hasConnection() const
    {
      return (!m_callbacks.empty());
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    inline Signal<void>::~Signal()
    {
      this->disconnectAll();
    }

    //----------------------------------------------------------------------------------

    inline void Signal<void>::process()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        (*iter)->process();
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::connect(T *pClass, S function)
    {
      // check for existing connection
      if(this->isConnected(pClass))
        return false;

      // add the callback
      m_callbacks.push_back(new SignalT<T, void>(pClass, function));

      return true;
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline bool Signal<void>::disconnect(T *pClass)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, void> *pCallBackT(dynamic_cast<SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::disconnect(T *pClass, S function)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, void> *pCallBackT(dynamic_cast<SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    inline void Signal<void>::disconnectAll()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        delete *iter;

      m_callbacks.clear();
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline bool Signal<void>::isConnected(T *pClass) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, void> *pCallBackT(dynamic_cast<const SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::isConnected(T *pClass, S function) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, void> *pCallBackT(dynamic_cast<const SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    inline bool Signal<void>::hasConnection() const
    {
      return (!m_callbacks.empty());
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline SignalT<T, Args...>::SignalT(T *pClass, Function function) :
      m_pClass(pClass),
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline void SignalT<T, Args...>::process(Args... args)
    {
      (m_pClass->*m_function)(args...);
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline const T *SignalT<T, Args...>::getClass() const
    {
      return m_pClass;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline const typename SignalT<T, Args...>::Function SignalT<T, Args...>::getFunction() const
    {
      return m_function;
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename T>
    inline SignalT<T, void>::SignalT(T *pClass, Function function) :
      m_pClass(pClass),
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline void SignalT<T, void>::process()
    {
      (m_pClass->*m_function)();
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline const T *SignalT<T, void>::getClass() const
    {
      return m_pClass;
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline const typename SignalT<T, void>::Function SignalT<T, void>::getFunction() const
    {
      return m_function;
    }

  }

}

#endif  //  DQMNET_H
