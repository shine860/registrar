// Module
// File: classroombroker.cppm   Version: 0.1.0   License: AGPLv3
// Created: 苏茜（2024051604029）   3236863614@qq.com   2026-01-24 21:32:34
// Description:教室实体的代管者，继承基类
//
module;
#include "pqxx/pqxx"

export module registrar:dm.classroombroker;

import :dm.base;
import :domain.classroom;
import std;

export class ClassroomBroker : public RelationalBroker {
public:
    static ClassroomBroker& singleton();
    void createTable() override;
    void initData() override;

    std::shared_ptr<Classroom> findById(const std::string& rid);
    //判断教室是否存在
    bool isClassroomExists(const std::string& rid) const;

    bool addClassroom(const Classroom& classroom);
    bool deleteClassroom(const std::string& rid);

    bool updateClassroomStatus(const std::string& rid, bool isAvailable);
    //提供给排课使用的查询接口
    std::vector<std::shared_ptr<Classroom>> findAll();

    //检查教室是否在特定时间段空闲
    bool isRoomAvailable(const std::string& rid, const std::string& timeSlot);
private:
    ClassroomBroker();
    ClassroomBroker(const ClassroomBroker&) = delete;
    ClassroomBroker& operator=(const ClassroomBroker&) = delete;

    std::vector<std::shared_ptr<Classroom>> _classrooms;
};
//构造函数
ClassroomBroker::ClassroomBroker()
{
    initConnection();
}
//创建单例
ClassroomBroker& ClassroomBroker::singleton()
{
    static ClassroomBroker instance;
    return instance;
}
//创建数据表
void ClassroomBroker::createTable() {

    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS Classroom (
            id VARCHAR(20) PRIMARY KEY,
            building VARCHAR(50) NOT NULL,
            capacity INT NOT NULL,
            is_available BOOLEAN DEFAULT TRUE
        );
    )";
    query(sql);
}

//初始化数据表中的数据
void ClassroomBroker::initData() {
    // query("DELETE FROM Classroom;");//如果需要删除数据就取消注释
    _classrooms.clear();


    std::vector<Classroom> data = {
        {"A101", "一号教学楼", 50},
        {"B202", "二号教学楼", 30}
    };

    try {
        pqxx::work tx(*m_conn);
        for(auto& c : data) {

            tx.exec(
                "INSERT INTO Classroom(id, building, capacity, is_available) VALUES($1, $2, $3, TRUE) ON CONFLICT(id) DO NOTHING ",
                pqxx::params{c.m_roomNum, c.m_building, c.m_capacity}
            );
        }
        tx.commit();

        for(auto& c : data) {
            _classrooms.push_back(std::make_shared<Classroom>(c));
        }
        std::cout << "教室数据初始化完成。" << std::endl;
    } catch(const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
    }
}

//教室是否还存在
bool ClassroomBroker::isClassroomExists(const std::string& rid) const {
    for(auto& c : _classrooms) {
        if(c->hasId(rid)) return true;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT 1 FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();
        return !res.empty();
    } catch(...) {
        return false;
    }
}
//根据id找到对应的教室
std::shared_ptr<Classroom> ClassroomBroker::findById(const std::string& rid) {
    for(auto& c : _classrooms) {
        if(c->hasId(rid)) return c;
    }
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();

        if(!res.empty()) {
            const auto& r = res[0];
            auto c = std::make_shared<Classroom>(
                r["id"].as<std::string>(),
                r["building"].as<std::string>(),
                r["capacity"].as<int>()
            );
            c->updateAvailable(r["is_available"].as<bool>());
            _classrooms.push_back(c);
            return c;
        }
    } catch(const std::exception& e) {
        std::cerr << "查询教室失败: " << e.what() << std::endl;
    }
    return nullptr;
}

//添加教室
bool ClassroomBroker::addClassroom(const Classroom& classroom) {
    if(isClassroomExists(classroom.m_roomNum)) {
        std::cerr << "教室ID " << classroom.m_roomNum << " 已存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        tx.exec("INSERT INTO Classroom(id, building, capacity, is_available) VALUES($1, $2, $3, TRUE)",
            pqxx::params{classroom.m_roomNum, classroom.m_building, classroom.m_capacity});
        tx.commit();
        _classrooms.push_back(std::make_shared<Classroom>(classroom));
        std::cout << "教室 " << classroom.m_roomNum << " 添加成功" << std::endl;
        return true;
    } catch(const std::exception& e) {
        std::cerr << "添加教室失败: " << e.what() << std::endl;
        return false;
    }
}
//更新教室状态
bool ClassroomBroker::updateClassroomStatus(const std::string& rid, bool isAvailable) {
    if(!isClassroomExists(rid)) {
        std::cerr << "教室 " << rid << " 不存在" << std::endl;
        return false;
    }

    try {
        pqxx::work tx(*m_conn);
        std::string sql = "UPDATE Classroom SET is_available = ";
        sql += (isAvailable ? "TRUE" : "FALSE");
        sql += " WHERE id = '" + rid + "'";

        tx.exec(sql);
        tx.commit();

        // 更新缓存
        for(auto& c : _classrooms) {
            if(c->hasId(rid)) {
                c->updateAvailable(isAvailable);
                break;
            }
        }
        return true;
    } catch(const std::exception& e) {
        std::cerr << "更新状态失败: " << e.what() << std::endl;
        return false;
    }
}
//展示所有
std::vector<std::shared_ptr<Classroom>> ClassroomBroker::findAll() {
    std::vector<std::shared_ptr<Classroom>> result;
    try {
        pqxx::work tx(*m_conn);
        auto res = tx.exec("SELECT * FROM Classroom ORDER BY id");
        tx.commit();

        for(const auto& r : res) {
            auto c = std::make_shared<Classroom>(
                r["id"].as<std::string>(),
                r["building"].as<std::string>(),
                r["capacity"].as<int>()
            );
            c->updateAvailable(r["is_available"].as<bool>());
            result.push_back(c);
        }
    } catch(const std::exception& e) {
        std::cerr << "查询所有教室失败: " << e.what() << std::endl;
    }
    return result;
}
//删除教室
bool ClassroomBroker::deleteClassroom(const std::string& rid) {
    if(!isClassroomExists(rid)) {
        std::cerr << "教室 " << rid << " 不存在" << std::endl;
        return false;
    }
    try {
        pqxx::work tx(*m_conn);
        tx.exec("DELETE FROM Classroom WHERE id=$1", pqxx::params{rid});
        tx.commit();

        // 清理缓存
        _classrooms.erase(std::remove_if(_classrooms.begin(), _classrooms.end(), [&](const auto& c){ return c->hasId(rid); }),_classrooms.end());
        return true;
    } catch(const std::exception& e) {
        std::cerr << "删除教室失败: " << e.what() << std::endl;
        return false;
    }
}

//检测教室是否忙碌
bool ClassroomBroker::isRoomAvailable(const std::string& rid, const std::string& timeSlot) {
    try {
        pqxx::work tx(*m_conn);
        // 查询：在排课表中，该教室在该时间段是否有记录
        auto res = tx.exec("SELECT 1 FROM ClassroomSchedule WHERE classroom_id=$1 AND time_slot=$2",
            pqxx::params{rid, timeSlot});
        tx.commit();

        // 如果查到了记录，说明被占用了，返回 false
        return res.empty();
    } catch (...) {
        return false;
    }
}
