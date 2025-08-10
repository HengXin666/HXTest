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
        - 05-完美哈希
            - [gpt给的, 完美哈希源码, 但是数量太多(50)个就不行了](src/06-std-analyse/demo/05-pmh/01_test_pmh_map.cpp)
            - [复制粘贴其他的库来研究源码... 大部分都是编译期的](src/06-std-analyse/demo/05-pmh/02_cp_pmh_test.cpp)
            - [编译期完美哈希思想的比较探测的哈希表, 即便元素数量有1233个(我随便找的), 也毫无压力](src/06-std-analyse/demo/05-pmh/03_test_pmh.cpp)
            - [自己学习、注释、替换了第三方库的代码, 一个很小的可复现的code](src/06-std-analyse/demo/05-pmh/04_hx_pmh_map.cpp) (学习使用)

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
        - 10-临时对象声明周期探索实验
            - [实验代码](src/06-std-analyse/test/10-tmp-obj-test/01_tmp_obj_test.cpp)
            - [对返回 A&& 的实验探讨(ub无法展现)](src/06-std-analyse/test/10-tmp-obj-test/02_rxv_ub.cpp)
            - [A&& 导致的悬挂引用(实例)](src/06-std-analyse/test/10-10-tmp-obj-test/03_xv_ub.cpp)
        - 11-模板名字查找的实验
            - [模板函数与类中的模板函数在重载时候的区别](src/06-std-analyse/test/11-template-find/01_test_g_or_class.cpp) (全局模板函数指定具体类型时候, 是当场实例化, 不会二阶段查找; 而类中本身就知道本类的所有声明, 因此可以找到后面的方法声明)
            - [测试类的特化、偏特化、全特化在某些情况下是否有先后顺序问题](/src/06-std-analyse/test/11-template-find/02_partial_specialization.cpp) (有, 如果主模板被实例化, 而对应匹配的子模板没有实例化(声明顺序引起的), 就会导致原本期望匹配到特化上的, 却匹配到实例化上) (偏特化可以被二阶段查找, 但是如果指定了特化, 二阶段查找会被提前. 导致问题)
            - [类中也有声明顺序的问题(类方法)](/src/06-std-analyse/test/11-template-find/03_auto_in_class.cpp)
        - 12-测试反射
            - [编译期获取成员个数细节](/src/06-std-analyse/test/12-reflection/membersCount.cpp)
        - 13-宏魔法学习 ~~(图灵完备的哦)~~
            - [目录](src/06-std-analyse/test/13-MacroMagic/)
7. QT
   - 01-QML [QML-目录](./src/07-qt/01-qml/) | 笔记: [现代C++QT-QML](https://hengxin666.github.io/HXLoLi/docs/%E7%A8%8B%E5%BA%8F%E8%AF%AD%E8%A8%80/C++/%E7%8E%B0%E4%BB%A3C++/%E7%8E%B0%E4%BB%A3C++QT/QML/Window%E4%BB%8B%E7%BB%8D%E4%B8%B6%E5%88%9D%E8%AF%86)

8. OpenGL 图形学
    - [00-测试程序](src/08-OpenGL/demo/00-example/01_test_opengl_is_install.cpp)
    - [01-古代OpenGL-V2.0.0版本](src/08-OpenGL/demo/01-opengl-v200)
        - [01-OpenGL基本API函数](src/08-OpenGL/demo/01-opengl-v200/01_opengl_func.cpp)
        - [02-使用三角形绘制圆](src/08-OpenGL/demo/01-opengl-v200/02_opengl_yuan.cpp)
        - [03-回家作业:使用OpenGL绘制OpenCV的Logo](src/08-OpenGL/demo/01-opengl-v200/03_opengl_hw01.cpp) & [带动画版本](src/08-OpenGL/demo/01-opengl-v200/04_opengl_hw01_prime.cpp)

    - 03-三维模型的加载与相机控制
        - 回顾三角形大作: 从固定的点数量到动态点数量
            - [动态添加点: 鼠标点击三个点, 绘制三角形](03-src/08-OpenGL/demo/03-LoadingAndCameraControl/01_ClickAddPoint01.cpp)
            - [鼠标点击: 修改点的位置, 发现没有关联变化](src/08-OpenGL/demo/03-LoadingAndCameraControl/01_ClickAddPoint02.cpp)
            - [更新存储方式, 关联点变化](src/08-OpenGL/demo/03-LoadingAndCameraControl/01_ClickAddPoint03Ref.cpp)
        - [OBJ模型简单解析器 + 手动计算法线](src/08-OpenGL/demo/03-LoadingAndCameraControl/02_ObjParser01.cpp)
        - [OBJ模型简单解析器(支持解析法线和贴图)](src/08-OpenGL/demo/03-LoadingAndCameraControl/02_ObjParser02.cpp)
        - [已经支持摄像机5种模式的平直/平滑双🐒渲染](src/08-OpenGL/demo/03-LoadingAndCameraControl/03_CameraState.cpp)
        - [回家作业:使用OpenGL绘制OpenCV的Logo(3D带摄像机)](src/08-OpenGL/demo/03-LoadingAndCameraControl/04_hw.cpp)

9. OS & 平台API
    > 特别的, 下面为实验性质, 存在已知Bug, 并且未修复! 正确实现可以看 https://github.com/HengXin666/HXLibs
    - 01-io_uring
        - [基于协程的io_uring简易框架(目前是一个 **带超时** 的 `cin` 示例)](./src/09-os/demo/01-io_uring/02_io_uring_co.cpp)
            - [WhenAny 协程](src/09-os/include/coroutine/awaiter/WhenAny.hpp)
            - [UninitializedNonVoidVariant (擦除`void`类型的类型可重复的共用体)](src/09-os/include/tools/UninitializedNonVoidVariant.hpp) (也算的 `06` 的内容...) `// @todo 支持从可构造的进行构造, 而不是一定要类型一样 (std::string <- const char *)`
    - 02-thread
        - [线程池](src/09-os/demo/02-thread/01_cpp_threadPool.cpp) 一个现代的线程池, 支持动态扩容和返回值 (基于C++ STL API)
    - 03-iocp
        - [基于协程的IOCP简易框架](src/09-os/demo/03-iocp/01_iocp_test.cpp) 支持超时机制!
            - [TimerLoop](src/09-os/include/coroutine/loop/TimerLoop.hpp) 基于红黑树的协程定时器
            - [RootTask](src/09-os/include/coroutine/task/RootTask.hpp) 可以被分离的协程, 独自成为根协程 (内部必需要有合法的 `co_await`, 否则无法挂起以继续原协程, 从而会导致分离的任务直接运行到结束)

10. 赤石C++
    - 01-实现一个 `<-` 左指针运算符
        - [知己知皮: `->` 运算符重载实验](01-src/10-cs-code/demo/01-left-ptr/01_right_ptr.cpp)
        - [简单实现一个左指针运算符](src/10-cs-code/demo/01-left-ptr/02_left_ptr.cpp)
        - [通过宏封装, 让左指针运算符可以注册并反射生成代码, 并且支持左指针调用函数](src/10-cs-code/demo/01-left-ptr/03_left_ptr_macro.cpp)
        - [赤石 `<一` 运算符, 支持指针类型(雾)](src/10-cs-code/demo/01-left-ptr/04_left_pptr.cpp)
    - 02-解决循环依赖无法使用引用的问题 (通过模板二阶段名称查找)
        - [01-一个循环依赖的实例](./src/10-cs-code/demo/02-CircularDependency/01_what_is.cpp)
        - [02-加上模板变为二阶段名称查找](./src/10-cs-code/demo/02-CircularDependency/02_to_template.cpp)
        - [03-上宏隐藏细节](./02-src/10-cs-code/demo/02-CircularDependency/03_macro.cpp)