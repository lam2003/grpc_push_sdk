#pragma once
#include <string>
#define RES_TIMEOUT 100   // SDK��ΪӦ��ʱ
#define RES_SUCCESS 200   /* ���ܳɹ����,һ������,���ص��� */
#define RES_ACCEPTED 202  /* ���������ѽ���,�Ⱥ��� */
#define RES_NOCHANGED 204 /* ���û�б䣬����CHECKSUM */

#define RES_ERETRY 300 /* ��ʱ�޷�����,�����Ժ����� */

#define RES_EREQUEST 400 /* ��������/����������󣻷������޷����Ĺ������� */
#define RES_EAUTH 401 /* ���������δ����֤,��˺�̨�ܾ���ɹ��� */
#define RES_EPERM 403     /* �Է�ʵ��û��Ȩ(�ɲ��Ǻ�̨������) */
#define RES_ENONEXIST 404 /* Ŀ��(������û�)������ */
#define RES_EACCESS 405   /* ��Ȩ���������� */
#define RES_EQUOTA 406    /* �û����ݴ洢�������޶� */
#define RES_EVOLATILE 407 /* ĳЩ������ʱ�����Ƶ���Դ�Ѿ���ʱ������ */
#define RES_ETIMEOUT 408  /* ������̳�ʱ */
#define RES_ECONFLICT 409 /* ��Դ���߶����ͻ(��������) */
#define RES_EPARAM 414 /* �������ݳ���.(Խ��,����,��������ԭ��) */
#define RES_EDBERROR 415  /* ���ݿ����ʧ�� */
#define RES_EDBNVALID 416 /* ���ݿ���ʱ�����ã�����������ά�� */
#define RES_NODEMOVED 418 /* �ڵ��Ѿ���ת��*/
#define RES_EOVERTIMES 453 /* ��������̫���� */

#define RES_EUNKNOWN 500 /* ������.����ԭ��δ��,���ǲ���͸¶���� */
#define RES_EBUSY 504 /* ��̨æ,�ܾ����� */
#define RES_EPROT 505 /* ��̨��֧�ִ�Э��汾 */
#define RES_EDATANOSYNC 506 /* �����ݿ����ݲ�ͬ��,client��Ҫ���»������ */
#define RES_ENOTENOUGH 507     /* �������� */
#define RES_ENONEXIST_REAL 508 /* Ŀ��(������û�)������ */
#define RES_ESERVICE 550       /* ��֧�ֵķ��� */
#define RES_EDAEMON 551        /* ������δ�ҵ� */
#define RES_EUNUSABLE 552      /* ������ʱ������ */
#define RES_ECONNMISS 553      /* �ڲ����󣬸���connid�Ҳ������� */
#define RES_EBUFOVR 554 /* ����������ڲ�������ʱ�����岻����ƿ�������á�*/
#define RES_ECONNECT 555  /* �ڲ��������Ӳ��� */
#define RES_ESENDREQ 556  /* �����������쳣 */
#define RES_EHASHTYPE 557 /* �ڲ����ô��� */
#define RES_EPACKET 558   /* �������ݰ����� */
#define RES_ELOCATE 559 /* ��λ���󣬷������յ��������Լ������� */
#define RES_LIMITUSER 580 /* ������һ̨�����ϵ�¼������ͬ�˺� */
                          //
                          //
                          // typedef unsigned short RES_CODE;
//
// enum class signal_errc {
//  kNull = 0,
//  kSuccess = 200,
//  kNochanged = 204,
//  kRetry = 300,
//  kNonExist = 404,
//  kQuota = 406,
//  kVolatile = 407,
//  kConflict = 409,
//  kDaemon = 551,
//  kHashType = 557,
//};
//
// std::string message(int i) const override {
//  switch (static_cast<signal_errc>(i)) {
//  case signal_errc::kNull: return "empty";
//  case signal_errc::kSuccess: return "�ɹ�";
//  case signal_errc::kNochanged: return "���û�б䣬����CHECKSUM";
//  case signal_errc::kRetry: return "��ʱ�޷�����,�����Ժ�����";
//  case signal_errc::kNonExist: return "Ŀ��(������û�)������";
//  case signal_errc::kQuota: return "�û����ݴ洢�������޶�";
//  case signal_errc::kVolatile: return
//  "ĳЩ������ʱ�����Ƶ���Դ�Ѿ���ʱ������"; case signal_errc::kConflict:
//  return "��Դ���߶����ͻ(��������)"; case signal_errc::kDaemon: return
//  "������δ�ҵ�"; case signal_errc::kHashType: return "�ڲ����ô���";
//  default:return "unknow";
//  }
//}