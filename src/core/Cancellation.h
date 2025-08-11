#pragma once
#include <atomic>
#include <functional>
class CancellationToken {
public:
    CancellationToken() : flag_(nullptr) {}
    explicit CancellationToken(std::atomic_bool* f) : flag_(f) {}
    bool cancelled() const { return flag_ && flag_->load(std::memory_order_relaxed); }
    explicit operator bool() const { return flag_ != nullptr; }
private:
    std::atomic_bool* flag_;
};
class CancellationSource {
public:
    CancellationSource() : flag_(false) {}
    void cancel() { flag_.store(true, std::memory_order_relaxed); if (onCancel_) onCancel_(); }
    void reset()  { flag_.store(false, std::memory_order_relaxed); }
    CancellationToken token() { return CancellationToken(&flag_); }
    void onCancel(std::function<void()> cb) { onCancel_ = std::move(cb); }
private:
    std::atomic_bool flag_;
    std::function<void()> onCancel_;
};
