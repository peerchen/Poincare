#include <iostream>
#include <string.h>
#include "utils/utils.h"
#include "utils/log.h"
using namespace std;

#include "Trader_Handler.h"
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep Sleep
#endif


// �����ж�
bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

Trader_Handler::Trader_Handler(CThostFtdcTraderApi* TraderApi, TraderConfig* trader_config)
{
	m_trader_config = trader_config;
	m_trader_api = TraderApi;
	m_trader_api->RegisterSpi(this);			// ע���¼���
	m_trader_api->RegisterFront(m_trader_config->TRADER_FRONT);		// connect
}

void Trader_Handler::OnFrontConnected()
{
	CThostFtdcReqUserLoginField req = { 0 };
	strcpy(req.BrokerID, m_trader_config->TBROKER_ID);
	strcpy(req.UserID, m_trader_config->TUSER_ID);
	strcpy(req.Password, m_trader_config->TPASSWORD);
	m_trader_api->ReqUserLogin(&req, ++m_request_id);
}

void update_trader_info(TraderInfo& info, CThostFtdcRspUserLoginField *pRspUserLogin) {
	// ����Ự����
	info.FrontID = pRspUserLogin->FrontID;
	info.SessionID = pRspUserLogin->SessionID;
	int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
	iNextOrderRef++;
	sprintf(info.MaxOrderRef, "%d", iNextOrderRef);
	strlcpy(info.TradingDay, pRspUserLogin->TradingDay, TRADING_DAY_LEN);
	strlcpy(info.LoginTime, pRspUserLogin->LoginTime, TRADING_DAY_LEN);
}

void Trader_Handler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
		m_is_ready = true;
		update_trader_info(m_trader_info, pRspUserLogin);
		PRINT_SUCCESS("TradingDay: %s LoginTime: %s", m_trader_info.TradingDay, m_trader_info.LoginTime);

		//Ͷ���߽�����ȷ��
		ReqSettlementInfo();
		
		//�����ѯ�ʽ��˻�
		//ReqTradingAccount();

		//�����ѯ��Լ
		/*for (int i = 0; i < m_trader_config->INSTRUMENT_COUNT; i++) {
			ReqInstrument(m_trader_config->INSTRUMENTS[i]);
		}*/

		//�����ѯͶ���ֲ߳�
		//ReqInvestorPosition();

		//�����µ�
		ReqOrderInsert();
		ReqOrderAction(&m_cancel);
	} else {
		PRINT_ERROR("Login Failed!");
		exit(-1);
	}
}

void Trader_Handler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	PRINT_SUCCESS("Comform settlement!");
}

void Trader_Handler::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	PRINT_DEBUG("%s %s %s %s %s %f", pInstrument->InstrumentID, pInstrument->ExchangeID, pInstrument->ProductID, pInstrument->CreateDate, pInstrument->ExpireDate, pInstrument->PriceTick);
}


void Trader_Handler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	PRINT_DEBUG("%s %f %f %f %f %f %f %f", pTradingAccount->AccountID, pTradingAccount->Interest, pTradingAccount->Deposit, pTradingAccount->Withdraw, pTradingAccount->CurrMargin, pTradingAccount->CloseProfit, pTradingAccount->PositionProfit, pTradingAccount->Available);
}

void Trader_Handler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInvestorPosition) {
		printf("\nInstrumentID: %s, "
			"PosiDirection: %c, "
			"HedgeFlag: %c, "
			"YdPosition: %d, "
			"Position: %d, "
			"LongFrozen: %d, "
			"ShortFrozen: %d, "
			"LongFrozenAmount: %f, "
			"ShortFrozenAmount: %f, "
			"OpenVolume: %d, "
			"CloseVolume: %d, "
			"OpenAmount: %f, "
			"CloseAmount: %f, "
			"PositionCost: %f, "
			"UseMargin: %f, "
			"FrozenMargin: %f, "
			"FrozenCash: %f, "
			"FrozenCommission: %f, "
			"CashIn: %f, "
			"Commission: %f, "
			"CloseProfit: %f, "
			"PositionProfit: %f, "
			"OpenCost: %f, \n"
			"ExchangeMargin: %f, "
			"CloseProfitByDate: %f, "
			"CloseProfitByTrade: %f, "
			"PositionProfit: %f, "
			"TodayPosition: %d, "
			"MarginRateByMoney: %f, "
			"MarginRateByVolume: %f, "
			"StrikeFrozenAmount: %f \n",
			pInvestorPosition->InstrumentID,
			pInvestorPosition->PosiDirection,
			pInvestorPosition->HedgeFlag,
			pInvestorPosition->YdPosition,
			pInvestorPosition->Position,
			pInvestorPosition->LongFrozen,
			pInvestorPosition->ShortFrozen,
			pInvestorPosition->LongFrozenAmount,
			pInvestorPosition->ShortFrozenAmount,
			pInvestorPosition->OpenVolume,
			pInvestorPosition->CloseVolume,
			pInvestorPosition->OpenAmount,
			pInvestorPosition->CloseAmount,
			pInvestorPosition->PositionCost,
			pInvestorPosition->UseMargin,
			pInvestorPosition->FrozenMargin,
			pInvestorPosition->FrozenCash,
			pInvestorPosition->FrozenCommission,
			pInvestorPosition->CashIn,
			pInvestorPosition->Commission,
			pInvestorPosition->CloseProfit,
			pInvestorPosition->PositionProfit,
			pInvestorPosition->OpenCost,
			pInvestorPosition->ExchangeMargin,
			pInvestorPosition->CloseProfitByDate,
			pInvestorPosition->CloseProfitByTrade,
			pInvestorPosition->PositionProfit,
			pInvestorPosition->TodayPosition,
			pInvestorPosition->MarginRateByMoney,
			pInvestorPosition->MarginRateByVolume,
			pInvestorPosition->StrikeFrozenAmount);
	}
	//	PRINT_DEBUG("%s %d %d %d %f", pInvestorPosition->InstrumentID, pInvestorPosition->Position, pInvestorPosition->TodayPosition, pInvestorPosition->YdPosition, pInvestorPosition->UseMargin);
		///����¼������
		//ReqOrderInsert();
	//}
}

void Trader_Handler::ReqSettlementInfo()
{
	CThostFtdcSettlementInfoConfirmField req = { 0 };
	strcpy(req.BrokerID, m_trader_config->TBROKER_ID);
	strcpy(req.InvestorID, m_trader_config->TUSER_ID);
	int ret = m_trader_api->ReqSettlementInfoConfirm(&req, ++m_request_id);
	if (ret)
		PRINT_ERROR("Send settlement request fail!");
}

void Trader_Handler::ReqTradingAccount()
{
	CThostFtdcQryTradingAccountField requ = { 0 };
	strcpy(requ.BrokerID, m_trader_config->TBROKER_ID);
	strcpy(requ.InvestorID, m_trader_config->TUSER_ID);
	strcpy(requ.CurrencyID, "CNY");
	while (true) {
		int iResult = m_trader_api->ReqQryTradingAccount(&requ, ++m_request_id);
		if (!IsFlowControl(iResult)) {
			PRINT_INFO("send query account: %s %s", requ.InvestorID, iResult == 0 ? "success" : "fail");
			break;
		} else {
			sleep(1);
		}
	} // while
}

void Trader_Handler::ReqInstrument(char* symbol) {
	while (true) {
		strcpy(m_req_contract.InstrumentID, symbol);
		int iResult = m_trader_api->ReqQryInstrument(&m_req_contract, ++m_request_id);
		if (!IsFlowControl(iResult)) {
			PRINT_INFO("send query contract: %s %s", m_req_contract.InstrumentID, iResult == 0 ? "success" : "fail");
			break;
		} else {
			sleep(1);
		}
	}
}

void Trader_Handler::ReqInvestorPosition()
{
	CThostFtdcQryInvestorPositionField req_pos = { 0 };
	strcpy(req_pos.BrokerID, m_trader_config->TBROKER_ID);
	strcpy(req_pos.InvestorID, m_trader_config->TUSER_ID);
	while (true) {
		//strcpy(m_req_pos.InstrumentID, symbol);
		int iResult = m_trader_api->ReqQryInvestorPosition(&req_pos, ++m_request_id);
		if (!IsFlowControl(iResult)) {
			PRINT_INFO("send query contract position %s", iResult == 0 ? "success" : "fail");
			break;
		} else {
			sleep(1);
		}
	}
}

void Trader_Handler::ReqOrderInsert()
{
	PRINT_SUCCESS("lalal");
	CThostFtdcInputOrderField req = { 0 };
	///���͹�˾����
	strcpy(req.BrokerID, m_trader_config->TBROKER_ID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, m_trader_config->TUSER_ID);
	///��Լ����
	strcpy(req.InstrumentID, m_trader_config->INSTRUMENTS[0]);
	///��������
	strcpy(req.OrderRef, m_trader_info.MaxOrderRef);
	///�û�����
	strcpy(req.UserID, m_trader_config->TUSER_ID);
	///�����۸�����: �޼�
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	//req.OrderPriceType = THOST_FTDC_OPT_AnyPrice; //shijia
	///��������: 
	req.Direction = THOST_FTDC_D_Buy;
	///��Ͽ�ƽ��־: ����
	// req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;//  THOST_FTDC_OF_Close
													 ///���Ͷ���ױ���־
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///�۸�
	req.LimitPrice = 3800;
	//req.LimitPrice = 0; // shijia
	///����: 1
	req.VolumeTotalOriginal = 1;
	///��Ч������: ������Ч
	req.TimeCondition = THOST_FTDC_TC_GFD;
	///GTD����
	//	TThostFtdcDateType	GTDDate;
	///�ɽ�������: �κ�����
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///��С�ɽ���: 1
	req.MinVolume = 1;
	///��������: ����
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///ֹ���
	//	TThostFtdcPriceType	StopPrice;
	///ǿƽԭ��: ��ǿƽ
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///�Զ������־: ��
	req.IsAutoSuspend = 0;
	///ҵ��Ԫ
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	///������
	//	TThostFtdcRequestIDType	RequestID;
	///�û�ǿ����־: ��
	req.UserForceClose = 0;

	m_cancel = req;
	int iResult = m_trader_api->ReqOrderInsert(&req, ++m_request_id);
	cerr << "--->>> Send order: " << iResult << ((iResult == 0) ? ", success" : ", fail") << endl;
}

void Trader_Handler::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderInsert" << endl;
	IsErrorRspInfo(pRspInfo, bIsLast);
}

void Trader_Handler::ReqOrderAction(CThostFtdcInputOrderField *pOrder)
{
	static bool ORDER_ACTION_SENT = false;		//�Ƿ����˱���
	if (ORDER_ACTION_SENT)
		return;

	CThostFtdcInputOrderActionField req = { 0 };
	///���͹�˾����
	strcpy(req.BrokerID, pOrder->BrokerID);
	///Ͷ���ߴ���
	strcpy(req.InvestorID, pOrder->InvestorID);
	///������������
	//	TThostFtdcOrderActionRefType	OrderActionRef;
	///��������
	strcpy(req.OrderRef, pOrder->OrderRef);
	///������
	req.RequestID = m_request_id;
	///ǰ�ñ��
	req.FrontID = m_trader_info.FrontID;
	///�Ự���
	req.SessionID = m_trader_info.SessionID;
	///����������
	//	TThostFtdcExchangeIDType	ExchangeID;
	///�������
	//	TThostFtdcOrderSysIDType	OrderSysID;
	///������־
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///�۸�
	//	TThostFtdcPriceType	LimitPrice;
	///�����仯
	//	TThostFtdcVolumeType	VolumeChange;
	///�û�����
	//	TThostFtdcUserIDType	UserID;
	///��Լ����
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult = m_trader_api->ReqOrderAction(&req, ++m_request_id);
	cerr << "--->>> Order action: " << iResult << ((iResult == 0) ? ", success" : ", fail") << endl;
	ORDER_ACTION_SENT = true;
}

void Trader_Handler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderAction" << endl;
	IsErrorRspInfo(pRspInfo, bIsLast);
}

///����֪ͨ
void Trader_Handler::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	cerr << "--->>> " << "OnRtnOrder" << endl;
	if (IsMyOrder(pOrder))
	{
		if (pOrder) {
			printf("[OnRtnOrder]  Inv_ID: %s,symbol:%s ,Ref: %d, LocalID: %d, O_Sys_ID: %d, OrderStatus: %c,InsertTime:%s,VolumeTraded:%d,VolumeTotal:%d, Dir:%c, open_close:%c,price:%f ,vol:%d,Broker_Seq:%d \n",
				pOrder->InvestorID,
				pOrder->InstrumentID,
				atoi(pOrder->OrderRef),
				atoi(pOrder->OrderLocalID),
				atoi(pOrder->OrderSysID),
				pOrder->OrderStatus,
				pOrder->InsertTime,
				pOrder->VolumeTraded,
				pOrder->VolumeTotal,
				pOrder->Direction,
				pOrder->CombOffsetFlag[0],
				pOrder->LimitPrice,
				pOrder->VolumeTotalOriginal,
				pOrder->BrokerOrderSeq);
		}
		/*if (IsTradingOrder(pOrder))
			ReqOrderAction(pOrder);
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
			cout << "--->>> cancel" << endl;
	*/}
}

///�ɽ�֪ͨ
void Trader_Handler::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if (pTrade) {
		printf("[OnRtnTrade] Inv_ID:%s ,Ref: %d, symbol:%s ,exhg_ID:%s,LocalID: %d,O_Sys_ID:%d,Dir:%c,open_close:%c, price:%f,deal_vol:%d, TradeTime:%s, TradeID:%s,Broker_Seq:%d \n",
			pTrade->InvestorID,
			atoi(pTrade->OrderRef),
			pTrade->InstrumentID,
			pTrade->ExchangeID,
			atoi(pTrade->OrderLocalID),
			atoi(pTrade->OrderSysID),
			pTrade->Direction,
			pTrade->OffsetFlag,
			pTrade->Price,
			pTrade->Volume,
			pTrade->TradeTime,
			pTrade->TradeID,
			pTrade->BrokerOrderSeq);
	}
}

void Trader_Handler::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << "OnFrontDisconnected" << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}

void Trader_Handler::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << "OnHeartBeatWarning" << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void Trader_Handler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo, bIsLast);
}

bool Trader_Handler::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == m_trader_info.FrontID) &&
		(pOrder->SessionID == m_trader_info.SessionID) &&
		(strcmp(pOrder->OrderRef, m_trader_info.MaxOrderRef) == 0));
}

bool Trader_Handler::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}