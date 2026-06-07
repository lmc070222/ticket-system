#ifndef TRAINMANAGER_HPP
#define TRAINMANAGER_HPP
#include "src/vector.hpp"
#include "train.hpp"
#include "user.hpp"
#include "usermanger.hpp"
#include "utils.hpp"
#include <cstring>
#include <utility>
struct trainKey {
  char str[22];
  trainKey() { std::memset(str, 0, sizeof(str)); }
  trainKey(const char *s) {
    std::memset(str, 0, sizeof(str));
    std::strncpy(str, s, 21);
  }
  bool operator<(const trainKey &other) const {
    return std::strcmp(str, other.str) < 0;
  }
  bool operator==(const trainKey &other) const {
    return std::strcmp(str, other.str) == 0;
  }
  bool operator>(const trainKey &other) const {
    return std::strcmp(str, other.str) > 0;
  }
  bool operator>=(const trainKey &other) const {
    return *this == other or *this > other;
  }
  bool operator<=(const trainKey &other) const {
    return *this == other or *this < other;
  }
};

class trainmanager {
  public:
  friend usermanager;
  Bplustree<trainKey, Train, 80> traintree;
  Bplustree<SeatKey, SeatInfo, 80> seattree;
  Bplustree<StationKey, StationInfo, 70> stationtree;
  Bplustree<OrderKey, Order, 40> ordertree;
  Bplustree<WaitlistKey, WaitlistInfo, 50> waitlisttree;
  trainmanager () : traintree("traintree"),seattree("seattree"),stationtree("stationtree") ,ordertree("ordertree"),
  waitlisttree("waitlisttree"){

  }
  bool add_train(char trainid[22], int stationnum, int seatnum,
                 char stations[102][52], int prices[102], daytime starttimes,
                 int traveltimes[102], int stopovertimes[102],
                 saledate sale_date, char type) {
    trainKey x(trainid);
    sjtu::vector<Train> y = traintree.find_by_index(x);
    if (y.size() != 0)
      return false;
    Train z;
    z.station_num = stationnum;
    z.seat_num = seatnum;
    memcpy(z.stations, stations, 102 * 52);
    for (int i = 0; i < 102; i++)
      z.prices[i] = prices[i];
    z.type = type;
    z.sale_date = sale_date;
    z.is_released = false;
    z.arrival_times[1] = 0;
    z.leaving_times[1] = 0;
    z.starttimes = starttimes;
    for (int i = 2; i < stationnum + 1; i++) {
      z.arrival_times[i] = z.leaving_times[i - 1] + traveltimes[i - 1];
      z.leaving_times[i] = z.arrival_times[i] + stopovertimes[i];
    }
    traintree.Insert(x, z);
    return true;
  }
  bool delete_train(char *trainID) {
    trainKey x(trainID);
    sjtu::vector<Train> y = traintree.find_by_index(trainID);
    if (y.size() == 0)
      return false;
    Train target = y.back();
    if (target.is_released == true)
      return false;
    traintree.deletenode(x, target);
    return true;
  }
  bool release_train(char *trainid) {
    trainKey x(trainid);
    sjtu::vector<Train> y = traintree.find_by_index(trainid);
    if (y.size() == 0)
      return false;
    Train target = y.back();
    if (target.is_released == true)
      return false;
    traintree.deletenode(x, target);
    target.is_released = true;
    traintree.Insert(x, target);
    SeatKey sk;
    memset(sk.trainID, 0, 22);
    strcpy (sk.trainID , trainid );
    SeatInfo si;
    for (int i = 0; i < 102; i++)
      si.seats[i] = target.seat_num;
    si.waitlist_count = 0;
    for (auto i = target.sale_date.startdate; i <= target.sale_date.enddate;
         i++) {
      sk.startdate = i;
      seattree.Insert(sk, si);
    }
    StationKey stationkey;
    StationInfo stationinfo;
    memset (stationinfo.trainID,0,22);
    strcpy(stationinfo.trainID, trainid);
    stationinfo.station_rank = 0;
    stationinfo.price_prefix = 0;
    stationinfo.time_prefix = 0;
    for (int i = 1; i <= target.station_num; i++) {
      memset (stationkey.station_name ,0,52);
      strcpy(stationkey.station_name, target.stations[i]);
      stationinfo.station_rank++;
      stationinfo.price_prefix += target.prices[i - 1];
      stationinfo.time_prefix = target.arrival_times[i];
      stationtree.Insert(stationkey, stationinfo);
    }
    return true;
  }
  bool query_train(date date_, char *trainid) {
    trainKey x(trainid);
    SeatKey y;
    y.startdate = date_;
    memset (y.trainID,0,22);
    strcpy(y.trainID, trainid);
    sjtu::vector<Train> trai = traintree.find_by_index(x);
    if (trai.size() == 0)
      return false;
    Train tr = trai.back();
    if (!(tr.sale_date.startdate <= date_ and tr.sale_date.enddate >= date_))
      return false;
    if (tr.is_released == false) {
      std::cout << trainid << ' ' << tr.type << '\n';
      std::cout << tr.stations[1] << ' ' << "xx-xx xx:xx" << " -> " << date_
                << ' ' << tr.starttimes << ' ' << 0 << ' ' << tr.seat_num
                << '\n';
      int totalprice = 0;
      for (int i = 2; i < tr.station_num; i++) {
        totalprice += tr.prices[i - 1];
        daytime tmp_arrival = tr.starttimes + tr.arrival_times[i];
        daytime tmp_leaving = tr.starttimes + tr.leaving_times[i];
        struct date d_arrival = date_ + tmp_arrival.day;
        struct date d_leaving = date_ + tmp_leaving.day;
        std::cout << tr.stations[i] << ' ' << d_arrival << ' ' << tmp_arrival
                  << " -> " << d_leaving << ' ' << tmp_leaving << ' '
                  << totalprice << ' ' << tr.seat_num << '\n';
      }
      totalprice += tr.prices[tr.station_num - 1];
      daytime tmp_arrival = tr.starttimes + tr.arrival_times[tr.station_num];
      struct date d_arrival = date_ + tmp_arrival.day;
      std::cout << tr.stations[tr.station_num] << ' ' << d_arrival << ' '
                << tmp_arrival << " -> " << "xx-xx xx:xx " << totalprice << ' '
                << 'x' << '\n';
      return true;
    }
    sjtu::vector<SeatInfo> sea = seattree.find_by_index(y);
    if (sea.size() == 0)
      return false;
    SeatInfo seat = sea.back();
    std::cout << trainid << ' ' << tr.type << '\n';
    std::cout << tr.stations[1] << ' ' << "xx-xx xx:xx" << " -> " << date_ << ' '
              << tr.starttimes << ' ' << 0 << ' ' << seat.seats[1] << '\n';
    int totalprice = 0;
    for (int i = 2; i < tr.station_num; i++) {
      totalprice += tr.prices[i - 1];
      daytime tmp_arrival = tr.starttimes + tr.arrival_times[i];
      daytime tmp_leaving = tr.starttimes + tr.leaving_times[i];
      struct date d_arrival = date_ + tmp_arrival.day;
      struct date d_leaving = date_ + tmp_leaving.day;
      std::cout << tr.stations[i] << ' ' << d_arrival << ' ' << tmp_arrival
                << " -> " << d_leaving << ' ' << tmp_leaving << ' '
                << totalprice << ' ' << seat.seats[i] << '\n';
    }
    totalprice += tr.prices[tr.station_num - 1];
    daytime tmp_arrival = tr.starttimes + tr.arrival_times[tr.station_num];
    struct date d_arrival = date_ + tmp_arrival.day;
    std::cout << tr.stations[tr.station_num] << ' ' << d_arrival << ' '
              << tmp_arrival << " -> " << "xx-xx xx:xx " << totalprice << ' '
              << 'x' << '\n';
    return true;
  }
  void quick_sort_stations(sjtu::vector<StationInfo> &arr, int left,
                           int right) {
    if (left >= right)
      return;
    int i = left;
    int j = right;
    StationInfo pivot = arr[(left + right) / 2];
    while (i <= j) {
      while (std::strcmp(arr[i].trainID, pivot.trainID) < 0) {
        i++;
      }
      while (std::strcmp(arr[j].trainID, pivot.trainID) > 0) {
        j--;
      }
      if (i <= j) {
        StationInfo temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        i++;
        j--;
      }
    }
    if (left < j)
      quick_sort_stations(arr, left, j);
    if (i < right)
      quick_sort_stations(arr, i, right);
  }
  void sort_stations(sjtu::vector<StationInfo> &arr) {
    if (arr.size() <= 1)
      return;
    quick_sort_stations(arr, 0, arr.size() - 1);
  }
  static bool comparetime(std::pair<StationInfo, StationInfo> x,
                          std::pair<StationInfo, StationInfo> y) {
    int time_x = x.second.time_prefix - x.first.time_prefix;
    int time_y = y.second.time_prefix - y.first.time_prefix;
    bool rule_other = memcmp(x.first.trainID, y.first.trainID, 22) < 0;
    return time_x < time_y or (time_x == time_y and rule_other);
  }
  static bool comparecost(std::pair<StationInfo, StationInfo> x,
                          std::pair<StationInfo, StationInfo> y) {
    int cost_x = x.second.price_prefix - x.first.price_prefix;
    int cost_y = y.second.price_prefix - y.first.price_prefix;
    bool rule_other = memcmp(x.first.trainID, y.first.trainID, 22) < 0;
    return cost_x < cost_y or (cost_x == cost_y and rule_other);
  }
  struct TicketResult {
    char trainID[22];
    int time;            
    int cost;            
    int from_rank;
    int to_rank;
    date dep_date;       
    daytime leaving_time;
    date leaving_date;  
    daytime arrival_time;
    date arrival_date;   
    int seat_num; 
  };

  bool query_ticket(date d, char *from, char *to, int flag) {
    StationKey x;
    std::memset(&x, 0, sizeof(StationKey));
    std::strcpy(x.station_name, from);
    sjtu::vector<StationInfo> sta1 = stationtree.find_by_index(x);
    
    StationKey y;
    std::memset(&y, 0, sizeof(StationKey));
    std::strcpy(y.station_name, to);
    sjtu::vector<StationInfo> sta2 = stationtree.find_by_index(y);
    
    sort_stations(sta1);
    sort_stations(sta2);
    
    sjtu::vector<TicketResult> valid_results;
    int p1 = 0, p2 = 0;
    
    while (p1 < sta1.size() && p2 < sta2.size()) {
      int cmp = std::strcmp(sta1[p1].trainID, sta2[p2].trainID);
      if (cmp < 0) {
        p1++;
      } else if (cmp > 0) {
        p2++;
      } else {
        if (sta1[p1].station_rank < sta2[p2].station_rank) {
          trainKey key(sta1[p1].trainID);
          sjtu::vector<Train> tv = traintree.find_by_index(key);
          if (!tv.empty()) {
            Train train = tv.back();
            if (train.is_released) {
              daytime tmp_leaving_offset = train.starttimes + train.leaving_times[sta1[p1].station_rank];
              date dep_date = d - tmp_leaving_offset.day;
              
              if (train.sale_date.startdate <= dep_date && dep_date <= train.sale_date.enddate) {
                TicketResult res;
                std::memset(&res, 0, sizeof(TicketResult)); 
                std::strcpy(res.trainID, sta1[p1].trainID);
                res.time = train.arrival_times[sta2[p2].station_rank] - train.leaving_times[sta1[p1].station_rank];
                res.cost = sta2[p2].price_prefix - sta1[p1].price_prefix;
                
                res.from_rank = sta1[p1].station_rank;
                res.to_rank = sta2[p2].station_rank;
                res.dep_date = dep_date;
                
                res.leaving_time = tmp_leaving_offset;
                res.leaving_date = d; 
                
                daytime arr_offset = train.starttimes + train.arrival_times[sta2[p2].station_rank];
                res.arrival_time = arr_offset;
                res.arrival_date = dep_date + arr_offset.day;
                res.seat_num = train.seat_num;
                
                valid_results.push_back(res);
              }
            }
          }
        }
        p1++;
        p2++;
      }
    }
    
    if (valid_results.size() == 0) {
      std::cout << 0 << '\n';
      return true;
    }
    if (flag == 1) {
        my_sort(valid_results.begin(), valid_results.end(), [](const TicketResult &a, const TicketResult &b) {
        if (a.time != b.time) return a.time < b.time;
        return std::strcmp(a.trainID, b.trainID) < 0;
      });
    } else if (flag == 2) {
      my_sort(valid_results.begin(), valid_results.end(), [](const TicketResult &a, const TicketResult &b) {
        if (a.cost != b.cost) return a.cost < b.cost;
        return std::strcmp(a.trainID, b.trainID) < 0;
      });
    }
    
    std::cout << valid_results.size() << '\n';
    
    for (int i = 0; i < valid_results.size(); i++) {
      SeatKey seatkey;
      std::memset(&seatkey, 0, sizeof(SeatKey));
      seatkey.startdate = valid_results[i].dep_date;
      std::strcpy(seatkey.trainID, valid_results[i].trainID);
      
      sjtu::vector<SeatInfo> sea = seattree.find_by_index(seatkey);
      int remain_num = valid_results[i].seat_num;
      
      if (!sea.empty()) {
        SeatInfo seatinfo = sea.back();
        for (int j = valid_results[i].from_rank; j < valid_results[i].to_rank; j++) {
          if (seatinfo.seats[j] < remain_num) {
            remain_num = seatinfo.seats[j];
          }
        }
      }
      std::cout << valid_results[i].trainID << ' ' << from << ' '
                << valid_results[i].leaving_date << ' ' << valid_results[i].leaving_time << " -> "
                << to << ' ' << valid_results[i].arrival_date << ' ' << valid_results[i].arrival_time
                << ' ' << valid_results[i].cost << ' ' << remain_num << '\n';
    }
    
    return true;
  }
  struct TransferResult {
    char train1[22];
    char train2[22];
    char transfer_station[52];
    
    date date_leaving_A;
    daytime time_leaving_A;
    date date_arrival_A;
    daytime time_arrival_A;
    
    date date_leaving_B;
    daytime time_leaving_B;
    date date_arrival_B;
    daytime time_arrival_B;
    
    int priceA = 0;
    int priceB = 0;
    int total_time = 2147483647;
    int total_cost = 2147483647;
    
    bool valid = false;
    
    date dep_date_A_origin;
    date dep_date_B_origin;
    int rankA_from = 0;
    int rankA_to = 0;
    int rankB_from = 0;
    int rankB_to = 0;
    int seatNumA = 0;
    int seatNumB = 0;
};

bool query_transfer(date d, char* from, char* to, int flag) {
    StationKey x;
    memset (x.station_name,0,52);
    std::strcpy(x.station_name, from);
    sjtu::vector<StationInfo> sta1 = stationtree.find_by_index(x);
    
    StationKey y;
    memset (y.station_name,0,52);
    std::strcpy(y.station_name, to);
    sjtu::vector<StationInfo> sta2 = stationtree.find_by_index(y);

    struct CandA {
        char trainID[22];
        Train train;
        int rank;
        date dep_date;
    };
    sjtu::vector<CandA> valid_A;
    
    for (int i = 0; i < sta1.size(); i++) {
        Train tA = traintree.find_by_index(trainKey(sta1[i].trainID)).back();
        if (!tA.is_released) continue;
        
        daytime tmp_leaving_offset = tA.starttimes + tA.leaving_times[sta1[i].station_rank];
        date dep_date_A = d - tmp_leaving_offset.day;
        
        if (dep_date_A >= tA.sale_date.startdate && dep_date_A <= tA.sale_date.enddate) {
            CandA ca;
            memset (ca.trainID,0,22);
            std::strcpy(ca.trainID, sta1[i].trainID);
            ca.train = tA;
            ca.rank = sta1[i].station_rank;
            ca.dep_date = dep_date_A;
            valid_A.push_back(ca);
        }
    }

    struct CandB {
        char trainID[22];
        Train train;
        int rank;
    };
    sjtu::vector<CandB> valid_B;
    
    for (int i = 0; i < sta2.size(); i++) {
        Train tB = traintree.find_by_index(trainKey(sta2[i].trainID)).back();
        if (tB.is_released) {
            CandB cb;
            memset (cb.trainID,0,22);
            std::strcpy(cb.trainID, sta2[i].trainID);
            cb.train = tB;
            cb.rank = sta2[i].station_rank;
            valid_B.push_back(cb);
        }
    }

    TransferResult best_plan;

    for (int i = 0; i < valid_A.size(); i++) {
        Train& tA = valid_A[i].train;
        char* trainID_A = valid_A[i].trainID;
        int rankA_from = valid_A[i].rank;
        date dep_date_A = valid_A[i].dep_date;
        
        for (int j = 0; j < valid_B.size(); j++) {
            Train& tB = valid_B[j].train;
            char* trainID_B = valid_B[j].trainID;
            int rankB_to = valid_B[j].rank;
            
            if (std::strcmp(trainID_A, trainID_B) == 0) continue;
            
            for (int sA = rankA_from + 1; sA <= tA.station_num; sA++) {
                bool found_transfer = false;
                int sB = 1;
                
                for (; sB < rankB_to; sB++) {
                    if (std::strcmp(tA.stations[sA], tB.stations[sB]) == 0) {
                        found_transfer = true;
                        break;
                    }
                }
                
                if (found_transfer) {
                    daytime tmp_leaving_offset = tA.starttimes + tA.leaving_times[rankA_from];
                    daytime arr_offset_A = tA.starttimes + tA.arrival_times[sA];
                    date date_arr_A = dep_date_A + arr_offset_A.day;
                    
                    daytime dep_offset_B = tB.starttimes + tB.leaving_times[sB];
                    date dep_date_B = date_arr_A - dep_offset_B.day;
                    
                    int day_offset_arr_A = arr_offset_A.day - tmp_leaving_offset.day;
                    int day_diff_B_leave_S = day_offset_arr_A;
                    
                    bool missed_today = false; 
                    if (dep_offset_B.hour * 60 + dep_offset_B.minute < arr_offset_A.hour * 60 + arr_offset_A.minute) {
                        missed_today = true;
                    }
                    if (missed_today) {
                        dep_date_B = dep_date_B + 1;
                        day_diff_B_leave_S++;
                    }
                    
                    while (dep_date_B < tB.sale_date.startdate) {
                        dep_date_B = dep_date_B + 1;
                        day_diff_B_leave_S++;
                    }
                    
                    if (dep_date_B > tB.sale_date.enddate) {
                        continue; 
                    }
                    
                    int costA = 0;
                    for (int k = rankA_from; k < sA; k++) costA += tA.prices[k];
                    int costB = 0;
                    for (int k = sB; k < rankB_to; k++) costB += tB.prices[k];
                    
                    daytime arr_offset_B_to = tB.starttimes + tB.arrival_times[rankB_to];
                    date date_arr_B = dep_date_B + arr_offset_B_to.day;
                    
                    int day_offset_arr_B = day_diff_B_leave_S + (arr_offset_B_to.day - dep_offset_B.day);
                    
                    int abs_arr_B_to = day_offset_arr_B * 1440 + arr_offset_B_to.hour * 60 + arr_offset_B_to.minute;
                    int abs_dep_A_from = tmp_leaving_offset.hour * 60 + tmp_leaving_offset.minute;
                    
                    int total_time = abs_arr_B_to - abs_dep_A_from;
                    
                    bool replace = false;
                    if (!best_plan.valid) {
                        replace = true;
                    } else {
                        if (flag == 1) {
                            if (total_time != best_plan.total_time) {
                                replace = (total_time < best_plan.total_time);
                            } else if (costA + costB != best_plan.total_cost) {
                                replace = (costA + costB < best_plan.total_cost);
                            } else if (std::strcmp(trainID_A, best_plan.train1) != 0) {
                                replace = (std::strcmp(trainID_A, best_plan.train1) < 0);
                            } else {
                                replace = (std::strcmp(trainID_B, best_plan.train2) < 0);
                            }
                        } else if (flag == 2) {
                            if (costA + costB != best_plan.total_cost) {
                                replace = (costA + costB < best_plan.total_cost);
                            } else if (total_time != best_plan.total_time) {
                                replace = (total_time < best_plan.total_time);
                            } else if (std::strcmp(trainID_A, best_plan.train1) != 0) {
                                replace = (std::strcmp(trainID_A, best_plan.train1) < 0);
                            } else {
                                replace = (std::strcmp(trainID_B, best_plan.train2) < 0);
                            }
                        }
                    }
                    
                    if (replace) {
                        best_plan.valid = true;
                        std::strcpy(best_plan.train1, trainID_A);
                        std::strcpy(best_plan.train2, trainID_B);
                        std::strcpy(best_plan.transfer_station, tA.stations[sA]);
                        
                        best_plan.date_leaving_A = d;
                        best_plan.time_leaving_A = tmp_leaving_offset;
                        best_plan.date_arrival_A = date_arr_A;
                        best_plan.time_arrival_A = arr_offset_A;
                        
                        best_plan.date_leaving_B = dep_date_B + dep_offset_B.day;
                        best_plan.time_leaving_B = dep_offset_B;
                        best_plan.date_arrival_B = date_arr_B;
                        best_plan.time_arrival_B = arr_offset_B_to;
                        
                        best_plan.priceA = costA;
                        best_plan.priceB = costB;
                        best_plan.total_time = total_time;
                        best_plan.total_cost = costA + costB;
                        
                        best_plan.dep_date_A_origin = dep_date_A;
                        best_plan.dep_date_B_origin = dep_date_B;
                        best_plan.rankA_from = rankA_from;
                        best_plan.rankA_to = sA;
                        best_plan.rankB_from = sB;
                        best_plan.rankB_to = rankB_to;
                        best_plan.seatNumA = tA.seat_num;
                        best_plan.seatNumB = tB.seat_num;
                    }
                }
            }
        }
    }

    if (!best_plan.valid) {
        std::cout << 0 << '\n';
        return true;
    }

    int remain_A = best_plan.seatNumA;
    int remain_B = best_plan.seatNumB;
    
    SeatKey seatkeyA;
    memset (seatkeyA.trainID,0,22);
    std::strcpy(seatkeyA.trainID, best_plan.train1);
    seatkeyA.startdate = best_plan.dep_date_A_origin;
    sjtu::vector<SeatInfo> seaA = seattree.find_by_index(seatkeyA);
    if (seaA.size() > 0) {
        for (int k = best_plan.rankA_from; k < best_plan.rankA_to; k++) {
            if (seaA.back().seats[k] < remain_A) {
                remain_A = seaA.back().seats[k];
            }
        }
    }

    SeatKey seatkeyB;
    memset (seatkeyB.trainID,0,22);
    std::strcpy(seatkeyB.trainID, best_plan.train2);
    seatkeyB.startdate = best_plan.dep_date_B_origin;
    sjtu::vector<SeatInfo> seaB = seattree.find_by_index(seatkeyB);
    if (seaB.size() > 0) {
        for (int k = best_plan.rankB_from; k < best_plan.rankB_to; k++) {
            if (seaB.back().seats[k] < remain_B) {
                remain_B = seaB.back().seats[k];
            }
        }
    }
    
    std::cout << best_plan.train1 << ' ' << from << ' ' << best_plan.date_leaving_A << ' ' << best_plan.time_leaving_A << " -> " 
              << best_plan.transfer_station << ' ' << best_plan.date_arrival_A << ' ' << best_plan.time_arrival_A << ' ' << best_plan.priceA << ' ' << remain_A << '\n';
    std::cout << best_plan.train2 << ' ' << best_plan.transfer_station << ' ' << best_plan.date_leaving_B << ' ' << best_plan.time_leaving_B << " -> " 
              << to << ' ' << best_plan.date_arrival_B << ' ' << best_plan.time_arrival_B << ' ' << best_plan.priceB << ' ' << remain_B << '\n';

    return true;
}

 bool buyticket(int timestamp, char *username, char *trainid, date date_,
               int number, char *from, char *to, bool flag,
               usermanager &usersystem) {
    auto it = usersystem.logstack.find(UsernameKey(username));
    sjtu::vector<user> u = usersystem.usertree.find_by_index(UsernameKey(username));
    if (u.size() == 0 || it == usersystem.logstack.end()) {
        std::cout << -1 << '\n';
        return false;
    }
    trainKey trainidkey = trainKey(trainid);
    sjtu::vector<Train> train_v = traintree.find_by_index(trainidkey);
    if (train_v.size() == 0) {
        std::cout << -1 << '\n';
        return false;
    }
    Train train = train_v.back();
    if (train.is_released == false) {
        std::cout << -1 << '\n';
        return false;
    }
    int p1 = 0, p2 = 0;
    for (int i = 1; i <= train.station_num; i++) {
        if (std::string(train.stations[i]) == std::string(from)) 
            p1 = i;
        if (std::string(train.stations[i]) == std::string(to)) 
            p2 = i;
    }
    if (p1 == 0 || p2 == 0 || p1 >= p2) {
        std::cout << -1 << '\n';
        return false;
    }
    if (train.seat_num < number) {
        std::cout << -1 << '\n';
        return false;
    }
    daytime daytime_ = train.starttimes;
    daytime calculate_daytime = train.starttimes + train.leaving_times[p1];
    date dep_date = date_ - calculate_daytime.day;
    if (!(dep_date <= train.sale_date.enddate && dep_date >= train.sale_date.startdate)) {
        std::cout << -1 << '\n';
        return false;
    }
    SeatKey seatkey;
    std::memset(&seatkey, 0, sizeof(SeatKey));
    seatkey.startdate = dep_date;
    std::strcpy(seatkey.trainID, trainid);

    sjtu::vector<SeatInfo> sea = seattree.find_by_index(seatkey);
    if (sea.size() == 0) {
        std::cout << -1 << '\n';
        return false;
    }
    SeatInfo seatinfo = sea.back();
    int min_seat = train.seat_num;
    for (int i = p1; i < p2; i++) {
        min_seat = (min_seat < seatinfo.seats[i]) ? min_seat : seatinfo.seats[i];
    }
    int price = 0;
    for (int i = p1; i < p2; i++) {
        price += train.prices[i];
    }
    long long totalcost = 1ll * price * number;
    if (min_seat >= number) {
        seattree.deletenode(seatkey, seatinfo);
        for (int i = p1; i < p2; i++) {
            seatinfo.seats[i] -= number;
        }
        seattree.Insert(seatkey, seatinfo);

        OrderKey orderkey;
        std::memset(&orderkey, 0, sizeof(OrderKey));
        std::strcpy(orderkey.username, username);
        Order order;
        std::memset(&order, 0, sizeof(Order));
        order.timestamp = timestamp;
        order.leaving_date = date_;
        order.leaving_daytime = calculate_daytime;
        daytime a_daytime = train.starttimes + train.arrival_times[p2];
        date a_day = dep_date + a_daytime.day;
        order.arrival_date = a_day;
        order.arrival_dattime = a_daytime;
        order.num = number;
        order.price = price;
        order.status = 1;
        std::strcpy(order.from, from);
        std::strcpy(order.to, to);
        std::strcpy(order.trainID, trainid);

        ordertree.Insert(orderkey, order);
        std::cout << totalcost << '\n';
        return true;
    } 
    else if (flag == false) {
        std::cout << -1 << '\n';
        return false;
    } 
    else {
        OrderKey orderkey;
        std::memset(&orderkey, 0, sizeof(OrderKey));
        std::strcpy(orderkey.username, username);
        Order order;
        std::memset(&order, 0, sizeof(Order));
        order.leaving_date = date_;
        order.leaving_daytime = calculate_daytime;
        daytime a_daytime = train.starttimes + train.arrival_times[p2];
        date a_day = dep_date + a_daytime.day;
        order.arrival_date = a_day;
        order.arrival_dattime = a_daytime;
        order.num = number;
        order.price = price;
        order.status = 0; 
        order.timestamp = timestamp;
        std::strcpy(order.from, from);
        std::strcpy(order.to, to);
        std::strcpy(order.trainID, trainid);
        seattree.deletenode(seatkey, seatinfo);
        seatinfo.waitlist_count++;
        seattree.Insert(seatkey, seatinfo);
        ordertree.Insert(orderkey, order);
        WaitlistKey waitlistkey;
        std::memset(&waitlistkey, 0, sizeof(WaitlistKey));
        waitlistkey.date_ = dep_date;
        std::strcpy(waitlistkey.trainID, trainid);
        WaitlistInfo waitlistinfo;
        std::memset(&waitlistinfo, 0, sizeof(WaitlistInfo));
        waitlistinfo.from_rank = p1;
        waitlistinfo.to_rank = p2;
        waitlistinfo.num = number;
        waitlistinfo.order_timestamp = timestamp;
        std::strcpy(waitlistinfo.username, username);
        waitlisttree.Insert(waitlistkey, waitlistinfo);
        std::cout << "queue" << '\n';
        return true;
    }
}
  bool query_order(char *username, usermanager &usersystem) {
    auto it = usersystem.logstack.find(UsernameKey(username));
    sjtu::vector<user> u =
        usersystem.usertree.find_by_index(UsernameKey(username));
    if (u.size() == 0 or it == usersystem.logstack.end()) {
      std::cout << -1 << '\n';
      return false;
    }
    OrderKey searchkey;
std::memset(&searchkey, 0, sizeof(OrderKey));
std::strcpy(searchkey.username, username);
    sjtu::vector<Order> orders = ordertree.find_by_index(searchkey);
    std::cout << orders.size() << '\n';
    if (orders.size() == 0) {
      return true;
    }
    my_sort(orders.begin(), orders.end(), [](const Order &a, const Order &b) {
      return a.timestamp > b.timestamp;
    });

    for (int i = 0; i < orders.size(); i++) {
      const char *status_str = "";
      if (orders[i].status == 1) {
        status_str = "[success]";
      } else if (orders[i].status == 0) {
        status_str = "[pending]";
      } else if (orders[i].status == 2) {
        status_str = "[refunded]";
      }
      std::cout << status_str << ' ' << orders[i].trainID << ' '
                << orders[i].from << ' ' << orders[i].leaving_date << ' '
                << orders[i].leaving_daytime << " -> " << orders[i].to << ' '
                << orders[i].arrival_date << ' ' << orders[i].arrival_dattime
                << ' ' << orders[i].price << ' ' << orders[i].num << '\n';
    }

    return true;
  }
  static bool comparewaitlistinfo(WaitlistInfo x, WaitlistInfo y) {
    return x.order_timestamp < y.order_timestamp;
  }
  bool refund_ticket(int n, char *username, usermanager &usersystem) {
    auto it = usersystem.logstack.find(UsernameKey(username));
    sjtu::vector<user> u = usersystem.usertree.find_by_index(UsernameKey(username));
    if (u.size() == 0 || it == usersystem.logstack.end()) {
        std::cout << -1 << '\n';
        return false;
    }

    OrderKey orderkey;
    std::memset(&orderkey, 0, sizeof(OrderKey));
    std::strcpy(orderkey.username, username);
    sjtu::vector<Order> order = ordertree.find_by_index(orderkey);
    if (order.size() == 0) {
        std::cout << -1 << '\n';
        return false;
    }
    
    my_sort(order.begin(), order.end(), [](const Order &a, const Order &b) {
        return a.timestamp > b.timestamp;
    });

    if (n <= 0 || n > order.size()) {
        std::cout << -1 << '\n';
        return false;
    }

    Order target_order = order[n - 1];
    if (target_order.status == 2) { 
        std::cout << -1 << '\n';
        return false;
    }
    OrderKey actual_orderkey;
    std::memset(&actual_orderkey, 0, sizeof(OrderKey));
    std::strcpy(actual_orderkey.username, username);
    if (target_order.status == 0) {
        date dep_date = target_order.leaving_date - target_order.leaving_daytime.day;
        ordertree.deletenode(actual_orderkey, target_order);
        target_order.status = 2;
        ordertree.Insert(actual_orderkey, target_order);

        Train train = traintree.find_by_index(trainKey(target_order.trainID)).back();
        int p1 = 0, p2 = 0;
        for (int i = 1; i <= train.station_num; i++) {
            if (std::string(train.stations[i]) == std::string(target_order.from))
                p1 = i;
            if (std::string(train.stations[i]) == std::string(target_order.to))
                p2 = i;
        }
        WaitlistKey waitlistkey;
        std::memset(&waitlistkey, 0, sizeof(WaitlistKey));
        waitlistkey.date_ = dep_date;
        std::strcpy(waitlistkey.trainID, target_order.trainID);
        WaitlistInfo waitlistinfo;
        std::memset(&waitlistinfo, 0, sizeof(WaitlistInfo));
        waitlistinfo.from_rank = p1;
        waitlistinfo.to_rank = p2;
        waitlistinfo.num = target_order.num;
        waitlistinfo.order_timestamp = target_order.timestamp;
        std::strcpy(waitlistinfo.username, username);
        
        waitlisttree.deletenode(waitlistkey, waitlistinfo);

        SeatKey seatkey;
        std::memset(&seatkey, 0, sizeof(SeatKey));
        seatkey.startdate = dep_date;
        std::strcpy(seatkey.trainID, target_order.trainID);
        sjtu::vector<SeatInfo> sea = seattree.find_by_index(seatkey);
        if (sea.size() > 0) {
            SeatInfo seatinfo = sea.back();
            seattree.deletenode(seatkey, seatinfo);
            seatinfo.waitlist_count--;
            seattree.Insert(seatkey, seatinfo);
        }

        std::cout << 0 << '\n';
        return true;
    } 
    else if (target_order.status == 1) {
        date dep_date = target_order.leaving_date - target_order.leaving_daytime.day;
        ordertree.deletenode(actual_orderkey, target_order);
        target_order.status = 2; 
        ordertree.Insert(actual_orderkey, target_order);
        Train train = traintree.find_by_index(trainKey(target_order.trainID)).back();
        int p1 = 0, p2 = 0;
        for (int i = 1; i <= train.station_num; i++) {
            if (std::string(train.stations[i]) == std::string(target_order.from))
                p1 = i;
            if (std::string(train.stations[i]) == std::string(target_order.to))
                p2 = i;
        }
        SeatKey seatkey;
        std::memset(&seatkey, 0, sizeof(SeatKey));
        seatkey.startdate = dep_date;
        std::strcpy(seatkey.trainID, target_order.trainID);
        
        sjtu::vector<SeatInfo> sea = seattree.find_by_index(seatkey);
        if (sea.size() == 0) {
            std::cout << -1 << '\n';
            return false;
        }
        SeatInfo seatinfo = sea.back();
        seattree.deletenode(seatkey, seatinfo);
        for (int i = p1; i < p2; i++) {
            seatinfo.seats[i] += target_order.num;
        }
        WaitlistKey key;
        std::memset(&key, 0, sizeof(WaitlistKey));
        std::strcpy(key.trainID, target_order.trainID);
        key.date_ = dep_date;
        
        auto waitlist_vec = waitlisttree.find_by_index(key);
        my_sort(waitlist_vec.begin(), waitlist_vec.end(), comparewaitlistinfo);
        for (int i = 0; i < waitlist_vec.size(); i++) {
            auto &wl_key = key;
            auto &wl_info = waitlist_vec[i];
            int cur_min_seat = train.seat_num;
            for (int j = wl_info.from_rank; j < wl_info.to_rank; j++) {
                if (seatinfo.seats[j] < cur_min_seat) {
                    cur_min_seat = seatinfo.seats[j];
                }
            }
            if (cur_min_seat >= wl_info.num) {
                for (int j = wl_info.from_rank; j < wl_info.to_rank; j++) {
                    seatinfo.seats[j] -= wl_info.num;
                }
                
                OrderKey wl_orderkey;
                std::memset(&wl_orderkey, 0, sizeof(OrderKey));
                std::strcpy(wl_orderkey.username, wl_info.username);
                sjtu::vector<Order> wl_orders = ordertree.find_by_index(wl_orderkey);
                
                for (int k = 0; k < wl_orders.size(); k++) {
                    if (wl_orders[k].timestamp == wl_info.order_timestamp && wl_orders[k].status == 0) {
                        Order wl_order = wl_orders[k];
                        ordertree.deletenode(wl_orderkey, wl_order);
                        wl_order.status = 1;
                        ordertree.Insert(wl_orderkey, wl_order);
                        break;
                    }
                }
                waitlisttree.deletenode(wl_key, wl_info);
                seatinfo.waitlist_count--;
            }
        }
        seattree.Insert(seatkey, seatinfo);
        std::cout << 0 << '\n';
        return true;
    }

    return false;
}
  void clean (usermanager & x) {
    x.logstack.clear();
    x.usertree.clear();
    traintree.clear();
    ordertree.clear();
    seattree.clear();
    stationtree.clear();
    waitlisttree.clear();
    std::cout << 0 << '\n';
  }
};

#endif