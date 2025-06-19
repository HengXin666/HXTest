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
            - [shared_ptr](./src/06-std-analyse/demo/03-memory/02_SharedPtr.cpp)

        - 04-sort
            - [sort](./src/06-std-analyse/demo/04-sort/01_sort.cpp) (一个支持迭代器、谓词的单边快排)

        - 坑:
            - std::is_convertible、 is_constructible 实现
    - 实验 & 学习
        - 01-使用模版名称二阶段查找, 规避循环依赖问题 (虽然没有任何实际用处)
            - [示例1](./ser/../src/06-std-analyse/test/01-tp-ForwardDeclaration/test_01.cpp)
        - 02-CRTP
            - [奇异递归模版 | 编译期多态](./src/06-std-analyse/test/02-crtp/01_crtp.cpp)
        - 03-协程
            - 暂时不提供示例(因为太乱了), 请见我的笔记: [协程速记](https://hengxin666.github.io/HXLoLi/docs/%E7%A8%8B%E5%BA%8F%E8%AF%AD%E8%A8%80/C++/%E7%8E%B0%E4%BB%A3C++/%E7%8E%B0%E4%BB%A3C++%E5%8D%8F%E7%A8%8B/%E5%8D%8F%E7%A8%8B%E9%80%9F%E8%AE%B0)
        - 04-使用`constexpr`检测UB
            - [`constexpr`检测UB | 常见UB大全](src/06-std-analyse/test/04-select-ub/01_ub_look.cpp)
        - 05-深入学习虚析构的各种情况, 并且简单实现了shared_ptr智能指针的类型擦除
            - [实验代码](src/06-std-analyse/test/05-vBaseClass/01-v_base_class.cpp)
        - 06-学习全局运算符重载和自定义字面量
            - [实验代码](src/06-std-analyse/test/06-my-op/01_my_op.cpp)
        - 07-类型萃取
            - [实验代码](src/06-std-analyse/test/07-TypeExtraction/01_type_extraction.cpp) (实现了 `is_void_v`/`remove_reference_t`/`is_same_v`)
        - 08-Lambda
            - [实验代码](src/06-std-analyse/test/08-Lambda/01_Lambda.cpp) (`auto&&` 做参数的 Lambda, 其实例化后, 类型也是不同的!(相当于延迟实例化了), 因此即便使用模板传参, 模板也一直模板...)
        - 09-auto
            - [对于auto返回值的讨论](src/06-std-analyse/test/09-auto/01_auto_return.cpp) 讨论了 `auto`、`auto&`、`auto&&`、`decltype(auto)` 在各种常见情况下作为函数返回值时候的效果, 对比其不同.
7. QT
   - 01-QML [QML-目录](./src/07-qt/01-qml/) | 笔记: [现代C++QT-QML](https://hengxin666.github.io/HXLoLi/docs/%E7%A8%8B%E5%BA%8F%E8%AF%AD%E8%A8%80/C++/%E7%8E%B0%E4%BB%A3C++/%E7%8E%B0%E4%BB%A3C++QT/QML/Window%E4%BB%8B%E7%BB%8D%E4%B8%B6%E5%88%9D%E8%AF%86)

8. OpenGL 图形学
    - [00-测试程序](src/08-OpenGL/demo/00-example/01_test_opengl_is_install.cpp)

    - [01-古代OpenGL-V2.0.0版本](src/08-OpenGL/demo/01-opengl-v200)
        - [01-OpenGL基本API函数](src/08-OpenGL/demo/01-opengl-v200/01_opengl_func.cpp)
        - [02-使用三角形绘制圆](src/08-OpenGL/demo/01-opengl-v200/02_opengl_yuan.cpp)
        - [03-回家作业:使用OpenGL绘制OpenCV的Logo](src/08-OpenGL/demo/01-opengl-v200/03_opengl_hw01.cpp) & [带动画版本](src/08-OpenGL/demo/01-opengl-v200/04_opengl_hw01_prime.cpp)

9. OS & 平台API
    - 01-io_uring
        - [基于协程的io_uring简易框架(目前是一个 **带超时** 的 `cin` 示例)](./src/09-os/demo/01-io_uring/02_io_uring_co.cpp)
            - [WhenAny 协程](src/09-os/include/coroutine/awaiter/WhenAny.hpp)
            - [UninitializedNonVoidVariant (擦除`void`类型的类型可重复的共用体)](src/09-os/include/tools/UninitializedNonVoidVariant.hpp) (也算的 `06` 的内容...) `// @todo 支持从可构造的进行构造, 而不是一定要类型一样 (std::string <- const char *)`
    - 02-thread
        - [线程池](src/09-os/demo/02-thread/01_cpp_threadPool.cpp) 一个现代的线程池, 支持动态扩容和返回值 (基于C++ STL API)
    - 03-iocp
        - [基于协程的IOCP简易框架](src/09-os/demo/03-iocp/01_iocp_test.cpp) 待支持超时.
            - [TimerLoop](src/09-os/include/coroutine/loop/TimerLoop.hpp) 基于红黑树的协程定时器