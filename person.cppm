//
// File: person.cppm
// Created: 刘城2024051604051 3214528114@qq.com      2026-01-16
// Version: 1.0      License: AGPLv3
export module domain.person;
import std;
using std::print;
using std::string;

export class Person
{
private:
    string id;
    string name;
    string gender;

public:
    Person(string id, string name, string gender);
    virtual ~Person() = default;
    void showInfo() const;
    virtual bool authenticate(const std::string& password) = 0;
    bool isIdMatch(const string& targetId) const;
    bool isNameMatch(const string& targetName) const;
};

Person::Person(string id, string name, string gender)
    : id(std::move(id)), name(std::move(name)), gender(std::move(gender)) {}

void Person::showInfo() const
{
    print("ID: {}, Name: {}, Gender: {}", this->id, this->name, this->gender);
}

bool Person::isIdMatch(const string& targetId) const
{
    return this->id == targetId;
}

bool Person::isNameMatch(const string& targetName) const
{
    return this->name == targetName;
}
