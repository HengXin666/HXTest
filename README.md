<h1 align="center" style="color:yellow">HXTest</h1>

Heng_Xin 学习C++, 并且做实验所使用的项目.

## 个人学习笔记

1. [C++无宏反射](./src/01-reflection/C++无宏反射.md)

2. [inline作用(链接属性)](./src/02-link-property/inline关键字的作用.md)

3. [C++11现代伪随机数生成](./src/03-random/现代伪随机数生成.md)

4. [tbb-理解常用并行算法及其实现原理](./src/04-tbb/tbb-理解常用并行算法及其实现原理.md)

5. [现代C++异常与`noexcept`表达式 (仅写了代码)](./src/05-exception/demo/01-noexcept/01_main.cpp)

6. 长期需更新: 
    - std的剖析! (包含一些stl源码的实现)
        - 01-constexpr `编译期操作`
            - [编译期if](./src/06-std-analyse/demo/01-constexpr/01_if.cpp) (待更新C++11的基于模版的实现)
            - [编译期for](./src/06-std-analyse/demo/01-constexpr/02_for.cpp) (递归/迭代的实现) (内含`std::make_index_constexpr系列`と`std::visit`の秘密)
            - [玩具: 编译期toString](./src/06-std-analyse/demo/01-constexpr/03_toString.cpp)
            - [玩具: 编译期求质数、质数筛(性能较差)](./src/06-std-analyse/demo/01-constexpr/04_primeNumber.cpp)
        - 02-tuple
            - [tuple](./src/06-std-analyse/demo/02-tuple/01_tuple.cpp) (包含`get(tuple)`、`tuple_size_v`、`make_tuple`、`tie`的实现, *~~引用多态真奇妙~~*)
            - [trivially_copyable的tuple](./src/06-std-analyse/demo/02-tuple/02_tuple.cpp)
        
        - 03-memory
            - [unique_ptr](./src/06-std-analyse/demo/03-memory/01_UniquePtr.cpp)

        - 坑:
            - std::is_convertible、 is_constructible 实现
    - 实验 & 学习
        - 01-使用模版名称二阶段查找, 规避循环依赖问题 (虽然没有任何实际用处)
            - [示例1](./ser/../src/06-std-analyse/test/01-tp-ForwardDeclaration/test_01.cpp)
        - 02-CRTP
            - [奇异递归模版 | 编译期多态](./src/06-std-analyse/test/02-crtp/01_crtp.cpp)