#pragma once

namespace EgcCommon {



  struct OptStatus {

	OptStatus(void) {
	  m_nStatus = eNone;
	}
	OptStatus(int nStatus) {
	  m_nStatus = nStatus;
	}
	OptStatus(const OptStatus& obj) {
	  *this = obj;
	}
	OptStatus& operator=(const OptStatus& obj) {
	  this->m_nStatus = obj.m_nStatus;
        return *this;  // <-- return something
	}
	// 已经执行完成
	bool isDone(void) const {
	  return eDone == m_nStatus;
	}
	// 还没有初始化
	bool isNone(void) const {
	  return eNone == m_nStatus;
	}
	// 正在执行中
	bool isWork(void) const {
	  return !(isDone() || isNone());
	}

	void setNone(void) {
	  m_nStatus = eNone;
	}
	void setDone(void) {
	  m_nStatus = eDone;
	}
	void setWork(void) {
	  m_nStatus = eWork;
	}


	enum {
	  eNone = 0,
	  eDone,
	  eWork,
	};
	int m_nStatus;
  };

  struct StatusCheck {
	StatusCheck(OptStatus& st1, OptStatus& st2)
	  : m_st1(st1)
	  , m_st2(st2) {
	}

	bool isWork(void) const {
	  return isWork1() || isWork2();
	}
	bool isWork1(void) const {
	  return m_st1.isWork();
	}
	bool isWork2(void) const {
	  return m_st2.isWork();
	}

	void setDone(void) {
	  setDone1();
	  setDone2();
	}
	void setDone1(void) {
	  m_st1.setDone();
	}
	void setDone2(void) {
	  m_st2.setDone();
	}

	void setNone(void) {
	  setNone1();
	  setNone2();
	}
	void setNone1(void) {
	  m_st1.setNone();
	}
	void setNone2(void) {
	  m_st2.setNone();
	}

	void setWork(void) {
	  setWork1();
	  setWork2();
	}
	void setWork1(void) {
	  m_st1.setWork();
	}
	void setWork2(void) {
	  m_st2.setWork();
	}

	void reset(void) {
	  m_st1.setNone();
	  m_st2.setNone();
	}

	OptStatus& m_st1;// 正
	OptStatus& m_st2;// 反
  };
}
