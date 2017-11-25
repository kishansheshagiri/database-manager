#ifndef SRC_BASE_DUPLICATE_FINDER_H
#define SRC_BASE_DUPLICATE_FINDER_H

#include <unordered_set>
#include <vector>

template <class container>
class DuplicateFinder : protected std::unordered_set<container> {
 public:
  DuplicateFinder(std::vector<container> input) : has_duplicates_(false) {
    for (auto member : input) {
      auto insert_response = this->insert(member);
      if (insert_response.second == false) {
        has_duplicates_ = true;
        break;
      }
    }
  }
  virtual ~DuplicateFinder() {}

  bool HasDuplicates() { return has_duplicates_; }

 private:
  bool has_duplicates_;
};

#endif // SRC_BASE_DUPLICATE_FINDER_H
