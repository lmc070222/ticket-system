#ifndef TRAIN_HPP
#define TRAIN_HPP

#include <cstring>
#include <ostream>

struct date {
  int month;
  int day;
  static int days_in_month(int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[m - 1];
  }
  date &operator++() {
    int max_day = days_in_month(month);
    if (day < max_day) {
      ++day;
    } else {
      day = 1;
      if (month < 12)
        ++month;
      else
        month = 1;
    }
    return *this;
  }
  date operator++(int) {
    date tmp = *this;
    ++(*this);
    return tmp;
  }
  date operator + (int x) {
    date tmp = *this;
    for (int i = 1;i <= x;i++) ++tmp;
    return tmp;
  }
  date operator - (int x) {
    date tmp = *this;
    for (int i = 1;i <= x;i++) --tmp;
    return tmp;
  }
  date &operator--() {
    if (day > 1) {
      --day;
    } else {
      if (month > 1)
        --month;
      else
        month = 12;
      day = days_in_month(month);
    }
    return *this;
  }
  date operator--(int) {
    date tmp = *this;
    --(*this);
    return tmp;
  }
  bool operator<(const date &other) const {
    return month < other.month || (month == other.month && day < other.day);
  }
  bool operator==(const date &other)const {
    return month == other.month && day == other.day;
  }
  bool operator>(const date &other)const {
    return month > other.month || (month == other.month && day > other.day);
  }
  bool operator>=(const date &other) const { return *this > other || *this == other; }
  bool operator<=(const date &other) const { return *this < other || *this == other; }
  friend std::ostream& operator<<(std::ostream& os, const date& d) {
        if (d.month < 10) os << '0';
        os << d.month << '-';
        if (d.day < 10) os << '0';
        os << d.day;
        return os;
    }
};
struct saledate {
  date startdate;
  date enddate;
};
struct daytime {
    int hour;   
    int minute; 
    int day;    
    daytime(int d = 0, int h = 0, int m = 0) : day(d), hour(h), minute(m) {
        normalize();
    }
    void normalize() {
        int total_minutes = day * 1440 + hour * 60 + minute;
        day = total_minutes / 1440;
        int remainder = total_minutes % 1440;
        if (remainder < 0) {
            remainder += 1440;
            day--; 
        }
        hour = remainder / 60;
        minute = remainder % 60;
    }
    daytime& operator=(const daytime& other) {
        if (this != &other) {
            hour = other.hour;
            minute = other.minute;
            day = other.day;
        }
        return *this;
    }
    daytime addMinutes(int minutes, int& daysCrossed) const {
        int total = (day * 1440 + hour * 60 + minute) + minutes;
        int new_day = total / 1440;
        int remainder = total % 1440;
        if (remainder < 0) {
            remainder += 1440;
            new_day--;
        }
        daysCrossed = new_day - day; 
        return daytime(new_day, remainder / 60, remainder % 60);
    }
    daytime operator+(int minutes) const {
        int dummy;
        return addMinutes(minutes, dummy);
    }
    daytime operator-(int minutes) const {
        int dummy;
        return addMinutes(-minutes, dummy);
    }
    daytime& operator+=(int minutes) {
        int dummy;
        *this = addMinutes(minutes, dummy);
        return *this;
    }
    daytime& operator-=(int minutes) {
        int dummy;
        *this = addMinutes(-minutes, dummy);
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const daytime& dt) {
    if (dt.hour < 10) os << '0';
    os << dt.hour << ':';
    if (dt.minute < 10) os << '0';
    os << dt.minute;
    return os;
}
};
struct Train {
  int station_num;
  int seat_num;
  char stations[102][52];
  int prices[102];
  daytime starttimes;
  int arrival_times[102];
  int leaving_times[102];
  char type;
  bool is_released;
  saledate sale_date;
};
struct SeatKey {
  char trainID[22];
  date startdate;
  bool operator<(const SeatKey &other) const {
    return strncmp(trainID, other.trainID, sizeof(trainID)) < 0 or
           (strncmp(trainID, other.trainID, sizeof(trainID)) == 0 and
            startdate < other.startdate);
  }
  bool operator>(const SeatKey &other) const {
    return strncmp(trainID, other.trainID, sizeof(trainID)) > 0 or
           (strncmp(trainID, other.trainID, sizeof(trainID)) == 0 and
            startdate > other.startdate);
  }
  bool operator==(const SeatKey &other) const {
    return (strncmp(trainID, other.trainID, sizeof(trainID)) == 0 and
            startdate == other.startdate);
  }
  bool operator>=(const SeatKey &other) const  { return *this > other or *this == other; }
  bool operator<=(const SeatKey &other) const { return *this < other or *this == other; }
};
struct SeatInfo {
  int seats[102];
  int waitlist_count; // 该天该车排队候补的人数
};
struct StationKey {
  char station_name[52];
  bool operator<(const StationKey &other) const {
    return strcmp(station_name, other.station_name) < 0;
  }
  bool operator>(const StationKey &other) const {
    return strcmp(station_name, other.station_name) > 0 ;
  }
  bool operator==(const StationKey &other) const {
    return strcmp(station_name, other.station_name) == 0;
  }
  bool operator>=(const StationKey &other) const { return *this > other or *this == other; }
  bool operator<=(const StationKey &other)  const { return *this < other or *this == other; }
};
struct StationInfo {
  char trainID[22];
  int station_rank;
  int price_prefix; // 从始发站到该站的票价前缀和
  int time_prefix;  // 从始发站到该站的时间差
};
struct WaitlistKey {
  char trainID[22];
  date date_;
  bool operator<(const WaitlistKey &other) const {
    return strcmp(trainID, other.trainID) < 0 or
           (strcmp(trainID, other.trainID) == 0 and date_ < other.date_);
  }
  bool operator>(const WaitlistKey &other) const {
    return strcmp(trainID, other.trainID) > 0 or
           (strcmp(trainID, other.trainID) == 0 and date_ > other.date_);
  }
  bool operator==(const WaitlistKey &other) const {
    return (strcmp(trainID, other.trainID) == 0 and date_ == other.date_);
  }
  bool operator>=(const WaitlistKey &other) const {
    return *this > other or *this == other;
  }
  bool operator<=(const WaitlistKey &other) const {
    return *this < other or *this == other;
  }
};
struct WaitlistInfo {
  int timestamp; 
  char username[22];
  int from_rank;
  int to_rank;
  int num;
  int order_timestamp;
};
#endif