#ifndef TRAINMANAGER_HPP
#define TRAINMANAGER_HPP

#include "src/vector.hpp"
#include "train.hpp"
#include "user.hpp"
#include "usermanger.hpp"
#include "utils.hpp"
#include <cstring>
#include <utility>

struct StationToStationKey {
  char from[52];
  char to[52];

  StationToStationKey() {
    std::memset(from, 0, sizeof(from));
    std::memset(to, 0, sizeof(to));
  }
  StationToStationKey(const char *f, const char *t) {
    std::memset(from, 0, sizeof(from));
    std::memset(to, 0, sizeof(to));
    std::strncpy(from, f, 51);
    std::strncpy(to, t, 51);
  }

  
  bool operator<(const StationToStationKey &other) const {
    int cmp = std::strcmp(from, other.from);
    if (cmp != 0) return cmp < 0;
    return std::strcmp(to, other.to) < 0;
  }
  bool operator==(const StationToStationKey &other) const {
    return std::strcmp(from, other.from) == 0 && std::strcmp(to, other.to) == 0;
  }
  bool operator>(const StationToStationKey &other) const { return other < *this; }
  bool operator>=(const StationToStationKey &other) const { return !(*this < other); }
  bool operator<=(const StationToStationKey &other) const { return !(*this > other); }
};

struct StationToStationInfo {
  char trainID[22];
  int from_rank;
  int to_rank;
  int price_delta;          
  int time_delta;           
  int leaving_time_offset;  
  int arrival_time_offset;  
  daytime starttimes;      
  saledate sale_date;      
  int seat_num;            

  
  bool operator==(const StationToStationInfo &other) const {
    return std::strcmp(trainID, other.trainID) == 0 &&
           from_rank == other.from_rank &&
           to_rank == other.to_rank;
  }
};

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

  Bplustree<trainKey, Train, 3> traintree;                            
  Bplustree<SeatKey, SeatInfo, 10> seattree;                          
  Bplustree<StationKey, StationInfo, 45> stationtree;                   
  Bplustree<OrderKey, Order, 22> ordertree;                            
  Bplustree<WaitlistKey, WaitlistInfo, 60> waitlisttree;              
  Bplustree<StationToStationKey, StationToStationInfo, 23> station_to_stationtree;
  trainmanager () : traintree("traintree"), seattree("seattree"), stationtree("stationtree"),
                    ordertree("ordertree"), waitlisttree("waitlisttree"), 
                    station_to_stationtree("station_to_stationtree") {}

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
    strcpy(sk.trainID, trainid);
    SeatInfo si;
    for (int i = 0; i < 102; i++)
      si.seats[i] = target.seat_num;
    si.waitlist_count = 0;
    for (auto i = target.sale_date.startdate; i <= target.sale_date.enddate; i++) {
      sk.startdate = i;
      seattree.Insert(sk, si);
    }

    StationKey stationkey;
    StationInfo stationinfo;
    memset(stationinfo.trainID, 0, 22);
    strcpy(stationinfo.trainID, trainid);
    stationinfo.station_rank = 0;
    stationinfo.price_prefix = 0;
    stationinfo.time_prefix = 0;
    int local_price_prefix[105];
    int cur_price = 0;
    for (int i = 1; i <= target.station_num; i++) {
      cur_price += target.prices[i - 1];
      local_price_prefix[i] = cur_price;
    }

    for (int i = 1; i <= target.station_num; i++) {
      memset(stationkey.station_name, 0, 52);
      strcpy(stationkey.station_name, target.stations[i]);
      stationinfo.station_rank++;
      stationinfo.price_prefix += target.prices[i - 1];
      stationinfo.time_prefix = target.arrival_times[i];
      stationtree.Insert(stationkey, stationinfo);
    }
    for (int i = 1; i < target.station_num; i++) {
      for (int j = i + 1; j <= target.station_num; j++) {
        StationToStationKey sts_key(target.stations[i], target.stations[j]);
        StationToStationInfo sts_info;
        std::strcpy(sts_info.trainID, trainid);
        sts_info.from_rank = i;
        sts_info.to_rank = j;
        sts_info.price_delta = local_price_prefix[j] - local_price_prefix[i];
        sts_info.time_delta = target.arrival_times[j] - target.leaving_times[i];
        sts_info.leaving_time_offset = target.leaving_times[i];
        sts_info.arrival_time_offset = target.arrival_times[j];
        sts_info.starttimes = target.starttimes;
        sts_info.sale_date = target.sale_date;
        sts_info.seat_num = target.seat_num;

        station_to_stationtree.Insert(sts_key, sts_info);
      }
    }
    return true;
  }

  bool query_train(date date_, char *trainid) {
    trainKey x(trainid);
    SeatKey y;
    y.startdate = date_;
    memset(y.trainID, 0, 22);
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

  void quick_sort_stations(sjtu::vector<StationInfo> &arr, int left, int right) {
    if (left >= right) return;
    int i = left;
    int j = right;
    StationInfo pivot = arr[(left + right) / 2];
    while (i <= j) {
      while (std::strcmp(arr[i].trainID, pivot.trainID) < 0) i++;
      while (std::strcmp(arr[j].trainID, pivot.trainID) > 0) j--;
      if (i <= j) {
        StationInfo temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        i++;
        j--;
      }
    }
    if (left < j) quick_sort_stations(arr, left, j);
    if (i < right) quick_sort_stations(arr, i, right);
  }

  void sort_stations(sjtu::vector<StationInfo> &arr) {
    if (arr.size() <= 1) return;
    quick_sort_stations(arr, 0, arr.size() - 1);
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
    StationToStationKey sts_key(from, to);
    sjtu::vector<StationToStationInfo> sts_vec = station_to_stationtree.find_by_index(sts_key);
    sjtu::vector<TicketResult> valid_results;
    
    for (int i = 0; i < sts_vec.size(); i++) {
      const auto &info = sts_vec[i];
      daytime tmp_leaving_offset = info.starttimes + info.leaving_time_offset;
      date dep_date = d - tmp_leaving_offset.day;
      if (info.sale_date.startdate <= dep_date && dep_date <= info.sale_date.enddate) {
        TicketResult res;
        std::strcpy(res.trainID, info.trainID);
        res.time = info.time_delta;
        res.cost = info.price_delta;
        res.from_rank = info.from_rank;
        res.to_rank = info.to_rank;
        res.dep_date = dep_date;
        res.leaving_time = tmp_leaving_offset;
        res.leaving_date = d; 
        
        daytime arr_offset = info.starttimes + info.arrival_time_offset;
        res.arrival_time = arr_offset;
        res.arrival_date = dep_date + arr_offset.day;
        res.seat_num = info.seat_num;
        
        valid_results.push_back(res);
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
    std::memset(x.station_name, 0, 52);
    std::strcpy(x.station_name, from);
    sjtu::vector<StationInfo> sta1 = stationtree.find_by_index(x);

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
            std::memset(ca.trainID, 0, 22);
            std::strcpy(ca.trainID, sta1[i].trainID);
            ca.train = tA;
            ca.rank = sta1[i].station_rank;
            ca.dep_date = dep_date_A;
            valid_A.push_back(ca);
        }
    }

    TransferResult best_plan;

    for (int i = 0; i < valid_A.size(); i++) {
        Train& tA = valid_A[i].train;
        char* trainID_A = valid_A[i].trainID;
        int rankA_from = valid_A[i].rank;
        date dep_date_A = valid_A[i].dep_date;
        
        for (int sA = rankA_from + 1; sA <= tA.station_num; sA++) {
            char* transfer_station = tA.stations[sA];
            if (std::strcmp(transfer_station, to) == 0) continue; 
            StationToStationKey sts_key(transfer_station, to);
            sjtu::vector<StationToStationInfo> sts_B_vec = station_to_stationtree.find_by_index(sts_key);
            
            for (int j = 0; j < sts_B_vec.size(); j++) {
                const auto& infoB = sts_B_vec[j];
                if (std::strcmp(trainID_A, infoB.trainID) == 0) continue; 
                
                daytime tmp_leaving_offset = tA.starttimes + tA.leaving_times[rankA_from];
                daytime arr_offset_A = tA.starttimes + tA.arrival_times[sA];
                date date_arr_A = dep_date_A + arr_offset_A.day;
                
                daytime dep_offset_B = infoB.starttimes + infoB.leaving_time_offset;
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
                
                while (dep_date_B < infoB.sale_date.startdate) {
                    dep_date_B = dep_date_B + 1;
                    day_diff_B_leave_S++;
                }
                
                if (dep_date_B > infoB.sale_date.enddate) {
                    continue; 
                }
                
                int costA = 0;
                for (int k = rankA_from; k < sA; k++) costA += tA.prices[k];
                int costB = infoB.price_delta;
                
                daytime arr_offset_B_to = infoB.starttimes + infoB.arrival_time_offset;
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
                            replace = (std::strcmp(infoB.trainID, best_plan.train2) < 0);
                        }
                    } else if (flag == 2) {
                        if (costA + costB != best_plan.total_cost) {
                            replace = (costA + costB < best_plan.total_cost);
                        } else if (total_time != best_plan.total_time) {
                            replace = (total_time < best_plan.total_time);
                        } else if (std::strcmp(trainID_A, best_plan.train1) != 0) {
                            replace = (std::strcmp(trainID_A, best_plan.train1) < 0);
                        } else {
                            replace = (std::strcmp(infoB.trainID, best_plan.train2) < 0);
                        }
                    }
                }
                
                if (replace) {
                    best_plan.valid = true;
                    std::strcpy(best_plan.train1, trainID_A);
                    std::strcpy(best_plan.train2, infoB.trainID);
                    std::strcpy(best_plan.transfer_station, transfer_station);
                    
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
                    best_plan.rankB_from = infoB.from_rank;
                    best_plan.rankB_to = infoB.to_rank;
                    best_plan.seatNumA = tA.seat_num;
                    best_plan.seatNumB = infoB.seat_num;
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
    std::memset(seatkeyA.trainID, 0, 22);
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
    std::memset(seatkeyB.trainID, 0, 22);
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
    StationToStationKey sts_key(from, to);
    sjtu::vector<StationToStationInfo> sts_vec = station_to_stationtree.find_by_index(sts_key);
    bool found_train = false;
    StationToStationInfo info;
    for (int i = 0; i < sts_vec.size(); i++) {
        if (std::strcmp(sts_vec[i].trainID, trainid) == 0) {
            info = sts_vec[i];
            found_train = true;
            break;
        }
    }
    if (!found_train) {
        std::cout << -1 << '\n';
        return false;
    }

    int p1 = info.from_rank;
    int p2 = info.to_rank;

    if (info.seat_num < number) {
        std::cout << -1 << '\n';
        return false;
    }

    daytime calculate_daytime = info.starttimes + info.leaving_time_offset;
    date dep_date = date_ - calculate_daytime.day;
    if (!(dep_date <= info.sale_date.enddate && dep_date >= info.sale_date.startdate)) {
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
    int min_seat = info.seat_num;
    for (int i = p1; i < p2; i++) {
        min_seat = (min_seat < seatinfo.seats[i]) ? min_seat : seatinfo.seats[i];
    }

    int price = info.price_delta;
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
        
        daytime a_daytime = info.starttimes + info.arrival_time_offset;
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
        
        daytime a_daytime = info.starttimes + info.arrival_time_offset;
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
    sjtu::vector<user> u = usersystem.usertree.find_by_index(UsernameKey(username));
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
    station_to_stationtree.clear();
    std::cout << 0 << '\n';
  }
};

#endif