#include <iostream>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <cstring>


class Any {
    union Storage {
        void* ptr;
        char buf[sizeof(ptr)];
    };

    enum class ManagerAction {
        DELETE,
        CLONE,
    };
    
    template <typename Tp>
    struct ManagerSmall {
        static_assert(std::is_trivially_copyable_v<Tp>, "ManagerSmall requires trivially copyable types");
        // Static methods, so no need to create an object + encapsulation
        template <typename ... Args>
        static void construct_object(Storage& storage, Args&&... args) {
            ::new (storage.buf) Tp(std::forward<Args>(args)...);
        }
        static void* access_object(Storage& storage) {
            return reinterpret_cast<void*>(storage.buf);
        }
        static void destroy_object(Storage& storage) noexcept {
            Tp* ptr = reinterpret_cast<Tp*>(storage.buf);
            ptr->~Tp();
        }
        static Storage clone(const Storage& storage) noexcept {
            Storage temp_storage;
            memcpy(temp_storage.buf, storage.buf, sizeof(storage.buf));
            return temp_storage;
        }
    };

    template <typename Tp>
    struct ManagerBig {
        // Static methods, so no need to create an object + encapsulation
        template <typename ... Args>
        static void construct_object(Storage& storage, Args&&... args) {
            auto* temp = new Tp(std::forward<Args>(args)...);
            storage.ptr = temp;
        }
        static void* access_object(const Storage& storage) {
            return storage.ptr;
        }
        static void destroy_object(Storage& storage) noexcept {
            Tp* ptr = static_cast<Tp*>(storage.ptr);
            delete ptr;
        }
        static Storage clone(const Storage& storage) {
            Storage temp_storage;
            auto* ptr = static_cast<Tp*>(storage.ptr);
            temp_storage.ptr = new Tp(*ptr);
            return temp_storage;
        }
    };
    
    const std::type_info* current_object_type_{nullptr};
    Storage storage_;
    void (*destroy_func_)(Storage&){nullptr};
    Storage (*clone_func_)(const Storage&){nullptr};
public:
    template <typename Tp>
    using ValueType = typename std::decay_t<Tp>; // discard stuff like constness, volatilness and etc,

    template <typename Tp>
    using Manager = std::conditional_t<(sizeof(ValueType<Tp>) <= sizeof(Storage::buf) && 
        alignof(ValueType<Tp>) <= sizeof(Storage::buf) &&
        std::is_nothrow_move_constructible_v<ValueType<Tp>>), 
        ManagerSmall<ValueType<Tp>>, ManagerBig<ValueType<Tp>>>;
    // ManagerSmall for <= 8 bytes (x64) types to store on the stack
    // ManagerBig to store on the heap

    Any() = default;
    ~Any() {
        reset();
    }
    Any(const Any& rhs) {
        if (rhs.has_value()) {
            storage_ = rhs.clone_func_(rhs.storage_); // Cloning underlying value
            current_object_type_ = rhs.current_object_type_;
            destroy_func_ = rhs.destroy_func_;
            clone_func_ = rhs.clone_func_;
        }
    }
    Any& operator=(const Any& rhs) {
        auto temp = Any(rhs); // Value is cloned we're all good
        swap(temp);
        return *this;
    }
    Any(Any&& rhs) noexcept
    : current_object_type_(rhs.current_object_type_),
      storage_(std::move(rhs.storage_)),
      destroy_func_(rhs.destroy_func_),
      clone_func_(rhs.clone_func_)
    {
        std::memset((void*)&rhs.storage_, 0, sizeof(rhs.storage_));
        rhs.reset();
    }
    Any& operator=(Any&& rhs) noexcept {
        if (!rhs.has_value()) {
            reset();
        } else if (this != &rhs) {
            swap(rhs); // No need to reset this, cuz it will be swapped and i will reset rhs
            rhs.reset();
        }
        return *this;
    }
    
    template <typename Tp, typename = std::enable_if_t<std::is_copy_constructible_v<ValueType<Tp>> && !std::is_same_v<ValueType<Tp>, Any>, void>>
    Any(Tp&& value) {
        do_emplace<Tp>(value);
    }

    template<typename Tp, typename ... Args>
    std::enable_if_t<std::is_copy_constructible_v<ValueType<Tp>>, void> emplace(Args&&... args) {
        do_emplace<Tp>(args...);
    }

    bool has_value() const {
        return current_object_type_ != nullptr; // If object type isn't set, then 100% any is empty kinda
    }

    void reset() noexcept {
        if (has_value()) {
            current_object_type_ = nullptr;
            destroy_func_(storage_);
            destroy_func_ = nullptr;
            clone_func_ = nullptr;
        }
    }
    
    void swap(Any& rhs) noexcept {
        std::swap(storage_, rhs.storage_);
        std::swap(current_object_type_, rhs.current_object_type_);
        std::swap(destroy_func_, rhs.destroy_func_);
        std::swap(clone_func_, rhs.clone_func_);
    }

    const std::type_info& type() const {
        return *current_object_type_;
    }
    template<typename Tp>
    friend Tp anycast_impl(Any& any);
    template<typename Tp>
    Tp* anycast(Any* any);
private:    
    template <typename Tp, typename ... Args>
    void do_emplace(Args&&... args) {
        using Decayed = ValueType<Tp>; // Decay and remove const-volatile if there
        reset();
        Manager<Decayed>::construct_object(storage_, args...); // Construct an object

        current_object_type_ = &typeid(Decayed); // Save it's type info for anycasting 
        destroy_func_ = Manager<Decayed>::destroy_object;
        clone_func_ = Manager<Decayed>::clone;
    }
};
template<typename Tp>
Tp anycast_impl(Any& any) {
    using Decayed = typename std::remove_reference_t<Tp>;
    using ManagerT = typename Any::Manager<Decayed>;

    if (!any.has_value() || typeid(Decayed) != *any.current_object_type_) {
        throw std::bad_cast();
    }
    void* object = ManagerT::access_object(any.storage_);
    return *static_cast<Decayed*>(object);
}

template <typename Tp>
Tp anycast(Any& any) {
    return anycast_impl<Tp>(any);
}

template <typename Tp>
Tp anycast(const Any& any) {
    return anycast_impl<Tp>(const_cast<Any&>(any));
}
