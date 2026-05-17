#include "Bplustree.hpp"
#include <iostream>
using namespace std;
#include <cstring>
// 复合键：先比较 index，再比较 value
static void merge(sjtu::vector<int> &v, sjtu::vector<int> &tmp, int l, int m,
                  int r) {
  int i = l, j = m, k = l;
  while (i < m && j < r) {
    tmp[k++] = (v[i] <= v[j]) ? v[i++] : v[j++];
  }
  while (i < m)
    tmp[k++] = v[i++];
  while (j < r)
    tmp[k++] = v[j++];
  for (int t = l; t < r; ++t)
    v[t] = tmp[t];
}
static void merge_sort_rec(sjtu::vector<int> &v, sjtu::vector<int> &tmp, int l,
                           int r) {
  if (r - l <= 1)
    return;
  int m = l + (r - l) / 2;
  merge_sort_rec(v, tmp, l, m);
  merge_sort_rec(v, tmp, m, r);
  merge(v, tmp, l, m, r);
}
void merge_sort(sjtu::vector<int> &v) {
  if (v.size() <= 1)
    return;
  sjtu::vector<int> tmp;
  for (int i = 0; i < v.size(); i++)
    tmp.push_back(0);
  merge_sort_rec(v, tmp, 0, v.size());
}
int main() {
    ios::sync_with_stdio(false); cin.tie(0);   // 加速 I/O
    int n;
    cin >> n;
    char a[65];
    int value;
    Bplustree<IndexValueKey, int, 50> tree;   // 注意：ValueType 仍为 int，但我们不再使用 values 数组中的值
    string order;
    while (n--) {
        cin >> order;
        if (order == "insert") {
            cin >> a >> value;
            String64 idx(a);
            IndexValueKey x(idx, value);
            tree.Insert(x, value);   // 第二个参数 value 会被写入 values 数组，但不影响查找
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
                for (int i = 1; i < ans.size(); ++i) cout << ' ' << ans[i];
                cout << '\n';
            } else {
                cout << "null\n";
            }
        }
    }
}