#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>

const int MAX_USERS = 100000;
const int MAX_TRAINS = 10000;
const int MAX_ORDERS = 200000;
const int MAX_STATIONS_PER_TRAIN = 100;
const int MAX_STATION_NAME_LEN = 31; // 10 Chinese characters * 3 bytes + 1 for null terminator
const int MAX_USERNAME_LEN = 21;
const int MAX_PASSWORD_LEN = 31;
const int MAX_NAME_LEN = 16; // 5 Chinese characters * 3 bytes + 1
const int MAX_MAIL_LEN = 31;
const int MAX_TRAIN_ID_LEN = 21;

struct User {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char name[MAX_NAME_LEN];
    char mailAddr[MAX_MAIL_LEN];
    int privilege;
    bool isLoggedIn;

    User() : privilege(0), isLoggedIn(false) {
        username[0] = '\0';
        password[0] = '\0';
        name[0] = '\0';
        mailAddr[0] = '\0';
    }
};

struct Station {
    char name[MAX_STATION_NAME_LEN];

    Station() {
        name[0] = '\0';
    }
};

struct Train {
    char trainID[MAX_TRAIN_ID_LEN];
    int stationNum;
    int seatNum;
    Station stations[MAX_STATIONS_PER_TRAIN];
    int prices[MAX_STATIONS_PER_TRAIN - 1]; // prices between stations
    int startTime; // in minutes from 00:00
    int travelTimes[MAX_STATIONS_PER_TRAIN - 1]; // in minutes
    int stopoverTimes[MAX_STATIONS_PER_TRAIN - 2]; // in minutes
    int saleDateStart; // days from June 1 (0 = June 1)
    int saleDateEnd; // days from June 1
    char type;
    bool isReleased;

    Train() : stationNum(0), seatNum(0), startTime(0), saleDateStart(0), saleDateEnd(0), type(' '), isReleased(false) {
        trainID[0] = '\0';
    }
};

struct Order {
    char username[MAX_USERNAME_LEN];
    char trainID[MAX_TRAIN_ID_LEN];
    int date; // days from June 1
    int fromStationIdx;
    int toStationIdx;
    int numTickets;
    int price;
    int status; // 0: success, 1: pending, 2: refunded
    int timestamp; // order timestamp for sorting

    Order() : date(0), fromStationIdx(0), toStationIdx(0), numTickets(0), price(0), status(0), timestamp(0) {
        username[0] = '\0';
        trainID[0] = '\0';
    }
};

// Simple hash table for users
struct UserHashTable {
    User* users[MAX_USERS];
    int userCount;

    UserHashTable() : userCount(0) {
        for (int i = 0; i < MAX_USERS; i++) {
            users[i] = nullptr;
        }
    }

    ~UserHashTable() {
        for (int i = 0; i < MAX_USERS; i++) {
            if (users[i]) {
                delete users[i];
            }
        }
    }
};

// Simple hash table for trains
struct TrainHashTable {
    Train* trains[MAX_TRAINS];
    int trainCount;

    TrainHashTable() : trainCount(0) {
        for (int i = 0; i < MAX_TRAINS; i++) {
            trains[i] = nullptr;
        }
    }

    ~TrainHashTable() {
        for (int i = 0; i < MAX_TRAINS; i++) {
            if (trains[i]) {
                delete trains[i];
            }
        }
    }
};

// Simple array for orders
struct OrderArray {
    Order orders[MAX_ORDERS];
    int orderCount;

    OrderArray() : orderCount(0) {}
};

// Structure for station index
struct StationIndex {
    char stationName[MAX_STATION_NAME_LEN];
    int trainIndex;
    int stationIdx; // Index within the train

    StationIndex() : trainIndex(-1), stationIdx(-1) {
        stationName[0] = '\0';
    }
};

// Array for station indexing
struct StationIndexArray {
    StationIndex indices[MAX_TRAINS * MAX_STATIONS_PER_TRAIN];
    int count;

    StationIndexArray() : count(0) {}
};

// Seat data for a train on a specific date
struct TrainSeats {
    int trainIdx;
    int date; // days from June 1
    int seats[MAX_STATIONS_PER_TRAIN - 1]; // seats available between station i and i+1

    TrainSeats() : trainIdx(-1), date(-1) {
        for (int i = 0; i < MAX_STATIONS_PER_TRAIN - 1; i++) {
            seats[i] = 0;
        }
    }
};

// Array for train seats
struct TrainSeatsArray {
    TrainSeats seatsData[MAX_TRAINS * 100]; // Up to 100 days per train
    int count;

    TrainSeatsArray() : count(0) {}
};

#endif // DATA_STRUCTURES_H