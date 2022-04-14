#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define base 1000000000

typedef unsigned long long ull;
typedef long long ll;

//структура числа
struct bn_s {
	ull *body;
	ull bodysize;
	char sign;
};

typedef struct bn_s bn;

//выделение памяти для bn
int mc_bn_body(bn *t) { /// OK
	t->body = (ull*)malloc(sizeof(ull) * t->bodysize);
	if (t->body == NULL) {
		return 1;
	}
	return 0;
}

//изменение размера выделенной памяти для bn
int rc_bn_body(bn *t) {  /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	t->body = (ull*)realloc(t->body, sizeof(ull) * t->bodysize);
	if (t->body == NULL) {
		return 1;
	}
	return 0;
}

//создание нового bn
bn* bn_new() { /// OK
	bn* r = (bn*)malloc(sizeof(bn));
	if (r == NULL) {
		return NULL;
	}
	r->bodysize = 1;
	r->sign = 0;
	mc_bn_body(r);
	if (r->body == NULL) {
		free(r);
		return NULL;
	}
	r->body[0] = 0;
	return r;
}

//создание копии существующего bn
bn* bn_init(bn const *orig) { /// OK
	if (orig == NULL || orig->body == NULL) {
		return NULL;
	}
	bn *e = bn_new();
    e->sign = orig->sign;
	e->bodysize = orig->bodysize;
	rc_bn_body(e);
	for (ull j = 0; j != e->bodysize; ++j) {
		e->body[j] = orig->body[j];
	}
	return e;
}

//копия bn с предоставлением переменной
int bn_copy(bn *e, bn const *orig) { /// OK
	if (orig == NULL || orig->body == NULL) {
		return 1;
	}
	e->bodysize = orig->bodysize;
	e->sign = orig->sign;
	rc_bn_body(e);
	for (ull j = 0; j != e->bodysize; ++j) {
		e->body[j] = orig->body[j];
	}
	return 0;
}

//функция замещения копией оригинала
int bn_change(bn *e, bn *orig) { /// OK
	if (orig == NULL || orig->body == NULL) {
		return 1;
	}
	if (e->body != NULL) {
		free(e->body);
	}
	e->body = orig->body;
    e->sign = orig->sign;
	e->bodysize = orig->bodysize;
	free(orig);
	return 0;
}

//уничтожить bn
int bn_delete(bn *t) { /// OK
	if (t->body != NULL) {
		free(t->body);
	}
	else {
		return 1;
	}
	if (t != NULL) {
		free(t);
	}
	else {
		return 1;
	}
	return 0;
}

//сравнение по модулю
int bn_cmp_abs(bn const *left, bn const *right) { /// OK
	if (left->bodysize < right->bodysize) {
		return -1;
	}
	if (left->bodysize > right->bodysize) {
		return 1;
	}
	if (left->bodysize == right->bodysize) {
		for (ll j = left->bodysize - 1; j >= 0; --j) {
			if (left->body[j] < right->body[j]) {
				return -1;
			}
			if (left->body[j] > right->body[j]) {
				return 1;
			}
		}
	}
	return 0;
}

//сравнение
int bn_cmp(bn const *left, bn const *right) { /// OK
	if (left->sign < right->sign) {
		return -1;
	}
	if (left->sign > right->sign) {
		return 1;
	}
	if (left->sign == 0 && right->sign == 0) {
		return 0;
	}
	if (left->sign == -1 && right->sign == -1) {
		return -1 * bn_cmp_abs(left, right);
	}
	if (left->sign == 1 && right->sign == 1) {
		return bn_cmp_abs(left, right);
	}
	return 0;
}

//выбор минимального числа из двух
ull minimum_ull(ull a, ull b) { /// OK
	if (a >= b) {
		return b;
	}
	else {
		return a;
	}
}

//выбор максимального числа из двух
ull maximum_ull(ull a, ull b) { /// OK
	if (a >= b) {
		return a;
	}
	else {
		return b;
	}
}

//изменение знака на противоположный
int bn_neg(bn *t) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	t->sign *= -1;
	return 0;
}

//взятие модуля
int bn_abs(bn *t) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (t->sign == -1) {
		t->sign = 1;
	}
	return 0;
}

//осуществление взятия модуля
bn* bn_abs_do(bn const *t) { /// OK
	bn *d = bn_init(t);
	if (d->sign == -1) {
		d->sign = 1;
	}
	return d;
}

//взятие знака
int bn_sign(bn const *t) { /// OK
	return t->sign;
}

//взятие символа из строки для bn
ull string_b(char st, ull len, ll n) { /// OK
	return ((ull)st - (ull)'0') * (ull)(pow(10, (len - 1 - n) % 9));
}

//инициализация bn в строку
int bn_init_string(bn *t, const char *init_string) { /// OK
	ull len = strlen(init_string);
	if (t == NULL || t->body == NULL || len == 0) {
		return 1;
	}
	if (init_string[0] == '-' && init_string[1] != '0') {
		t->sign = -1;
		--len;
	}
	else if (init_string[0] == '0' || (init_string[1] == '0' && len != 1 && init_string[0] == '-')) {
		t->sign = 0;
	}
	else {
		t->sign = 1;
	}
	t->bodysize = len / 9;
	if (len % 9 != 0) {
		++t->bodysize;
	}
	if (t->sign == -1) {
		++len;
	}
	rc_bn_body(t);
	for (ull i = 0; i != t->bodysize; ++i) {
		t->body[i] = 0;
	}
	for (ll i = len - 1, j = 0; j != t->bodysize && i >= 0; --i) {
		if (init_string[i] == '-') {
			continue;
		}
		if (((len - 1 - i) % 9 == 0) && (i != len - 1)) {
			++j;
		}
		t->body[j] += string_b(init_string[i], len, i);
	}
	return 0;
}

//инициализация bn заданным целым числом
int bn_init_int(bn *t, int init_int) {  /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (init_int == 0) {
        t->sign = 0;
		t->bodysize = 1;
		rc_bn_body(t);
		t->body[0] = 0;
	}
	else if (init_int < 0) {
		t->sign = -1;
		init_int *= -1;
	}
	else {
		t->sign = 1;
	}
	if (init_int < base) {
		t->bodysize = 1;
		rc_bn_body(t);
		t->body[0] = init_int;
	}
	else {
		t->bodysize = 2;
		rc_bn_body(t);
		t->body[1] = init_int / base;
		t->body[0] = init_int % base;
	}
	return 0;
}

//инициализация bn заданным длинным целым числом
int bn_init_ull(bn *t, ull init_ull) {   /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (init_ull == 0) {
            t->sign = 0;
		t->bodysize = 1;
		rc_bn_body(t);
		t->body[0] = 0;
	}
	else if (init_ull < 0) {
		t->sign = -1;
		init_ull *= -1;
	}
	else {
		t->sign = 1;
	}
	if (init_ull < base) {
		t->bodysize = 1;
		rc_bn_body(t);
		t->body[0] = init_ull;
	}
	else {
		t->bodysize = 2;
		rc_bn_body(t);
		t->body[1] = init_ull / base;
		t->body[0] = init_ull % base;
	}
	return 0;
}

//счет неотображаемых нулей в числе
ull noview0_col(ull a) { /// OK
	ull count = 0;
	for (; (a / (ull)pow(10, ((ull)8 - (ull)count)) == 0) && count != 8; ++count);
	return count;
}

//вывод по цифрам
int print_num(const bn *t) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (t->sign == -1) {
		printf("- ");
	}
	for (ull i = 0; i != t->bodysize; ++i) {
		printf("%llu ", t->body[i]);
	}
	printf("\n");
	return 0;
}

//вывод по цифрам в обратном порядке
int print_num_rev(const bn *t) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (t->sign == -1) {
		printf("- ");
	}
	for (ll i = t->bodysize - 1; i >= 0; --i) {
		printf("%llu ", t->body[i]);
	}
	printf("\n");
	return 0;
}

//вывод
int bn_print(const bn *t) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	if (t->sign == -1) {
		printf("-");
	}
	printf("%llu", t->body[t->bodysize - 1]);
	for (ll i = t->bodysize - 2; i >= 0; --i) {
		ull count = noview0_col(t->body[i]);
		for (ull j = 0; j != count; ++j) {
			printf("%d", 0);
		}
		printf("%llu", t->body[i]);
	}
	printf("\n");
	return 0;
}

//сложение двух чисел по модулю
bn* bn_abs_add(bn const *left, bn const *right) { /// OK
	if (right == NULL || left == NULL || right->body == NULL || left->body == NULL) {
		return bn_new();
	}
	bn *a = bn_new();
    ull q = 0;
    ull min_bodysize = minimum_ull(left->bodysize, right->bodysize);
	ull max_bodysize = maximum_ull(left->bodysize, right->bodysize);
	a->bodysize = max_bodysize + 1;
	rc_bn_body(a);
	for (ull i = 0; i != min_bodysize; ++i) {
		a->body[i] = right->body[i] + left->body[i] + q;
		q = a->body[i] / base;
		a->body[i] %= base;
	}
	int cmp = bn_cmp_abs(left, right);
	if (cmp >= 0) {
		for (ull i = min_bodysize; i != max_bodysize; ++i) {
			a->body[i] = left->body[i] + q;
			q = a->body[i] / base;
			a->body[i] %= base;
		}
	}
	else if (cmp == -1) {
		for (ull i = min_bodysize; i != max_bodysize; ++i) {
			a->body[i] = right->body[i] + q;
			q = a->body[i] / base;
			a->body[i] %= base;
		}
	}
	if (q > 0) {
		a->body[a->bodysize - 1] = q;
	}
	else {
		a->bodysize -= 1;
		rc_bn_body(a);
	}
	return a;
}

//разность чисел по модулю (первое число заведомо больше)
bn* bn_abs_submax(bn const *left, bn const *right) { /// OK
	bn *a = bn_new();
	a->bodysize = left->bodysize;
	rc_bn_body(a);
	for (ull j = 0; j != right->bodysize; ++j) {
		if (left->body[j] < right->body[j]) {
			left->body[j] += base;
			for (ull k = 1; ; ++k) {
				if (left->body[j + k] == 0) {
					left->body[j + k] += base - 1;
					continue;
				}
				left->body[j + k] -= 1;
				break;
			}
		}
		a->body[j] = left->body[j] - right->body[j];
	}
	for (ull j = right->bodysize; j != left->bodysize; ++j) {
		a->body[j] = left->body[j];
	}
	ull col0 = 0;
	for (ull j = 1; a->body[a->bodysize - j] == 0; ++col0, ++j);
	a->bodysize -= col0;
	rc_bn_body(a);
	return a;
}

//сложение двух чисел
bn* bn_add(bn const *left, bn const *right) { /// OK
	bn *a = bn_new();
	if (bn_cmp(left, right) == 1) {
		if (right->sign == 1) {
			bn_change(a, bn_abs_add(left, right));
			a->sign = 1;
		}
		else if (right->sign == 0) {
			bn_copy(a, left);
			a->sign = 1;
		}
		else {
			if (left->sign == -1) {
				bn_change(a, bn_abs_add(left, right));
				a->sign = -1;
			}
			else if (left->sign == 0) {
				bn_copy(a, right);
				a->sign = -1;
			}
			else {
				int cmp1 = bn_cmp_abs(left, right);
				if (cmp1 == 1) {
					bn_change(a, bn_abs_submax(left, right));
					a->sign = 1;
				}
				else if (cmp1 == -1) {
					bn_change(a, bn_abs_submax(right, left));
					a->sign = -1;
				}
			}
		}
	}
	else if (bn_cmp(left, right) == -1) {
		if (left->sign == 1) {
			bn_change(a, bn_abs_add(right, left));
			a->sign = 1;
		}
		else if (left->sign == 0) {
			bn_copy(a, right);
			a->sign = 1;
		}
		else {
			if (right->sign == -1) {
				bn_change(a, bn_abs_add(right, left));
				a->sign = -1;
			}
			else if (right->sign == 0) {
				bn_copy(a, left);
				a->sign = -1;
			}
			else {
				int cmp1 = bn_cmp_abs(right, left);
				if (cmp1 == 1) {
					bn_change(a, bn_abs_submax(right, left));
					a->sign = 1;
				}
				else if (cmp1 == -1) {
					bn_change(a, bn_abs_submax(left, right));
					a->sign = -1;
				}
			}
		}
	}
	else {
		bn_change(a, bn_abs_add(left, right));
		if (left->sign == 1) {
			a->sign = 1;
		}
		else if (left->sign == -1) {
			a->sign = -1;
		}
		else {
			a->sign = 0;
		}
	}
	return a;
}

//+=
int bn_add_to(bn *left, bn const *right) { /// OK
	if (right == NULL || right->body == NULL || left == NULL || left->body == NULL) {
		return 1;
	}
	bn_change(left, bn_add(left, right));
	return 0;
}

//разность по модулю
bn* bn_abs_sub(bn *left, bn const *right) { /// OK
	bn *a = bn_new();
	if (bn_cmp(left, right) == 1) {
		bn_change(a, bn_abs_submax(left, right));
	}
	else if (bn_cmp(left, right) == -1) {
		bn_change(a, bn_abs_submax(right, left));
	}
	return a;
}

//разность
bn* bn_sub(bn const *left, bn const *right) { /// OK
	bn *a = bn_new();
	int cmp1 = bn_cmp_abs(left, right);
	if (cmp1 == 1) {
		if (right->sign * left->sign == 1) {
			bn_change(a, bn_abs_submax(left, right));
			a->sign = left->sign;
		}
		else if (right->sign * left->sign == -1) {
			bn_change(a, bn_abs_add(left, right));
			a->sign = left->sign;
		}
		else if (((left->sign == -1) || (left->sign == 1)) && right->sign == 0) {
			bn_copy(a, left);
		}
	}
	else if (cmp1 == -1) {
		if (right->sign * left->sign == 1) {
			bn_change(a, bn_abs_submax(right, left));
			a->sign = -1 * right->sign;
		}
		else if (right->sign * left->sign == -1) {
			bn_change(a, bn_abs_add(right, left));
			a->sign = left->sign;
		}
		else if (((right->sign == -1) || (right->sign == 1)) && left->sign == 0) {
			bn_copy(a, right);
			a->sign *= -1;
		}
	}
	else if (cmp1 == 0) {
		if (left->sign * right->sign == -1) {
			bn_change(a, bn_abs_add(left, left));
			a->sign = left->sign;
		}
	}
	return a;
}

// -=
int bn_sub_to(bn *left, bn const *right) { /// OK
if (right == NULL || right->body == NULL || left == NULL || left->body == NULL) {
		return 1;
	}
	bn_change(left, bn_sub(left, right));
	return 0;
}

//количество нулей
ull col0(bn* r) { /// OK
	ull count = 0;
	ll p = r->bodysize;
	if (p > 0) {
	    for (ll i = r->bodysize - 1; i >= 0 && r->body[i] == 0; --i, ++count);
	    return count;
	}
	else
        return 0;
}

//произведение на целое число типа int
bn* bn_mul_int(bn const *left, int right) { /// OK
	bn *a = bn_new();
	bn_copy(a, left);
    ull q = 0;
	a->bodysize += 1;
	rc_bn_body(a);
	a->body[a->bodysize - 1] = 0;
	for (ull i = 0; i != left->bodysize; ++i) {
		a->body[i] *= right;
		a->body[i] += q;
		q = a->body[i] / base;
		a->body[i] %= base;
	}
	a->body[a->bodysize - 1] = q;
	a->bodysize -= col0(a);
	rc_bn_body(a);
	return a;
}

//произведение по модулю
bn* bn_abs_mul(bn const *left, bn const *right) { /// OK
	bn *a = bn_new();
	if (right == NULL || left == NULL || right->body == NULL || left->body == NULL) {
		return a;
	}
	if (bn_cmp_abs(right, a) == 0 || bn_cmp_abs(left, a) == 0) {
		return a;
	}
	a->bodysize = right->bodysize + left->bodysize + 1;
	rc_bn_body(a);
	for (ull i = 0; i != a->bodysize; ++i) {
		a->body[i] = 0;
	}
	ull q = 0;
	for (ull i = 0; i != left->bodysize; ++i) {
		q = 0;
		for (ull j = 0; q != 0 || j < right->bodysize; ++j) {
			if (j < right->bodysize) {
				a->body[i + j] += left->body[i] * right->body[j];
			}
			a->body[i + j] += q;
			q = a->body[i + j] / base;
			a->body[i + j] %= base;
		}
	}
	a->bodysize -= col0(a);
	rc_bn_body(a);
	return a;
}

//произведение
bn* bn_mul(bn const *left, bn const *right) { /// OK
	bn* a = bn_new();
	if (!(right->sign == 0) || left->sign == 0) {
		bn_change(a, bn_abs_mul(left, right));
		a->sign = right->sign * left->sign;
	}
	return a;
}

// *=
int bn_mul_to(bn *t, bn const *right) { /// OK
	if (t == NULL || right == NULL || t->body == NULL || right->body == NULL) {
		return 1;
	}
	bn_change(t, bn_mul(t, right));
	return 0;
}

//бинарный поиск для деления
int bin_div(bn *a, bn const *right) { /// OK
	int x = 0, y = 0, z = base;
	char f = 1;
	while (y <= z && f) {
		ull p = (y + z) / 2;
		if (p == 0 && f) {
			++p;
			f = 0;
		}
		bn *c = bn_new();
		bn_change(c, bn_mul_int(right, p));
		if (bn_cmp_abs(c, a) != 1) {
			x = p;
			y = p + 1;
		}
		else {
			z = p - 1;
		}
		bn_delete(c);
	}
	return x;
}

//деление по модулю
bn* bn_abs_div(bn const* left, bn const* right) { /// OK
	bn* res = bn_new();
	if (bn_cmp_abs(left, right) == -1) {
		return res;
	}
	else if (bn_cmp_abs(left, right) == 0) {
		res->body[0] = 1;
		res->sign = 1;
		return res;
	}
	bn* temp = bn_new();
	bn* bn_null = bn_new();
	temp->sign = 1;
	res->sign = 1;
	res->bodysize = 0;
	for (ll i = left->bodysize - 1; i >= 0; --i) {
		temp->body[0] += left->body[i];
		if (bn_cmp_abs(temp, bn_null) == 0) {
			res->bodysize += 1;
			rc_bn_body(res);
			for (ull j = res->bodysize - 1; j != 0; --j) {
				res->body[j] = res->body[j - 1];
			}
			res->body[0] = 0;
		}
		else {
			ull mod = temp->body[0] / base;
			temp->body[0] %= base;
			if (mod > 0) {
				for (ll j = 1; j != temp->bodysize && mod > 0; ++j) {
					temp->body[j] += mod;
					if (j != temp->bodysize - 1) {
						mod = temp->body[j] / base;
						temp->body[j] %= base;
					}
				}
				if (temp->body[temp->bodysize - 1] >= base) {
					temp->bodysize += 1;
					rc_bn_body(temp);
					temp->body[temp->bodysize - 1] = temp->body[temp->bodysize - 2] / base;
					temp->body[temp->bodysize - 2] %= base;
				}
			}
			if (bn_cmp_abs(temp, right) == -1) {
				temp->bodysize += 1;
				rc_bn_body(temp);
				for (ll j = temp->bodysize - 1; j != 0; --j) {
					temp->body[j] = temp->body[j - 1];
				}
				temp->body[0] = 0;
				if (bn_cmp_abs(res, bn_null) == 1) {
					res->bodysize += 1;
					rc_bn_body(res);
					for (ull j = res->bodysize - 1; j != 0; --j) {
						res->body[j] = res->body[j - 1];
					}
					res->body[0] = 0;
				}
				continue;
			}
			int x = bin_div(temp, right);
			res->bodysize += 1;
			if (res->bodysize != 1) {
				rc_bn_body(res);
				for (ull j = res->bodysize - 1; j != 0; --j) {
					res->body[j] = res->body[j - 1];
				}
			}
			res->body[0] = x;
			if (i != 0) {
				temp->sign = 1;
				bn* abs_right = bn_new();
				bn_change(abs_right, bn_abs_do(right));
				bn* mult = bn_new();
				bn_change(mult, bn_mul_int(abs_right, x));
				bn_sub_to(temp, mult);
				bn_delete(mult);
				bn_delete(abs_right);
				temp->bodysize += 1;
				rc_bn_body(temp);
				for (ll j = temp->bodysize - 1; j != 0; --j) {
					temp->body[j] = temp->body[j - 1];
				}
				temp->bodysize -= col0(temp);
				if (temp->bodysize == 0) {
					++temp->bodysize;
				}
				rc_bn_body(temp);
				temp->body[0] = 0;
			}
		}
	}
	bn_delete(temp);
	bn_delete(bn_null);
	res->bodysize -= col0(res);
	rc_bn_body(res);
	return res;
}

//прибавление ++ по модулю
bn* abs_pp_bn(bn *orig) { /// OK
	bn *a = bn_init(orig);
	++a->body[0];
	ull q = a->body[0] / base;
	a->body[0] %= base;
	if (q > 0) {
		for (ll i = 1; i != a->bodysize && q > 0; ++i) {
			a->body[i] += q;
			if (i != a->bodysize - 1) {
				q = a->body[i] / base;
				a->body[i] %= base;
			}
		}
		if (a->body[a->bodysize - 1] >= base) {
			a->bodysize += 1;
			rc_bn_body(a);
			a->body[a->bodysize - 1] = a->body[a->bodysize - 2] / base;
			a->body[a->bodysize - 2] %= base;
		}
	}
	return a;
}

//деление
bn* bn_div(bn const *left, bn const *right) { /// OK
	bn *nl = bn_new();
	bn *a = bn_new();
	if (bn_cmp_abs(left, nl) == 0) {
		bn_delete(nl);
		return a;
	}
	bn_change(a, bn_abs_div(left, right));
	if (bn_cmp_abs(a, nl) == 1) {
		a->sign = left->sign * right->sign;
		bn *ml = bn_new();
		bn_change(ml, bn_abs_mul(a, right));
		if (bn_cmp_abs(ml, left) != 0 && a->sign == -1) {
			bn_change(a, abs_pp_bn(a));
		}
		bn_delete(ml);
	}
	bn_delete(nl);
	return a;
}

// /=
int bn_div_to(bn* left, bn const* right) { /// OK
	if (right == NULL || right->body == NULL || left == NULL || left->body == NULL) {
		return 1;
	}
	bn_change(left, bn_div(left, right));
	return 0;
}

//остаток по модулю
bn* bn_abs_mod(bn const *left, bn const *right) { /// OK
	bn *a = bn_init(left);
	bn *d = bn_new();
	bn_change(d, bn_abs_div(left, right));
	bn *ml = bn_new();
	bn_change(ml, bn_abs_mul(right, d));
	bn_delete(d);
	bn *a1 = bn_new();
	bn_change(a1, bn_abs_do(a));
	ml->sign = 1;
	bn_change(a, bn_abs_sub(a1, ml));
	bn_delete(a1);
	bn_delete(ml);
	return a;
}

//остаток от деления
bn* bn_mod(bn const *left, bn const *right) { /// OK
	bn *a = bn_new();
	bn_change(a, bn_abs_mod(left, right));
	a->sign = 0;
	bn *nl = bn_new();
	if (bn_cmp_abs(a, nl) == 1) {
		if (right->sign == 1 && left->sign == -1) {
			a->sign = 1;
			bn_change(a, bn_sub(right, a));
		}
		else if (right->sign == -1 && left->sign == -1) {
			a->sign = -1;
		}
		else if (right->sign == -1 && left->sign == 1) {
			a->sign = -1;
			bn *rt = bn_new();
			bn_change(rt, bn_abs_do(right));
			bn_change(a, bn_add(rt, a));
			a->sign = -1;
			bn_delete(rt);
		}
		else {
			a->sign = 1;
		}
	}
	bn_delete(nl);
	return a;
}

// %=
int bn_mod_to(bn *left, bn const *right) { /// OK
	if (right == NULL || right->body == NULL || left == NULL || left->body == NULL) {
		return 1;
	}
	bn_change(left, bn_mod(left, right));
	return 0;
}

//возведение в степень по модулю
bn* bn_abs_pow(bn const *t, int const degree) { /// OK
	bn *x = bn_init(t);
	int d = degree;
	bn *a = bn_new();
	bn_init_int(a, 1);
	while (d != 0) {
		if (d % 2 != 0) {
			bn_mul_to(a, x);
			--d;
		}
		else {
			bn_mul_to(x, x);
			d /= 2;
		}
	}
	bn_delete(x);
	return a;
}

//возведение в степень
int bn_pow_to(bn *t, int degree) { /// OK
	if (t->body == NULL || t == NULL) {
		return 1;
	}
	bn *nl = bn_new();
	if (bn_cmp_abs(t, nl) == 0) {
		bn_change(t, nl);
		return 0;
	}
	bn_delete(nl);
	if (t->body[0] == 1 && t->bodysize == 1) {
		if (degree % 2 == 0) {
			t->sign = 1;
		}
		return 0;
	}
	char neg = (degree % 2 != 0 && t->sign == -1);
	bn_change(t, bn_abs_pow(t, degree));
	if (neg) {
		t->sign = -1;
	}
	else {
		t->sign = 1;
	}
	return 0;
}

//извлечение корня по модулю
bn* abs_bn_root(bn const *t, int reciprocal) { /// OK
	bn *a = bn_new();
	if (bn_cmp_abs(t, a) == 0) {
		return a;
	}
	a->sign = 1;
	int p = (t->bodysize + reciprocal-1) / reciprocal;
	a->bodysize = p--;
	if (a->bodysize != 1) {
		rc_bn_body(a);
	}
	for (ull i = 0; i != a->bodysize; ++i) {
		a->body[i] = 0;
	}
	while (p >= 0) {
		ull x = 0, y = base, cd = 0;
		while (x <= y) {
			ull z = (x + y) / 2;
			a->body[p] = z;
			if (z != 0) {
				bn *ap = bn_init(a);
				bn_pow_to(ap, reciprocal);
				if (bn_cmp(ap, t) != 1) {
					cd = z;
					x = z + 1;
				}
				else {
					y = z - 1;
				}
				bn_delete(ap);
			}
		}
		a->body[p] = cd;
		--p;
	}
	ull k = col0(a);
	if (k != 0) {
		a->bodysize -= k;
		rc_bn_body(a);
	}
	return a;
}


int bn_root_to(bn *t, int reciprocal) { /// OK
	if (t == NULL || t->body == NULL) {
		return 1;
	}
	bn *nl = bn_new();
	if (reciprocal % 2 != 0 && bn_cmp(t, nl) == -1) {
		bn *a = bn_new();
		bn_change(a, bn_abs_do(t));
		bn_change(t, abs_bn_root(a, reciprocal));
		bn_neg(t);
		bn_delete(a);
	}
	else if (bn_cmp(t, nl) == 0) {
		bn_delete(nl);
		return 0;
    }
	else if (bn_cmp(t, nl) == 1) {
		bn_change(t, abs_bn_root(t, reciprocal));
	}
	bn_delete(nl);
	return 0;
}

//возведение в степень большого числа
ull pow_big(int const nb, ull const n_degree) { /// OK
	ull r = 1, d = n_degree, n = nb;
	while (d != 0)
		if (d % 2 != 0) {
			r *= n;
			--d;
		}
		else {
			n *= n;
			d /= 2;
		}
	return r;
}

// инициализация BN в виде строки в системе счисления: radix
int bn_init_string_radix(bn *t, const char *init_string, int radix) {
	if (init_string == NULL || t->body == NULL || t == NULL) {
		return 1;
	}
	if (radix == 10) {
		bn_init_string(t, init_string);
		return 0;
	}
	int l = strlen(init_string);
	int inx = l;
	char neg = 0;
	if (init_string[0] == '-') {
		neg = 1;
		--inx;
	}
	else if (init_string[0] == '0') {
		t->sign = 0;
		return 0;
	}
	t->sign = 1;
	bn *br = bn_new();
	bn_init_int(br, radix);
	for (int i = 0; i != inx; ++i) {
		bn *std = bn_new();
		if (init_string[l - (i + 1)] <= 'Z' && init_string[l - (i + 1)] >= 'A') {
			bn_init_ull(std, (ull)init_string[l - (i + 1)] + 10 - (ull)'A');
			bn *p = bn_new();
			bn_change(p, bn_abs_pow(br, i));
			bn_mul_to(std, p);
			bn_delete(p);
		}
		else {
			bn_init_ull(std, (ull)init_string[l - (i + 1)] - (ull)'0');
			bn *p = bn_new();
			bn_change(p, bn_abs_pow(br, i));
			bn_mul_to(std, p);
			bn_delete(p);
		}
		bn_add_to(t, std);
		bn_delete(std);
	}
	bn_delete(br);
	if (neg) {
		t->sign = -1;
	}
	return 0;
}

//представление BN в сист счисл radix в виде строки
const char* bn_to_string(bn const* t, int radix) { /// OK
	char *s;
	if (radix == 10) {
		ull cfd = 1;
		for (; t->body[t->bodysize - 1] / pow_big(10, cfd); ++cfd);
		ull l = (t->bodysize - 1) * 9 + cfd;
		ull stinx = 0;
		if (t->sign == -1) {
			++stinx;
			++l;
			s = (char*)malloc((l + 1) * sizeof(char));
			s[0] = '-';
		}
		else {
			s = (char*)malloc((l + 1) * sizeof(char));
		}
		for (ull i = cfd; i != 0; --i) {
			ull ld = t->body[t->bodysize - 1];
			ull f = (ld / pow_big(10, (i - 1))) % 10;
			s[stinx + (cfd - i)] = f + '0';
		}
		stinx += cfd;
		for (ll i = t->bodysize - 2, k = 0; i >= 0; --i, ++k) {
			ull st = stinx + 9 * k;
			ull ld = t->body[i];
			for (ull j = 0; j != 9; ++j) {
				char f = (ld / pow_big(10, (8 - j))) % 10;
				s[st + j] = f + '0';
			}
		}
		s[l] = '\0';
		return s;
	}
	if (t->body[t->bodysize - 1] == 0) {
		s = (char*)malloc(2 * sizeof(char));
		s[0] = '0';
		s[1] = '\0';
		return s;
	}
	bn* f = bn_init(t);
	bn* br = bn_new();
	bn_init_int(br, radix);
	ull x = 0, md = 0, y = 100000;
	while (x <= y) {
		ull z = (x + y) / 2;
		bn* a = bn_new();
		bn_change(a, bn_abs_pow(br, z));
		if (bn_cmp_abs(a, t) != 1) {
			md = z;
			x = z + 1;
		}
		else {
			y = z - 1;
		}
		bn_delete(a);
	}
	ll j = 0;
	if (t->sign == -1) {
		s = (char*)malloc((md + 3) * sizeof(char));
		s[0] = '-';
		j = 1;
		s[md + 2] = '\0';
	}
	else {
		s = (char*)malloc((md + 2) * sizeof(char));
		s[md + 1] = '\0';
	}
	for (ll i = md; i >= 0; --i, ++j) {
		bn* rp = bn_new();
		bn_change(rp, bn_abs_pow(br, i));
		bn* div = bn_new();
		bn_change(div, bn_abs_div(f, rp));
		ull d = div->body[0]; //
		if (0 <= d && d <= 9) {
			s[j] = '0' + d;
		}
		else if (10 <= d) {
			s[j] = 'A' + d - 10;
		}
		f->sign = 1;
		bn_mod_to(f, rp);
		bn_delete(div);
		bn_delete(rp);
	}
	bn_delete(f);
	bn_delete(br);
	return s;
}
