#include "illion/setset.h"

#include <cstdlib>

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

setset::iterator::iterator(const setset& ss)
    : zdd_(ss.zdd_), weights_(ss.weights_) {
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

setset::setset(const map<string, set<elem_t> >& m) {
  // TODO: check map keys that are not include/exclude
  // TODO: check if common elements found in include and exclude
  const auto in_it = m.find("include");
  const auto ex_it = m.find("exclude");
  const set<elem_t>& in_s = in_it != m.end() ? in_it->second : set<elem_t>();
  const set<elem_t>& ex_s = ex_it != m.end() ? ex_it->second : set<elem_t>();
  for (const auto& e : in_s) single(e);
  for (const auto& e : ex_s) single(e);
  vector<zdd_t> n(num_elems() + 2);
  n[0] = bot(), n[1] = top();
  for (elem_t v = num_elems(); v > 0; --v) {
    elem_t i = num_elems() - v + 2;
    n[i] = in_s.find(v) != in_s.end() ? n[0]   + single(v) * n[i-1]
         : ex_s.find(v) != ex_s.end() ? n[i-1] + single(v) * n[0]
         :                              n[i-1] + single(v) * n[i-1];
  }
  this->zdd_ = n[num_elems() + 1];
}

setset::setset(const vector<map<string, set<elem_t> > >& v ) : zdd_(bot()) {
  for (const auto& m : v)
    this->zdd_ += setset(m).zdd_;
}

setset::setset(const initializer_list<set<elem_t> >& v) : zdd_(bot()) {
  for (auto i = v.begin(); i != v.end(); ++i)
    this->zdd_ += setset(*i).zdd_;
}

setset::setset(istream& in) : zdd_(load(in)) {
}
/*
setset::setset(const initializer_list<int>& s) : zdd_(top()) {
  for (auto i = s.begin(); i != s.end(); ++i)
    this->zdd_ *= single(*i);
}
*/
setset setset::operator~() const {
  return setset(_not(this->zdd_));
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

setset setset::operator*(const setset& ss) const {
  return setset(this->zdd_ * ss.zdd_);
}

setset setset::operator^(const setset& ss) const {
  return setset((this->zdd_ - ss.zdd_) + (ss.zdd_ - this->zdd_));
}

setset setset::operator/(const setset& ss) const {
  return setset(this->zdd_ / ss.zdd_);
}

setset setset::operator%(const setset& ss) const {
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

void setset::operator*=(const setset& ss) {
  this->zdd_ *= ss.zdd_;
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

setset setset::find(elem_t e) const {
  zdd_t z1 = setset({{e}}).zdd_;
  zdd_t z2 = this->zdd_ / z1;
  return setset(z2 * z1);
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
  setset ss = this->find(e);
  *this -= ss;
  return atoll(ss.size().c_str());
}

void setset::clear() {
  this->zdd_ = bot();
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

setset setset::smaller(size_t max_set_size) const {
  return setset(this->zdd_.PermitSym(max_set_size));
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

ostream& operator<<(ostream& out, const setset& ss) {
  save(ss.zdd_, out);
  return out;
}

istream& operator>>(istream& in, setset& ss) {
  ss.zdd_ = load(in);
  return in;
}

void setset::dump() const {
  illion::dump(this->zdd_);
}

}  // namespace illion
