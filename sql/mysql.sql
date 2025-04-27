-- 建立yourdb库
create database webservedb;

-- // 创建user表
USE webservedb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

-- 添加数据
INSERT INTO user(username, password) VALUES('admin', '123456');