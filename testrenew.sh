#!/bin/bash
# renewOS 自动测试脚本
# 执行清理和构建流程，并提供文件系统创建的选项

echo "=== renewOS 自动测试脚本 ==="
echo "正在执行测试流程..."

# 清理所有文件
echo -n "1. 清理所有文件... "
make cleanall > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ 完成"
else
    echo "✗ 失败"
    exit 1
fi

# 构建内核和用户程序
echo -n "2. 构建内核和用户程序... "
make build > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ 完成"
else
    echo "✗ 失败"
    exit 1
fi

echo "=== 自动构建流程执行完毕 ==="
echo ""

# 询问用户是否继续创建文件系统和运行
read -p "是否要继续创建文件系统并运行系统？(y/n): " answer

if [ "$answer" = "y" ] || [ "$answer" = "Y" ]; then
    echo -n "3. 创建文件系统... "
    make fs > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✓ 完成"
        echo -n "4. 运行系统... "
        make run
    else
        echo "✗ 失败"
        exit 1
    fi
else
    echo "已取消后续操作。您可以手动执行：sudo make fs && make run"
fi

echo ""
echo "=== 测试流程执行完毕 ==="