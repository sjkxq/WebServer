#!/bin/bash

# 测试参数传递的脚本

generate_coverage() {
    echo "参数数量: $#"
    local i=1
    for arg in "$@"; do 
        echo "参数 $i: $arg"
        i=$((i+1))
    done
    
    # 检查参数
    local AUTO_INSTALL=false
    for arg in "$@"; do
        if [[ "$arg" == "--auto-install" ]]; then
            AUTO_INSTALL=true
            break
        fi
    done
    
    echo "AUTO_INSTALL 标志: $AUTO_INSTALL"
}

echo "直接调用测试:"
generate_coverage --auto-install

echo ""
echo "通过变量传递测试:"
generate_coverage "$@"