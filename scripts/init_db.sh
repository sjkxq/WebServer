#!/bin/bash
# 数据库初始化脚本
# 自动执行mysql.sql文件中的SQL命令

# 检查MySQL服务是否运行
if ! pgrep mysqld > /dev/null; then
    echo "MySQL服务未运行，请先启动MySQL服务"
    exit 1
fi

# 设置数据库连接参数
DB_USER="root"
DB_PASS="123456"  # 请根据实际情况修改密码
SQL_FILE="../sql/mysql.sql"

# 检查SQL文件是否存在
if [ ! -f "$SQL_FILE" ]; then
    echo "SQL文件 $SQL_FILE 不存在"
    exit 1
fi

# 执行SQL文件
echo "正在初始化数据库..."
mysql -u"$DB_USER" -p"$DB_PASS" < "$SQL_FILE"

# 检查执行结果
if [ $? -eq 0 ]; then
    echo "数据库初始化成功"
else
    echo "数据库初始化失败"
    exit 1
fi