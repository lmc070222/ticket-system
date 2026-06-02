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
  
  user() = default;
  
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
  
  Order() = default;
  
  bool operator==(const Order &other) const {
    if (status != other.status) return false;
    if (price != other.price) return false;
    if (num != other.num) return false;
    if (timestamp != other.timestamp) return false;
    if (memcmp(trainID, other.trainID, sizeof(trainID)) != 0) return false;
    if (memcmp(from, other.from, sizeof(from)) != 0) return false;
    if (memcmp(to, other.to, sizeof(to)) != 0) return false;
    return true;
  }
};
#endif