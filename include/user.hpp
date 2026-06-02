#ifndef USER_HPP
#define USER_HPP
#include "train.hpp"
#include <cstring>
struct user {
  char username[22];
  char password[52];
  char name[24];
  char mailadd[32];
  int privilege;
  user() {
    std::memset(username, 0, sizeof(username));
    std::memset(password, 0, sizeof(password));
    std::memset(name, 0, sizeof(name));
    std::memset(mailadd, 0, sizeof(mailadd));
    privilege = 0;
  }
  user(const char *username_, const char *password_, const char *name_,
       const char *mailadd_, int privilege_) {
    std::memset(username, 0, sizeof(username));
    std::strncpy(username, username_, sizeof(username) - 1);

    std::memset(password, 0, sizeof(password));
    std::strncpy(password, password_, sizeof(password) - 1);

    std::memset(name, 0, sizeof(name));
    std::strncpy(name, name_, sizeof(name) - 1);

    std::memset(mailadd, 0, sizeof(mailadd));
    std::strncpy(mailadd, mailadd_, sizeof(mailadd) - 1);

    this->privilege = privilege_;
  }
  bool operator<(const user &other) const {
    return std::strcmp(username, other.username) < 0;
  }
  bool operator>(const user &other) const {
    return std::strcmp(username, other.username) > 0;
  }
  bool operator==(const user &other) const {
    return std::strcmp(username, other.username) == 0;
  }
  bool operator>=(const user &other) const { return !(*this < other); }
  bool operator<=(const user &other) const { return !(*this > other); }
};
struct OrderKey {
    char username[22];
    bool operator < (const OrderKey & other) const {
      return memcmp (username,other.username,22) < 0;
    }
    bool operator <= (const OrderKey & other) const {
      return memcmp (username,other.username,22) <= 0;
    }
    bool operator > (const OrderKey & other) const {
      return memcmp (username,other.username,22) > 0;
    }
    bool operator >= (const OrderKey & other) const {
      return memcmp (username,other.username,22) >= 0;
    }
    bool operator == (const OrderKey & other) const {
      return memcmp (username,other.username,22) == 0;
    }
};
struct Order {
  char trainID[22];
  int status;
  char from[52];
  date leaving_date ;
  daytime leaving_daytime;
  date arrival_date;
  daytime arrival_dattime;
  char to[52];
  int price;
  int num;
  int timestamp;
};
#endif