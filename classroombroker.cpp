#include "classroombroker.h"

// 仅在实现文件中包含pqxx（解决头文件嵌套/冲突问题）
// #include <pqxx/pqxx>
#include <iostream>
#include <stdexcept>

// ===== 单例实现 =====
ClassroomBroker& ClassroomBroker::singleton() {
    static ClassroomBroker instance;  // 局部静态变量，保证全局唯一、线程安全（C++11+）
    return instance;
}

// ===== 内部工具方法：检查教室是否存在 =====
bool ClassroomBroker::isClassroomExists(const std::string& classroomId) {
    try {
        // 1. 创建事务（基于基类的数据库连接m_conn）
        pqxx::work txn(*m_conn);
        // 2. 执行查询
        std::string sql = "SELECT id FROM classroom WHERE id = '" + txn.esc(classroomId) + "'";
        pqxx::result res = txn.exec(sql);
        // 3. 提交事务（只读查询也建议提交，避免事务残留）
        txn.commit();
        // 4. 返回结果：非空则存在
        return !res.empty();
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 检查教室存在失败：" << e.what() << std::endl;
        return false;
    }
}

// ===== 重写基类方法：创建教室表 =====
void ClassroomBroker::createTable() {
    try {
        pqxx::work txn(*m_conn);
        // SQL语句：创建教室表，字段和Domain层Classroom实体对应，但只存数据（无业务逻辑）
        std::string createSql = R"(
            CREATE TABLE IF NOT EXISTS classroom (
                id VARCHAR(20) PRIMARY KEY,       -- 教室ID（主键）
                building VARCHAR(50) NOT NULL,    -- 教学楼名称
                capacity INT NOT NULL,            -- 教室容量
                is_available BOOLEAN DEFAULT TRUE -- 是否可用
            )
        )";
        txn.exec(createSql);
        txn.commit();
        std::cout << "[ClassroomBroker] 教室表创建/检查完成" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 创建教室表失败：" << e.what() << std::endl;
        throw std::runtime_error("教室表初始化失败"); // 抛异常让上层处理
    }
}

// ===== 重写基类方法：初始化测试数据 =====
void ClassroomBroker::initData() {
    // 初始化2个测试教室（仅当不存在时添加）
    if (!isClassroomExists("A101")) {
        addClassroom(domain::Classroom("A101", "一号教学楼", 50));
    }
    if (!isClassroomExists("B202")) {
        addClassroom(domain::Classroom("B202", "二号教学楼", 30));
    }
    std::cout << "[ClassroomBroker] 教室测试数据初始化完成" << std::endl;
}

// ===== 核心接口：根据ID查询教室 =====
domain::Classroom ClassroomBroker::findById(const std::string& classroomId) {
    try {
        pqxx::work txn(*m_conn);
        std::string sql = "SELECT * FROM classroom WHERE id = '" + txn.esc(classroomId) + "'";
        pqxx::result res = txn.exec(sql);
        txn.commit();

        // 无数据：返回空的Classroom实体（Domain层自行判断有效性）
        if (res.empty()) {
            std::cerr << "[ClassroomBroker] 教室ID " << classroomId << " 不存在" << std::endl;
            return domain::Classroom();
        }

        // 有数据：转换为Domain层的Classroom实体（仅赋值，无业务逻辑）
        const auto& row = res[0];
        domain::Classroom classroom(
            row["id"].as<std::string>(),
            row["building"].as<std::string>(),
            row["capacity"].as<int>()
        );
        // 设置可用状态（Domain层实体的属性）
        classroom.setAvailable(row["is_available"].as<bool>());
        return classroom;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 查询教室失败：" << e.what() << std::endl;
        return domain::Classroom();
    }
}

// ===== 核心接口：添加新教室 =====
bool ClassroomBroker::addClassroom(const domain::Classroom& classroom) {
    // 前置校验：调用Domain层的校验逻辑（DM层不做业务校验，只做数据校验）
    if (!classroom.isValid()) {
        std::cerr << "[ClassroomBroker] 教室实体无效，添加失败" << std::endl;
        return false;
    }
    // 检查是否已存在
    if (isClassroomExists(classroom.getId())) {
        std::cerr << "[ClassroomBroker] 教室ID " << classroom.getId() << " 已存在，添加失败" << std::endl;
        return false;
    }

    try {
        pqxx::work txn(*m_conn);
        // 拼接SQL（使用txn.esc防止SQL注入）
        std::string insertSql = "INSERT INTO classroom (id, building, capacity, is_available) VALUES ("
                                "'" + txn.esc(classroom.getId()) + "', "
                                "'" + txn.esc(classroom.getBuilding()) + "', "
                                "" + std::to_string(classroom.getCapacity()) + ", "
                                "" + (classroom.isAvailable() ? "TRUE" : "FALSE") + ")";
        txn.exec(insertSql);
        txn.commit();
        std::cout << "[ClassroomBroker] 教室 " << classroom.getId() << " 添加成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 添加教室失败：" << e.what() << std::endl;
        return false;
    }
}

// ===== 核心接口：更新教室可用状态 =====
bool ClassroomBroker::updateClassroomStatus(const std::string& classroomId, bool isAvailable) {
    if (!isClassroomExists(classroomId)) {
        std::cerr << "[ClassroomBroker] 教室ID " << classroomId << " 不存在，更新状态失败" << std::endl;
        return false;
    }

    try {
        pqxx::work txn(*m_conn);
        std::string updateSql = "UPDATE classroom SET is_available = " + (isAvailable ? "TRUE" : "FALSE") +
                                " WHERE id = '" + txn.esc(classroomId) + "'";
        txn.exec(updateSql);
        txn.commit();
        std::cout << "[ClassroomBroker] 教室 " << classroomId << " 状态更新为：" << (isAvailable ? "可用" : "不可用") << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 更新教室状态失败：" << e.what() << std::endl;
        return false;
    }
}

// ===== 核心接口：查询所有教室 =====
std::vector<domain::Classroom> ClassroomBroker::findAll() {
    std::vector<domain::Classroom> classrooms;
    try {
        pqxx::work txn(*m_conn);
        std::string sql = "SELECT * FROM classroom ORDER BY id";
        pqxx::result res = txn.exec(sql);
        txn.commit();

        // 遍历结果，转换为Domain层实体
        for (const auto& row : res) {
            domain::Classroom classroom(
                row["id"].as<std::string>(),
                row["building"].as<std::string>(),
                row["capacity"].as<int>()
            );
            classroom.setAvailable(row["is_available"].as<bool>());
            classrooms.push_back(classroom);
        }
        std::cout << "[ClassroomBroker] 查询到 " << classrooms.size() << " 个教室" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 查询所有教室失败：" << e.what() << std::endl;
    }
    return classrooms;
}

// ===== 核心接口：删除教室 =====
bool ClassroomBroker::deleteClassroom(const std::string& classroomId) {
    if (!isClassroomExists(classroomId)) {
        std::cerr << "[ClassroomBroker] 教室ID " << classroomId << " 不存在，删除失败" << std::endl;
        return false;
    }

    try {
        pqxx::work txn(*m_conn);
        std::string deleteSql = "DELETE FROM classroom WHERE id = '" + txn.esc(classroomId) + "'";
        txn.exec(deleteSql);
        txn.commit();
        std::cout << "[ClassroomBroker] 教室 " << classroomId << " 删除成功" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ClassroomBroker] 删除教室失败：" << e.what() << std::endl;
        return false;
    }
}

