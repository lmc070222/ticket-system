#include "Bplustree.hpp"
#include <iostream>
using namespace std;
#include <cstring>
int main() {
  ios::sync_with_stdio(false);
  cin.tie(0);
  int n;
  cin >> n;
  char a[65];
  int value;
  Bplustree<IndexValueKey, int, 50> tree("data");
  string order;
  while (n--) {
    cin >> order;
    if (order == "insert") {
      cin >> a >> value;
      String64 idx(a);
      IndexValueKey x(idx, value);
      tree.Insert(x, value);
    } else if (order == "delete") {
      cin >> a >> value;
      String64 idx(a);
      IndexValueKey x(idx, value);
      tree.deletenode(x, value);
    } else if (order == "find") {
      cin >> a;
      String64 idx(a);
      sjtu::vector<int> ans = tree.find_by_index(idx);
      if (ans.size() != 0) {
        cout << ans[0];
        for (int i = 1; i < ans.size(); ++i)
          cout << ' ' << ans[i];
        cout << '\n';
      } else {
        cout << "null\n";
      }
    }
  }
}