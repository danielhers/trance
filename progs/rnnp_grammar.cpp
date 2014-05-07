//
//  Copyright(C) 2014 Taro Watanabe <taro.watanabe@nict.go.jp>
//

//
// collect grammar
//

#include <iostream>

#include <rnnp/symbol.hpp>
#include <rnnp/tree.hpp>
#include <rnnp/rule.hpp>
#include <rnnp/grammar.hpp>
#include <rnnp/binarize.hpp>
#include <rnnp/signature.hpp>

#include "utils/compress_stream.hpp"
#include "utils/compact_map.hpp"
#include "utils/unordered_set.hpp"
#include "utils/bithack.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

typedef boost::filesystem::path path_type;

typedef rnnp::Symbol  symbol_type;
typedef rnnp::Tree    tree_type;
typedef rnnp::Rule    rule_type;

typedef uint64_t count_type;
typedef utils::compact_map<symbol_type, count_type,
			   utils::unassigned<symbol_type>, utils::unassigned<symbol_type>,
			   boost::hash<symbol_type>, std::equal_to<symbol_type>,
			   std::allocator<std::pair<const symbol_type, count_type> > > unigram_type;


struct Grammar
{
  typedef utils::unordered_set<rule_type,
			       boost::hash<rule_type>, std::equal_to<rule_type>,
			       std::allocator<rule_type> >::type rule_set_type;
  
  
  symbol_type  goal_;
  unigram_type sentence_;
  
  rule_set_type binary_;
  rule_set_type unary_;
  rule_set_type preterminal_;
};

typedef Grammar grammar_type;

typedef rnnp::Signature signature_type;

void collect_rules(const path_type& path,
		   grammar_type& grammar,
		   unigram_type& unigram,
		   const bool left=true);
void cutoff_terminal(const signature_type& signature,
		     grammar_type& grammar,
		     unigram_type& unigram);
void output_grammar(const path_type& path,
		    const grammar_type& grammar);

path_type input_file = "-";
path_type output_file = "-";

std::string signature_name = "none";

bool binarize_left = false;
bool binarize_right = false;

int cutoff = 3;

int debug = 0;

void options(int argc, char** argv);

int main(int argc, char** argv)
{
  try {
    options(argc, argv);
    
    if (int(binarize_left) + binarize_right > 1)
      throw std::runtime_error("either one of --binarize-{left,right}");
    
    if (int(binarize_left) + binarize_right == 0)
      binarize_left = true;
    
    grammar_type grammar;
    unigram_type unigram;

    collect_rules(input_file, grammar, unigram, binarize_left);

    if (cutoff > 0)
      cutoff_terminal(*signature_type::create(signature_name), grammar, unigram);
    
    output_grammar(output_file, grammar);
  }
  catch (const std::exception& err) {
    std::cerr << "error: " << err.what() << std::endl;
    return 1;
  }
  return 0;
}

struct CollectRules
{
  CollectRules(grammar_type& grammar,
	       unigram_type& unigram,
	       bool left)
    : grammar_(grammar), unigram_(unigram), left_(left), unary_max_(0) {}

  void operator()(const tree_type& tree)
  {
    if (left_)
      rnnp::binarize_left(tree, binarized_);
    else
      rnnp::binarize_right(tree, binarized_);
    
    if (grammar_.goal_ == symbol_type())
      grammar_.goal_ = binarized_.label_;
    else if (grammar_.goal_ != binarized_.label_)
      throw std::runtime_error("different goal: previous = " + static_cast<const std::string&>(grammar_.goal_)
			       + " current = " + static_cast<const std::string&>(binarized_.label_));
    
    switch (binarized_.antecedent_.size()) {
    case 1:
      ++ grammar_.sentence_[binarized_.antecedent_.front().label_];
      break;
    case 2:
      ++ grammar_.sentence_[binarized_.antecedent_.front().label_];
      ++ grammar_.sentence_[binarized_.antecedent_.back().label_];
      break;
    default: 
      throw std::runtime_error("invalid binary tree");
    }
    
    int unary = 0;
    extract(binarized_, unary);
  }
  
  void extract(const tree_type& tree, int& unary)
  {
    switch (tree.antecedent_.size()) {
    case 1: {
      rule_type rule(tree.label_, 1);
      rule.rhs_.front() = tree.antecedent_.front().label_;
      
      if (rule.unary()) {
	++ unary;
	grammar_.unary_.insert(rule);
	
	extract(tree.antecedent_.front(), unary);
      } else if (rule.preterminal()) {
	grammar_.preterminal_.insert(rule);
	
	++ unigram_[rule.rhs_.front()];
      } else
	std::runtime_error("invalid rule: " + rule.string());
    } break;
    case 2: {
      unary_max_ = utils::bithack::max(unary_max_, unary);
      
      rule_type rule(tree.label_, 2);
      rule.rhs_.front() = tree.antecedent_.front().label_;
      rule.rhs_.back()  = tree.antecedent_.back().label_;

      if (! rule.binary())
	throw std::runtime_error("invalid rule: " + rule.string());
      
      grammar_.binary_.insert(rule);

      int unary_left = 0;
      int unary_right = 0;
      
      extract(tree.antecedent_.front(), unary_left);
      extract(tree.antecedent_.back(),  unary_right);
    } break;
    default:
      throw std::runtime_error("invalid binary tree");
    }
  }

  tree_type binarized_;
  
  grammar_type& grammar_;
  unigram_type& unigram_;
  bool left_;

  int unary_max_;
};

void collect_rules(const path_type& path,
		   grammar_type& grammar,
		   unigram_type& unigram,
		   const bool left)
{
  CollectRules collect(grammar, unigram, left);

  tree_type tree;

  utils::compress_istream is(path, 1024 * 1024);
  
  while (is >> tree)
    if (! tree.empty())
      collect(tree);

  if (debug)
    std::cerr << "maximum unary size: " << collect.unary_max_ << std::endl;
}

void cutoff_terminal(const signature_type& signature,
		     grammar_type& grammar,
		     unigram_type& unigram)
{
  unigram_type unigram_cutoff;

  // add workaround for penntreebank
  unigram_type::const_iterator uiter_end = unigram.end();
  for (unigram_type::const_iterator uiter = unigram.begin(); uiter != uiter_end; ++ uiter) {
    if (uiter->second >= cutoff || static_cast<const std::string&>(uiter->first).find("``", 0, 2) != std::string::npos)
      unigram_cutoff.insert(*uiter);
    else
      unigram_cutoff[signature(uiter->first)] += uiter->second;
  }
  
  unigram.swap(unigram_cutoff);
  
  grammar_type::rule_set_type preterminal;
  grammar_type::rule_set_type preterminal_oov;
  bool has_fallback = false;
  
  grammar_type::rule_set_type::const_iterator piter_end = grammar.preterminal_.end();
  for (grammar_type::rule_set_type::const_iterator piter = grammar.preterminal_.begin(); piter != piter_end; ++ piter)
    if (unigram.find(piter->rhs_.front()) != unigram.end())
      preterminal.insert(*piter);
    else {
      if (debug >= 2)
	std::cerr << "removing preterminal: " << *piter << std::endl;
      
      const symbol_type sig = signature(piter->rhs_.front());
      
      preterminal.insert(rule_type(piter->lhs_, rule_type::rhs_type(1, sig)));
      preterminal_oov.insert(rule_type(piter->lhs_, rule_type::rhs_type(1, symbol_type::UNK)));
      
      has_fallback |= (sig == symbol_type::UNK);
    }

  if (! has_fallback)
    preterminal.insert(preterminal_oov.begin(), preterminal_oov.end());
  
  grammar.preterminal_.swap(preterminal);
}

void output_grammar(const path_type& path,
		    const grammar_type& grammar)
{
  utils::compress_ostream os(path, 1024 * 1024);

  symbol_type sentence;
  count_type  count = 0;

  if (grammar.sentence_.empty())
    throw std::runtime_error("invalid pre-goal label");
  
  unigram_type::const_iterator siter_end = grammar.sentence_.end();
  for (unigram_type::const_iterator siter = grammar.sentence_.begin(); siter != siter_end; ++ siter)
    if (siter->second > count) {
      sentence = siter->first;
      count = siter->second;
    }
  
  os << grammar.goal_ << '\n';
  os << sentence << '\n';
  os << '\n';

  grammar_type::rule_set_type::const_iterator uiter_end = grammar.unary_.end();
  for (grammar_type::rule_set_type::const_iterator uiter = grammar.unary_.begin(); uiter != uiter_end; ++ uiter)
    os << *uiter << '\n';
  os << '\n';
  
  grammar_type::rule_set_type::const_iterator biter_end = grammar.binary_.end();
  for (grammar_type::rule_set_type::const_iterator biter = grammar.binary_.begin(); biter != biter_end; ++ biter)
    os << *biter << '\n';
  os << '\n';

  grammar_type::rule_set_type::const_iterator piter_end = grammar.preterminal_.end();
  for (grammar_type::rule_set_type::const_iterator piter = grammar.preterminal_.begin(); piter != piter_end; ++ piter)
    os << *piter << '\n';
}


void options(int argc, char** argv)
{
  namespace po = boost::program_options;

  po::options_description desc("options");
  desc.add_options()
    ("input",     po::value<path_type>(&input_file)->default_value(input_file),   "input file")
    ("output",    po::value<path_type>(&output_file)->default_value(output_file), "output")

    ("signature", po::value<std::string>(&signature_name)->default_value(signature_name), "language specific signature")
    
    ("binarize-left",  po::bool_switch(&binarize_left),  "left recursive (or left heavy) binarization (default)")
    ("binarize-right", po::bool_switch(&binarize_right), "right recursive (or right heavy) binarization")
    
    ("cutoff",    po::value<int>(&cutoff)->default_value(cutoff),           "OOV cutoff")
    
    ("debug", po::value<int>(&debug)->implicit_value(1), "debug level")
    
    ("help", "help message");
  
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc, po::command_line_style::unix_style & (~po::command_line_style::allow_guessing)), vm);
  po::notify(vm);
  
  if (vm.count("help")) {
    std::cout << argv[0] << " [options]" << '\n' << desc << '\n';
    exit(0);
  }
}


