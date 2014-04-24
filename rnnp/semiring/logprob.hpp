// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__SEMIRING__LOGPROB__HPP__
#define __RNNP__SEMIRING__LOGPROB__HPP__ 1

#include <cmath>
#include <cfloat>
#include <climits>

#include <limits>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <rnnp/semiring/traits.hpp>

#include <utils/mathop.hpp>

namespace rnnp
{
  namespace semiring
  {

    template <typename Tp>
    class Logprob
    {
    public:
      typedef Tp weight_type;
      typedef Tp value_type;
      typedef Logprob<Tp> self_type;

    public:
      struct proxy_type
      {
	friend class Logprob;
	
	proxy_type(const weight_type& x) : value_(x) {}
	
	operator Logprob() const { return Logprob(*this); }
	
      private:
	weight_type value_;
      };
      
    public:
      static inline self_type exp(const Tp& x) { return proxy_type(x); }
      static inline self_type pow(const self_type& x, const Tp& y) { return proxy_type(x.value_ * y); }
      static inline self_type zero() { return proxy_type(impl::traits_infinity<value_type>::minus()); }
      static inline self_type one()  { return proxy_type(0); }
      static inline self_type max()  { return proxy_type(impl::traits_infinity<value_type>::plus()); }
      static inline self_type min()  { return proxy_type(impl::traits_infinity<value_type>::minus()); }
      
    public:
      // any better wayt to make an assignment...?
      Logprob() : value_(impl::traits_infinity<value_type>::minus()) {}
      Logprob(const weight_type& x) : value_(std::log(x)) {}
      explicit Logprob(const proxy_type& x) : value_(x.value_) {}


      operator Tp() const { return std::exp(value_); }
      
    public:
      template <typename T>
      friend
      T log(const Logprob<T>& x);

      Logprob& operator+=(const Logprob& x)
      {
	if (*this == zero()) {
	  value_ = x.value_;
	  return *this;
	} else if (x == zero())
	  return *this;
	
	if (value_ <= x.value_) 
	  value_ = x.value_ + utils::mathop::log1p(std::exp(value_ - x.value_));
	else
	  value_ = value_ + utils::mathop::log1p(std::exp(x.value_ - value_));
	
	return *this;
      }

      Logprob& operator-=(const Logprob& x)
      {
	if (x > *this)
	  throw std::runtime_error("invalid minus");
	
	if (*this == x)
	  *this = zero();
	else {
	  const Tp exp_value = std::exp(x.value_ - value_);
	  if (exp_value == 1.0)
	    *this = zero();
	  else
	    value_ = value_ + utils::mathop::log1p(- exp_value);
	}
	
	return *this;
      }
      
      Logprob& operator*=(const Logprob& x)
      {
	value_ += x.value_;
	return *this;
      }

      Logprob& operator/=(const Logprob& x)
      {
	value_ -= x.value_;
	return *this;
      }
      
      friend
      bool operator==(const self_type& x, const self_type& y) { return x.value_ == y.value_; }
      friend
      bool operator!=(const self_type& x, const self_type& y) { return x.value_ != y.value_; }
      friend
      bool operator>(const self_type& x, const self_type& y) { return x.value_ > y.value_; }
      friend
      bool operator<(const self_type& x, const self_type& y) { return x.value_ < y.value_; }
      friend
      bool operator>=(const self_type& x, const self_type& y) { return x.value_ >= y.value_; }
      friend
      bool operator<=(const self_type& x, const self_type& y) { return x.value_ <= y.value_; }

      friend
      std::ostream& operator<<(std::ostream& os, const self_type& x)
      {
	os << x.value_;
	return os;
      }
      
      friend
      std::istream& operator>>(std::istream& is, self_type& x)
      {
	is >> x.value_;
	return is;
      }
      
    private:
      weight_type value_;
    };

    template <typename Tp>
    inline
    Tp log(const Logprob<Tp>& x)
    {
      return x.value_;
    }
    
    template <typename Tp>
    inline
    Logprob<Tp> operator+(const Logprob<Tp>& x, const Logprob<Tp>& y)
    {
      Logprob<Tp> value(x);
      value += y;
      return value;
    }

    template <typename Tp>
    inline
    Logprob<Tp> operator-(const Logprob<Tp>& x, const Logprob<Tp>& y)
    {
      Logprob<Tp> value(x);
      value -= y;
      return value;
    }

    template <typename Tp>
    inline
    Logprob<Tp> operator*(const Logprob<Tp>& x, const Logprob<Tp>& y)
    {
      Logprob<Tp> value(x);
      value *= y;
      return value;
    }

    template <typename Tp>
    inline
    Logprob<Tp> operator/(const Logprob<Tp>& x, const Logprob<Tp>& y)
    {
      Logprob<Tp> value(x);
      value /= y;
      return value;
    }
    
    template <typename Tp>
    struct traits<Logprob<Tp> >
    {
      static inline Logprob<Tp> exp(const Tp& x) { return Logprob<Tp>::exp(x); }
      static inline Logprob<Tp> pow(const Logprob<Tp>& x, const Tp& y) { return Logprob<Tp>::pow(x, y); }
      static inline Logprob<Tp> zero() { return Logprob<Tp>::zero();  }
      static inline Logprob<Tp> one()  { return Logprob<Tp>::one(); }
      static inline Logprob<Tp> max()  { return Logprob<Tp>::max(); }
      static inline Logprob<Tp> min()  { return Logprob<Tp>::min(); }
    };

  };
};


#endif
