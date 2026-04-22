#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "data_structures.h"

using namespace std;

// Simple bubble sort implementation (since we can't use algorithm library)
template<typename T>
void bubbleSort(T* arr, int n, bool (*compare)(const T&, const T&)) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (!compare(arr[j], arr[j + 1])) {
                T temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Global data structures
UserHashTable userTable;
TrainHashTable trainTable;
OrderArray orderArray;
StationIndexArray stationIndex;
TrainSeatsArray trainSeatsArray;
int currentTimestamp = 0;
bool firstUserCreated = false;

// Helper functions
unsigned int hashString(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

User* findUser(const char* username) {
    unsigned int hash = hashString(username) % MAX_USERS;
    // Linear probing
    for (int i = 0; i < MAX_USERS; i++) {
        int idx = (hash + i) % MAX_USERS;
        if (userTable.users[idx] == nullptr) {
            return nullptr;
        }
        if (strcmp(userTable.users[idx]->username, username) == 0) {
            return userTable.users[idx];
        }
    }
    return nullptr;
}

bool addUser(User* user) {
    if (userTable.userCount >= MAX_USERS) {
        return false;
    }

    unsigned int hash = hashString(user->username) % MAX_USERS;
    // Linear probing
    for (int i = 0; i < MAX_USERS; i++) {
        int idx = (hash + i) % MAX_USERS;
        if (userTable.users[idx] == nullptr) {
            userTable.users[idx] = user;
            userTable.userCount++;
            return true;
        }
        if (strcmp(userTable.users[idx]->username, user->username) == 0) {
            return false; // Username already exists
        }
    }
    return false;
}

Train* findTrain(const char* trainID) {
    unsigned int hash = hashString(trainID) % MAX_TRAINS;
    // Linear probing
    for (int i = 0; i < MAX_TRAINS; i++) {
        int idx = (hash + i) % MAX_TRAINS;
        if (trainTable.trains[idx] == nullptr) {
            return nullptr;
        }
        if (strcmp(trainTable.trains[idx]->trainID, trainID) == 0) {
            return trainTable.trains[idx];
        }
    }
    return nullptr;
}

bool addTrain(Train* train) {
    if (trainTable.trainCount >= MAX_TRAINS) {
        return false;
    }

    unsigned int hash = hashString(train->trainID) % MAX_TRAINS;
    // Linear probing
    for (int i = 0; i < MAX_TRAINS; i++) {
        int idx = (hash + i) % MAX_TRAINS;
        if (trainTable.trains[idx] == nullptr) {
            trainTable.trains[idx] = train;
            trainTable.trainCount++;

            // Add to station index
            for (int j = 0; j < train->stationNum; j++) {
                if (stationIndex.count < MAX_TRAINS * MAX_STATIONS_PER_TRAIN) {
                    strncpy(stationIndex.indices[stationIndex.count].stationName,
                           train->stations[j].name, MAX_STATION_NAME_LEN - 1);
                    stationIndex.indices[stationIndex.count].trainIndex = idx;
                    stationIndex.indices[stationIndex.count].stationIdx = j;
                    stationIndex.count++;
                }
            }

            return true;
        }
        if (strcmp(trainTable.trains[idx]->trainID, train->trainID) == 0) {
            return false; // TrainID already exists
        }
    }
    return false;
}

struct Command {
    string name;
    string params[20];
    int paramCount;

    Command() : paramCount(0) {}
};

bool parseLine(Command& cmd) {
    string line;
    if (!getline(cin, line)) {
        return false;
    }

    // Skip empty lines
    if (line.empty()) {
        return parseLine(cmd);
    }

    // Parse command
    size_t pos = 0;
    size_t spacePos = line.find(' ');
    if (spacePos == string::npos) {
        cmd.name = line;
        return true;
    }

    cmd.name = line.substr(0, spacePos);
    pos = spacePos + 1;

    // Parse parameters
    while (pos < line.length()) {
        // Skip spaces
        while (pos < line.length() && line[pos] == ' ') pos++;
        if (pos >= line.length()) break;

        // Check for -parameter
        if (line[pos] == '-') {
            size_t nextSpace = line.find(' ', pos);
            if (nextSpace == string::npos) nextSpace = line.length();

            string param = line.substr(pos, nextSpace - pos);
            cmd.params[cmd.paramCount++] = param;
            pos = nextSpace;
        } else {
            // Value
            size_t nextSpace = line.find(' ', pos);
            if (nextSpace == string::npos) nextSpace = line.length();

            string value = line.substr(pos, nextSpace - pos);
            cmd.params[cmd.paramCount++] = value;
            pos = nextSpace;
        }
    }

    return true;
}

// Helper function to get parameter value
string getParamValue(const Command& cmd, const string& param) {
    for (int i = 0; i < cmd.paramCount - 1; i++) {
        if (cmd.params[i] == param) {
            return cmd.params[i + 1];
        }
    }
    return "";
}

// Helper functions for date/time parsing
int parseTime(const string& timeStr) {
    // Format: "hh:mm"
    int hour = atoi(timeStr.substr(0, 2).c_str());
    int minute = atoi(timeStr.substr(3, 2).c_str());
    return hour * 60 + minute;
}

int parseDate(const string& dateStr) {
    // Format: "mm-dd", convert to days from June 1
    int month = atoi(dateStr.substr(0, 2).c_str());
    int day = atoi(dateStr.substr(3, 2).c_str());

    // June is month 6
    int daysFromJune1 = 0;
    if (month == 6) {
        daysFromJune1 = day - 1;
    } else if (month == 7) {
        daysFromJune1 = 30 + day - 1; // June has 30 days
    } else if (month == 8) {
        daysFromJune1 = 30 + 31 + day - 1; // June 30, July 31
    }
    return daysFromJune1;
}

string formatTime(int minutes) {
    int hour = minutes / 60;
    int minute = minutes % 60;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    return string(buffer);
}

string formatDate(int daysFromJune1) {
    int month, day;
    if (daysFromJune1 < 30) {
        month = 6;
        day = daysFromJune1 + 1;
    } else if (daysFromJune1 < 61) {
        month = 7;
        day = daysFromJune1 - 30 + 1;
    } else {
        month = 8;
        day = daysFromJune1 - 61 + 1;
    }
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%02d-%02d", month, day);
    return string(buffer);
}

// Command implementations
int handleAddUser(const Command& cmd) {
    string curUsername = getParamValue(cmd, "-c");
    string username = getParamValue(cmd, "-u");
    string password = getParamValue(cmd, "-p");
    string name = getParamValue(cmd, "-n");
    string mailAddr = getParamValue(cmd, "-m");
    string privilegeStr = getParamValue(cmd, "-g");

    if (username.empty() || password.empty() || name.empty() || mailAddr.empty()) {
        return -1;
    }

    // Check if user already exists
    if (findUser(username.c_str()) != nullptr) {
        return -1;
    }

    if (!firstUserCreated) {
        // First user
        User* user = new User();
        strncpy(user->username, username.c_str(), MAX_USERNAME_LEN - 1);
        strncpy(user->password, password.c_str(), MAX_PASSWORD_LEN - 1);
        strncpy(user->name, name.c_str(), MAX_NAME_LEN - 1);
        strncpy(user->mailAddr, mailAddr.c_str(), MAX_MAIL_LEN - 1);
        user->privilege = 10;

        if (addUser(user)) {
            firstUserCreated = true;
            return 0;
        } else {
            delete user;
            return -1;
        }
    } else {
        // Not first user
        User* curUser = findUser(curUsername.c_str());
        if (curUser == nullptr || !curUser->isLoggedIn) {
            return -1;
        }

        int privilege = atoi(privilegeStr.c_str());
        if (privilege >= curUser->privilege) {
            return -1;
        }

        User* user = new User();
        strncpy(user->username, username.c_str(), MAX_USERNAME_LEN - 1);
        strncpy(user->password, password.c_str(), MAX_PASSWORD_LEN - 1);
        strncpy(user->name, name.c_str(), MAX_NAME_LEN - 1);
        strncpy(user->mailAddr, mailAddr.c_str(), MAX_MAIL_LEN - 1);
        user->privilege = privilege;

        if (addUser(user)) {
            return 0;
        } else {
            delete user;
            return -1;
        }
    }
}

int handleLogin(const Command& cmd) {
    string username = getParamValue(cmd, "-u");
    string password = getParamValue(cmd, "-p");

    if (username.empty() || password.empty()) {
        return -1;
    }

    User* user = findUser(username.c_str());
    if (user == nullptr) {
        return -1;
    }

    if (user->isLoggedIn) {
        return -1; // Already logged in
    }

    if (strcmp(user->password, password.c_str()) != 0) {
        return -1;
    }

    user->isLoggedIn = true;
    return 0;
}

int handleLogout(const Command& cmd) {
    string username = getParamValue(cmd, "-u");

    if (username.empty()) {
        return -1;
    }

    User* user = findUser(username.c_str());
    if (user == nullptr || !user->isLoggedIn) {
        return -1;
    }

    user->isLoggedIn = false;
    return 0;
}

int handleQueryProfile(const Command& cmd) {
    string curUsername = getParamValue(cmd, "-c");
    string username = getParamValue(cmd, "-u");

    if (curUsername.empty() || username.empty()) {
        return -1;
    }

    User* curUser = findUser(curUsername.c_str());
    User* targetUser = findUser(username.c_str());

    if (curUser == nullptr || targetUser == nullptr || !curUser->isLoggedIn) {
        return -1;
    }

    if (curUser->privilege <= targetUser->privilege && strcmp(curUser->username, targetUser->username) != 0) {
        return -1;
    }

    cout << targetUser->username << " " << targetUser->name << " "
         << targetUser->mailAddr << " " << targetUser->privilege << endl;
    return 0;
}

int handleModifyProfile(const Command& cmd) {
    string curUsername = getParamValue(cmd, "-c");
    string username = getParamValue(cmd, "-u");
    string password = getParamValue(cmd, "-p");
    string name = getParamValue(cmd, "-n");
    string mailAddr = getParamValue(cmd, "-m");
    string privilegeStr = getParamValue(cmd, "-g");

    if (curUsername.empty() || username.empty()) {
        return -1;
    }

    User* curUser = findUser(curUsername.c_str());
    User* targetUser = findUser(username.c_str());

    if (curUser == nullptr || targetUser == nullptr || !curUser->isLoggedIn) {
        return -1;
    }

    if (curUser->privilege <= targetUser->privilege && strcmp(curUser->username, targetUser->username) != 0) {
        return -1;
    }

    // Check privilege modification
    if (!privilegeStr.empty()) {
        int newPrivilege = atoi(privilegeStr.c_str());
        if (newPrivilege >= curUser->privilege) {
            return -1;
        }
        targetUser->privilege = newPrivilege;
    }

    // Update other fields if provided
    if (!password.empty()) {
        strncpy(targetUser->password, password.c_str(), MAX_PASSWORD_LEN - 1);
    }
    if (!name.empty()) {
        strncpy(targetUser->name, name.c_str(), MAX_NAME_LEN - 1);
    }
    if (!mailAddr.empty()) {
        strncpy(targetUser->mailAddr, mailAddr.c_str(), MAX_MAIL_LEN - 1);
    }

    cout << targetUser->username << " " << targetUser->name << " "
         << targetUser->mailAddr << " " << targetUser->privilege << endl;
    return 0;
}

// Helper function to split string by delimiter
void splitString(const string& str, char delimiter, string result[], int& count) {
    count = 0;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != string::npos) {
        result[count++] = str.substr(start, end - start);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    result[count++] = str.substr(start);
}

int handleAddTrain(const Command& cmd) {
    string trainID = getParamValue(cmd, "-i");
    string stationNumStr = getParamValue(cmd, "-n");
    string seatNumStr = getParamValue(cmd, "-m");
    string stationsStr = getParamValue(cmd, "-s");
    string pricesStr = getParamValue(cmd, "-p");
    string startTimeStr = getParamValue(cmd, "-x");
    string travelTimesStr = getParamValue(cmd, "-t");
    string stopoverTimesStr = getParamValue(cmd, "-o");
    string saleDateStr = getParamValue(cmd, "-d");
    string typeStr = getParamValue(cmd, "-y");

    if (trainID.empty() || stationNumStr.empty() || seatNumStr.empty() ||
        stationsStr.empty() || pricesStr.empty() || startTimeStr.empty() ||
        travelTimesStr.empty() || stopoverTimesStr.empty() || saleDateStr.empty() ||
        typeStr.empty()) {
        return -1;
    }

    // Check if train already exists
    if (findTrain(trainID.c_str()) != nullptr) {
        return -1;
    }

    Train* train = new Train();
    strncpy(train->trainID, trainID.c_str(), MAX_TRAIN_ID_LEN - 1);
    train->stationNum = atoi(stationNumStr.c_str());
    train->seatNum = atoi(seatNumStr.c_str());
    train->startTime = parseTime(startTimeStr);
    train->type = typeStr[0];

    // Parse sale dates
    string saleDates[2];
    int dateCount = 0;
    splitString(saleDateStr, '|', saleDates, dateCount);
    if (dateCount != 2) {
        delete train;
        return -1;
    }
    train->saleDateStart = parseDate(saleDates[0]);
    train->saleDateEnd = parseDate(saleDates[1]);

    // Parse stations
    string stations[MAX_STATIONS_PER_TRAIN];
    int stationCount = 0;
    splitString(stationsStr, '|', stations, stationCount);
    if (stationCount != train->stationNum) {
        delete train;
        return -1;
    }
    for (int i = 0; i < stationCount; i++) {
        strncpy(train->stations[i].name, stations[i].c_str(), MAX_STATION_NAME_LEN - 1);
    }

    // Parse prices
    string priceStrs[MAX_STATIONS_PER_TRAIN];
    int priceCount = 0;
    splitString(pricesStr, '|', priceStrs, priceCount);
    if (priceCount != train->stationNum - 1) {
        delete train;
        return -1;
    }
    for (int i = 0; i < priceCount; i++) {
        train->prices[i] = atoi(priceStrs[i].c_str());
    }

    // Parse travel times
    string travelTimeStrs[MAX_STATIONS_PER_TRAIN];
    int travelTimeCount = 0;
    splitString(travelTimesStr, '|', travelTimeStrs, travelTimeCount);
    if (travelTimeCount != train->stationNum - 1) {
        delete train;
        return -1;
    }
    for (int i = 0; i < travelTimeCount; i++) {
        train->travelTimes[i] = atoi(travelTimeStrs[i].c_str());
    }

    // Parse stopover times
    if (train->stationNum > 2) {
        string stopoverTimeStrs[MAX_STATIONS_PER_TRAIN];
        int stopoverTimeCount = 0;
        splitString(stopoverTimesStr, '|', stopoverTimeStrs, stopoverTimeCount);
        if (stopoverTimeCount != train->stationNum - 2) {
            delete train;
            return -1;
        }
        for (int i = 0; i < stopoverTimeCount; i++) {
            train->stopoverTimes[i] = atoi(stopoverTimeStrs[i].c_str());
        }
    }

    if (addTrain(train)) {
        return 0;
    } else {
        delete train;
        return -1;
    }
}

int handleReleaseTrain(const Command& cmd) {
    string trainID = getParamValue(cmd, "-i");

    if (trainID.empty()) {
        return -1;
    }

    Train* train = findTrain(trainID.c_str());
    if (train == nullptr || train->isReleased) {
        return -1;
    }

    train->isReleased = true;
    return 0;
}

int handleDeleteTrain(const Command& cmd) {
    string trainID = getParamValue(cmd, "-i");

    if (trainID.empty()) {
        return -1;
    }

    Train* train = findTrain(trainID.c_str());
    if (train == nullptr || train->isReleased) {
        return -1;
    }

    // Remove from hash table
    unsigned int hash = hashString(trainID.c_str()) % MAX_TRAINS;
    for (int i = 0; i < MAX_TRAINS; i++) {
        int idx = (hash + i) % MAX_TRAINS;
        if (trainTable.trains[idx] != nullptr &&
            strcmp(trainTable.trains[idx]->trainID, trainID.c_str()) == 0) {
            delete trainTable.trains[idx];
            trainTable.trains[idx] = nullptr;
            trainTable.trainCount--;
            return 0;
        }
    }

    return -1;
}

int handleQueryTrain(const Command& cmd) {
    string trainID = getParamValue(cmd, "-i");
    string dateStr = getParamValue(cmd, "-d");

    if (trainID.empty() || dateStr.empty()) {
        return -1;
    }

    Train* train = findTrain(trainID.c_str());
    if (train == nullptr) {
        return -1;
    }

    int queryDate = parseDate(dateStr);
    if (queryDate < train->saleDateStart || queryDate > train->saleDateEnd) {
        return -1;
    }

    // Calculate arrival and departure times
    int currentTime = train->startTime; // minutes from 00:00 on departure day
    int cumulativePrice = 0;

    cout << train->trainID << " " << train->type << endl;

    for (int i = 0; i < train->stationNum; i++) {
        string stationName = train->stations[i].name;
        string arrivingTime, leavingTime, priceStr, seatStr;

        if (i == 0) {
            // Starting station
            arrivingTime = "xx-xx xx:xx";
            int departureDay = queryDate;
            int departureTime = currentTime;
            leavingTime = formatDate(departureDay) + " " + formatTime(departureTime);
            priceStr = "0";
            seatStr = to_string(train->seatNum);
        } else if (i == train->stationNum - 1) {
            // Terminal station
            int arrivalDay = queryDate + currentTime / (24 * 60);
            int arrivalTimeOfDay = currentTime % (24 * 60);
            arrivingTime = formatDate(arrivalDay) + " " + formatTime(arrivalTimeOfDay);
            leavingTime = "xx-xx xx:xx";
            priceStr = to_string(cumulativePrice);
            seatStr = "x";
        } else {
            // Intermediate station
            int arrivalDay = queryDate + currentTime / (24 * 60);
            int arrivalTimeOfDay = currentTime % (24 * 60);
            arrivingTime = formatDate(arrivalDay) + " " + formatTime(arrivalTimeOfDay);

            // Add stopover time
            currentTime += train->stopoverTimes[i - 1];
            int departureDay = queryDate + currentTime / (24 * 60);
            int departureTimeOfDay = currentTime % (24 * 60);
            leavingTime = formatDate(departureDay) + " " + formatTime(departureTimeOfDay);

            priceStr = to_string(cumulativePrice);
            seatStr = to_string(train->seatNum);
        }

        cout << stationName << " " << arrivingTime << " -> "
             << leavingTime << " " << priceStr << " " << seatStr << endl;

        // Update for next station
        if (i < train->stationNum - 1) {
            cumulativePrice += train->prices[i];
            currentTime += train->travelTimes[i];
        }
    }

    return 0;
}

struct TicketResult {
    char trainID[MAX_TRAIN_ID_LEN];
    char fromStation[MAX_STATION_NAME_LEN];
    char toStation[MAX_STATION_NAME_LEN];
    char leavingTime[20];
    char arrivingTime[20];
    int price;
    int seat;
    int travelTime; // in minutes
    int trainIdx;
};

const int MAX_RESULTS = 10000;
TicketResult ticketResults[MAX_RESULTS];
int resultCount = 0;

bool compareByTime(const TicketResult& a, const TicketResult& b) {
    if (a.travelTime != b.travelTime) {
        return a.travelTime < b.travelTime;
    }
    return strcmp(a.trainID, b.trainID) < 0;
}

bool compareByCost(const TicketResult& a, const TicketResult& b) {
    if (a.price != b.price) {
        return a.price < b.price;
    }
    return strcmp(a.trainID, b.trainID) < 0;
}

int handleQueryTicket(const Command& cmd) {
    string fromStation = getParamValue(cmd, "-s");
    string toStation = getParamValue(cmd, "-t");
    string dateStr = getParamValue(cmd, "-d");
    string sortType = getParamValue(cmd, "-p");

    if (fromStation.empty() || toStation.empty() || dateStr.empty()) {
        return -1;
    }

    if (sortType.empty()) {
        sortType = "time";
    }

    int queryDate = parseDate(dateStr);
    resultCount = 0;

    // Simple linear search through all trains
    for (int trainIdx = 0; trainIdx < MAX_TRAINS; trainIdx++) {
        Train* train = trainTable.trains[trainIdx];
        if (train == nullptr || !train->isReleased) {
            continue;
        }

        // Check if query date is within sale range
        if (queryDate < train->saleDateStart || queryDate > train->saleDateEnd) {
            continue;
        }

        // Find fromStation and toStation indices
        int fromIdx = -1, toIdx = -1;
        for (int i = 0; i < train->stationNum; i++) {
            if (strcmp(train->stations[i].name, fromStation.c_str()) == 0) {
                fromIdx = i;
            }
            if (strcmp(train->stations[i].name, toStation.c_str()) == 0) {
                toIdx = i;
            }
        }

        if (fromIdx == -1 || toIdx == -1 || fromIdx >= toIdx) {
            continue;
        }

        // Calculate departure time from fromStation
        int currentTime = train->startTime; // minutes from 00:00 on departure day from starting station
        int cumulativePrice = 0;

        for (int i = 0; i < fromIdx; i++) {
            cumulativePrice += train->prices[i];
            currentTime += train->travelTimes[i];
            if (i > 0) {
                currentTime += train->stopoverTimes[i - 1];
            }
        }

        // Calculate departure day from fromStation
        int departureDay = queryDate;
        int departureTime = currentTime;

        // Calculate arrival time at toStation
        for (int i = fromIdx; i < toIdx; i++) {
            cumulativePrice += train->prices[i];
            currentTime += train->travelTimes[i];
            if (i > fromIdx && i < train->stationNum - 1) {
                currentTime += train->stopoverTimes[i - 1];
            }
        }

        int arrivalDay = queryDate + currentTime / (24 * 60);
        int arrivalTime = currentTime % (24 * 60);

        // Calculate available seats (simplified - always returns seatNum for now)
        int availableSeats = train->seatNum;

        if (resultCount < MAX_RESULTS) {
            TicketResult& result = ticketResults[resultCount];
            strncpy(result.trainID, train->trainID, MAX_TRAIN_ID_LEN - 1);
            strncpy(result.fromStation, fromStation.c_str(), MAX_STATION_NAME_LEN - 1);
            strncpy(result.toStation, toStation.c_str(), MAX_STATION_NAME_LEN - 1);

            string leavingTimeStr = formatDate(departureDay) + " " + formatTime(departureTime);
            string arrivingTimeStr = formatDate(arrivalDay) + " " + formatTime(arrivalTime);
            strncpy(result.leavingTime, leavingTimeStr.c_str(), 19);
            strncpy(result.arrivingTime, arrivingTimeStr.c_str(), 19);

            result.price = cumulativePrice;
            result.seat = availableSeats;
            result.travelTime = currentTime - departureTime;
            result.trainIdx = trainIdx;
            resultCount++;
        }
    }

    // Sort results
    if (sortType == "time") {
        bubbleSort(ticketResults, resultCount, compareByTime);
    } else { // "cost"
        bubbleSort(ticketResults, resultCount, compareByCost);
    }

    // Output results
    cout << resultCount << endl;
    for (int i = 0; i < resultCount; i++) {
        const TicketResult& result = ticketResults[i];
        cout << result.trainID << " " << result.fromStation << " "
             << result.leavingTime << " -> " << result.toStation << " "
             << result.arrivingTime << " " << result.price << " "
             << result.seat << endl;
    }

    return 0;
}

// Helper function to get or create train seats for a specific date
int* getTrainSeats(int trainIdx, int date) {
    // First, try to find existing seats
    for (int i = 0; i < trainSeatsArray.count; i++) {
        if (trainSeatsArray.seatsData[i].trainIdx == trainIdx &&
            trainSeatsArray.seatsData[i].date == date) {
            return trainSeatsArray.seatsData[i].seats;
        }
    }

    // Create new seats entry
    if (trainSeatsArray.count < MAX_TRAINS * 100) {
        TrainSeats& seats = trainSeatsArray.seatsData[trainSeatsArray.count];
        seats.trainIdx = trainIdx;
        seats.date = date;

        Train* train = trainTable.trains[trainIdx];
        if (train) {
            for (int i = 0; i < train->stationNum - 1; i++) {
                seats.seats[i] = train->seatNum;
            }
        }

        trainSeatsArray.count++;
        return seats.seats;
    }

    return nullptr;
}

int handleBuyTicket(const Command& cmd) {
    string username = getParamValue(cmd, "-u");
    string trainID = getParamValue(cmd, "-i");
    string dateStr = getParamValue(cmd, "-d");
    string numStr = getParamValue(cmd, "-n");
    string fromStation = getParamValue(cmd, "-f");
    string toStation = getParamValue(cmd, "-t");
    string queueStr = getParamValue(cmd, "-q");

    if (username.empty() || trainID.empty() || dateStr.empty() ||
        numStr.empty() || fromStation.empty() || toStation.empty()) {
        return -1;
    }

    // Check if user is logged in
    User* user = findUser(username.c_str());
    if (user == nullptr || !user->isLoggedIn) {
        return -1;
    }

    // Find train
    Train* train = findTrain(trainID.c_str());
    if (train == nullptr || !train->isReleased) {
        return -1;
    }

    int date = parseDate(dateStr);
    int numTickets = atoi(numStr.c_str());
    bool queue = (queueStr == "true");

    // Check if date is within sale range
    // Note: date is departure date from fromStation, not from starting station
    // This is simplified - need to calculate actual departure date from starting station
    if (date < train->saleDateStart || date > train->saleDateEnd) {
        return -1;
    }

    // Find station indices
    int fromIdx = -1, toIdx = -1;
    for (int i = 0; i < train->stationNum; i++) {
        if (strcmp(train->stations[i].name, fromStation.c_str()) == 0) {
            fromIdx = i;
        }
        if (strcmp(train->stations[i].name, toStation.c_str()) == 0) {
            toIdx = i;
        }
    }

    if (fromIdx == -1 || toIdx == -1 || fromIdx >= toIdx) {
        return -1;
    }

    // Calculate actual departure date from starting station
    // This is simplified - assuming date is departure from starting station
    int actualDate = date; // Should calculate based on travel time from starting station

    // Get seats for this date
    // Find train index
    int trainIdx = -1;
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (trainTable.trains[i] == train) {
            trainIdx = i;
            break;
        }
    }

    if (trainIdx == -1) {
        return -1;
    }

    int* seats = getTrainSeats(trainIdx, actualDate);
    if (seats == nullptr) {
        return -1;
    }

    // Check available seats
    int minSeats = train->seatNum;
    for (int i = fromIdx; i < toIdx; i++) {
        if (seats[i] < minSeats) {
            minSeats = seats[i];
        }
    }

    // Calculate price
    int price = 0;
    for (int i = fromIdx; i < toIdx; i++) {
        price += train->prices[i];
    }
    int totalPrice = price * numTickets;

    if (minSeats >= numTickets) {
        // Enough seats available
        for (int i = fromIdx; i < toIdx; i++) {
            seats[i] -= numTickets;
        }

        // Create order
        if (orderArray.orderCount < MAX_ORDERS) {
            Order& order = orderArray.orders[orderArray.orderCount];
            strncpy(order.username, username.c_str(), MAX_USERNAME_LEN - 1);
            strncpy(order.trainID, trainID.c_str(), MAX_TRAIN_ID_LEN - 1);
            order.date = date;
            order.fromStationIdx = fromIdx;
            order.toStationIdx = toIdx;
            order.numTickets = numTickets;
            order.price = totalPrice;
            order.status = 0; // success
            order.timestamp = currentTimestamp;
            orderArray.orderCount++;
        }

        cout << totalPrice << endl;
        return 0;
    } else if (queue && numTickets <= train->seatNum) {
        // Add to queue (standby)
        if (orderArray.orderCount < MAX_ORDERS) {
            Order& order = orderArray.orders[orderArray.orderCount];
            strncpy(order.username, username.c_str(), MAX_USERNAME_LEN - 1);
            strncpy(order.trainID, trainID.c_str(), MAX_TRAIN_ID_LEN - 1);
            order.date = date;
            order.fromStationIdx = fromIdx;
            order.toStationIdx = toIdx;
            order.numTickets = numTickets;
            order.price = totalPrice;
            order.status = 1; // pending
            order.timestamp = currentTimestamp;
            orderArray.orderCount++;
        }

        cout << "queue" << endl;
        return 0;
    } else {
        return -1;
    }
}

int handleQueryOrder(const Command& cmd) {
    string username = getParamValue(cmd, "-u");

    if (username.empty()) {
        return -1;
    }

    // Check if user is logged in
    User* user = findUser(username.c_str());
    if (user == nullptr || !user->isLoggedIn) {
        return -1;
    }

    // Count orders for this user
    int userOrderCount = 0;
    for (int i = 0; i < orderArray.orderCount; i++) {
        if (strcmp(orderArray.orders[i].username, username.c_str()) == 0) {
            userOrderCount++;
        }
    }

    cout << userOrderCount << endl;

    // Output orders (newest first)
    for (int i = orderArray.orderCount - 1; i >= 0; i--) {
        Order& order = orderArray.orders[i];
        if (strcmp(order.username, username.c_str()) != 0) {
            continue;
        }

        // Find train
        Train* train = findTrain(order.trainID);
        if (train == nullptr) {
            continue;
        }

        // Calculate times (simplified)
        string statusStr;
        if (order.status == 0) {
            statusStr = "success";
        } else if (order.status == 1) {
            statusStr = "pending";
        } else {
            statusStr = "refunded";
        }

        // Simplified output - in real implementation need to calculate times
        cout << "[" << statusStr << "] " << order.trainID << " "
             << train->stations[order.fromStationIdx].name << " "
             << formatDate(order.date) << " 00:00 -> "
             << train->stations[order.toStationIdx].name << " "
             << formatDate(order.date) << " 00:00 "
             << order.price << " " << order.numTickets << endl;
    }

    return 0;
}

int handleRefundTicket(const Command& cmd) {
    string username = getParamValue(cmd, "-u");
    string nStr = getParamValue(cmd, "-n");

    if (username.empty()) {
        return -1;
    }

    int n = 1;
    if (!nStr.empty()) {
        n = atoi(nStr.c_str());
    }

    // Check if user is logged in
    User* user = findUser(username.c_str());
    if (user == nullptr || !user->isLoggedIn) {
        return -1;
    }

    // Find the n-th order for this user (newest first)
    int orderIdx = -1;
    int count = 0;
    for (int i = orderArray.orderCount - 1; i >= 0; i--) {
        if (strcmp(orderArray.orders[i].username, username.c_str()) == 0) {
            count++;
            if (count == n) {
                orderIdx = i;
                break;
            }
        }
    }

    if (orderIdx == -1) {
        return -1;
    }

    Order& order = orderArray.orders[orderIdx];
    if (order.status == 2) { // Already refunded
        return -1;
    }

    // Mark as refunded
    order.status = 2;

    // TODO: Return seats to train (complicated with pending queue)
    cout << "0" << endl;
    return 0;
}

int handleClean() {
    // Clear all data
    for (int i = 0; i < MAX_USERS; i++) {
        if (userTable.users[i]) {
            delete userTable.users[i];
            userTable.users[i] = nullptr;
        }
    }
    userTable.userCount = 0;

    for (int i = 0; i < MAX_TRAINS; i++) {
        if (trainTable.trains[i]) {
            delete trainTable.trains[i];
            trainTable.trains[i] = nullptr;
        }
    }
    trainTable.trainCount = 0;

    orderArray.orderCount = 0;
    stationIndex.count = 0;
    trainSeatsArray.count = 0;
    currentTimestamp = 0;
    firstUserCreated = false;

    return 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Command cmd;
    while (parseLine(cmd)) {
        currentTimestamp++;

        if (cmd.name == "add_user") {
            cout << handleAddUser(cmd) << endl;
        } else if (cmd.name == "login") {
            cout << handleLogin(cmd) << endl;
        } else if (cmd.name == "logout") {
            cout << handleLogout(cmd) << endl;
        } else if (cmd.name == "query_profile") {
            int result = handleQueryProfile(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "modify_profile") {
            int result = handleModifyProfile(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "add_train") {
            cout << handleAddTrain(cmd) << endl;
        } else if (cmd.name == "release_train") {
            cout << handleReleaseTrain(cmd) << endl;
        } else if (cmd.name == "query_train") {
            int result = handleQueryTrain(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "delete_train") {
            cout << handleDeleteTrain(cmd) << endl;
        } else if (cmd.name == "query_ticket") {
            int result = handleQueryTicket(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "query_transfer") {
            cout << "-1" << endl; // TODO
        } else if (cmd.name == "buy_ticket") {
            int result = handleBuyTicket(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "query_order") {
            int result = handleQueryOrder(cmd);
            if (result == -1) {
                cout << "-1" << endl;
            }
        } else if (cmd.name == "refund_ticket") {
            cout << handleRefundTicket(cmd) << endl;
        } else if (cmd.name == "clean") {
            cout << handleClean() << endl;
        } else if (cmd.name == "exit") {
            cout << "bye" << endl;
            break;
        } else {
            cout << "-1" << endl;
        }

        cmd = Command(); // Reset for next command
    }

    return 0;
}