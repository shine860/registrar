//
// File: classroom.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.classroom;
import std;
using std::print;
using std::string;
using std::vector;

export class Classroom
{
private:
    string roomNum;
    int capacity;
    bool isOccupied;
    vector<string> courseSchedule;

public:
    Classroom(string roomNum, int capacity);
    bool checkAvailability(const string& timeSlot) const;
    bool occupyClassroom(const string& timeSlot, const string& courseId);
    bool releaseClassroom(const string& timeSlot);
    void showClassroomInfo() const;
};

Classroom::Classroom(string roomNum, int capacity)
    : roomNum(std::move(roomNum)), capacity(capacity), isOccupied(false) {}

bool Classroom::checkAvailability(const string& timeSlot) const
{
    return !this->isOccupied;
}

bool Classroom::occupyClassroom(const string& timeSlot, const string& courseId)
{
    if (this->isOccupied) return false;
    this->isOccupied = true;
    this->courseSchedule.push_back(timeSlot + ": " + courseId);
    return true;
}

bool Classroom::releaseClassroom(const string& timeSlot)
{
    if (!this->isOccupied) return false;
    this->isOccupied = false;
    return true;
}

void Classroom::showClassroomInfo() const
{
    print("Room: {}, Capacity: {}, Occupied: {}", this->roomNum, this->capacity, this->isOccupied);
}

