//
// File: person.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.person;
import std;
using std::string;

export class Person
{
protected:
    string m_id;
    string m_name;
    string m_gender;
public:
    Person(string id, string name, string gender);
    string info() const;
};

Person::Person(string id, string name, string gender)
    : m_id(std::move(id)), m_name(std::move(name)), m_gender(std::move(gender)) {}

string Person::info() const
{
    return "ID: " + this->m_id + ", 姓名: " + this->m_name;
}
