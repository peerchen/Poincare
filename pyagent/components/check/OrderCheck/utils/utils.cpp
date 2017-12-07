#include <stdio.h>
#include "utils.h"
#include "log.h"
#include <string>

#define CHAR_EQUAL_ZERO(a, b, c) do{\
	if (a != b) goto end;\
	if (a == 0) {c = 0; goto end;}\
}while(0)

int my_strcmp(const char * s1, const char * s2)
{
	int ret = -1;
	CHAR_EQUAL_ZERO(s1[0], s2[0], ret);
	CHAR_EQUAL_ZERO(s1[1], s2[1], ret);
	CHAR_EQUAL_ZERO(s1[2], s2[2], ret);
	CHAR_EQUAL_ZERO(s1[3], s2[3], ret);
	CHAR_EQUAL_ZERO(s1[4], s2[4], ret);
	CHAR_EQUAL_ZERO(s1[5], s2[5], ret);
	CHAR_EQUAL_ZERO(s1[6], s2[6], ret);
	CHAR_EQUAL_ZERO(s1[7], s2[7], ret);

	ret = strcmp(s1 + 8, s2 + 8);
end:
	return ret;
}

uint64_t my_hash_value(const char *str_key)
{
	char *pt = (char *)str_key;
	unsigned int hash = 1315423911;

	if (pt[0] == 0) goto end;
	hash ^= ((hash << 5) + pt[0] + (hash >> 2));
	if (pt[1] == 0) goto end;
	hash ^= ((hash << 5) + pt[1] + (hash >> 2));
	if (pt[2] == 0) goto end;
	hash ^= ((hash << 5) + pt[2] + (hash >> 2));
	if (pt[3] == 0) goto end;
	hash ^= ((hash << 5) + pt[3] + (hash >> 2));

	if (pt[4] == 0) goto end;
	hash ^= ((hash << 5) + pt[4] + (hash >> 2));
	if (pt[5] == 0) goto end;
	hash ^= ((hash << 5) + pt[5] + (hash >> 2));
	if (pt[6] == 0) goto end;
	hash ^= ((hash << 5) + pt[6] + (hash >> 2));
	if (pt[7] == 0) goto end;
	hash ^= ((hash << 5) + pt[7] + (hash >> 2));

	pt = (char *)(str_key + 8);
	while (*pt != 0) {
		hash ^= ((hash << 5) + *pt + (hash >> 2));
		pt++;
	}

end:
	return hash;
}

int
double_compare(double a, double b, double epsilon)
{
	if (a - b > epsilon)
		return 1;
	else if (a - b < -epsilon)
		return -1;
	else
		return 0;
}

size_t
strlcpy(char * dst, const char * src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';              /* NUL-terminate dst */
		while (*s++);
	}

	return(s - src - 1);    /* count does not include NUL */
}

void
my_strncpy(char *dst, const char *src, size_t siz)
{
	dst[0] = src[0]; if (src[0] == 0) goto end;
	dst[1] = src[1]; if (src[1] == 0) goto end;
	dst[2] = src[2]; if (src[2] == 0) goto end;
	dst[3] = src[3]; if (src[3] == 0) goto end;
	dst[4] = src[4]; if (src[4] == 0) goto end;
	dst[5] = src[5]; if (src[5] == 0) goto end;
	dst[6] = src[6]; if (src[6] == 0) goto end;
	dst[7] = src[7]; if (src[7] == 0) goto end;
	dst[8] = src[8]; if (src[8] == 0) goto end;
	dst[9] = src[9]; if (src[9] == 0) goto end;
	dst[10] = src[10]; if (src[10] == 0) goto end;
	dst[11] = src[11]; if (src[11] == 0) goto end;
	strlcpy(dst + 12, src + 12, siz - 12);
end:
	return;
}

void duplicate_contract(Contract *src, Contract *dest)
{
	dest->exch = src->exch;
	dest->max_pos = src->max_pos;
	dest->max_accum_open_vol = src->max_accum_open_vol;
	dest->single_side_max_pos = src->single_side_max_pos;
	strlcpy(dest->symbol, src->symbol, SYMBOL_LEN);
}

//----------positions related-------
double
avg_px(Position &pos)
{
	if (pos.qty > 0)
		return pos.notional / (double)pos.qty;
	else
		return 0.0;
}

int
position(Contract *cont)
{
	return long_position(cont) - short_position(cont);
}

int 
position(DIRECTION side, Contract * cont)
{
	if (side == ORDER_BUY)
		return long_position(cont);
	else
		return short_position(cont);
}

int
long_position(Contract *cont)
{
	return cont->positions[LONG_OPEN].qty - cont->positions[SHORT_CLOSE].qty;
}

int
short_position(Contract *cont)
{
	return cont->positions[SHORT_OPEN].qty - cont->positions[LONG_CLOSE].qty;
}

double
long_notional(Contract *cont)
{
	return cont->positions[LONG_OPEN].notional + cont->positions[LONG_CLOSE].notional;
}

double
short_notional(Contract *cont)
{
	return cont->positions[SHORT_OPEN].notional + cont->positions[SHORT_CLOSE].notional;
}

double
avg_buy_price(Contract *cont)
{
	return (cont->positions[LONG_OPEN].notional + cont->positions[LONG_CLOSE].notional)
		/ (cont->positions[LONG_OPEN].qty + cont->positions[LONG_CLOSE].qty);
}

double
avg_sell_price(Contract *cont)
{
	return (cont->positions[SHORT_OPEN].notional + cont->positions[SHORT_CLOSE].notional)
		/ (cont->positions[SHORT_OPEN].qty + cont->positions[SHORT_CLOSE].qty);
}

double
get_transaction_fee(Contract *cont, int size, double price, bool flag_close_yes)
{
	double l_fee = 0.0;
	double exchange_fee = 0.0;
	if (flag_close_yes) {
		exchange_fee = cont->yes_exchange_fee;
	}
	else {
		exchange_fee = cont->exchange_fee;
	}
	if (cont->fee_by_lot)
		l_fee = size * (exchange_fee + cont->broker_fee); // Caution, for futures right now, broker fee is 0.0
	else
		l_fee = size * price * (exchange_fee + cont->broker_fee);
	if (cont->exch == SSE)
		l_fee += size * price * cont->acc_transfer_fee;

	return l_fee;
}

const static std::string STATUS[] = {"SUCCEED", "ENTRUSTED", "PARTED", "CANCELED", "REJECTED", "CANCEL_REJECTED", "INTRREJECTED", "UNDEFINED_STATUS" };

int process_debug_info(int type, int length, void *data) {
	switch (type) {
	case S_STRATEGY_DEBUG_LOG: {
		PRINT_DEBUG("LOG: %s", (char*)data);
		break;
	}
	case S_PLACE_ORDER_DEFAULT: {
		order_t *l_ord = (order_t *)((st_data_t *)data)->info;
		//l_ord->order_id = ++order_count * 10000000000 + l_ord->st_id;  //only in test
		PRINT_DEBUG("Send Order: %c %s %d %f %d %d %d %d %d %lld %lld %lld", l_ord->exch, l_ord->symbol,
			l_ord->volume, l_ord->price, l_ord->direction, l_ord->open_close,
			l_ord->investor_type, l_ord->order_type, l_ord->time_in_force,
			l_ord->st_id, l_ord->order_id, l_ord->org_ord_id);
		LOG_LN("Send Order: %c %s %d %f %d %d %d %d %d %lld %lld %lld", l_ord->exch, l_ord->symbol,
			l_ord->volume, l_ord->price, l_ord->direction, l_ord->open_close,
			l_ord->investor_type, l_ord->order_type, l_ord->time_in_force,
			l_ord->st_id, l_ord->order_id, l_ord->org_ord_id);
		break;
	}
	case S_CANCEL_ORDER_DEFAULT: {
		order_t *l_ord = (order_t *)((st_data_t *)data)->info;
		PRINT_DEBUG("Cancel Order: %c %s %d %f %d %d %d %d %d %lld %lld %lld", l_ord->exch, l_ord->symbol,
			l_ord->volume, l_ord->price, l_ord->direction, l_ord->open_close,
			l_ord->investor_type, l_ord->order_type, l_ord->time_in_force,
			l_ord->st_id, l_ord->order_id, l_ord->org_ord_id);
		LOG_LN("Cancel Order: %c %s %d %f %d %d %d %d %d %lld %lld %lld", l_ord->exch, l_ord->symbol,
			l_ord->volume, l_ord->price, l_ord->direction, l_ord->open_close,
			l_ord->investor_type, l_ord->order_type, l_ord->time_in_force,
			l_ord->st_id, l_ord->order_id, l_ord->org_ord_id);
		break;
	}
	case S_STRATEGY_PASS_RSP: {
		st_response_t *l_ord = (st_response_t *)((st_data_t *)data)->info;
		
		PRINT_DEBUG("Order Resp: %lld %s %d %d %f %d %s %d %s", l_ord->order_id, l_ord->symbol,
			l_ord->direction, l_ord->open_close, l_ord->exe_price, l_ord->exe_volume,
			STATUS[l_ord->status].c_str(), l_ord->error_no, l_ord->error_info);
		LOG_LN("Order Resp: %lld %s %d %d %f %d %s %d %s", l_ord->order_id, l_ord->symbol,
			l_ord->direction, l_ord->open_close, l_ord->exe_price, l_ord->exe_volume,
			STATUS[l_ord->status].c_str(), l_ord->error_no, l_ord->error_info);
		break;
	}
	}
	return 0;
}			

int reverse_index(uint64_t ord_id) {
	return ord_id / 10000000000;
}