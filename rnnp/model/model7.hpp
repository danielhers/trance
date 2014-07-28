// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __RNNP__MODEL__MODEL7__HPP__
#define __RNNP__MODEL__MODEL7__HPP__ 1

#include <rnnp/model.hpp>

#include <boost/random/uniform_real_distribution.hpp>

namespace rnnp
{
  namespace model
  {
    class Model7 : public Model
    {
    public:
      
    public:
      Model7() : Model() {}
      Model7(const path_type& path) { read(path); }
      Model7(const size_type& hidden,
	     const size_type& embedding,
	     const grammar_type& grammar)
      { initialize(hidden, embedding, grammar); }
      
      void initialize(const size_type& hidden,
		      const size_type& embedding,
		      const grammar_type& grammar);
      
      // IO
      void write(const path_type& path) const;
      void read(const path_type& path);
      void embedding(const path_type& path);
    
      friend
      std::ostream& operator<<(std::ostream& os, const Model7& x);
      friend
      std::istream& operator>>(std::istream& is, Model7& x);
    
      Model7& operator+=(const Model7& x);
      Model7& operator-=(const Model7& x);
      Model7& operator*=(const double& x);
      Model7& operator/=(const double& x);
    
    private:
      template <typename Gen>
      struct __randomize
      {
	__randomize(Gen& gen, const double range=0.01) : gen_(gen), range_(range) {}
      
	template <typename Tp>
	Tp operator()(const Tp& x) const
	{
	  return boost::random::uniform_real_distribution<Tp>(-range_, range_)(const_cast<Gen&>(gen_));
	}
      
	Gen& gen_;
	double range_;
      };

    public:
      template <typename Gen>
      void random(Gen& gen)
      {
	const double range_embed = std::sqrt(6.0 / (embedding_ + 1));
	const double range_c  = std::sqrt(6.0 / (hidden_ + 1));
	const double range_h  = std::sqrt(6.0 / (hidden_ + hidden_));
	const double range_sh = std::sqrt(6.0 / (hidden_ + hidden_ + embedding_ + hidden_));
	const double range_re = std::sqrt(6.0 / (hidden_ + hidden_ + hidden_ + hidden_ + hidden_));
	const double range_u  = std::sqrt(6.0 / (hidden_ + hidden_ + hidden_ + hidden_));
	const double range_qu = std::sqrt(6.0 / (hidden_ + hidden_ + embedding_));
	const double range_f  = std::sqrt(6.0 / (hidden_ + hidden_));
	const double range_i  = std::sqrt(6.0 / (hidden_ + hidden_));
	
	const size_type num_category = vocab_category_.size() - std::count(vocab_category_.begin(), vocab_category_.end(), category_type());
	
	terminal_ = terminal_.array().unaryExpr(__randomize<Gen>(gen, range_embed));
	
	Wc_.block(0, 0, Wc_.rows(), hidden_)
	  = Wc_.block(0, 0, Wc_.rows(), hidden_).array().unaryExpr(__randomize<Gen>(gen, range_c));
	
	Wc_.block(0, hidden_, Wc_.rows(), 1).setConstant(- std::log(double(num_category)));
	
	Wh_ = Wh_.array().unaryExpr(__randomize<Gen>(gen, range_h));
	
	Wsh_ = Wsh_.array().unaryExpr(__randomize<Gen>(gen, range_sh));
	Wre_ = Wre_.array().unaryExpr(__randomize<Gen>(gen, range_re));
	
	Wu_  = Wu_.array().unaryExpr(__randomize<Gen>(gen, range_u));
	
	Wqu_ = Wqu_.array().unaryExpr(__randomize<Gen>(gen, range_qu));
	
	Wf_ = Wf_.array().unaryExpr(__randomize<Gen>(gen, range_f));
	Wi_ = Wi_.array().unaryExpr(__randomize<Gen>(gen, range_i));
      }
      
      void swap(Model7& x)
      {
	Model::swap(static_cast<Model&>(x));
	
	terminal_.swap(x.terminal_);
      
	Wc_.swap(x.Wc_);
	Wfe_.swap(x.Wfe_);
	
	Wh_.swap(x.Wh_);
	Bh_.swap(x.Bh_);
      
	Wsh_.swap(x.Wsh_);
	Bsh_.swap(x.Bsh_);
      
	Wre_.swap(x.Wre_);
	Bre_.swap(x.Bre_);

	Wu_.swap(x.Wu_);
	Bu_.swap(x.Bu_);

	Wqu_.swap(x.Wqu_);
	Bqu_.swap(x.Bqu_);
	Bqe_.swap(x.Bqe_);

	Wf_.swap(x.Wf_);
	Bf_.swap(x.Bf_);
      
	Wi_.swap(x.Wi_);
	Bi_.swap(x.Bi_);
      
	Ba_.swap(x.Ba_);
      }
    
      void clear()
      {
	Model::clear();

	terminal_.setZero();
      
	Wc_.setZero();
	Wfe_.clear();
      
	Wh_.setZero();
	Bh_.setZero();

	Wsh_.setZero();
	Bsh_.setZero();

	Wre_.setZero();
	Bre_.setZero();

	Wu_.setZero();
	Bu_.setZero();
      
	Wf_.setZero();
	Bf_.setZero();
      
	Wi_.setZero();
	Bi_.setZero();

	Wqu_.setZero();
	Bqu_.setZero();
	Bqe_.setZero();
	
	Ba_.setZero();
      }
    
    public:
      double l1() const
      {
	double norm = 0.0;
      
	norm += Wc_.lpNorm<1>();
	
	norm += Wh_.lpNorm<1>();
	
	norm += Wsh_.lpNorm<1>();
	norm += Wre_.lpNorm<1>();
      
	norm += Wu_.lpNorm<1>();
	norm += Wf_.lpNorm<1>();
	norm += Wi_.lpNorm<1>();
	
	norm += Wqu_.lpNorm<1>();
	
	return norm;
      }
    
      double l2() const
      {
	double norm = 0.0;
      
	norm += Wc_.squaredNorm();
	
	norm += Wh_.squaredNorm();
      
	norm += Wsh_.squaredNorm();
	norm += Wre_.squaredNorm();
      
	norm += Wu_.squaredNorm();
	norm += Wf_.squaredNorm();
	norm += Wi_.squaredNorm();
	
	norm += Wqu_.squaredNorm();
	
	return std::sqrt(norm);
      }
      
    public:
      // terminal embedding
      tensor_type terminal_;
    
      // classification
      tensor_type Wc_;
      
      // features
      weights_type Wfe_;
      
      // pre-classification
      tensor_type Wh_;
      tensor_type Bh_;
    
      // shift
      tensor_type Wsh_;
      tensor_type Bsh_;
    
      // reduce
      tensor_type Wre_;
      tensor_type Bre_;
      
      // unary
      tensor_type Wu_;
      tensor_type Bu_;

      // final
      tensor_type Wf_;
      tensor_type Bf_;

      // idle
      tensor_type Wi_;
      tensor_type Bi_;
      
      // queue
      tensor_type Wqu_;
      tensor_type Bqu_;
      tensor_type Bqe_;
      
      // axiom
      tensor_type Ba_;
    };
  };
};

namespace std
{
  inline
  void swap(rnnp::model::Model7& x, rnnp::model::Model7& y)
  {
    x.swap(y);
  }
};

#endif
