#pragma once

#include "include/mysql.h"
#include <codecvt>
// Query �Լ� ������ ������� ���� üũ 
#include "include/errmsg.h"
#include "LockFreeStack.h"


/////////////////////////////////////////////////////////
// MySQL DB ���� Ŭ����
//
// �ܼ��ϰ� MySQL Connector �� ���� DB ���Ḹ �����Ѵ�.
//
// �����忡 �������� �����Ƿ� ���� �ؾ� ��.
// ���� �����忡�� ���ÿ� �̸� ����Ѵٸ� ������ ��.
//
/////////////////////////////////////////////////////////


#define dfQUERY_TIME_MAX				1000

#define dfRETRY_MAX						5

class CTLSDBConnector;
class CDBConnector
{
public:

	enum en_DB_CONNECTOR
	{
		eQUERY_MAX_LEN = 2048

	};

	CDBConnector(const char *szDBIP, const char *szUser, const char *szPassword, const char *szDBName, int iDBPort);
	CDBConnector(const WCHAR *szDBIP, const WCHAR *szUser, const WCHAR *szPassword, const WCHAR *szDBName, int iDBPort);
	virtual		~CDBConnector();

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//////////////////////////////////////////////////////////////////////
	bool		Connect();

	//////////////////////////////////////////////////////////////////////
	// MySQL DB ����
	//////////////////////////////////////////////////////////////////////
	bool		Disconnect();


	//////////////////////////////////////////////////////////////////////
	// ���� ������ ����� �ӽ� ����
	//
	//////////////////////////////////////////////////////////////////////
	bool		Query(const WCHAR *szStringFormat, ...);
	bool		QueryCompleteString(const WCHAR *szCompleteString);

	// DBWriter �������� Save ���� ����
	// ������� �������� ����.
	bool		Query_Save(const WCHAR *szStringFormat, ...);
	bool		Query_SaveCompleteString(const WCHAR *szCompleteString);

	//////////////////////////////////////////////////////////////////////
	// ������ ���� �ڿ� ��� �̾ƿ���.
	//
	// ����� ���ٸ� NULL ����.
	//////////////////////////////////////////////////////////////////////
	MYSQL_ROW	FetchRow();

	//////////////////////////////////////////////////////////////////////
	// �� ������ ���� ��� ��� ��� �� ����.
	//////////////////////////////////////////////////////////////////////
	void		FreeResult();


	//////////////////////////////////////////////////////////////////////
	// Error ���.�� ������ ���� ��� ��� ��� �� ����.
	//////////////////////////////////////////////////////////////////////
	int			GetLastError() { return m_iLastError; };
	WCHAR		*GetLastErrorMsg() { return m_szLastErrorMsg; }


private:

	//////////////////////////////////////////////////////////////////////
	// mysql �� LastError �� �ɹ������� �����Ѵ�.
	//////////////////////////////////////////////////////////////////////
	void												SaveLastError();

private:



	//-------------------------------------------------------------
	// MySQL ���ᰴü ��ü
	//-------------------------------------------------------------
	MYSQL		m_MySQL;

	//-------------------------------------------------------------
	// MySQL ���ᰴü ������. �� ������ ��������. 
	// �� �������� null ���η� ������� Ȯ��.
	//-------------------------------------------------------------
	MYSQL		*m_pMySQL;

	//-------------------------------------------------------------
	// ������ ���� �� Result �����.
	//
	//-------------------------------------------------------------
	MYSQL_RES	*m_pSqlResult;

	char		m_szDBIP[16];
	char		m_szDBUser[64];
	char		m_szDBPassword[64];
	char		m_szDBName[64];
	int			m_iDBPort;


	WCHAR		m_szQuery[eQUERY_MAX_LEN];
	char		m_szQueryUTF8[eQUERY_MAX_LEN];

	UINT		m_iLastError;
	WCHAR		m_szLastErrorMsg[128];

};

class CTLSDBConnector
{
public:

	CTLSDBConnector(const WCHAR *szDBIP, const WCHAR *szUser, const WCHAR *szPassword, const WCHAR *szDBName, int iDBPort);
	virtual		~CTLSDBConnector();

	//////////////////////////////////////////////////////////////////////
	// DB Connector ��ü ������ ���
	//////////////////////////////////////////////////////////////////////
	CDBConnector*	GetDBConnector();

private:

	int										m_iTLSIndex;							
	int										m_iDBPort;
	CLockFreeStack<CDBConnector*>			*m_pDBConnectorStack;

	WCHAR									m_szDBIP[16];
	WCHAR									m_szDBUser[64];
	WCHAR									m_szDBPassword[64];
	WCHAR									m_szDBName[64];
};
