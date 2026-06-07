#ifndef Bplustree_HPP
#define Bplustree_HPP
#include "src/vector.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <type_traits>

struct String64 {
  char data[64];
  String64() { memset(data, 0, 64); }
  String64(const char *s) {
    strncpy(data, s, 63);
    data[63] = '\0';
  }
  String64 &operator=(const String64 &other) {
    memcpy(data, other.data, 64);
    return *this;
  }
  bool operator<(const String64 &other) const {
    return strcmp(data, other.data) < 0;
  }
  bool operator>(const String64 &other) const {
    return strcmp(data, other.data) > 0;
  }
  bool operator>=(const String64 &other) const {
    return strcmp(data, other.data) >= 0;
  }
  bool operator<=(const String64 &other) const {
    return strcmp(data, other.data) <= 0;
  }
  bool operator==(const String64 &other) const {
    return strcmp(data, other.data) == 0;
  }
  bool operator!=(const String64 &other) const {
    return strcmp(data, other.data) != 0;
  }
};

struct IndexValueKey {
  String64 index;
  int value;

  IndexValueKey() : value(0) {}
  IndexValueKey(const String64 &idx, int val) : index(idx), value(val) {}
  bool operator<(const IndexValueKey &other) const {
    if (index < other.index)
      return true;
    if (other.index < index)
      return false;
    return value < other.value;
  }
  bool operator>(const IndexValueKey &other) const { return other < *this; }
  bool operator>=(const IndexValueKey &other) const { return !(*this < other); }
  bool operator<=(const IndexValueKey &other) const { return !(other < *this); }
  bool operator==(const IndexValueKey &other) const {
    return index == other.index && value == other.value;
  }
  bool operator!=(const IndexValueKey &other) const {
    return !(*this == other);
  }
  static bool compareIndex(const IndexValueKey &a, const IndexValueKey &b) {
    return a.index < b.index;
  }
};

template <typename KeyType, typename ValueType, int M> class Bplustree {
public:
  struct FileHeader {
    uint32_t root_page;
    uint32_t leaf_head;
  };

#pragma pack(push, 1)
  struct NodePage {
    bool is_leaf;
    int key_num;
    KeyType keys[M + 2];
    uint32_t parent;
    
    static const size_t ELEM_SIZE = (sizeof(ValueType) > sizeof(uint32_t)) ? sizeof(ValueType) : sizeof(uint32_t);
    char values_or_children_data[ELEM_SIZE * (M + 2)];
    
    uint32_t next;
    
    NodePage() : is_leaf(false), key_num(0), parent(0), next(0) {
      memset(values_or_children_data, 0, sizeof(values_or_children_data));
    }
    
    ValueType& get_value(int idx) {
      return *reinterpret_cast<ValueType*>(values_or_children_data + idx * ELEM_SIZE);
    }
    
    const ValueType& get_value(int idx) const {
      return *reinterpret_cast<const ValueType*>(values_or_children_data + idx * ELEM_SIZE);
    }
    
    uint32_t& get_child(int idx) {
      return *reinterpret_cast<uint32_t*>(values_or_children_data + idx * ELEM_SIZE);
    }
    
    const uint32_t& get_child(int idx) const {
      return *reinterpret_cast<const uint32_t*>(values_or_children_data + idx * ELEM_SIZE);
    }
    
    void set_value(int idx, const ValueType& val) {
      *reinterpret_cast<ValueType*>(values_or_children_data + idx * ELEM_SIZE) = val;
    }
    
    void set_child(int idx, uint32_t child) {
      *reinterpret_cast<uint32_t*>(values_or_children_data + idx * ELEM_SIZE) = child;
    }
  };
#pragma pack(pop)

  class FileManager {
  public:
    std::fstream s;
    std::string fname;

    struct CachePage {
      uint32_t page_no;
      bool is_dirty;
      NodePage page;
      
      CachePage() : page_no(0), is_dirty(false) {}
    };
    static const int CACHE_SIZE = 16;
    CachePage cache[CACHE_SIZE];
    int replace_idx;

    int getvalue(uint32_t a) {
      if (a == 0)
        return -1;
      for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].page_no == a)
          return i;
      }
      return -1;
    }

    int getfree() {
      for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].page_no == 0)
          return i;
      }
      int idx = replace_idx;
      replace_idx = (replace_idx + 1) % CACHE_SIZE;
      return idx;
    }

    void flush(int a) {
      if (cache[a].page_no != 0 && cache[a].is_dirty == true) {
        s.clear();
        s.seekp((cache[a].page_no - 1) * sizeof(NodePage) + sizeof(FileHeader));
        s.write(reinterpret_cast<char *>(&cache[a].page), sizeof(NodePage));
        cache[a].is_dirty = false;
      }
    }
    
    FileManager(const std::string &filename) : fname(filename), replace_idx(0) {
      for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].page_no = 0;
        cache[i].is_dirty = false;
      }
      s.open(filename, std::ios::in | std::ios::out | std::ios::binary);
      if (!s.is_open()) {
        s.clear();
        s.open(filename, std::ios::out | std::ios::binary);
        s.close();
        s.open(filename, std::ios::in | std::ios::out | std::ios::binary);
      }
    }
    
    void clear_file() {
      if (s.is_open())
        s.close();
      s.open(fname,
             std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
      for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].page_no = 0;
        cache[i].is_dirty = false;
      }
      replace_idx = 0;
    }

    void ReadPage(uint32_t page_no, NodePage *buf) {
      memset(buf, 0, sizeof(NodePage));
      if (page_no == 0)
        return;

      int idx = getvalue(page_no);
      if (idx != -1) {
        *buf = cache[idx].page;
        return;
      }
      s.clear();
      s.seekg((page_no - 1) * sizeof(NodePage) + sizeof(FileHeader));
      s.read(reinterpret_cast<char *>(buf), sizeof(NodePage));

      int idx_free = getfree();
      flush(idx_free);
      cache[idx_free].page_no = page_no;
      cache[idx_free].is_dirty = false;
      cache[idx_free].page = *buf;
    }

    void WritePage(uint32_t page_no, NodePage *buf) {
      int idx = getvalue(page_no);
      if (idx == -1) {
        idx = getfree();
        flush(idx);
      }
      cache[idx].page_no = page_no;
      cache[idx].is_dirty = true;
      cache[idx].page = *buf;
    }

    uint32_t AllocPage(NodePage *buf) {
      s.clear();
      s.seekp(0, std::ios::end);
      auto current_pos = s.tellp();
      uint32_t new_page =
          (static_cast<long long>(current_pos) - sizeof(FileHeader)) /
              sizeof(NodePage) +
          1;
      s.write(reinterpret_cast<char *>(buf), sizeof(NodePage));
      int idx = getvalue(new_page);
      if (idx == -1)
        idx = getfree();
      flush(idx);
      cache[idx].page_no = new_page;
      cache[idx].is_dirty = false;
      cache[idx].page = *buf;
      return new_page;
    }

    void ReadHeader(FileHeader *buf) {
      s.clear();
      s.seekg(0);
      s.read(reinterpret_cast<char *>(buf), sizeof(FileHeader));
    }

    void WriteHeader(FileHeader *buf) {
      s.clear();
      s.seekp(0);
      s.write(reinterpret_cast<char *>(buf), sizeof(FileHeader));
    }

    ~FileManager() {
      for (int i = 0; i < CACHE_SIZE; i++) {
        flush(i);
      }
      if (s.is_open())
        s.close();
    }
  };
  
  std::string filename_path;
  FileManager fm;
  uint32_t root_page_;
  uint32_t leaf_head_;
  
  Bplustree(const std::string &filename)
      : filename_path(filename), fm(filename_path), root_page_(0),
        leaf_head_(0) {
    fm.s.seekg(0, std::ios::end);
    auto fileSize = fm.s.tellg();
    if (fileSize < sizeof(FileHeader)) {
      FileHeader init;
      init.root_page = 0;
      init.leaf_head = 0;
      fm.WriteHeader(&init);
    } else {
      FileHeader header;
      fm.ReadHeader(&header);
      root_page_ = header.root_page;
      leaf_head_ = header.leaf_head;
    }
  }
  
  void clear() {
    fm.clear_file();
    root_page_ = 0;
    leaf_head_ = 0;
    FileHeader init;
    init.root_page = 0;
    init.leaf_head = 0;
    fm.WriteHeader(&init);
  }
  
  sjtu::vector<ValueType> find_by_index(const KeyType &index) {
    sjtu::vector<ValueType> ans;
    if (root_page_ == 0)
      return ans;
    uint32_t cur_page = root_page_;
    NodePage cur_node;
    while (true) {
      fm.ReadPage(cur_page, &cur_node);
      if (cur_node.is_leaf)
        break;
      int i = 0;
      for (; i < cur_node.key_num; ++i) {
        if (cur_node.keys[i] >= index)
          break;
      }
      cur_page = cur_node.get_child(i);
    }
    while (true) {
      fm.ReadPage(cur_page, &cur_node);
      for (int i = 0; i < cur_node.key_num; ++i) {
        if (cur_node.keys[i] == index) {
          ans.push_back(cur_node.get_value(i));
        } else if (cur_node.keys[i] > index) {
          return ans;
        }
      }
      if (cur_node.next == 0)
        break;
      cur_page = cur_node.next;
    }
    return ans;
  }

  void SplitInternal(uint32_t page_no, NodePage &curnode) {
    NodePage new_node;
    new_node.is_leaf = false;
    int total = curnode.key_num;
    int mid = total / 2;
    KeyType promote_key = curnode.keys[mid];

    new_node.key_num = total - mid - 1;
    for (int i = mid + 1; i < total; i++) {
      new_node.keys[i - (mid + 1)] = curnode.keys[i];
    }
    for (int i = mid + 1; i <= total; i++) {
      new_node.set_child(i - (mid + 1), curnode.get_child(i));
    }
    curnode.key_num = mid;

    new_node.parent = curnode.parent;
    uint32_t new_page = fm.AllocPage(&new_node);
    for (int i = 0; i <= new_node.key_num; i++) {
      uint32_t child_no = new_node.get_child(i);
      NodePage child;
      fm.ReadPage(child_no, &child);
      child.parent = new_page;
      fm.WritePage(child_no, &child);
    }
    fm.WritePage(page_no, &curnode);
    InsertIntoParent(page_no, promote_key, new_page);
  }
  
  void InsertIntoParent(uint32_t leftPageNo, KeyType key,
                        uint32_t rightPageNo) {
    NodePage leftnode;
    NodePage rightnode;
    fm.ReadPage(leftPageNo, &leftnode);
    fm.ReadPage(rightPageNo, &rightnode);
    if (leftnode.parent == 0) {
      NodePage new_parent;
      new_parent.parent = 0;
      new_parent.is_leaf = false;
      new_parent.key_num = 1;
      new_parent.keys[0] = key;
      new_parent.set_child(0, leftPageNo);
      new_parent.set_child(1, rightPageNo);
      leftnode.parent = fm.AllocPage(&new_parent);
      rightnode.parent = leftnode.parent;
      FileHeader fileheader;
      fm.ReadHeader(&fileheader);
      fileheader.root_page = leftnode.parent;
      root_page_ = leftnode.parent;
      fm.WriteHeader(&fileheader);
      fm.WritePage(leftPageNo, &leftnode);
      fm.WritePage(rightPageNo, &rightnode);
      return;
    }
    uint32_t parentno = leftnode.parent;
    NodePage parent;
    fm.ReadPage(parentno, &parent);
    int k = -1;
    for (int i = 0; i <= parent.key_num; i++) {
      if (parent.get_child(i) == leftPageNo) {
        k = i;
        break;
      }
    }
    for (int i = parent.key_num; i > k; i--) {
      parent.set_child(i + 1, parent.get_child(i));
    }
    for (int i = parent.key_num - 1; i >= k; i--) {
      parent.keys[i + 1] = parent.keys[i];
    }
    parent.keys[k] = key;
    parent.set_child(k + 1, rightPageNo);
    parent.key_num++;
    fm.WritePage(parentno, &parent);
    if (parent.key_num <= M - 1)
      return;
    SplitInternal(parentno, parent);
  }
  
  void SplitLeaf(uint32_t page_no, NodePage &curnode) {
    NodePage new_leaf;
    new_leaf.is_leaf = true;
    int t = 0;
    for (int i = M / 2; i < curnode.key_num; i++) {
      new_leaf.keys[t] = curnode.keys[i];
      new_leaf.set_value(t, curnode.get_value(i));
      t++;
    }
    curnode.key_num = M / 2;
    new_leaf.key_num = t;
    new_leaf.parent = curnode.parent;

    new_leaf.next = curnode.next;
    curnode.next = fm.AllocPage(&new_leaf);

    fm.WritePage(page_no, &curnode);
    InsertIntoParent(page_no, curnode.keys[curnode.key_num - 1], curnode.next);
  }
  
  bool empty() const {
    return root_page_ == 0;
  }
  
  void Insert(KeyType &index, ValueType &value) {
    if (root_page_ == 0) {
      NodePage new_;
      new_.is_leaf = true;
      new_.key_num = 1;
      new_.keys[0] = index;
      new_.set_value(0, value);
      new_.parent = 0;
      new_.next = 0;
      uint32_t new_page = fm.AllocPage(&new_);
      FileHeader new_header;
      new_header.root_page = new_header.leaf_head = new_page;
      root_page_ = leaf_head_ = new_page;
      fm.WriteHeader(&new_header);
      return;
    } else {
      NodePage curnode;
      uint32_t x = root_page_;
      while (true) {
        fm.ReadPage(x, &curnode);
        if (curnode.is_leaf == true)
          break;
        else {
          int i = 0;
          for (; i < curnode.key_num; i++) {
            if (curnode.keys[i] >= index)
              break;
          }
          x = curnode.get_child(i);
        }
      }
      NodePage check_node = curnode;
      while (true) {
        bool stop_search = false;
        for (int i = 0; i < check_node.key_num; i++) {
          if (check_node.keys[i] == index && check_node.get_value(i) == value) {
            return;
          }
          if (check_node.keys[i] > index) {
            stop_search = true;
            break;
          }
        }

        if (stop_search)
          break;
        if (check_node.key_num > 0 &&
            check_node.keys[check_node.key_num - 1] == index &&
            check_node.next != 0) {
          fm.ReadPage(check_node.next, &check_node);
        } else {
          break;
        }
      }
      int k = curnode.key_num;
      for (int i = 0; i < curnode.key_num; i++) {
        if (curnode.keys[i] >= index) {
          k = i;
          break;
        }
      }

      for (int j = curnode.key_num - 1; j >= k; j--) {
        curnode.keys[j + 1] = curnode.keys[j];
        curnode.set_value(j + 1, curnode.get_value(j));
      }
      curnode.key_num++;
      curnode.keys[k] = index;
      curnode.set_value(k, value);
      fm.WritePage(x, &curnode);

      if (curnode.key_num <= M)
        return;
      else {
        SplitLeaf(x, curnode);
      }
    }
  }
  
  void sibling(uint32_t curno, uint32_t afterno) {
    NodePage curnode;
    NodePage afternode;
    fm.ReadPage(curno, &curnode);
    fm.ReadPage(afterno, &afternode);
    NodePage parentnode;
    fm.ReadPage(curnode.parent, &parentnode);
    if (curnode.is_leaf == true) {
      int t = 0;
      for (int i = curnode.key_num; i < curnode.key_num + afternode.key_num;
           i++) {
        curnode.keys[i] = afternode.keys[t];
        curnode.set_value(i, afternode.get_value(t));
        t++;
      }
      curnode.key_num = curnode.key_num + afternode.key_num;
      curnode.next = afternode.next;
    } else {
      int cur, after;
      for (int i = 0; i <= parentnode.key_num; i++) {
        if (parentnode.get_child(i) == curno)
          cur = i;
        if (parentnode.get_child(i) == afterno)
          after = i;
      }
      int t = 0, t1 = 0;
      curnode.keys[curnode.key_num] = parentnode.keys[cur];
      for (int i = curnode.key_num + 1;
           i < curnode.key_num + afternode.key_num + 1; i++) {
        curnode.keys[i] = afternode.keys[t];
        t++;
      }
      for (int i = curnode.key_num + 1;
           i < curnode.key_num + afternode.key_num + 2; i++) {
        curnode.set_child(i, afternode.get_child(t1));
        uint32_t moved_child_no = curnode.get_child(i);
        NodePage moved_child;
        fm.ReadPage(moved_child_no, &moved_child);
        moved_child.parent = curno;
        fm.WritePage(moved_child_no, &moved_child);
        t1++;
      }
      curnode.key_num = curnode.key_num + afternode.key_num + 1;
    }
    fm.WritePage(curno, &curnode);
    fm.WritePage(afterno, &afternode);

    int curk = -1, afterk = -1;
    for (int i = 0; i <= parentnode.key_num; i++) {
      if (parentnode.get_child(i) == curno)
        curk = i;
      if (parentnode.get_child(i) == afterno)
        afterk = i;
    }
    // 【不变】使用左移来覆盖被合并掉的路由键与子节点指针
    for (int i = curk; i < parentnode.key_num - 1; i++) {
      parentnode.keys[i] = parentnode.keys[i + 1];
    }
    for (int i = afterk; i < parentnode.key_num; i++) {
      parentnode.set_child(i, parentnode.get_child(i + 1));
    }
    parentnode.key_num--;
    fm.WritePage(curnode.parent, &parentnode);
    
    if (parentnode.key_num >= M / 2)
      return;
    if (parentnode.parent == 0) {
      if (parentnode.key_num == 0) {
        if (parentnode.is_leaf == false) {
          uint32_t onlyChild = parentnode.get_child(0);
          NodePage only;
          fm.ReadPage(onlyChild, &only);
          only.parent = 0;
          root_page_ = onlyChild;
          FileHeader header;
          header.root_page = root_page_;
          header.leaf_head = leaf_head_;
          fm.WriteHeader(&header);
          fm.WritePage(onlyChild, &only);
        }
      }
      return;
    }
  }
  
  void deletenode(KeyType &index, ValueType &value) {
    uint32_t cur_page = root_page_;
    NodePage curnode;
    while (true) {
      fm.ReadPage(cur_page, &curnode);
      if (curnode.is_leaf == true)
        break;
      else {
        int i = 0;
        for (; i < curnode.key_num; i++) {
          if (curnode.keys[i] >= index)
            break;
        }
        cur_page = curnode.get_child(i);
      }
    }
    while (true) {
      fm.ReadPage(cur_page, &curnode);
      for (int k = 0; k < curnode.key_num; k++) {
        if (curnode.keys[k] == index && curnode.get_value(k) == value) {
          for (int i = k; i < curnode.key_num - 1; i++) {
            curnode.keys[i] = curnode.keys[i + 1];
            curnode.set_value(i, curnode.get_value(i + 1));
          }
          curnode.key_num--;
          fm.WritePage(cur_page, &curnode);
          if (curnode.key_num >= M / 2 || curnode.parent == 0)
            return;
            
          uint32_t parentpage = curnode.parent;
          NodePage parentnode;
          fm.ReadPage(parentpage, &parentnode);
          int curk = -1;
          for (int i = 0; i <= parentnode.key_num; i++) {
            if (parentnode.get_child(i) == cur_page) {
              curk = i;
              break;
            }
          }
          // 向左借位
          if (curk > 0) {
            NodePage leftsibling;
            uint32_t leftpage = parentnode.get_child(curk - 1);
            fm.ReadPage(leftpage, &leftsibling);
            if (leftsibling.key_num > M / 2) {
              for (int i = curnode.key_num - 1; i >= 0; i--) {
                curnode.keys[i + 1] = curnode.keys[i];
                curnode.set_value(i + 1, curnode.get_value(i));
              }
              // 拉取左兄弟的最后一位到 curnode 的第一位
              curnode.keys[0] = leftsibling.keys[leftsibling.key_num - 1];
              curnode.set_value(0, leftsibling.get_value(leftsibling.key_num - 1));
              curnode.key_num++;
              leftsibling.key_num--;
              
              // 【核心修复点】：由于你的实现是父节点保存的是"左子树的最大值"
              // 所以更新父节点时，应该用左兄弟更新后的最末尾（即最大）值
              parentnode.keys[curk - 1] = leftsibling.keys[leftsibling.key_num - 1]; 
              
              fm.WritePage(cur_page, &curnode);
              fm.WritePage(leftpage, &leftsibling);
              fm.WritePage(parentpage, &parentnode);
              return;
            }
          }
          // 向右借位
          if (curk < parentnode.key_num) {
            NodePage rightsibling;
            uint32_t rightpage = parentnode.get_child(curk + 1);
            fm.ReadPage(rightpage, &rightsibling);
            if (rightsibling.key_num > M / 2) {
              // 拉取右兄弟的第一位到 curnode 的末尾
              curnode.keys[curnode.key_num] = rightsibling.keys[0];
              curnode.set_value(curnode.key_num, rightsibling.get_value(0));
              curnode.key_num++;
              for (int i = 0; i < rightsibling.key_num - 1; i++) {
                rightsibling.keys[i] = rightsibling.keys[i + 1];
                rightsibling.set_value(i, rightsibling.get_value(i + 1));
              }
              rightsibling.key_num--;
              
              // 【核心修复点】：父节点存放的是 curnode (即左子树) 的最大值
              // 当前 curnode 的最大值就是我们刚刚追加到末尾的那个键
              parentnode.keys[curk] = curnode.keys[curnode.key_num - 1]; 
              
              fm.WritePage(cur_page, &curnode);
              fm.WritePage(rightpage, &rightsibling);
              fm.WritePage(parentpage, &parentnode);
              return;
            }
          }
          if (curk > 0) {
            uint32_t leftpage = parentnode.get_child(curk - 1);
            sibling(leftpage, cur_page);
          } else if (curk < parentnode.key_num) {
            uint32_t rightpage = parentnode.get_child(curk + 1);
            sibling(cur_page, rightpage);
          }
          return;
        }
      }
      if (curnode.next == 0)
        break;
      cur_page = curnode.next;
    }
  }
};

#endif