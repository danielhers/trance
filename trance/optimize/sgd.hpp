// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __TRANCE__OPTIMIZE_SGD__HPP__
#define __TRANCE__OPTIMIZE_SGD__HPP__ 1

#include <trance/optimize.hpp>
#include <trance/model_traits.hpp>

namespace trance
{
  namespace optimize
  {
    template <typename Theta>
    struct SGD : public Optimize
    {
      typedef typename model_traits<Theta>::model_type    model_impl_type;
      typedef typename model_traits<Theta>::gradient_type gradient_impl_type;
      
      SGD(const Theta& theta,
	  const option_type& option)
	: lambda_(option.lambda_), eta0_(option.eta0_) { }

      double decay()
      {
	eta0_ *= 0.5;
	return eta0_;
      }
      
      void operator()(model_impl_type& theta,
		      const gradient_impl_type& gradient,
		      const option_type& option) const;
      
      void update(tensor_type& theta,
		  const matrix_embedding_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	matrix_embedding_type::const_iterator eiter_end = grad.end();
	for (matrix_embedding_type::const_iterator eiter = grad.begin(); eiter != eiter_end; ++ eiter)
	  theta.col(eiter->first.id()) -= (eta0_ * scale) * eiter->second;
      }
      
      void update(tensor_type& theta,
		  const matrix_category_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	typedef gradient_type::matrix_category_type matrix_category_type;
	
	if (regularize && lambda_ != 0.0)
	  theta *= 1.0 - eta0_ * lambda_;
	
	matrix_category_type::const_iterator giter_end = grad.end();
	for (matrix_category_type::const_iterator giter = grad.begin(); giter != giter_end; ++ giter)
	  theta.block(giter->second.rows() * giter->first.non_terminal_id(), 0, giter->second.rows(), giter->second.cols())
	    -= (eta0_ * scale) * giter->second;
      }
      
      void update(model_type::weights_type& theta,
		  const gradient_type::weights_type& grad,
		  const double scale,
		  const bool regularize) const
      {
	gradient_type::weights_type::const_iterator fiter_end = grad.end();
	for (gradient_type::weights_type::const_iterator fiter = grad.begin(); fiter != fiter_end; ++ fiter) 
	  if (fiter->second != 0)
	    theta[fiter->first] -= eta0_ * scale * fiter->second;
      }

      void update(tensor_type& theta,
		  const tensor_type& g,
		  const double scale,
		  const bool regularize) const
      {
	if (regularize && lambda_ != 0.0)
	  theta *= 1.0 - eta0_ * lambda_;
      
	theta -= eta0_ * scale * g;
      }
    
    private:
      double lambda_;
      double eta0_;
    };
    
    template <>
    inline
    void SGD<model::Model1>::operator()(model::Model1& theta,
					const gradient::Model1& gradient,
					const option_type& option) const
    {
      if (! gradient.count_) return;
	
      const double scale = 1.0 / gradient.count_;
	
      if (option.learn_embedding())
	update(theta.terminal_, gradient.terminal_, scale, false);
	
      if (option.learn_classification()) {
	update(theta.Wc_,  gradient.Wc_,  scale, true);
	update(theta.Bc_,  gradient.Bc_,  scale, false);
	update(theta.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, gradient.Bsh_, scale, false);
	  
	update(theta.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, gradient.Bu_, scale, false);
	  
	update(theta.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, gradient.Bf_, scale, false);
	  
	update(theta.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, gradient.Bi_, scale, false);
	  
	update(theta.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void SGD<model::Model2>::operator()(model::Model2& theta,
					const gradient::Model2& gradient,
					const option_type& option) const
    {
      if (! gradient.count_) return;
	
      const double scale = 1.0 / gradient.count_;
	
      if (option.learn_embedding())
	update(theta.terminal_, gradient.terminal_, scale, false);
	
      if (option.learn_classification()) {
	update(theta.Wc_,  gradient.Wc_,  scale, true);
	update(theta.Bc_,  gradient.Bc_,  scale, false);
	update(theta.Wfe_, gradient.Wfe_, scale, true);
      }
      
      if (option.learn_hidden()) {
	update(theta.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, gradient.Bsh_, scale, false);
	  
	update(theta.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, gradient.Bu_, scale, false);
	  
	update(theta.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, gradient.Bf_, scale, false);
	  
	update(theta.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, gradient.Bi_, scale, false);
	  
	update(theta.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void SGD<model::Model3>::operator()(model::Model3& theta,
					const gradient::Model3& gradient,
					const option_type& option) const
    {
      if (! gradient.count_) return;
	
      const double scale = 1.0 / gradient.count_;
	
      if (option.learn_embedding())
	update(theta.terminal_, gradient.terminal_, scale, false);
	
      if (option.learn_classification()) {
	update(theta.Wc_,  gradient.Wc_,  scale, true);
	update(theta.Bc_,  gradient.Bc_,  scale, false);
	update(theta.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, gradient.Bsh_, scale, false);
	  
	update(theta.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, gradient.Bu_, scale, false);
	  
	update(theta.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, gradient.Bf_, scale, false);
	
	update(theta.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void SGD<model::Model4>::operator()(model::Model4& theta,
					const gradient::Model4& gradient,
					const option_type& option) const
    {
      if (! gradient.count_) return;
	
      const double scale = 1.0 / gradient.count_;
	
      if (option.learn_embedding())
	update(theta.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  gradient.Wc_,  scale, true);
	update(theta.Bc_,  gradient.Bc_,  scale, false);
	update(theta.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, gradient.Bsh_, scale, false);
	  
	update(theta.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, gradient.Bu_, scale, false);
	  
	update(theta.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, gradient.Bf_, scale, false);
	  
	update(theta.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, gradient.Bi_, scale, false);
	  
	update(theta.Ba_, gradient.Ba_, scale, false);
      }
    }

    template <>
    inline
    void SGD<model::Model5>::operator()(model::Model5& theta,
					const gradient::Model5& gradient,
					const option_type& option) const
    {
      if (! gradient.count_) return;
	
      const double scale = 1.0 / gradient.count_;
	
      if (option.learn_embedding())
	update(theta.terminal_, gradient.terminal_, scale, false);
      
      if (option.learn_classification()) {
	update(theta.Wc_,  gradient.Wc_,  scale, true);
	update(theta.Bc_,  gradient.Bc_,  scale, false);
	update(theta.Wfe_, gradient.Wfe_, scale, true);
      }
	
      if (option.learn_hidden()) {
	update(theta.Wsh_, gradient.Wsh_, scale, true);
	update(theta.Bsh_, gradient.Bsh_, scale, false);
	  
	update(theta.Wre_, gradient.Wre_, scale, true);
	update(theta.Bre_, gradient.Bre_, scale, false);
	
	update(theta.Wu_, gradient.Wu_, scale, true);
	update(theta.Bu_, gradient.Bu_, scale, false);
	  
	update(theta.Wf_, gradient.Wf_, scale, true);
	update(theta.Bf_, gradient.Bf_, scale, false);
	
	update(theta.Wi_, gradient.Wi_, scale, true);
	update(theta.Bi_, gradient.Bi_, scale, false);
	
	update(theta.Wqu_, gradient.Wqu_, scale, true);
	update(theta.Bqu_, gradient.Bqu_, scale, false);
	update(theta.Bqe_, gradient.Bqe_, scale, false);
	
	update(theta.Ba_, gradient.Ba_, scale, false);
      }
    }
  };
};

#endif
