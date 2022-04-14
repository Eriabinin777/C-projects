#include <stdio.h>
#include <malloc.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <map>
#include <string>
#include <bitset>
using namespace std;

bitset<142> bites("10101010000000000000000000000000001111111111111110011011111001011010101010100000000000000010010110011001100110011001000001100110011001100101");

enum types {
	RM = 0,
	RR = 1,
	RI = 2,
	J  = 3
};

int get_type(short i){
	return (bites[i + i] << 1) + (bites[i + i + 1]);
}
short nums(int a, int b){
	short t = 0;
	t = t + a + (b << 8) ;
	return t;
}

map<string,short> code;
map<char,string> sys_code;

int init_code(void){
	code["halt"] = nums(0,RI);
	code["syscall"] = nums(1,RI);
	code["add"] = nums(2,RR);
	code["addi"] = nums(3,RI);
	code["sub"] = nums(4,RR);
	code["subi"] = nums(5,RI);
	code["mul"] = nums(6,RR);
	code["muli"] = nums(7,RI);
	code["div"] = nums(8,RR);
	code["divi"] = nums(9,RI);
	code["lc"] = nums(12,RI);
	code["shl"] = nums(13,RR);
	code["shli"] = nums(14,RI);
	code["shr"] = nums(15,RR);
	code["shri"] = nums(16,RI);
	code["and"] = nums(17,RR);
	code["andi"] = nums(18,RI);
	code["or"] = nums(19,RR);
	code["ori"] = nums(20,RI);
	code["xor"] = nums(21,RR);
	code["xori"] = nums(22,RI);
	code["not"] = nums(23,RI);
	code["mov"] = nums(24,RR);
	code["addd"] = nums(32,RR);
	code["subd"] = nums(33,RR);
	code["muld"] = nums(34,RR);
	code["divd"] = nums(35,RR);
	code["itod"] = nums(36,RR);
	code["dtoi"] = nums(37,RR);
	code["push"] = nums(38,RI);
	code["pop"] = nums(39,RI);
	code["call"] = nums(40,RR);
	code["calli"] = nums(41,J);
	code["ret"] = nums(42,J);
	code["cmp"] = nums(43,RR);
	code["cmpi"] = nums(44,RI);
	code["cmpd"] = nums(45,RR);
	code["jmp"] = nums(46,J);
	code["jne"] = nums(47,J);
	code["jeq"] = nums(48,J);
	code["jle"] = nums(49,J);
	code["jl"] = nums(50,J);
	code["jge"] = nums(51,J);
	code["jg"] = nums(52,J);
	code["load"] = nums(64,RM);
	code["store"] = nums(65,RM);
	code["load2"] = nums(66,RM);
	code["store2"] = nums(67,RM);
	code["loadr"] = nums(68,RR);
	code["storer"] = nums(69,RR);
	code["loadr2"] = nums(70,RR);
	code["storer2"] = nums(71,RR);
	return 0;
	}

int* registers = (int *) calloc(16, sizeof(*registers));
int* memo = (int *) calloc(1024 * 1024, sizeof(*memo));
int flag;

FILE* f = NULL;

union{
	long long a;
	double b;
} registers_1;

union{
	long long a;
	double b;
} registers_2;


map<string,unsigned int> check;

string str(char * c){
	string s = c;
	return s;
}

short get_code(char * t){
	return code[str(t)] & ((1 << 8) - 1);
}

unsigned int get_int_unsigned(char * t){
	int a = 0, i = 0;
	while(t[i] != '\0')
        a = 10 * a + (t[i++] - '0');
	return a;
}

int get_int_signed(char * t, int size){
	bool c = false;
	int a = 0, i = 0;
	if(t[0] == '-'){
		c = !c;
		++i;
	}
	while(t[i] != '\0')
        a = 10 * a + (t[i++] - '0');
	if(c) a = (((int) 1) << size) - a;
	return a;
}

int p = 0;
int p1 = 0;
char l[5][50];
char b[100];

bool isnum(const char * t){
	int i = 0;
	if(t[0] == '-') ++i;
    while (t[i] != '\0' && isdigit(t[i])) ++i;
    return t[i] == '\0';
}

void RM_(int * a, int fr){
	unsigned int t;
	if(isnum(l[fr+2] + 1))
        t = get_int_unsigned(l[fr + 2] + 1) & (((int)1 << 20) - 1);
	else{
		string s = str(l[fr + 2] +1);
		if(check.find(s) != check.end()) t = check[s];
		else {
			printf("error in line %d\n", p1);
			exit(1);
		}
	}

	*a = 0 + ((int)get_code(l[fr] + 1) << 24) + (get_int_unsigned(l[fr + 1] + 2) << 20) + t;
}

void RR_(int * a, int fr){
	*a = 0 + ((int)get_code(l[fr] + 1) << 24) + (get_int_unsigned(l[fr +1] + 2) << 20) + (get_int_unsigned(l[fr + 2] + 2) << 16)+ get_int_signed(l[fr + 3] + 1, 16);
}

void RI_( int * a, int fr){
	unsigned int t;
	string s = str(l[fr + 2] + 1);

	if(isnum(l[fr+2] + 1)){
		t = get_int_signed(l[fr + 2] + 1, 20);
	}
	else if (check.find(s) != check.end()){
		t = check[s];
	} else {cout << l[fr + 2] + 1;
			printf("error in line %d\n", p1);
			exit(2);
		}
	*a = 0 + ((int)get_code(l[fr] + 1) << 24) + (get_int_unsigned(l[fr + 1] + 2) << 20) + t;
}

void J_(int * a, int fr){
	unsigned int t;
	string s = str(l[fr + 1] +1);
	if (isnum(l[fr + 1] + 1))
        t = get_int_unsigned(l[fr + 1]+1);
	else if(check.find(s) != check.end()) t = check[s];
	else {
		fprintf(f, "error in line %d\n", p1);
		exit(3);
	}
	*a = 0 + ((int)get_code(l[fr] + 1) << 24) + t;
}


unsigned int get_l(void){
	short num = 0;
	int tk = 0, pr = 0;
	while(!isalpha(b[tk]))
        ++tk;
	pr = tk;
	while(b[tk] != '\0' && b[tk] != '\n'){
		if(b[tk] == ' '){
			int j = 1;
			l[num][0] = tk - pr;
			for(int i = pr; i < tk; ++i, ++j){
				l[num][j] = b[i];
			}
			l[num][j] = '\0';
			if(l[num][j-1] == ',') l[num][j-1] = '\0';
			++tk;
			++num;
			while(b[tk] == ' ') ++tk;
			pr = tk;
		}
		++tk;
	}
	int j = 1;
	l[num][0] = tk - pr;
	for(int i = pr; i  < tk; ++i, ++j){
		l[num][j] = b[i];
	}
	if(l[num][j-1] == ','){
		l[num][j-1] = '\0';
	}
	l[num][j] = '\0';

	++num;
	return num;
}

int assemb(string s){
	unsigned int n = 0, type = 0, fr = 0;
	p = 0, p1 = 0;
	int wr;
	FILE *g = fopen(s.c_str(), "r");
	if (!g) {
		printf("OOPS");
		return -1;
	}

	while(fgets(b, 100, g)){
		n = get_l();
		if(l[0][(int)l[0][0]] == ':'){
			l[0][(int)l[0][0]] = '\0';
			string s = str(l[0]+1);
			if(!isdigit(s[0]) && check.find(s) == check.end()){
				check[s] = p;
				if(n > 1) ++p;
			} else{
				fprintf(f, "error in line %d\n", p1);
				exit(2);
			}
		} else p +=1;
		++p1;
	}
	get_l();
	registers[15] = check[str(l[1]+1)];
	fseek(g, 0, SEEK_SET);
	p = 0;
	p1 = 0;
	while(fgets(b, 100, g)){
		n = get_l();
		if(n != 1 && strcmp(l[0] + 1, "end") != 0){
			fr = 0;
			if(l[0][(int)l[0][0]] == ':') fr = 1;
			if(strcmp(l[fr]+1, "word") == 0){
				memo[p++] = get_int_unsigned(l[fr + 1] + 1);
			}else if(strcmp(l[fr]+1, "double") == 0){
				double d;
				scanf("%lf", &d);
				registers_1.b = d;
				memo[p++] = (int)(registers_1.a >> 32);
				memo[p++] = (int)(registers_1.a);
			}else {
				type = code[l[fr]+1];
				switch(type >> 8){
					case RM:
						RM_(&wr, fr);
						break;
					case RR:
						RR_(&wr, fr);
						break;
					case RI:
						RI_(&wr, fr);
						break;
					case J:
						J_(&wr, fr);
						break;
					default:
						fprintf(f, "error in line %d\n", p1);
						break;
				}
				memo[p++] = wr;
			}
		}
		p1++;
	}

	fclose(g);
	return 0;

}

union{
	int x;
	char con[4];
} matr;
char matrc[4];

int read_s(char* s){
	matr.con[0] = s[3], matr.con[1] = s[2], matr.con[2] = s[1], matr.con[3] = s[0];
	return matr.x;
}

int read_b(char* s){
	matr.con[0] = s[0], matr.con[1] = s[1], matr.con[2] = s[2], matr.con[3] = s[3];
	return matr.x;
}

inline void write_s(int x){
	matr.x = x;
	matrc[0] = matr.con[3];
	matrc[1] = matr.con[2];
	matrc[2] = matr.con[1];
	matrc[3] = matr.con[0];
	return;
}

int write(string s){
	char con[513];
	FILE * g = fopen(s.c_str(),"w");
	fwrite("ThisIsMIPT2Exec", 1, 16, g);
	fseek(g, 28, SEEK_SET);
	write_s(registers[15]);
	fwrite(matrc, 1, 4, g);
	fseek(g,512, SEEK_SET);
	int p = 0;
	while( p < 1024*1024){
		int i = 0;
		for(; i < 512 && p < 1024*1024;){
			write_s(memo[p++]);
			con[i++] = matrc[0];
			con[i++] = matrc[1];
			con[i++] = matrc[2];
			con[i++] = matrc[3];
		}
		con[i] = '\0';
		fwrite(con, 1, i, g);
	}
	fclose(g);
	return 0;
}

int load(string s){
	char con[513];
	FILE * g = fopen(s.c_str(),"r");
	fseek(g, 28, SEEK_SET);
	fread(con, 1, 4, g);
	registers[15] = read_b(con);
	registers[14] = 1024*1024 - 1;
	fseek(g , 0 , SEEK_END);
	long size = ftell(g);
	int conc = size/512 - 1;
	fseek(g, 512, SEEK_SET);
	int count = 0;
	int p = 0;
	while(conc!= 0 && (count = fread(con, 1, 512, g)) != 0){
		for(int j = 0; j < count; j += 4){
			 int  xx = read_b(con + j);
			 memo[p++] = xx;
		}
		--conc;
	}
	size = size % 512;
	count = fread(con, 1, 512, g);
	for(int j = 0; j < count; j += 4){
			 int  xx = read_b(con + j);
			 memo[p++] = xx;
		}
	fclose(g);
	return 0;
}

void init_registers_1(unsigned int t1){
	registers_1.a = ((long long)registers[t1] << 32) + (long long)registers[t1 + 1];
}

void init_registers_2(unsigned int t1){
	registers_2.a = ((long long)registers[t1] << 32) + (long long)registers[t1 + 1];
}

int push(int a){
	memo[--registers[14]] = a;
	return a;
}

int pop(int a = 0){
	int k = memo[registers[14]];
	registers[14] += a + 1;
	return k;

}

void syscall(unsigned int r, unsigned int n){
	switch(n){
		case 0:{
			exit(0);
		}
		case 100:{
			scanf("%d", registers + r);
			break;
		}
		case 101:{
			scanf("%lf", &registers_1.b);
			registers[r] = (int)(registers_1.a >> 32);
			registers[r + 1] = (int)(registers_1.a);
			break;
		}
		case 102:{
			printf("%d", registers[r]);
			break;
		}
		case 103:{
			init_registers_1(r);
			printf("%lg", registers_1.b);
			break;
		}
		case 104:{
			registers[r] = (int)getchar();
			break;
		}
		case 105:{
			putchar((char)registers[r]);
			break;
		}
		default:
			exit(101);

	}
}

int emul(string s){
	int code, type, z;
	bool inc;
	unsigned int t1,t2,t3;
	while(true){
		inc = true;
		z = memo[registers[15]];
		code = (z) >> 24;
		type = get_type(code);
		t1 = t2 = t3 = 0;
		switch(type){
			case RM:{
				t1 = (z & (((unsigned int)1 << 24) - ((unsigned int)1 << 20))) >> 20;
				t2 = (z & (((unsigned int)1 << 20) - ((unsigned int)1 << 0)));
				break;
			}
			case RR:{
				t1 = (z & (((unsigned int)1 << 24) - ((unsigned int)1 << 20))) >> 20;
				t2 = (z & (((unsigned int)1 << 20) - ((unsigned int)1 << 16))) >> 16;
				t3 = (z & (((unsigned int)1 << 16) - ((unsigned int)1 << 0)));
				break;
			}
			case RI:{
				t1 = (z & (((unsigned int)1 << 24) - ((unsigned int)1 << 20))) >> 20;
				t2 = (z & (((unsigned int)1 << 20) - ((unsigned int)1 << 0)));
				break;
			}
			case J:{
				t1 = (z & (((unsigned int)1 << 20) - ((unsigned int)1 << 0)));
				break;
			}
			default:
			exit(3);
			break;
		}

		switch (code){
			case 0:{
				exit(10);
				break;
			}
			case 1:{
				syscall(t1, t2);
				break;
			}
			case 2:{
				registers[t1] = registers[t1] + registers[t2] + t3;
				break;
			}
			case 3:{
				registers[t1] = registers[t1] + t2;
				break;
			}
			case 4:{
				registers[t1] = registers[t1] - (registers[t2] + t3);
				break;
			}
			case 5:{
				registers[t1] = registers[t1] - t2;
				break;
			}
			case 6:{
				long long k = (long long)registers[t1]*(long long)(registers[t2] + t3);
				registers[t1 + 1] = (int)(k >> 32);
				registers[t1] = int(k);
				break;
			}
			case 7:{
				long long k = (long long)registers[t1]*(long long)t2;
				registers[t1 + 1] = (int)(k >> 32);
				registers[t1] = int(k);
				break;
			}
			case 8:{
				long long k = (long long)registers[t1] + (((long long)registers[t1 + 1]) << 32);
				registers[t1] = k / (long long)(registers[t2] + t3);
				registers[t1+1] = k % (long long)(registers[t2] + t3);
				break;
			}
			case 9:{
				long long k = (long long)registers[t1] + (((long long)registers[t1 + 1]) << 32);
				registers[t1] = k / (long long)t2;
				registers[t1+1] = k % (long long)(t2);
				break;
			}
			case 12:{
				registers[t1] = t2;
				break;
			}
			case 13:{
				registers[t1] = registers[t1] << registers[t2];
				break;
			}
			case 14:{
				registers[t1] = registers[t1] << t2;
				break;
			}
			case 15:{
				registers[t1] = registers[t1] >> registers[t2];
				break;
			}
			case 16:{
				registers[t1] = registers[t1] >> t2;
				break;
			}
			case 17:{
				registers[t1] = registers[t1] & registers[t2];
				break;
			}
			case 18:{
				registers[t1] = registers[t1] & t2;
				break;
			}
			case 19:{
				registers[t1] = registers[t1] | registers[t2];
				break;
			}
			case 20:{
				registers[t1] = registers[t1] | t2;
				break;
			}
			case 21:{
				registers[t1] = registers[t1] ^ registers[t2];
				break;
			}
			case 22:{
				registers[t1] = registers[t1] ^ t2;
				break;
			}
			case 23:{
				registers[t1] = !registers[t1];
				break;
			}
			case 24:{
				registers[t1] = registers[t2] + t3;
				break;
			}
			case 32:{
				init_registers_1(t1);
				init_registers_2(t2);
				registers_1.b += registers_2.b;
				registers[t1] = (int)(registers_1.a >> 32);
				registers[t1 + 1] = (int)(registers_1.a);
				break;
			}
			case 33:{
				init_registers_1(t1);
				init_registers_2(t2);
				registers_1.b -= registers_2.b;
				registers[t1] = (int)(registers_1.a >> 32);
				registers[t1 + 1] = (int)(registers_1.a);
				break;
			}
			case 34:{
				init_registers_1(t1);
				init_registers_2(t2);
				registers_1.b *= registers_2.b;
				registers[t1] = (int)(registers_1.a >> 32);
				registers[t1 + 1] = (int)(registers_1.a);
				break;
			}
			case 35:{
				init_registers_1(t1);
				init_registers_2(t2);
				registers_1.b /= registers_2.b;
				registers[t1] = (int)(registers_1.a >> 32);
				registers[t1 + 1] = (int)(registers_1.a);
				break;
			}
			case 36:{
				registers_1.b = registers[t2];
				registers[t1] = (int)(registers_1.a >> 32);
				registers[t1 + 1] = (int)(registers_1.a);
				break;
			}
			case 37:{
				init_registers_2(t2);
				if((((registers_1.a >> 52) & ((1 << 8) - 1)) > 31)) exit(4);
				registers[t1] = (int)registers_1.b;
				break;
			}
			case 38:{
				push(registers[t1] + t2);
				break;
			}
			case 39:{
				registers[t1] = pop() + t2;
				break;
			}
			case 40:{
				push(registers[15] + 1);
				inc = false;
				registers[15] = registers[t1] = registers[t2];
				break;
			}
			case 41:{
				push(registers[15] + 1);
				inc = false;
				registers[15] = t1;
				break;
			}
			case 42:{
				registers[15] = pop(t1);
				inc = false;
				break;
			}
			case 43:{
				flag = registers[t1] - registers[t2];
				break;
			}
			case 44:{
				flag = registers[t1] - t2;
				break;
			}
			case 45:{
				init_registers_1(t1);
				init_registers_2(t2);
				if(registers_1.b > registers_2.b) flag = 1;
				else if (registers_1.b == registers_2.b) flag = 0;
				else flag = -1;
				break;
			}
			case 46:{
				registers[15] = t1;
				inc = false;
				break;
			}
			case 47:{

				if(flag != 0) {
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 48:{

				if(flag == 0){
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 49:{
				if(flag <= 0){
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 50:{
				if(flag < 0) {
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 51:{
				if(flag >= 0){
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 52:{
				if(flag > 0){
					registers[15] = t1;
					inc = false;
				}
				break;
			}
			case 64:{
				registers[t1] = memo[t2];
				break;
			}
			case 65:{
				memo[t2] = registers[t1];
				break;
			}
			case 66:{
				registers[t1] = memo[t2];
				registers[t1 + 1] = memo[t2 + 1];
				break;
			}
			case 67:{
				memo[t2] = registers[t1];
				memo[t2 + 1] = registers[t1 + 1];
				break;
			}
			case 68:{
				registers[t1] = memo[registers[t2] + t3];
				break;
			}
			case 69:{
				memo[registers[t2] + t3] = registers[t1];
				break;
			}
			case 70:{
				registers[t1 + 1] = memo[registers[t2] + t3 + 1];
				registers[t1] = memo[registers[t2] + t3];
				break;
			}
			case 71:{
				memo[registers[t2] + t3] = registers[t1];
				memo[registers[t2] + t3 + 1] = registers[t1 + 1];
				break;
			}

		}
		if(inc) registers[15]++;
	}
}

int main(){
	init_code();
    load("input.bin");
	emul("ss");
	free(memo);
	free(registers);
}
