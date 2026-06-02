#ifndef USERMANAGER_HPP
#define USERMANAGER_HPP

#include "src/vector.hpp"
#include "map/map.hpp" 
#include "user.hpp"
#include <cstring>
#include "utils.hpp"
struct UsernameKey {
    char str[22];
    UsernameKey() { std::memset(str, 0, sizeof(str)); }
    UsernameKey(const char* s) {
        std::memset(str, 0, sizeof(str));
        std::strncpy(str, s, 21);
    }
    bool operator<(const UsernameKey& other) const {
        return std::strcmp(str, other.str) < 0;
    }
    bool operator==(const UsernameKey& other) const {
        return std::strcmp(str, other.str) == 0;
    }
    bool operator>(const UsernameKey& other) const {
        return std::strcmp(str, other.str) > 0;
    }
    bool operator >=(const UsernameKey& other) const {
      return *this == other or *this > other;
    }
    bool operator <=(const UsernameKey& other) const {
      return *this == other or *this < other;
    }
};

class usermanager {
public:
    std::string filename = "usertree";
    Bplustree<UsernameKey, user, 60> usertree; 
    sjtu::map<UsernameKey, user> logstack;
    usermanager () : usertree(filename) {
        
    }
    bool add_user(const char* cur_username, const char* username, const char* password, const char* name, const char* mailaddr, int g) {
        UsernameKey targetkey(username);
        sjtu::vector<user> x = usertree.find_by_index(targetkey);
        if (x.size() != 0) return false;
        UsernameKey cur_key(cur_username);
        auto it = logstack.find(cur_key);
        if (it == logstack.end() or (*it).second.privilege <= g) return false;

        user user_;
        memset(&user_, 0, sizeof(user_));
        strncpy(user_.username, username, 21);
        strncpy(user_.password, password, 51);
        strncpy(user_.name, name, 23);
        strncpy(user_.mailadd, mailaddr, 31);
        user_.privilege = g;
        usertree.Insert(targetkey, user_);
        return true;
    }
    bool add_user(const char* username, const char* password, const char* name, const char* mailaddr) {
        UsernameKey targetkey(username);

        user user_;
        memset(&user_, 0, sizeof(user_));
        strncpy(user_.username, username, 21);
        strncpy(user_.password, password, 51);
        strncpy(user_.name, name, 23);
        strncpy(user_.mailadd, mailaddr, 31);
        user_.privilege = 10;
        usertree.Insert(targetkey, user_);
        return true;
    }

    bool login(const char* username, const char* password) {
        UsernameKey target_key(username);
        auto it = logstack.find(target_key);
        if (it != logstack.end()) return false;

        sjtu::vector<user> x = usertree.find_by_index(target_key);
        if (x.size() == 0) return false;
        user y = x[0];
        if (std::strcmp(y.password, password) != 0) return false; 
        logstack.insert({target_key, y});
        return true;
    }

    bool logout(const char* username) {
        UsernameKey target_key(username);
        auto it = logstack.find(target_key);
        if (it == logstack.end()) return false;
        logstack.erase(it);
        return true;
    }
    bool query_profile(const char* cur_username, const char* username, user& searchresult) {
        UsernameKey cur_key(cur_username);
        auto it = logstack.find(cur_key);
        if (it == logstack.end()) return false;  
        user curuser = (*it).second; 
        UsernameKey target_key(username);
        sjtu::vector<user> b = usertree.find_by_index(target_key);
        if (b.size() == 0) return false;
        user nowuser = b.back();
        if (std::strcmp(cur_username, username) != 0 and curuser.privilege <= nowuser.privilege) {
            return false;
        }
        searchresult = nowuser;
        return true;
    }

    bool modify_profile(const char* cur_username, const char* username, user newuser) {
        UsernameKey cur_key(cur_username);
        auto it = logstack.find(cur_key);
        if (it == logstack.end()) return false;
        
        user curuser = (*it).second; 

        UsernameKey target_key(username);
        sjtu::vector<user> b = usertree.find_by_index(target_key);
        if (b.size() == 0) return false;
        user nowuser = b.back();
        if (std::strcmp(cur_username, username) == 0) {
            if (newuser.privilege > curuser.privilege) return false;
        } else {
            if (curuser.privilege <= nowuser.privilege or curuser.privilege <= newuser.privilege) {
                return false;
            }
        }
        usertree.deletenode(target_key, nowuser);
        usertree.Insert(target_key, newuser);
        auto it_target = logstack.find(target_key);
        if (it_target != logstack.end()) {
            (*it_target).second = newuser; 
        }
        return true;
    }
};

#endif