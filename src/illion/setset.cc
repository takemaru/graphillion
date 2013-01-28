#include "illion/setset.h"

#include <cstdlib>

#include <algorithm>
#include <sstream>

#include "illion/zdd.h"

namespace illion {

using std::initializer_list;
using std::istream;
using std::make_pair;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::vector;

setset::iterator::iterator() : zdd_(null()) {
}

setset::iterator::iterator(const setset& ss, vector<double> weights)
    : zdd_(ss.zdd_), weights_(weights) {
  this->next();
}

setset::iterator::iterator(const set<elem_t>& s) : zdd_(bot()), s_(s) {
}

setset::iterator& setset::iterator::operator++() {
  this->next();
  return *this;
}

void setset::iterator::next() {
  if (this->zdd_ == null() || is_bot(this->zdd_)) {
    this->zdd_ = null();
    this->s_ = set<elem_t>();
  } else if (this->weights_.empty()) {  // random sampling
    vector<elem_t> stack;
    static int idum = -1;  // TODO: can be set by users
    this->zdd_ -= choose_random(this->zdd_, &stack, &idum);
    this->s_ = set<elem_t>(stack.begin(), stack.end());
  } else {  // optimization
    set<elem_t> s;
    this->zdd_ -= choose_best(this->zdd_, this->weights_, &s);
    this->s_ = s;
  }
}

setset::setset() : zdd_(bot()) {
}

setset::setset(const set<elem_t>& s) : zdd_(top()) {
  for (const auto& e : s)
    this->zdd_ *= single(e);
}

setset::setset(const vector<set<elem_t> >& v) : zdd_(bot()) {
  for (const auto& s : v)
    this->zdd_ += setset(s).zdd_;
}

setset::setset(const map<string, vector<elem_t> >& m) {
  for (const auto& i : m)
    assert(i.first == "include" || i.first == "exclude");
  const auto in_i = m.find("include");
  const auto ex_i = m.find("exclude");
  const vector<elem_t>& in_v = in_i != m.end() ? in_i->second : vector<elem_t>();
  const vector<elem_t>& ex_v = ex_i != m.end() ? ex_i->second : vector<elem_t>();
  for (const auto& e : in_v) single(e);
  for (const auto& e : ex_v) single(e);
  vector<zdd_t> n(num_elems() + 2);
  n[0] = bot(), n[1] = top();
  for (elem_t v = num_elems(); v > 0; --v) {
    bool in_found = std::find(in_v.begin(), in_v.end(), v) != in_v.end();
    bool ex_found = std::find(ex_v.begin(), ex_v.end(), v) != ex_v.end();
    assert(!(in_found && ex_found));
    elem_t i = num_elems() - v + 2;
    n[i] = in_found ? n[0]   + single(v) * n[i-1]
         : ex_found ? n[i-1] + single(v) * n[0]
         :            n[i-1] + single(v) * n[i-1];
  }
  this->zdd_ = n[num_elems() + 1];
}

setset::setset(const initializer_list<set<elem_t> >& v) : zdd_(bot()) {
  for (auto i = v.begin(); i != v.end(); ++i)
    this->zdd_ += setset(*i).zdd_;
}

setset::setset(istream& in) : zdd_(illion::load(in)) {
}

setset setset::operator~() const {
  return setset(complement(this->zdd_));
}

setset setset::operator&(const setset& ss) const {
  return setset(this->zdd_ & ss.zdd_);
}

setset setset::operator|(const setset& ss) const {
  return setset(this->zdd_ + ss.zdd_);
}

setset setset::operator-(const setset& ss) const {
  return setset(this->zdd_ - ss.zdd_);
}

setset setset::operator^(const setset& ss) const {
  return setset((this->zdd_ - ss.zdd_) + (ss.zdd_ - this->zdd_));
}

setset setset::operator/(const setset& ss) const {
  assert(!is_bot(ss.zdd_) || is_term(this->zdd_))
  return setset(this->zdd_ / ss.zdd_);
}

setset setset::operator%(const setset& ss) const {
  assert(!is_bot(ss.zdd_) || is_term(this->zdd_))
  return setset(this->zdd_ % ss.zdd_);
}

void setset::operator&=(const setset& ss) {
  this->zdd_ &= ss.zdd_;
}

void setset::operator|=(const setset& ss) {
  this->zdd_ += ss.zdd_;
}

void setset::operator-=(const setset& ss) {
  this->zdd_ -= ss.zdd_;
}

void setset::operator^=(const setset& ss) {
  this->zdd_ = (this->zdd_ - ss.zdd_) + (ss.zdd_ - this->zdd_);
}

void setset::operator/=(const setset& ss) {
  this->zdd_ /= ss.zdd_;
}

void setset::operator%=(const setset& ss) {
  this->zdd_ %= ss.zdd_;
}

bool setset::operator<=(const setset& ss) const {
  return (this->zdd_ - ss.zdd_) == bot();
}

bool setset::operator<(const setset& ss) const {
  return (this->zdd_ - ss.zdd_) == bot() && this->zdd_ != ss.zdd_;
}

bool setset::operator>=(const setset& ss) const {
  return (ss.zdd_ - this->zdd_) == bot();
}

bool setset::operator>(const setset& ss) const {
  return (ss.zdd_ - this->zdd_) == bot() && this->zdd_ != ss.zdd_;
}

word_t setset::id() const {
  return illion::id(this->zdd_);
}

bool setset::is_disjoint(const setset& ss) const {
  return (this->zdd_ & ss.zdd_) == bot();
}

bool setset::is_subset(const setset& ss) const {
  return *this <= ss;
}

bool setset::is_superset(const setset& ss) const {
  return *this >= ss;
}

bool setset::empty() const {
  return this->zdd_ == bot();
}

string setset::size() const {
  stringstream ss;
  ss << algo_c(this->zdd_);
  return ss.str();
}

setset::iterator setset::find(const set<elem_t>& s) const {
  if (this->zdd_ - setset(s).zdd_ != this->zdd_)
    return setset::iterator(s);
  else
    return setset::iterator();
}

setset setset::include(elem_t e) const {
  zdd_t z1 = setset({{e}}).zdd_;
  zdd_t z2 = this->zdd_ / z1;
  return setset(z2 * z1);
}

setset setset::exclude(elem_t e) const {
  return setset(this->zdd_ % setset({{e}}).zdd_);
}

size_t setset::count(const set<elem_t>& s) const {
  return this->zdd_ / setset(s).zdd_ != bot() ? 1 : 0;
}

pair<setset::iterator, bool> setset::insert(const set<elem_t>& s) {
  if (this->find(s) != end()) {
    return make_pair(setset::iterator(s), false);
  } else {
    *this |= setset(s);
    return make_pair(setset::iterator(s), true);
  }
}

setset::iterator setset::insert(const_iterator /*hint*/, const set<elem_t>& s) {
  pair<iterator, bool> p = this->insert(s);
  return p.first;
}

void setset::insert(const initializer_list<set<elem_t> >& v) {
  for (auto i = v.begin(); i != v.end(); ++i)
    this->insert(*i);
}

setset::iterator setset::erase(const_iterator position) {
  this->erase(*position);
  return setset::iterator();
}

size_t setset::erase(const set<elem_t>& s) {
  if (this->find(s) != end()) {
    *this -= setset(s);
    return 1;
  } else {
    return 0;
  }
}

size_t setset::erase(elem_t e) {
  setset ss = this->include(e);
  *this -= ss;
  return strtoll(ss.size().c_str(), nullptr, 0);
}

void setset::clear() {
  this->zdd_ = bot();
}

void setset::swap(setset& ss) {
  zdd_t z = this->zdd_;
  this->zdd_ = ss.zdd_;
  ss.zdd_ = z;
}

setset setset::minimal() const {
  return setset(illion::minimal(this->zdd_));
}

setset setset::maximal() const {
  return setset(illion::maximal(this->zdd_));
}

setset setset::hitting() const {  // a.k.a cross elements
  return setset(illion::hitting(this->zdd_));
}

setset setset::smaller(size_t set_size) const {
  return setset(this->zdd_.PermitSym(set_size - 1));
}

setset setset::join(const setset& ss) const {
  return setset(illion::join(this->zdd_, ss.zdd_));
}

setset setset::meet(const setset& ss) const {
  return setset(illion::meet(this->zdd_, ss.zdd_));
}

setset setset::subsets(const setset& ss) const {
  return setset(this->zdd_.Permit(ss.zdd_));
}

setset setset::supersets(const setset& ss) const {
  return setset(this->zdd_.Restrict(ss.zdd_));
}

setset setset::nonsubsets(const setset& ss) const {
  return setset(illion::nonsubsets(this->zdd_, ss.zdd_));
}

setset setset::nonsupersets(const setset& ss) const {
  return setset(illion::nonsupersets(this->zdd_, ss.zdd_));
}

void setset::dump(ostream& out) const {
  illion::dump(this->zdd_, out);
}

void setset::dump(FILE* fp) const {
  illion::dump(this->zdd_, fp);
}

void setset::load(istream& in) {
  this->zdd_ = illion::load(in);
}

void setset::load(FILE* fp) {
  this->zdd_ = illion::load(fp);
}

void setset::_enum(ostream& out,
                   const pair<const char*, const char*> outer_braces,
                   const pair<const char*, const char*> inner_braces) const {
  illion::_enum(this->zdd_, out, outer_braces, inner_braces);
}

void setset::_enum(FILE* fp,
                   const pair<const char*, const char*> outer_braces,
                   const pair<const char*, const char*> inner_braces) const {
  illion::_enum(this->zdd_, fp, outer_braces, inner_braces);
}

elem_t setset::num_elems() {
  return illion::num_elems();
}

void setset::num_elems(elem_t num_elems) {
  illion::num_elems(num_elems);
}

ostream& operator<<(ostream& out, const setset& ss) {
  illion::dump(ss.zdd_, out);
  return out;
}

istream& operator>>(istream& in, setset& ss) {
  ss.zdd_ = illion::load(in);
  return in;
}

}  // namespace illion
