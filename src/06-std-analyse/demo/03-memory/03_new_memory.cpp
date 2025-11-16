#include <type_traits>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace hx {

namespace internal {

template <typename T, typename = void>
struct IsDeleterTypeImpl : std::false_type {};

template <typename T>
struct IsDeleterTypeImpl<T, std::void_t<decltype(std::declval<T>()(nullptr))>> : std::true_type {};

}  // namespace internal

// TODO(hengxinli): 再看看命名规范qwq
template <typename T>
constexpr bool kHasDeleteNoExcept = noexcept(delete std::declval<T>());

template <typename T>
constexpr bool kHasDereferenceNoExcept = noexcept(*std::declval<T>());

template <typename Base, typename T>
constexpr bool kIsBaseOrSameType = std::is_base_of_v<Base, T> || std::is_same_v<Base, T>;

template <typename T>
constexpr bool kIsDeleterType = internal::IsDeleterTypeImpl<T>::value;

/**
 * @brief 默认删除器
 * @tparam T
 */
template <typename T>
struct DefaultDeleter {
  using PointerType = T*;
  using ElementType = T;

  void operator()(PointerType ptr) noexcept(kHasDeleteNoExcept<PointerType>) { delete ptr; }
};

}  // namespace hx

namespace hx {


template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
  // 本库规定: 删除器, 其删除必须要为 noexcept, 否则是 ub
  // static_assert(noexcept(std::declval<Deleter>()(nullptr)) == true,
  //   "The deleter must be noexcept");

  // 保证 Deleter () 可以传入 ptr
  static_assert(kIsDeleterType<Deleter> == true, "It is necessary to ensure that the deleter can accept a pointer.");

 public:
  using PointerType = T*;
  using ElementType = T;

  constexpr UniquePtr() = default;
  constexpr explicit UniquePtr(std::nullptr_t) : UniquePtr() {}

  explicit UniquePtr(PointerType ptr) : t_{ptr, Deleter{}} {}

  explicit UniquePtr(PointerType ptr, Deleter deleter) : t_{ptr, deleter} {}

  UniquePtr(UniquePtr const&) = delete;
  UniquePtr& operator=(UniquePtr const&) = delete;

  UniquePtr(UniquePtr&& that) noexcept : t_{std::move(that.t_)} { that.GetPtrRef() = nullptr; }

  UniquePtr& operator=(UniquePtr&& that) noexcept {
    using std::swap;
    swap(t_, that.t_);
    return *this;
  }

  ~UniquePtr() noexcept { Del(); }

  // C++11 特殊: explicit operator bool 在 if 等判断语意下, 可隐式转换
  explicit operator bool() const noexcept { return std::get<PointerType>(t_); }

  PointerType operator->() const noexcept { return std::get<PointerType>(t_); }

  ElementType& operator*() noexcept(kHasDereferenceNoExcept<PointerType>) { return *Get(); }

  ElementType const& operator*() const noexcept(kHasDereferenceNoExcept<PointerType const>) {
    // 因为标准说对空指针解引用是ub, 所以我们假设用户不会这样调用
    return *Get();
  }

  PointerType Get() const noexcept { return std::get<PointerType>(t_); }

  Deleter& GetDeleter() noexcept { return std::get<Deleter>(t_); }

  Deleter const& GetDeleter() const noexcept { return std::get<Deleter>(t_); }

  void Reset(PointerType ptr = nullptr) noexcept {
    Del();
    GetPtrRef() = ptr;
  }

  PointerType Release() noexcept {
    auto* ptr = GetPtrRef();
    GetPtrRef() = nullptr;
    return ptr;
  }

  // 为了适配 std::swap, 满足某些情况下可以被 ADL 查找, 因此使用 swap 作为方法名. 而不是 Swap
  void swap(UniquePtr& that) noexcept { std::swap(t_, that.t_); }

 private:
  auto& GetPtrRef() noexcept { return std::get<PointerType>(t_); }

  void Del() noexcept {
    if (auto&& ptr = GetPtrRef()) {
      GetDeleter()(ptr);
      ptr = nullptr;
    }
  }

  // EBO 优化
  std::tuple<PointerType, Deleter> t_;
};

/**
 * @brief 独占智能指针工厂函数
 * @tparam T 元素类型
 * @tparam Args 构造函数参数类型
 * @param args 构造函数参数
 * @return UniquePtr<T>
 */
template <typename T, typename... Args>
UniquePtr<T> MakeUniquePtr(Args&&... args) {
  return UniquePtr{new T(std::forward<Args>(args)...)};
}


}  // namespace hx

#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace hx {

namespace internal {

/**
 * @brief 共享智能指针支持类型擦除
 * @note 理论上是擦除了任意类型!, 使用者应该约束只能类型相应方可使用 (如基类转换)
 */
struct TypeErasurePtr {
  virtual void* Release() noexcept = 0;
  virtual ~TypeErasurePtr() noexcept = default;
  
  template <typename Ptr>
  Ptr Get() const noexcept {
    return static_cast<Ptr>(Get());
  }

  template <typename DeleterPtr>
  std::add_const_t<DeleterPtr> GetDeleter() const noexcept {
    return static_cast<std::add_const_t<DeleterPtr>>(GetDeleter());
  }

private:
  virtual void* Get() const noexcept = 0;
  virtual void const* GetDeleter() const noexcept = 0;
};

template <typename T, typename Deleter = DefaultDeleter<T>>
struct TypeErasurePtrImpl final : public TypeErasurePtr {
  // 保证 Deleter () 可以传入 ptr
  static_assert(kIsDeleterType<Deleter> == true, "It is necessary to ensure that the deleter can accept a pointer.");

  using PointerType = T*;
  using ElementType = T;

  TypeErasurePtrImpl() = default;

  explicit TypeErasurePtrImpl(PointerType ptr) : ptr_{ptr} {}

  explicit TypeErasurePtrImpl(PointerType ptr, Deleter deleter) : ptr_{ptr, std::move(deleter)} {}

  void* Release() noexcept override { return ptr_.Release(); }
  
  ~TypeErasurePtrImpl() noexcept override { ptr_.Reset(); }
  
private:
  void* Get() const noexcept override { return ptr_.Get(); }
  void const* GetDeleter() const noexcept override { return static_cast<void const*>(&ptr_.GetDeleter()); }

  UniquePtr<T, Deleter> ptr_;
};

/**
 * @brief 引用计数块
 */
struct RefCntBlock {
  RefCntBlock() = default;

  explicit RefCntBlock(std::size_t shared_cnt, std::size_t weak_cnt) : shared_cnt_{shared_cnt}, weak_cnt_{weak_cnt} {}

  std::size_t AddSharedCnt() noexcept { return ++shared_cnt_; }

  std::size_t AddWeakCnt() noexcept { return ++weak_cnt_; }

  std::size_t ReduceSharedCnt() noexcept {
    // 因为合法的调用下, shared_cnt_ >= 1, 因此不用担心 shared_cnt_ == 0 情况
    // 因为合法正确的使用下, shared_cnt_ == 0 后就被释放了
    return --shared_cnt_;
  }

  std::size_t ReduceWeakCnt() noexcept { return --weak_cnt_; }

  std::size_t WeakCnt() const noexcept { return weak_cnt_; }

  std::size_t SharedCnt() const noexcept { return shared_cnt_; }

 private:
  std::atomic_size_t shared_cnt_{};
  std::atomic_size_t weak_cnt_{};
};

}  // namespace internal

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr {
  template <typename>
  friend class SharedPtr;

  template <typename>
  friend class WeakPtr;

 public:
  using PointerType = T*;
  using ElementType = T;

  constexpr SharedPtr() = default;
  constexpr explicit SharedPtr(std::nullptr_t) : SharedPtr() {}

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit SharedPtr(U* ptr)
      : ptr_{new internal::TypeErasurePtrImpl<U>{ptr}}, ref_cnt_block_{new internal::RefCntBlock{1, 0}} {}

  template <typename U, typename Deleter, std::enable_if_t<kIsBaseOrSameType<T, U> && kIsDeleterType<Deleter>, int> = 0>
  explicit SharedPtr(U* ptr, Deleter deleter)
      : ptr_{new internal::TypeErasurePtrImpl<U, Deleter>{ptr, std::move(deleter)}},
        ref_cnt_block_{new internal::RefCntBlock{1, 0}} {}

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  SharedPtr(SharedPtr<U> const& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    if (ref_cnt_block_) {
      ref_cnt_block_->AddSharedCnt();
    }
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  SharedPtr(SharedPtr<U>&& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    that.ptr_ = nullptr;
    that.ref_cnt_block_ = nullptr;
  }

  // 仅 weak_ptr 使用
  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit SharedPtr(WeakPtr<U> const& that) noexcept;

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  SharedPtr& operator=(SharedPtr<U> const& that) {
    // tip: 因为至少传入的 that 智能指针是同一线程的, 所以不会出现竞争,
    // 否则 如果存在可能 that 是悬挂, 那本调用为ub; 本身就非法
    ptr_ = that.ptr_;
    ref_cnt_block_ = that.ref_cnt_block_;
    if (ptr_ && ref_cnt_block_) {
      ref_cnt_block_->AddSharedCnt();
    }
    return *this;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  SharedPtr& operator=(SharedPtr<U>&& that) noexcept {
    if (this == std::addressof(that)) {
      return *this;
    }
    if constexpr (std::is_same_v<T, U>) {
      swap(that);
    } else {
      Del();
      ptr_ = that.ptr_;
      ref_cnt_block_ = that.ref_cnt_block_;
      that.ptr_ = nullptr;
      that.ref_cnt_block_ = nullptr;
    }
    return *this;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  SharedPtr& operator=(UniquePtr<U>&& that) {
    Del();
    ptr_ = new internal::TypeErasurePtrImpl<U>{that.Release()};
    ref_cnt_block_ = new internal::RefCntBlock{1, 0};
    return *this;
  }

  void Reset() noexcept { Del(); }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  void Reset(U* ptr) {
    *this = UniquePtr<U>{ptr};
  }

  template <typename U, typename Deleter, std::enable_if_t<kIsBaseOrSameType<T, U> && kIsDeleterType<Deleter>, int> = 0>
  void Reset(U* ptr, Deleter deleter) {
    *this = UniquePtr<U, Deleter>{ptr, std::move(deleter)};
  }

  PointerType Release() noexcept {
    if (ptr_ == nullptr) {
      return nullptr;
    }
    auto* ptr = ptr_->Release();
    delete ptr_;
    ptr_ = nullptr;
    if (ref_cnt_block_ && ref_cnt_block_->ReduceSharedCnt() == 0 && ref_cnt_block_->WeakCnt() == 0) {
      delete ref_cnt_block_;
      ref_cnt_block_ = nullptr;
    }
    return ptr;
  }

  // 为了适配 std::swap, 满足某些情况下可以被 ADL 查找, 因此使用 swap 作为方法名. 而不是 Swap
  void swap(SharedPtr& that) noexcept {
    std::swap(ptr_, that.ptr_);
    std::swap(ref_cnt_block_, that.ref_cnt_block_);
  }

  PointerType Get() noexcept { return ptr_ ? ptr_->Get<PointerType>() : nullptr; }

  PointerType Get() const noexcept { return ptr_ ? ptr_->Get<PointerType>() : nullptr; }

  explicit operator bool() const noexcept { return Get() != nullptr; }

  ElementType& operator*() noexcept(kHasDereferenceNoExcept<PointerType>) { return *Get(); }

  ElementType const& operator*() const noexcept(kHasDereferenceNoExcept<PointerType const>) { return *Get(); }

  PointerType operator->() noexcept { return Get(); }

  PointerType operator->() const noexcept { return Get(); }

  std::size_t UseCount() const noexcept { return ref_cnt_block_ ? ref_cnt_block_->SharedCnt() : 0u; }

  ~SharedPtr() noexcept { Del(); }

 private:
  void Del() noexcept {
    // 如果 ptr_ != nullptr, 那么 ref_cnt_block_ 保证一定 != nullptr
    if (ptr_) {
      if (ref_cnt_block_->ReduceSharedCnt() == 0) {
        delete ptr_;
        ptr_ = nullptr;
        // 判断 weak 引用
        if (ref_cnt_block_->WeakCnt() == 0) {
          delete ref_cnt_block_;
          ref_cnt_block_ = nullptr;
        }
      }
    } else if (ref_cnt_block_ && ref_cnt_block_->WeakCnt() == 0) {
      delete ref_cnt_block_;
      ref_cnt_block_ = nullptr;
    }
  }

  internal::TypeErasurePtr* ptr_{nullptr};
  internal::RefCntBlock* ref_cnt_block_{nullptr};

  template <typename Deleter, typename U>
  friend Deleter* GetDeleter(SharedPtr<U>& ptr) noexcept;
};

template <typename T, typename... Args>
inline SharedPtr<T> MakeShared(Args&&... args) {
  return SharedPtr<T>{new T(std::forward<Args>(args)...)};
}

template <typename Deleter, typename T>
inline Deleter* GetDeleter(SharedPtr<T>& ptr) noexcept {
  return ptr ? ptr.ptr_->template GetDeleter<Deleter>() : nullptr;
}

template <typename T>
class WeakPtr {
  template <typename>
  friend class SharedPtr;

 public:
  using PointerType = T*;
  using ElementType = T;

  constexpr WeakPtr() = default;
  constexpr explicit WeakPtr(std::nullptr_t) : WeakPtr() {}

  WeakPtr(WeakPtr const& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    if (ref_cnt_block_) {
      ref_cnt_block_->AddWeakCnt();
    }
  }

  WeakPtr(WeakPtr&& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    that.ptr_ = nullptr;
    that.ref_cnt_block_ = nullptr;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit WeakPtr(SharedPtr<U> const& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    if (ref_cnt_block_) {
      ref_cnt_block_->AddWeakCnt();
    }
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit WeakPtr(SharedPtr<U>&& that) noexcept : WeakPtr(that) {}

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit WeakPtr(WeakPtr<U>&& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    that.ptr_ = nullptr;
    that.ref_cnt_block_ = nullptr;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  explicit WeakPtr(WeakPtr<U> const& that) noexcept : ptr_{that.ptr_}, ref_cnt_block_{that.ref_cnt_block_} {
    if (ref_cnt_block_) {
      ref_cnt_block_->AddWeakCnt();
    }
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  WeakPtr& operator=(WeakPtr<U> const& that) {
    ptr_ = that.ptr_;
    ref_cnt_block_ = that.ref_cnt_block_;
    return *this;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  WeakPtr& operator=(WeakPtr<U>&& that) noexcept {
    if (this == std::addressof(that)) {
      return *this;
    }
    Del();
    ptr_ = that.ptr_;
    ref_cnt_block_ = that.ref_cnt_block_;
    that.ptr_ = nullptr;
    that.ref_cnt_block_ = nullptr;
    return *this;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  WeakPtr& operator=(SharedPtr<U> const& that) noexcept {
    Del();
    ptr_ = that.ptr_;
    ref_cnt_block_ = that.ref_cnt_block_;
    if (ref_cnt_block_) {
      ref_cnt_block_->AddWeakCnt();
    }
    return *this;
  }

  template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int> = 0>
  WeakPtr& operator=(SharedPtr<U>&& that) noexcept {
    return operator=(that);
  }

  /**
   * @brief 检查被引用的对象是否已删除
   * @return true 已删除
   * @return false 未删除
   */
  bool Expired() const noexcept { return UseCount() == 0u; }

  std::size_t UseCount() const noexcept { return ref_cnt_block_ ? ref_cnt_block_->SharedCnt() : 0u; }

  SharedPtr<T> Lock() const noexcept { return Expired() ? SharedPtr<T>{nullptr} : SharedPtr<T>{*this}; }

  // 为了适配 std::swap, 满足某些情况下可以被 ADL 查找, 因此使用 swap 作为方法名. 而不是 Swap
  void swap(WeakPtr& that) noexcept {
    std::swap(ptr_, that.ptr_);
    std::swap(ref_cnt_block_, that.ref_cnt_block_);
  }

  void Reset() noexcept { Del(); }

  ~WeakPtr() noexcept { Del(); }

 private:
  void Del() noexcept {
    ptr_ = nullptr;
    if (ref_cnt_block_ && ref_cnt_block_->ReduceWeakCnt() == 0 && ref_cnt_block_->SharedCnt() == 0) {
      delete ref_cnt_block_;
      ref_cnt_block_ = nullptr;
    }
  }

  internal::TypeErasurePtr* ptr_{nullptr};
  internal::RefCntBlock* ref_cnt_block_{nullptr};
};

template <typename T>
template <typename U, std::enable_if_t<kIsBaseOrSameType<T, U>, int>>
SharedPtr<T>::SharedPtr(WeakPtr<U> const& that) noexcept
    : ptr_{that.ref_cnt_block && that.ref_cnt_block_->SharedCnt() ? that.ptr_ : nullptr}, ref_cnt_block_{that.ref_cnt_block_} {
  if (ptr_) {
    ref_cnt_block_->AddSharedCnt();
  }
}

}  // namespace hx


struct DelExcept {
  ~DelExcept() noexcept(false) {}
};

static_assert(noexcept(std::unique_ptr<DelExcept>{}.~unique_ptr()) == true);
static_assert(noexcept(std::shared_ptr<DelExcept>{}.~shared_ptr()) == true);
static_assert(noexcept(hx::UniquePtr<DelExcept>{}.~UniquePtr()) == true); // gcc 10 以下会报错
static_assert(noexcept(hx::SharedPtr<DelExcept>{}.~SharedPtr()) == true);

class A {
public:
    ~A() noexcept {
        std::cout << "~A\n";
    }
};

class B : public A {
public:
    ~B() noexcept {
        std::cout << "~B\n";
    }
};

template <typename T>
using S_Ptr =
#if 1
hx::SharedPtr<T>;
#else
std::shared_ptr<T>;
#endif // !0

int main() {
    S_Ptr<B> ptr{new B{}};
    // [[maybe_unused]] auto ptr_2 = ptr;
    // [[maybe_unused]] auto ptr_3 = std::move(ptr);

    [[maybe_unused]] auto ptr_4 = S_Ptr<A>{ptr};
    [[maybe_unused]] auto ptr_5 = S_Ptr<A>{std::move(ptr)};
}
