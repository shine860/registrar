//
// File: teachingsecretary.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.teachingsecretary;
import std;
import domain.person;
import domain.course;
import domain.classroom;

using std::print;
using std::string;
using std::vector;

export class TeachingSecretary : public Person
{
private:
    string m_secretaryId;
    vector<string> _roomTimeSlots;
public:
    TeachingSecretary(string id, string name, string gender);
    bool arrangement(Course* course, Classroom* classroom);
};

TeachingSecretary::TeachingSecretary(string id, string name, string gender)
    : Person(std::move(id), std::move(name), std::move(gender)),
      m_secretaryId(std::move(id)) {}

bool TeachingSecretary::arrangement(Course* course, Classroom* classroom)
{
    string key = "classroomId:timeSlot";
    for (const auto& slot : this->_roomTimeSlots)
    {
        if (slot == key)
        {
            print("排课失败：时段冲突");
            return false;
        }
    }
    this->_roomTimeSlots.push_back(key);
    classroom->occupyClassroom("timeSlot", "courseId");
    print("教务秘书 {} 排课成功", this->m_name);
    return true;
}
