//
// File: classroom.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.classroom;
import std;
using std::print;
using std::string;
using std::vector;

class Course;

export class Classroom
{
protected:
    string m_roomNum;
    int m_capacity;
    bool m_isOccupied;
    vector<Course*> _courseSchedule;
public:
    Classroom(string roomNum, int capacity);
    bool checkAvailability();
    bool occupyClassroom(const string& timeSlot, const string& courseId);
    bool releaseClassroom();
    void showClassroomInfo();
};

Classroom::Classroom(string roomNum, int capacity)
    : m_roomNum(std::move(roomNum)), m_capacity(capacity), m_isOccupied(false) {}

bool Classroom::checkAvailability()
{
    return !this->m_isOccupied;
}

bool Classroom::occupyClassroom(const string& timeSlot, const string& courseId)
{
    if (this->m_isOccupied) return false;
    this->m_isOccupied = true;
    print("教室{}已占用", this->m_roomNum);
    return true;
}

bool Classroom::releaseClassroom()
{
    if (!this->m_isOccupied) return false;
    this->m_isOccupied = false;
    print("教室{}已释放", this->m_roomNum);
    return true;
}

void Classroom::showClassroomInfo()
{
    print("教室编号：{}，容量：{}，状态：{}", this->m_roomNum, this->m_capacity, this->m_isOccupied ? "已占用" : "空闲");
}
