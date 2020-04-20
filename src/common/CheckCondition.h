// 判断模块
#ifndef __CHECK_H__
#define __CHECK_H__

/// @brief 判断condition是否成立，
/// @param [in] condition可以是条件语句，也可以是指针
/// @param [in] tip 中断提示
/// @return condition无效是返回void
#define CHECK(condition) \
  if (!(condition)) {    \
    return;              \
  }

#define CHECK_ASSERT(condition) \
  if (!(condition)) {           \
    assert(0);                  \
    return;                     \
  }

#define CHECK_ASSERT_TIP(condition, tip) \
  if (!(condition)) {                    \
    assert(0 && #tip);                   \
    return;                              \
  }

/// @brief 判断condition是否成立，
/// @param [in] condition可以是条件语句，也可以是指针
/// @return condition无效是返回result
#define CHECK_RESULT(condition, result) \
  if (!(condition)) {                   \
    return result;                      \
  }

#define CHECK_RESULT_A(condition, result) \
  if ((condition <= 0)) {                 \
    return result;                        \
  }

#define CHECK1(term1) CHECK(term1)
#define CHECK2(term1, term2) CHECK(term1) CHECK(term2)
#define CHECK3(term1, term2, term3) CHECK(term1) CHECK(term2) CHECK(term3)

#define CHECK_RESULT1(term1, result) CHECK_RESULT(term1, result)
#define CHECK_RESULT2(term1, term2, result) \
  CHECK_RESULT(term1, result) CHECK_RESULT(term2, result)
#define CHECK_RESULT3(term1, term2, term3, result) \
  CHECK_RESULT(term1, result)                      \
  CHECK_RESULT(term2, result) CHECK_RESULT(term3, result)

/// @brief 判断condition是否成立，
/// @param [in] condition可以是条件语句，也可以是指针
/// @param [in] fun 退出执行单元
/// @param [in] result 返回结果
/// @return condition无效是返回result
#define TRUE_RETURN(condition) \
  if ((condition)) {           \
    return;                    \
  }

#define TRUE_RETURN_FUN(condition, fun) \
  if ((condition)) {                    \
    fun;                                \
    return;                             \
  }

#define TRUE_RETURN_RESULT(condition, result) \
  if ((condition)) {                          \
    return result;                            \
  }

#define TRUE_RETURN_RESULT_FUN(condition, result, fun) \
  if ((condition)) {                                   \
    fun;                                               \
    return result;                                     \
  }

#define FALSE_RETURN(condition) \
  if (!(condition)) {           \
    return;                     \
  }

#define FALSE_RETURN_FUN(condition, fun) \
  if (!(condition)) {                    \
    fun;                                 \
    return;                              \
  }

#define FALSE_RETURN_RESULT(condition, result) \
  if (!(condition)) {                          \
    return result;                             \
  }

#define FALSE_RETURN_RESULT_FUN(condition, result, fun) \
  if (!(condition)) {                                   \
    fun;                                                \
    return result;                                      \
  }

#endif  // __CHECK_H__