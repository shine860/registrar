//
// File: person.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module registrar:domain.person;
import std;
using std::print;
using std::string;

export class Person
{
public:
    Person(string id, string name, string gender,string dept);
    virtual ~Person() = default;
    string Info() const;
    bool hasId(string searchId);
    string get_id();
    string get_name();
protected:
    string m_id;//学号，工号
    string m_name; //姓名
    string m_gender;//性别
    string m_dept;//所在系
};

Person::Person(string id, string name, string gender,string dept)
    :m_id(id), m_name(name), m_gender(gender), m_dept(dept)
{}

string Person::Info() const
{
    return std::format("ID: {}, Name: {}, Gender: {},dept : {}\n", this->m_id, this->m_name, this->m_gender,this->m_dept);
}

bool Person::hasId(string searchId)
{
    return  searchId == this->m_id;
}

string Person::get_id()
{
    return m_id;
}

string Person::get_name()
{
    return m_name;
}
