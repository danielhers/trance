// -*- mode: c++ -*-
//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

#ifndef __TRANCE__GRAMMAR__HPP__
#define __TRANCE__GRAMMAR__HPP__ 1

#include <vector>

#include <trance/symbol.hpp>
#include <trance/rule.hpp>
#include <trance/signature.hpp>

#include <utils/bithack.hpp>
#include <utils/unordered_map.hpp>
#include <utils/alloc_vector.hpp>

#include <boost/filesystem/path.hpp>

namespace trance
{
  class Grammar
  {
  public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    
    typedef Symbol symbol_type;
    typedef Symbol word_type;
    
    typedef Rule      rule_type;
    typedef Signature signature_type;
    
    typedef boost::filesystem::path path_type;
    
  public:
    typedef std::vector<symbol_type, std::allocator<symbol_type> > label_set_type;
    typedef std::vector<rule_type, std::allocator<rule_type> > rule_set_type;
    
    typedef std::pair<symbol_type, symbol_type> symbol_pair_type;
    typedef utils::unordered_map<symbol_pair_type, rule_set_type,
				 boost::hash<symbol_pair_type>, std::equal_to<symbol_pair_type>,
				 std::allocator<std::pair<const symbol_pair_type, rule_set_type> > >::type rule_set_binary_type;
    
    typedef utils::unordered_map<symbol_type, rule_set_type,
				 boost::hash<symbol_type>, std::equal_to<symbol_type>,
				 std::allocator<std::pair<const symbol_type, rule_set_type> > >::type rule_set_unary_type;
    typedef utils::unordered_map<word_type, rule_set_type,
				 boost::hash<word_type>, std::equal_to<word_type>,
				 std::allocator<std::pair<const word_type, rule_set_type> > >::type rule_set_preterminal_type;

  public:
    Grammar() {}
    Grammar(const path_type& path) { read(path); }
    
  public:
    size_type binary_size() const;
    size_type unary_size() const;
    size_type preterminal_size() const;
    
    void open(const path_type& path) { read(path); }
    void read(const path_type& path);
    void write(const path_type& path) const;

    void clear()
    {
      goal_               = symbol_type();
      sentence_           = symbol_type();
      sentence_binarized_ = symbol_type();
      
      binary_.clear();
      unary_.clear();
      preterminal_.clear();
      
      terminal_.clear();
      non_terminal_.clear();
      pos_.clear();
    }

    const rule_set_type& binary(const symbol_type& left, const symbol_type& right) const
    {
      rule_set_binary_type::const_iterator biter = binary_.find(std::make_pair(left, right));
      if (biter == binary_.end()) {
	static const rule_set_type empty_;
	return empty_;
      } else
	return biter->second;
    }
    
    const rule_set_type& unary(const symbol_type& symbol) const
    {
      rule_set_unary_type::const_iterator uiter = unary_.find(symbol);
      if (uiter == unary_.end()) {
	static const rule_set_type empty_;
	return empty_;
      } else
	return uiter->second;
    }

    const rule_set_type& preterminal(const signature_type& signature, const word_type& terminal) const
    {
      rule_set_preterminal_type::const_iterator piter = preterminal_.find(terminal);
      if (piter == preterminal_.end()) {
	piter = preterminal_.find(signature(terminal));
	
	if (piter == preterminal_.end())
	  piter = preterminal_.find(symbol_type::UNK);
      }
      
      if (piter == preterminal_.end()) {
	static const rule_set_type empty_;
	return empty_;
      } else
	return piter->second;
    }
    
  public:
    // goal
    symbol_type goal_;
    symbol_type sentence_;
    symbol_type sentence_binarized_;
    
    // rule set
    rule_set_binary_type      binary_;
    rule_set_unary_type       unary_;
    rule_set_preterminal_type preterminal_;

    // a set of labels...
    label_set_type terminal_;
    label_set_type non_terminal_;
    label_set_type pos_;
  };
};

#endif
