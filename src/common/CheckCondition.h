// �ж�ģ��
#ifndef __CHECK_H__
#define __CHECK_H__

/// @brief �ж�condition�Ƿ������
/// @param [in] condition������������䣬Ҳ������ָ��
/// @param [in] tip �ж���ʾ
/// @return condition��Ч�Ƿ���void
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

/// @brief �ж�condition�Ƿ������
/// @param [in] condition������������䣬Ҳ������ָ��
/// @return condition��Ч�Ƿ���result
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

/// @brief �ж�condition�Ƿ������
/// @param [in] condition������������䣬Ҳ������ָ��
/// @param [in] fun �˳�ִ�е�Ԫ
/// @param [in] result ���ؽ��
/// @return condition��Ч�Ƿ���result
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