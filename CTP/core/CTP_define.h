#pragma once

#define SYMBOL_LEN 64
struct TraderConfig
{
	char  QUOTE_FRONT[SYMBOL_LEN]; // ����ǰ��
	char  QBROKER_ID[BROKER_ID_LEN];
	char  QUSER_ID[SYMBOL_LEN];
	char  QPASSWORD[SYMBOL_LEN];
	char  TRADER_FRONT[SYMBOL_LEN];	//����ǰ��
	char  TBROKER_ID[BROKER_ID_LEN];
	char  TUSER_ID[SYMBOL_LEN];
	char  TPASSWORD[SYMBOL_LEN];
	char *INSTRUMENTS[SYMBOL_LEN];
	int   INSTRUMENT_COUNT;
};

struct TraderInfo
{
    int FrontID;	//ǰ�ñ��
	char MaxOrderRef[ORDER_REF_LEN];	//��������
	char TradingDay[TRADING_DAY_LEN]; //��ǰ������