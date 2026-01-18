#pragma once
#include "relationalbroker.h"
#include <vector>
#include <string>
import registrar:domian.teachingsecretaryr;
class SecretaryBroker:public RelationalBroker
{
public:
    static SecretaryBroker& singleton();
    void createTable() override;
    void initData() override;
    bool isSecretaryExistsInternal(const std::string& sid);
    // 添加教学秘书
    bool addTeachingSecretary(const TeachingSecretary& secretary);
    // 删除秘书
    bool deleteTeachingSecretary(const string& id);

    bool addTeacherBySecretary(const std::string& secretaryId, const Teacher& teacher);
    bool deleteTeacherBySecretary(const std::string& secretaryId, const std::string& teacherId);
    //添加课程
    bool addCourseBySecretary(const std::string& secretaryId, const Course& course);
    //分配课程给教师
    bool assignCourseToTeacher(const std::string& secretaryId, const std::string& courseId, const std::string& teacherId);
    bool deleteCourseBySecretary(const std::string& secretaryId, const std::string& courseId);
private:
    TeachingSecretaryBroker();
    TeachingSecretaryBroker(const SecretaryBroker&) = delete;
    TeachingSecretaryBroker& operator=(const SecretaryBroker&) = delete;

    // 秘书缓存
    vector<shared_ptr<TeachingSecretary>> _secretarys;
};

