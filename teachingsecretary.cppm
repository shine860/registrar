//
// File: teachingsecretary.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.teachingsecretary;
import :domain.person;
import std;
using std::print;
using std::string;
using std::vector;

class Course;
class Classroom;
class Teacher;
class Student;

export class TeachingSecretary : public Person
{
friend class TeachingSecretaryBroker;
private:
    vector<string> _roomTimeSlots;
    vector<Course*> _courses;
    vector<Classroom*> _rooms;
public:
    TeachingSecretary(string id, string name, string gender,string dept);
    bool arrangement(Course* course, Classroom* classroom,Teacher* teacher,const string& timeslot);
};

TeachingSecretary::TeachingSecretary(string id, string name, string gender,string dept)
    : Person(id,name,gender,dept)
{}

