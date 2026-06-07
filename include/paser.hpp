#ifndef PASER_HPP
#define PASER_HPP

#include "src/vector.hpp"
#include "trainmanager.hpp"
#include "user.hpp"
#include "usermanger.hpp"
#include <cstring>
#include <iostream>
#include <string>

class paser {
  public:
  usermanager usersystem;
  trainmanager trainsystem;
  
  struct CommandArgs {
    int timestamp;
    std::string cmd_name;
    std::string args[128];
    bool has_arg(char flag) { return !args[flag].empty(); }
    void clear() {
      cmd_name.clear();
      for (int i = 0; i < 128; i++) {
        args[i].clear();
      }
    }
  };
  
  CommandArgs current_cmd;
  std::string token;

  bool readcommand() {
    if (!(std::cin >> token))
      return false;
    current_cmd.timestamp = std::stoi(token.substr(1, token.length() - 2));
    std::cin >> current_cmd.cmd_name;
    while (true) {
      char c = std::cin.get();
      if (c == '\n' or c == EOF)
        break;
      if (c == ' ' or c == '\r')
        continue;
      std::cin.unget();

      std::string flag_token, value_token;
      std::cin >> flag_token;
      char flag = flag_token[1];
      std::cin >> value_token;
      current_cmd.args[flag] = value_token;
    }
    return true;
  }

  int change_string_to_int(std::string x) {
    if (x.empty()) return 0;
    int ans = 0;
    for (size_t t = 0; t < x.size(); t++) {
      ans = 10 * ans + (x[t] - '0');
    }
    return ans;
  }

  daytime change_string_to_daytime (std::string x) {
    int hour = (x[0] - '0') * 10 + (x[1] - '0');
    int minute = (x[3] - '0') * 10 + (x[4] - '0');
    daytime ans;
    ans.hour = hour;
    ans.minute = minute;
    ans.day = 0;
    return ans;
  }

  date change_string_to_date (std::string x) {
    int month = (x[0] - '0') * 10 + (x[1] - '0');
    int day = (x[3] - '0') * 10 + (x[4] - '0');
    date ans;
    ans.month = month;
    ans.day = day;
    return ans;
  }

  sjtu::vector<std::string> split_string(const std::string &str, char del) {
    sjtu::vector<std::string> res;
    size_t start = 0;
    size_t end = str.find(del);
    while (end != std::string::npos) {
      res.push_back(str.substr(start, end - start));
      start = end + 1;
      end = str.find(del, start);
    }
    res.push_back(str.substr(start));
    return res;
  }

public:
  void exucute() {
    if (current_cmd.cmd_name == "add_user") {
      if (usersystem.usertree.empty() == false) {
        char cur_username[22];
        char username[22];
        char password[52];
        char name[24];
        char mailaddr[32];
        int privilege;
        strcpy(cur_username, current_cmd.args['c'].c_str());
        strcpy(username, current_cmd.args['u'].c_str());
        strcpy(password, current_cmd.args['p'].c_str());
        strcpy(name, current_cmd.args['n'].c_str());
        strcpy(mailaddr, current_cmd.args['m'].c_str());
        privilege = change_string_to_int(current_cmd.args['g']);
        
        bool x = usersystem.add_user(cur_username, username, password, name, mailaddr, privilege);
        std::cout << '[' << current_cmd.timestamp << ']' << ' ';
        if (x) std::cout << 0 << '\n';
        else std::cout << -1 << '\n';
      } else {
        char username[22];
        char password[52];
        char name[24];
        char mailaddr[32];
        strcpy(username, current_cmd.args['u'].c_str());
        strcpy(password, current_cmd.args['p'].c_str());
        strcpy(name, current_cmd.args['n'].c_str());
        strcpy(mailaddr, current_cmd.args['m'].c_str());
        
        bool x = usersystem.add_user(username, password, name, mailaddr);
        std::cout << '[' << current_cmd.timestamp << ']' << ' ';
        if (x) std::cout << 0 << '\n';
        else std::cout << -1 << '\n';
      }
    } else if (current_cmd.cmd_name == "login") {
      char username[22];
      char password[52];
      strcpy(username, current_cmd.args['u'].c_str());
      strcpy(password, current_cmd.args['p'].c_str());
      bool x = usersystem.login(username, password);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (x) std::cout << 0 << '\n';
      else std::cout << -1 << '\n';
    } else if (current_cmd.cmd_name == "logout") {
      char username[22];
      strcpy(username, current_cmd.args['u'].c_str());
      bool x = usersystem.logout(username);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (x) std::cout << 0 << '\n';
      else std::cout << -1 << '\n';
    } else if (current_cmd.cmd_name == "query_profile") {
      user tmp;
      char cur_username[22];
      char username[22];
      strcpy(cur_username, current_cmd.args['c'].c_str());
      strcpy(username, current_cmd.args['u'].c_str());
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      bool x = usersystem.query_profile(cur_username, username, tmp);
      if (x) {
        std::cout << tmp.username << ' ' << tmp.name << ' ' << tmp.mailadd
                  << ' ' << tmp.privilege << '\n';
      } else
        std::cout << -1 << '\n';
    } else if (current_cmd.cmd_name == "modify_profile") {
      char cur_username[22];
      char username[22];
      strcpy(cur_username, current_cmd.args['c'].c_str());
      strcpy(username, current_cmd.args['u'].c_str());
      sjtu::vector<user> tmp_v = usersystem.usertree.find_by_index(UsernameKey(username));
      sjtu::vector<user> tmp_cur = usersystem.usertree.find_by_index(UsernameKey(cur_username));
      user tmp;
      if (tmp_v.size() == 0 or tmp_cur.size() == 0) {
        std::cout << '[' << current_cmd.timestamp << ']' << ' ';
        std::cout << -1 << '\n';
        current_cmd.clear();
        return ;
      } 
      user cur = tmp_cur.back();
      tmp = tmp_v.back();
      if (current_cmd.has_arg('p')) {
        strcpy(tmp.password, current_cmd.args['p'].c_str());
      }
      if (current_cmd.has_arg('n')) {
        strcpy(tmp.name, current_cmd.args['n'].c_str());
      }
      if (current_cmd.has_arg('m')) {
        if (current_cmd.timestamp == 236892) std::cout << "wrong" ;
        strcpy(tmp.mailadd, current_cmd.args['m'].c_str());
      }
      if (current_cmd.has_arg('g')) {
        tmp.privilege = change_string_to_int(current_cmd.args['g']);
        if (cur.privilege <= tmp.privilege) {
          std::cout << '[' << current_cmd.timestamp << ']' << ' ';
          std::cout << -1 << '\n';
          current_cmd.clear();
          return ;
        }
      }
      bool x = usersystem.modify_profile(cur_username, username, tmp);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (x) {
        std::cout << tmp.username << ' ' << tmp.name << ' ' << tmp.mailadd
                  << ' ' << tmp.privilege << '\n';
      } else
        std::cout << -1 << '\n';
    } else if (current_cmd.cmd_name == "add_train") {
      char trainid[22];
      int station_num;
      int seat_num;
      char stations[102][52];
      int prices[102];
      daytime starttimes;
      int travel_times[102];
      int leaving_times[102];
      char type;
      
      strcpy (trainid, current_cmd.args['i'].c_str());
      station_num = change_string_to_int(current_cmd.args['n']);
      seat_num = change_string_to_int(current_cmd.args['m']);
      
      sjtu::vector<std::string> x = split_string(current_cmd.args['s'], '|');
      for (int i = 1; i <= station_num; i++) {
        memcpy(stations[i], x[i-1].c_str(), 52);
      }
      
      sjtu::vector<std::string> price_string = split_string(current_cmd.args['p'], '|');
      for (int i = 1; i < station_num; i++) {
        prices[i] = change_string_to_int(price_string[i-1]);
      }
      
      sjtu::vector<std::string> traveltimes_string = split_string(current_cmd.args['t'], '|');
      for (int i = 1; i < station_num; i++) {
        travel_times[i] = change_string_to_int(traveltimes_string[i - 1]);
      }
      leaving_times[1] = 0; 
      if (station_num > 2) {
        sjtu::vector<std::string> leavingtimes_string = split_string(current_cmd.args['o'], '|');
        for (int i = 2; i < station_num; i++) {
          leaving_times[i] = change_string_to_int(leavingtimes_string[i - 2]);
        }
      }
      leaving_times[station_num] = 0;

      std::string starttime_string = current_cmd.args['x'];
      starttimes = change_string_to_daytime(starttime_string);
      
      struct saledate sale;
      sjtu::vector<std::string> saledate_string = split_string(current_cmd.args['d'], '|');
      sale.startdate = change_string_to_date(saledate_string[0]);
      sale.enddate = change_string_to_date(saledate_string[1]);
      type = current_cmd.args['y'][0];
      
      bool flag = trainsystem.add_train(trainid, station_num, seat_num, stations, prices, starttimes, travel_times, leaving_times, sale, type);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (flag) std::cout << 0 << '\n';
      else std::cout << -1 << '\n';
    }
    else if (current_cmd.cmd_name == "delete_train") {
      char trainid[22];
      strcpy (trainid, current_cmd.args['i'].c_str());
      bool flag = trainsystem.delete_train(trainid);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (flag) std::cout << 0 << '\n';
      else std::cout << -1 << '\n';
    }
    else if (current_cmd.cmd_name == "release_train") {
      char trainid[22];
      strcpy (trainid, current_cmd.args['i'].c_str());
      bool flag = trainsystem.release_train(trainid);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      if (flag) std::cout << 0 << '\n';
      else std::cout << -1 << '\n';
    }
    else if (current_cmd.cmd_name == "query_train") {
      char trainid[22];
      date date_;
      strcpy (trainid, current_cmd.args['i'].c_str());
      date_ = change_string_to_date(current_cmd.args['d']);
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      bool flag = trainsystem.query_train(date_, trainid);
      if (!flag) std::cout << -1 << '\n';
    }
    else if (current_cmd.cmd_name == "query_order") {
      char user[22];
      strcpy (user,current_cmd.args['u'].c_str()) ;
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      trainsystem.query_order(user, usersystem) ;
    }
    else if (current_cmd.cmd_name == "query_ticket") {
      date date_;
      date_ = change_string_to_date(current_cmd.args['d']);
      char from[52];
      char to[52];
      strcpy (from, current_cmd.args['s'].c_str());
      strcpy (to, current_cmd.args['t'].c_str());
      int flag = 1;
      if (current_cmd.has_arg('p') && current_cmd.args['p'] == "cost") {
        flag = 2;
      }
      
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      bool x = trainsystem.query_ticket(date_, from, to, flag);
      if (!x) std::cout << -1 << '\n';
    }
    else if (current_cmd.cmd_name == "query_transfer") {
      date date_;
      date_ = change_string_to_date(current_cmd.args['d']);
      char from[52];
      char to[52];
      strcpy (from, current_cmd.args['s'].c_str());
      strcpy (to, current_cmd.args['t'].c_str());
      int flag = 1;
      if (current_cmd.has_arg('p') && current_cmd.args['p'] == "cost") {
        flag = 2;
      }
      
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      bool x = trainsystem.query_transfer(date_, from, to, flag);
    }
    else if (current_cmd.cmd_name == "buy_ticket") {
      char username[22];
      char trainid[22];
      date date_;
      int number;
      char from[52];
      char to[52];
      bool q = false; 
      if (current_cmd.has_arg('q') && current_cmd.args['q'] == "true") {
        q = true;
      }
      strcpy (username, current_cmd.args['u'].c_str());
      strcpy (trainid, current_cmd.args['i'].c_str());
      strcpy(from, current_cmd.args['f'].c_str());
      strcpy(to, current_cmd.args['t'].c_str());
      date_ = change_string_to_date(current_cmd.args['d']);
      number = change_string_to_int(current_cmd.args['n']);   
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      trainsystem.buyticket(current_cmd.timestamp, username, trainid, date_, number, from, to, q, usersystem);
    } else if (current_cmd.cmd_name == "refund_ticket") {
      char username[22];
      strcpy (username, current_cmd.args['u'].c_str());
      int n = 1;
      if (current_cmd.has_arg('n')) {
        n = change_string_to_int(current_cmd.args['n']);
      }
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      trainsystem.refund_ticket(n, username, usersystem);
    } else if (current_cmd.cmd_name == "clean") {
       std::cout << '[' << current_cmd.timestamp << ']' << ' ';
       std::cout << 0 << '\n';
       trainsystem.clean(usersystem);
    } else if (current_cmd.cmd_name == "exit") {
      std::cout << '[' << current_cmd.timestamp << ']' << ' ';
      std::cout << "bye" << '\n';
    }
    current_cmd.clear();
  }
};

#endif