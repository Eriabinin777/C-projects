#include <stdio.h>
#include <cstring>
#include <map>
#include <string>
#include <iostream>
using namespace std;

double flags;

char memo[1024*1024*2];

union{
	long long reg_i[32];
	double reg_d[32];
} registers;

union{
	long long a;
	double d;
} register_1;

union{
	char c[8];
	unsigned int b[2];
	long long a;
} wr;

void push_4(int w, unsigned int b){
	wr.b[0] = b;
	for(int i = 0; i <4 ; ++i)
        memo[w++] = wr.c[i];
}
void push_8(int w, long long b){
	wr.a = b;
	for (int i = 0; i < 8; ++i)
        memo[w++] = wr.c[i];
}

int read_4(int w){
	for (int i = 0; i < 4; ++i) wr.c[i] = memo[w++];
	return wr.b[0];
}
long long read_8(int w){
	for (int i = 0; i < 8; ++i)
        wr.c[i] = memo[w++];
	return wr.a;
}

map<string,char> code;
int init_code(void){
	code["halt"] = 0;
	code["svc"] = 1;
	code["add"] = 2;
	code["sub"] = 3;
	code["mul"] = 4;
	code["div"] = 5;
	code["mod"] = 6;
	code["and"] = 7;
	code["or"] = 8;
	code["xor"] = 9;
	code["nand"] = 10;
	code["shl"] = 11;
	code["shr"] = 12;
	code["addd"] = 13;
	code["subd"] = 14;
	code["muld"] = 15;
	code["divd"] = 16;
	code["itod"] = 17;
	code["dtoi"] = 18;
	code["bl"] = 19;
	code["cmp"] = 20;
	code["cmpd"] = 21;
	code["cne"] = 22;
	code["ceq"] = 23;
	code["cle"] = 24;
	code["clt"] = 25;
	code["cge"] = 26;
	code["cgt"] = 27;
	code["ld"] = 28;
	code["st"] = 29;
	code["word"] = -1;
	code["dword"] = -2;
	code["double"] = -3;
	code["bytes"] = -4;
	code["end"] = -5;
	for (int i = 0; i < 31; ++i)
        registers.reg_i[i] = 0;
	registers.reg_i[29] = 2097144;
	return 0;
}

char g_t1(string& s){
	if (s == "ld" || s == "st")
	    return 2;
	if (s == "bl")
        return 1;
	return 0;
}

unsigned int g_t(unsigned int a){
	if (a == 28 || a == 29)
        return 2;
	if (a == 19)
	    return 1;
	return 0;
}

unsigned int g_r_n(string s){
	if (s[0] == 'l')
        return 30;
	if (s[0] == 's')
	    return 29;
	if (s[0] == 'p')
	    return 31;
	if (s[0] == 'f')
	    return 28;
	if (s[0] == 'r' && s[1] == 'z')
	    return 27;
	if (s[2] == '\0')
	    return s[1] - '0';
	return (char) (s[1]) * 10 + s[2] - 11 * '0';
}

int g_u(string& s,map<string,short>* m = nullptr){
	if (isalpha(s[0]))
        return (*m)[s];
	int a;
	sscanf(s.c_str(),"%ud",&a);
	return a;
}

int g_s(string& s, int size = 16, map<string,short>* m = nullptr){
	if (isalpha(s[0]) && m)
        return (*m)[s];
	int a;
	sscanf(s.c_str(),"%d",&a);
	if(a < 0){
		a = ((int)1 << size) + a;
	}
	return a;
}

int t_s(int a, int size){
	if (a & (1 << (size - 1))) a = -((1 << size)-a);
	return a;
}

void RM(unsigned int& a, string* s, short fr, map<string,short>& m, short k){
	a = 0;
	int c_n = (int)code[s[fr]];
	int s_r = g_r_n(s[fr + 2]);
	if (s_r == 27 || s_r == 29)
        a = (c_n << 26) + (g_r_n(s[fr + 1]) << 21)+ (s_r << 16) +  g_u(s[fr + 3],&m);
	else if (s_r == 31)
	    a = (c_n << 26) + (g_r_n(s[fr + 1]) << 21)+ (s_r << 16) +  g_s(s[fr + 3], 16);
	else{
		int t_r = g_r_n(s[fr + 3]);
		if (t_r != 27)
		    a = (c_n << 26) + (g_r_n(s[fr + 1]) << 21) + (s_r << 16) + (t_r << 11) + (g_u(s[fr + 4], &m)<< 8) + g_s(s[fr + 5], 8);
		else
		    a = (c_n << 26) + (g_r_n(s[fr + 1]) << 21) + (s_r << 16) + (t_r << 11) + g_s(s[fr + 4], 11);
	}
}

void RR(unsigned int& a, string* s, short fr, map<string,short>& m, short k, short pc){
	a = 0;
	unsigned int c_n = (unsigned int)code[s[fr]];
	if (c_n >= 22 && c_n <= 27){
		if (k - fr == 2)
            s[fr+3] = s[fr + 1], s[fr+2] = "rz", s[fr+1] = "pc";
		else if (k - fr == 3)
		    s[fr+3] = s[fr + 2],s[fr+2] = "rz";
		k = 4;
	}
	unsigned int s_r = g_r_n(s[fr + 2]);
	if (s_r == 27)
        a = (c_n << 26) + (g_r_n(s[fr + 1])<< 21)+ (s_r << 16) +  g_s(s[fr + 3],16, &m);
	else if (s_r == 31 && k ==  4 + fr){
		int ar = g_s(s[fr+3],16, &m);
		if (isalpha(s[fr + 3][0]))
		    ar -= pc + 4;
		if (ar < 0)
		    ar = ((int)1 << 16) + ar;
 		a = (c_n << 26) + (g_r_n(s[fr + 1]) << 21) + (s_r << 16) + ar;
	}
	else
	    a = (c_n << 26) + (g_r_n(s[fr + 1])<< 21) + (s_r << 16) + (g_r_n(s[fr + 3]) << 11) + (g_u(s[fr + 4]) << 8) + g_s(s[fr + 5],8);
}

void B(unsigned int& a, string* s, short fr, map<string,short>& m, short k,short pc){
	a = 0;
	int c_n = (int)code[s[fr]];
	if (k - fr == 2  || k - fr == 4){
		s[fr+2] = s[fr+1];
		s[fr+1] = "pc";

	}
	int s_r = g_r_n(s[fr + 1]);
	if (s_r == 27)
        a = (c_n << 26) + (s_r << 21) + g_u(s[fr+2], &m);
	else if (s_r == 31){
        int ar = g_s(s[fr+2],21, &m);
		if(isalpha(s[fr + 2][0]))
            ar -= pc;
		if (ar < 0)
		    ar = ((int)1 << 21) + ar;
        a = (c_n << 26) + (s_r << 21) + ar;
	}
	else{
		int t_r = g_r_n(s[fr + 2]);
		if (t_r == 27)
		    a = (c_n << 26) + (s_r << 21) + (t_r << 16) + (g_u(s[fr + 3], &m) << 8) + g_u(s[fr + 3], &m);
		else
		    a = (c_n << 26) + (s_r << 21) + (t_r << 16)  + (g_u(s[fr + 3],&m) << 13) + g_s(s[fr + 4], 13);
	}
}

short g_l(char * s, string* ss, short& d){
	short k = 0, n = 0, pr = 0, t;
	bool p = true;
	while (!isalnum(s[(int)n]) && s[n]!= ';' && s[n]!= '\0')
        ++n;
	if ((s[n] == ';' || s[n] == '\0') && (n == 0 || !isalnum(s[(int)n])))
	    return 0;
	pr = n;
	while (s[n]!= ';' && s[n]!= '\0'){
		while ((isalnum(s[n]) || s[n] == '_') && s[n]!= ';' && s[n]!= '\0')
		    ++n;
		t = s[n];
		s[n] = '\0';
		ss[k++] = s + pr;
		s[n] = t;
		if(p){
			d = n;
			p= false;
		}
		while(!isalnum(s[(int)n]) && s[n]!= ';' && s[n]!= '\0')
		    ++n;
		pr = n;
	}
	return k;
}


int asmb(string ss){
    unsigned int pc = 0, to_wr = 0;
    short d = 0, k = 0, fr = 0;
	map<string,short> m;
	string s[10];
	char bf[70];
	FILE *f = fopen(ss.c_str(),"r");
	if (!f) {
		printf("OOPS");
		exit(2);
	}
	while (fgets(bf,70,f)){
		k = g_l(bf, s, d);
		if (k != 0) {
			if (bf[d] == ':'){
                if (!isdigit(s[0][0]) && m.find(s[0]) == m.end()){
                    m[s[0]] = pc;
                    if(k > 1)
                        pc +=4;
                } else {
                    printf("error in line %d\n", pc/4);
                    fclose(f);
                    exit(2);
                    }
            } else if (code.find(s[0]) == code.end()){
			fclose(f);
			printf("error in line %d\n", pc/4);
			exit(2);
            }
		else
            pc+=4;
		}
	}
	g_l(bf, s, d);
	fr = 0;
	if(bf[d] == ':'){
		if(!isdigit(s[0][0]) && m.find(s[0]) == m.end())
            m[s[0]] = pc;
		fr = 1;
	}

	registers.reg_i[31] = (long long)m[s[fr + 1]]+4;
	pc = 0;
	fseek(f, 0 , SEEK_SET);
	while(fgets(bf,70,f)){
		k = g_l(bf, s, d);
		if (k == 0)
            continue;
		fr = 0;
		if (bf[d] == ':')
            fr = 1;
		if (k != 1 && s[fr] != "end"){
			if (s[fr] == "word"){
				int z;
				sscanf(s[fr + 1].c_str(),"%d",&z);
				push_4(pc, z);
				pc += 4;
			} else if (s[fr] == "dword"){
				long long z;
				sscanf(s[fr + 1].c_str(),"%lld",&z);
				push_8(pc, z);
				pc += 8;
			}
			else if (s[fr] == "double"){
				sscanf(s[fr + 1].c_str(),"%lf", &register_1.d);
				push_8(pc, register_1.a);
				pc +=8;
			} else if(s[fr] == "bytes"){
				sscanf(s[fr + 1].c_str(),"%lld",&register_1.a);
				register_1.a = (((register_1.a & 3 )!= 0) << 2) + (register_1.a &(-4));
				for (int i = 0; i < register_1.a; ++i)
				    memo[pc++] = 0;
			} else {
				switch(g_t1(s[fr])){
					case 0:
						RR(to_wr, s, fr, m, k, pc);
						break;
					case 1:
						B(to_wr, s, fr, m, k, pc);
						break;
					case 2:
						RM(to_wr, s, fr, m, k);
						break;
					default:
						printf("error in line %d\n", pc);
						break;
				}
				push_4(pc, to_wr);
				pc += 4;
			}
		}
	}

	fclose(f);
	return 0;
}

bool ck = false;
int cks(){
	printf("r0 = %lld |_| r11 = %lld |_| r12 = %lld |_r13 = %lld|  rz = %lld |_| fp = %lld |_| sp = %lld |__| lr = %lld |_| pc = %lld | flag =%lf\n",
	registers.reg_i[0],registers.reg_i[11], registers.reg_i[12], registers.reg_i[13], registers.reg_i[27], registers.reg_i[28], registers.reg_i[29], registers.reg_i[30], registers.reg_i[31],flags);
	int a;
	scanf("%d",&a);
	return 0;
}

int emul(){
	int t1,t2,t3,t4,t5,t6;
	unsigned int z;
	while (true){
		t1 = t2=t3=t4=t5=t6=z=-10000000;
		z = (unsigned int) read_4(registers.reg_i[31]-4);
		t1 = (unsigned int)z>>26;
		t2 = ((z & (((unsigned int)1 << 26)- ((unsigned int)1 << 21))) >> 21);
		t3 = ((z & (((unsigned int)1 << 21)- ((unsigned int)1 << 16))) >> 16);
		switch(g_t(t1)){
			case 0:
				if(t3 == 27 || t3 == 31)
                    t4 = t_s(z&(((unsigned int)1 << 16) - ((unsigned int)1 << 0)), 16);
				else{
					t4 = ((z&(((unsigned int)1 << 16) - ((unsigned int)1 << 11))) >> 11);
					t5 = ((z&(((unsigned int)1 << 11) - ((unsigned int)1 << 8))) >> 8);
					t6 = t_s(z&(((unsigned int)1 << 8) - ((unsigned int)1 << 0)), 8);
				}
				break;
			case 1:
				if (t2 == 27)
                    t3 = z&(((unsigned int)1 << 21)- ((unsigned int)1 << 0));
				else if (t2 == 31)
				    t3 = t_s(z&(((unsigned int)1 << 21) - ((unsigned int)1 << 0)),21);
				else if (t3 == 27)
				    t4 = ((z&(((unsigned int)1 << 16) - ((unsigned int)1 << 8))) >> 8), t5 = ((z&(((unsigned int)1 << 8) - ((unsigned int)1))));
				else
				    t4 = ((z&(((unsigned int)1 << 16)- ((unsigned int)1 << 13))) >> 13), t5 = t_s(z&(((unsigned int)1 << 13) - ((unsigned int)1 << 0)), 13);
				break;
			case 2:
				if (t3 == 27 || t3 == 29)
                    t4 = z&(((unsigned int)1 << 16) - ((unsigned int)1 << 0));
				else if (t3 == 31)
				    t5 = t_s(z&(((unsigned int)1 << 16) - ((unsigned int)1 << 0)), 16);
				else{
					t4 = ((z&(((unsigned int)1 << 16) - ((unsigned int)1 << 11))) >> 11);
					if (t4 != 27){
						t5 =((z&(((unsigned int)1 << 11) - ((unsigned int)1 << 8))) >> 8);
						t6 = t_s(z&(((unsigned int)1 << 8) - ((unsigned int)1 << 0)), 8);
					} else t5 = t_s(z&(((unsigned int)1 << 11) - ((unsigned int)1 << 0)), 11);
				}
				break;
			default:
				printf("error");
				return -1;
		}

		if (ck)
            printf("ex = %lld |_| where = %lld |_| t1 = %d |_| t2 = %d |_| t3 = %d |__| t4 = %d |_| t5 = %d |_| t6 = %d\n",
				registers.reg_i[31]/4,registers.reg_i[31],t1,t2,t3,t4,t5,t6),cks();

		switch(t1){
			case 0:{
				exit(0);
			}
			break;
			case 1:{
				switch(t4){
					case 0:{
						return 0;
					}
					break;
					case 100:   scanf("%lld",&registers.reg_i[t2]);
					break;
					case 101:   scanf("%lg",&registers.reg_d[t2]);
					break;
					case 102:   printf("%lld",registers.reg_i[t2]);
					break;
					case 103:   printf("%lg",registers.reg_d[t2]);
					break;
					case 104:   registers.reg_i[t2] = getchar();
					break;
					case 105:   putchar(registers.reg_i[t2]);
					break;
				}
			}
			break;
			case 2:{
				if (t3 == 27 || t3 == 31)
				    registers.reg_i[t2] = registers.reg_i[t3] + t4;
				else
				    registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 3:{
				if (t3 == 27)
				    registers.reg_i[t2] -= t4;
				else
				    registers.reg_i[t2] = registers.reg_i[t3] - (registers.reg_i[t4] << t5) - t6;
			}
			break;
			case 4:{
                if (t3 == 27)
                    registers.reg_i[t2] *= t4;
				registers.reg_i[t2] = registers.reg_i[t3] * ((registers.reg_i[t4] << t5) + t6);
			}
			break;
			case 5:{
				if (((registers.reg_i[t4] << t5) - t6) == 0)
				    exit(-1);
				registers.reg_i[t2] = (unsigned long long)registers.reg_i[t3] /(unsigned long long)((registers.reg_i[t4] << t5) + t6) ;

			}
			break;
			case 6:{
				registers.reg_i[t2] = (unsigned long long)registers.reg_i[t3] % (unsigned long long)((registers.reg_i[t4] << t5) + t6) ;
			}
			break;
			case 7:{
				registers.reg_i[t2] = registers.reg_i[t3] & ((registers.reg_i[t4] << t5)+ t6);
			}
			break;
			case 8:{
				registers.reg_i[t2] = registers.reg_i[t3] | ((registers.reg_i[t4] << t5)+ t6);
			}
			break;
			case 9:{
				registers.reg_i[t2] = registers.reg_i[t3] ^ ((registers.reg_i[t4] << t5)+ t6);
			}
			break;
			case 10:{
				registers.reg_i[t2] = registers.reg_i[t3] & (~((registers.reg_i[t4] << t5)+ t6));
			}
			break;
			case 11:{
				registers.reg_i[t2] = registers.reg_i[t3] << ((registers.reg_i[t4] << t5)+ t6);
			}
			break;
			case 12:{
				registers.reg_i[t2] = registers.reg_i[t3] >> ((registers.reg_i[t4] << t5)+ t6);
			}
			break;
			case 13:{
				registers.reg_d[t2] = registers.reg_d[t3] + (registers.reg_d[t4]*(1 << t5) + t6);
			}
			break;
			case 14:{
				registers.reg_d[t2] = registers.reg_d[t3] - (registers.reg_d[t4]*(1 << t5) + t6);
			}
			break;
			case 15:{
				registers.reg_d[t2] = registers.reg_d[t3] * (registers.reg_d[t4]*(1 << t5) + t6);
			}
			break;
			case 16:{
				if (!(registers.reg_d[t4] * (1 << t5) + t6))
				    exit(-1);
				registers.reg_d[t2] = registers.reg_d[t3] / (registers.reg_d[t4]*(1 << t5) + t6);
			}
			break;
			case 17:{
				if (t3 == 27)
				    registers.reg_d[t2] = (double) t4;
				else
				    registers.reg_d[t2] = (double)(registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6);
			}
			break;
			case 18:{
				if (t3 == 27)
				    registers.reg_i[t2] = (long long) t4;
				else
				    registers.reg_i[t2] = (int64_t)(registers.reg_d[t3] + (registers.reg_d[t4]*(1 << t5)) + t6);
			}
			break;
			case 19:{
				registers.reg_i[30] = registers.reg_i[31];
				if (t2 == 27)
				    registers.reg_i[31] = t3;
				else if (t2 == 31)
				    registers.reg_i[31] = t3 + registers.reg_i[31] - 4;
				else
				    registers.reg_i[31] = registers.reg_i[t2] + (registers.reg_i[t3] << t4) + t5;
			}
			break;
			case 20:{
				t3 == 27 ? flags = registers.reg_i[t2] - t4 : flags = registers.reg_i[t2] - (registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6);
			}
			break;
			case 21:{
				t3 == 27 ? flags = registers.reg_d[t2] - t4 : flags = registers.reg_d[t2] - (registers.reg_d[t3] + (registers.reg_d[t4]*(1 << t5)) + t6);
			}
			break;
			case 22:{
				if (flags)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 23:{
				if (!flags)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 24:{
				if (flags <= 0)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 25:{
				if (flags < 0)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 26:{
				if (flags >= 0)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 27:{
				if (flags > 0)
				    t3 == 27 ? registers.reg_i[t2] = t4 : registers.reg_i[t2] = registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6;
			}
			break;
			case 28:{
				if (t3 == 31 || t3 == 27)
				    registers.reg_i[t2] = read_8(registers.reg_i[t3] + t4);
				else if (t3 == 29) {
					registers.reg_i[t2] = read_8(registers.reg_i[t3]);
					registers.reg_i[t3] += (((t4 & 3 )!= 0) << 2) + (t4 & (-4));
				} else{
					if (t4 != 27)
					    registers.reg_i[t2] = read_8(registers.reg_i[t3] + (registers.reg_i[t4] << t5) + t6);
					else
					    registers.reg_i[t2] = read_8(registers.reg_i[t3] + t5);
				}
			}
			break;
			case 29:{
				if (t3 == 31 || t3 == 27)
				    push_8(registers.reg_i[t3] + t4, registers.reg_i[t2]);
				else if (t3 == 29){
					registers.reg_i[t3] -= (((t4 & 3) != 0) << 2) + (t4 & (-4));
					push_8(registers.reg_i[t3], registers.reg_i[t2]);
				}
				else{
					if (t4 != 27)
                        push_8(registers.reg_i[t3] + (registers.reg_i[t4]<<t5) + t6, registers.reg_i[t2]);
                    else push_8(registers.reg_i[t3] + t5, registers.reg_i[t2]);
					}
			}
		}
		registers.reg_i[31] += 4;
	}
}

int main(){
	init_code();
	string s = "input.fasm";
	asmb(s);
	return emul();
}
