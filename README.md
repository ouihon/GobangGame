# 主要实现功能
* 实现双人五子棋或双人黑白棋的基本功能。即一人用鼠标操作下棋，一人用键盘操作下棋；
* 实现胜负判决的功能
* 实现悔棋等功能

# 操作
* 小黑子：鼠标
* 白子：方向键+空格

# 不用VS的编译语句(C99标准)
```
gcc main.c -o main.exe -lgdi32
```

# 用到的库
WIN32 SDK，只能在Windows上编译运行

# 主要文件
* main.exe 主程式
* main.c 主程式源代码
* doublectrl.c 黑白子都可以双控（键盘鼠标）源代码
* MyWindows.c 原项目 [wangying2016/GobangGame](https://github.com/wangying2016/GobangGame) 代码，实现基本功能

# 演示

![白子落子](/pics/007.png "白子落子")

![黑子落子](/pics/008.png "黑子落子")

![悔棋](/pics/030.png "悔棋")

![悔棋失败](/pics/033.png "悔棋失败")

![胜利](/pics/018.png "胜利")

# 特别鸣谢
项目 [wangying2016/GobangGame](https://github.com/wangying2016/GobangGame)