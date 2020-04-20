#pragma once
#include <string>
#include "ServiceTransPushIf.h"
#include "common/CheckCondition.h"
#include "common/SLog.h"
namespace EgcTrans {
using SUC = SmsTrans_SucCallback;
using ERR = SmsTrans_ErrCallback;
struct IEgcCallback {
  IEgcCallback() { name_ = "defaultname"; }
  virtual ~IEgcCallback() {}
  void* context_;
  std::string name_;
  virtual void reset(void) = 0;
};

template <typename T>
struct ParamCallback : public IEgcCallback {
  ParamCallback() { reset(); }
  ParamCallback(T cb, void* data) : clk_(cb), context_(data) {}
  ~ParamCallback() { reset(); }
  virtual void reset(void) override {
    context_ = 0;
    clk_ = 0;
  }
  //设置监听回调
  void set(T cb, void* data, std::string name) {
    clk_ = cb;
    context_ = data;
    name_ = name;
  }
  T clk(void) { return clk_; }
  std::string context() { return context_; }
  bool isExist() { return clk_ ? true : false; }
  template <class... ARGS>
  void call(ARGS... args) {
    if (!clk_) return;
    clk_(args..., context_);
  }

  T clk_;
  void* context_;
};
struct ResultCallback : public IEgcCallback {
  ResultCallback() { reset(); }
  ~ResultCallback() { reset(); }

  void reset() {
    context_ = 0;
    suc_ = 0;
    err_ = 0;
    name_ = "ResultCallback";
  }
  void set(SUC suc, ERR err, void* data, std::string name) {
    context_ = data;
    suc_ = suc;
    err_ = err;
    name_ = name;
  }
  // TODO 异步线程中回调通知
  void success() {
    CHECK(suc_);
    try {
      SLOGI("RESULT CALLBACK SUC  {}", name_);
      suc_(context_);
    } catch (const std::exception& e) {
      SLOGE("RESULT CALLBACK SUC EXCEPTION {}", e.what());
    }
    reset();
  }
  void fail(SmsTransCallErrType code, const char* desc) {
    SLOGI("RESULT CALLBACK FAIL  {}", name_);
    CHECK(err_);
    err_(code, desc, context_);
    reset();
  }
  SUC suc_;
  ERR err_;
};
}  // namespace EgcTrans
