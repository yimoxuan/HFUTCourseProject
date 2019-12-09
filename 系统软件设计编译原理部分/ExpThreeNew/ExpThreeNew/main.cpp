#include<iostream>  
#include<string>  
#include<map>  
#include<vector>  
#include<stack>  
#include<set>  
#include<cstring>  
#include<queue>  
#include<iomanip>
#include<fstream>

/*

E -> T E1
E1 -> w0 T E1 | ϵ
T -> F T1
T1 -> w1 F T1 | ϵ
F -> I | (E)

S -> A=E
A -> b
E→E+T∣E-T∣T|i
T→T*F∣T/F∣F
F→(E)∣i

S->f(W)H
W->P;J;K
W1->iOi
W2->iM
O->>|<|>=|<=
M->：|"			冒号为++，上引号为--
H->{A}

A->PB;
B->;PB|$
P->i=E
E->TC
C->+TC |-TC | $
T->FG
G->*FG |/FG| $
F->(E) | i |j

i=i+TB;
*/

using namespace std;
map<char, int>getnum;										//获得相应字符编号
char Getchar[1000];											//获得对应字符  
vector<string>proce;										//产生式存储  
int table[500][300];										//预测分析表 -1  
int table_op[500][300];										//是移进项还是规约项,-1,-2.  
int num = 0; int numvt = 0;									//num是终结符集合，0是‘#’，numvt表示空字所在的位置

string WORD[] = { "if","while","cout","cin","char","bool","int","float","double","for","return" ,"main" };//关键字
string OPRATOR[] = { "+","-","*","/","&","++","--" };		//关键字

string LaSign[] = { "<",">","=","<=", ">=","!=","<<",">>","!=" };
char SIGN[] = { '[',']','(',')','{','}', ';','"','\'','.' };
string Line[1024];											//原始过程存储待分析代码
string word;												//存放待分析的字符串
int Line_Num = 0;											//行号						
int RetNum = 0;												//返回值
vector<string> WordStack;									//存储原始字符串
vector<string> dealStack;									//存储中间字符串（有变换）
vector<string> siyuanshi;									//存储四元式
stack<string> proStack;										//存储中间过程（获得四元式）字符串

struct Detail {
	string	name;
	string	first;				//综合属性
	string	second;				//继承属性
	bool	initial;			//是否初始化
};

void firstGet(Detail &first, Detail second) {
	if (!second.initial)
	{
		cout << second.name << "使用了未初始化的属性\n";
		exit(1);
	}
	first.first = second.first;
	first.initial = true;
	return;
}

void secondGet(Detail &first, Detail second, Detail third, int type) {
	/*
	type =1 加法
	type =2 减法
	type = 3 无运算符
	*/
	if (!second.initial || !third.initial) {
		if (second.initial)
			cout << third.name << "未初始化\n";
		else
			cout << second.name << "未初始化\n";
		exit(1);
	}
	switch (type)
	{
	case 1:
		first.first = to_string(stod(second.first) + stod(third.first));
		break;
	case 2:
		first.first = to_string(stod(second.first) - stod(third.first));
		break;
	default:
		first.first = second.first;
		break;
	}
	first.initial = true;
	return;
}

void thirdGet(Detail &first, Detail second, Detail third, int type) {
	/*
	type =1 乘法
	type =2 除法
	type = 3 无运算符
	*/
	if (!second.initial || !third.initial) {
		if (second.initial)
			cout << third.name << "未初始化\n";
		else
			cout << second.name << "未初始化\n";
		exit(1);
	}

	switch (type)
	{
	case 1:
		first.first = to_string(stod(second.first) * stod(third.first));
		break;
	case 2:
		first.first = to_string(stod(second.first) / stod(third.first));
		break;
	default:
		first.first = second.first;
		break;
	}
	first.initial = true;
	return;
}


int isWORD(char chr[])//关键字判定
{
	for (int i = 0; i < sizeof(WORD) / sizeof(string); i++)
	{

		if (WORD[i] == (string)chr)
		{
			return i + 1;
		}
	}
	return 0;
}
/*是否是关键字*/
int isKeyWord(string str) {
	for (int i = 0; i < sizeof(WORD) / sizeof(string); i++)
	{

		if (WORD[i] == str)
		{
			return i + 1;
		}
	}
	return 0;
}


int isOPRATOR(string ch)//运算符判定
{
	for (int i = 0; i < sizeof(OPRATOR) / sizeof(string); i++)
	{
		if (OPRATOR[i] == ch)
		{
			return i + 1;
		}
	}
	return 0;
}

int isSIGN(string ch)//运算符判定
{
	for (int i = 0; i < sizeof(SIGN) / sizeof(SIGN[0]); i++)
	{
		string str = "";
		str += SIGN[i];
		if (str == ch)
		{
			return i + 1;
		}
	}
	return 0;
}
int isLaSign(string chr)//关键字判定
{
	for (int i = 0; i < sizeof(LaSign) / sizeof(LaSign[0]); i++)
	{

		if (LaSign[i] == chr)
		{
			return i + 1;
		}
	}
	return 0;
}

/*判断是否是变量名*/
bool isVaria(string str) {
	return !isLaSign(str) && !isOPRATOR(str) && !isSIGN(str) && !isKeyWord(str);
}
void StoreCh(string & str, char ch) {
	str += ch;
}

void StoreWord(string &str) {
	if (str == "")
		return;
	WordStack.push_back(str);
	str.clear();
	return;
}

/*判断是否可以组成变量名*/
bool isWord(char c) {
	return c >= 'A'&&c <= 'Z' || c >= 'a'&&c <= 'z' || c >= '0'&&c <= '9' || c == '_';
}

void wordAnaly() {
	ifstream input("F:\\compiletest.txt", ios::in);			/*文件读取*/
	if (!input)
	{
		cerr << "open error!" << endl;
		exit(1);
	}
	else {
		cout << "源程序如下：" << endl;
		while (getline(input, Line[Line_Num]))						/*读取一行*/
		{
			Line_Num++;												/*记录程序行数*/
		}
	}
	string temWord = "";
	for (int i = 0; i < Line_Num; i++) {
		for (int j = 0; j < Line[i].length(); j++) {
			StoreWord(temWord);
			if (Line[i][j] == ' ' || Line[i][j] == '\t') {
				while (Line[i][j] == ' ' || Line[i][j] == '\t') {					/*空格以及换行符制表符剔除*/
					j++;
				}
				j--;
				continue;
			}
			string temChar = "";
			temChar += Line[i][j];
			if (isWord(Line[i][j])) {
				while (isWord(Line[i][j])) {
					StoreCh(temWord, Line[i][j++]);
				}
				j--;
				continue;
			}
			if (isOPRATOR(temChar) || isOPRATOR(temChar + Line[i][j + 1])) {
				if (isOPRATOR(temChar + Line[i][j + 1])) {
					StoreCh(temWord, Line[i][j++]);
					StoreCh(temWord, Line[i][j++]);
				}
				else {
					StoreCh(temWord, Line[i][j++]);
				}
				j--;
				continue;
			}
			if (isSIGN(temChar)) {
				StoreCh(temWord, Line[i][j++]);
				j--;
				continue;
			}

			if (isLaSign(temChar) || isLaSign(temChar + Line[i][j + 1])) {
				if (isLaSign(temChar + Line[i][j + 1])) {
					StoreCh(temWord, Line[i][j++]);
					StoreCh(temWord, Line[i][j++]);
				}
				else {
					StoreCh(temWord, Line[i][j++]);
				}
				j--;
				continue;
			}
			j++;
		}
		if (temWord != "")
			StoreWord(temWord);
	}
	StoreWord(temWord);
	return;
}

/*
S->f(W)H
W->P;J;K
J->iOi
K->iM
O->>|<
M->：|"			冒号为++，上引号为--
H->{A}

A->PB
B->;PB|$
P->i=E
E->TC
C->+TC |-TC | $
T->FG
G->*FG |/FG| $
F->(E) | i |j

A->PBQ
Q->@ | f(W)HA
*/

void readin()											//读入vt,vn,编号1-num,读入所有产生式  
{
	memset(table, -1, sizeof(table));					//	清空table
	getnum['#'] = 0;
	Getchar[0] = '#';
	string x1 = ";i=+-*/()jf><:\"{}";
	for (int i = 0; i < x1.length(); i++) {
		getnum[x1[i]] = ++num;
		Getchar[num] = x1[i];
	}
	numvt = ++num;
	getnum['@'] = numvt;								//kong zi  
	Getchar[num] = ('@');
	x1 = "ABCEFGHPJKMOPQSTW";
	for (int i = 0; i < x1.length(); i++) {
		getnum[x1[i]] = ++num;
		Getchar[num] = x1[i];
	}
	string pro;
	string pro1[] = { "APBQ","B;PB","B@","Pi=E","ETC","C+TC","C-TC","C@","TFG","G*FG","G/FG","G@","F(E)","Fi","Fj",
		"Q@","Qf(W)H","WP;J;K","JiOi","KiM","O>","O<","M:","M\"","H{A}", };
	for (int i = 0; i < sizeof(pro1) / sizeof(string); i++) {
		proce.push_back(pro1[i]);
	}
}
void readin1()											//读入vt,vn,编号1-num,读入所有产生式  
{
	memset(table, -1, sizeof(table));					//	清空table
	getnum['#'] = 0;
	Getchar[0] = '#';
	string x1 = "+*()i";
	for (int i = 0; i < x1.length(); i++) {
		getnum[x1[i]] = ++num;
		Getchar[num] = x1[i];
	}
	numvt = ++num;
	getnum['@'] = numvt;								//kong zi  
	Getchar[num] = ('@');
	x1 = "ETF";
	for (int i = 0; i < x1.length(); i++) {
		getnum[x1[i]] = ++num;
		Getchar[num] = x1[i];
	}
	string pro;
	string pro1[6] = { "EE+T","ET","TT*F","TF","F(E)","Fi" };
	for (int i = 0; i < 6; i++) {
		proce.push_back(pro1[i]);
	}
}
struct xiangmu
{
	int nump;											//产生式编号  
	int id;												//.的位置  
	string fst;											//展望符
};

void printXMJ(vector<xiangmu> tem) {
	for (int i = 0; i < tem.size(); i++)
		cout << tem[i].fst << " " << tem[i].id << " " << tem[i].nump << "  xiangmuji\n";
}


string getp[1000];										//获得某终结符在左边的产生式集合  每一个非终结符对应的产生式序号
void getSameLeft()
{
	for (int i = 0; i < proce.size(); i++)
	{
		int temp = getnum[proce[i][0]];					//获得产生式左边非终结符的数字代码以及其对应产生式编码
		getp[temp] += char('0' + i);					//同一个非终结符可能对应多个产生式
	}
}
string first[1000];										//每个符号的first集  
bool gotfirst[1000];										//是否已经完成FIRST集合  
void dfsGetFirst(int nv, int nump)						//当前的符号，和对应产生式编号  
{
	int temp = getnum[proce[nump][1]];					//产生式推出来的首符  
	gotfirst[nump] = 1;									//标记  该产生式已经求过first集
	if (temp <= numvt)first[nv] += char('0' + temp);	//是终结符  
	else
	{
		for (int i = 0; i < getp[temp].size(); i++)		//所有temp可以推出来的符号对应的产生式  
		{
			if (proce[nump][0] == proce[nump][1])continue; //左递归的产生式不用不影响求fisrt集  
			dfsGetFirst(temp, getp[temp][i] - '0');
		}
		first[nv] += first[temp];						//回溯时候沿途保存  
	}
}
void getFirst()
{
	for (int i = 1; i <= numvt; i++)					//    终结符first集合是它自己.  
	{
		first[i] = char('0' + i);						//每一个first集里面存储的是他的getnum序号
	}
	for (int i = 0; i < proce.size(); i++)
	{
		if (proce[i][0] == proce[i][1])continue;		//左递归的产生式不用不影响求fisrt集  
		if (gotfirst[i])continue;						//已经生成。  
		int temp = getnum[proce[i][0]];
		dfsGetFirst(temp, i);
	}
}
vector<vector<xiangmu> >v;								//项目集族  
int e[2000][3]; int head[2000]; int nume = 0;				//链式前向星项目集族图  
void addegde(int from, int to, int w)					//添加边  
{
	//往哪去，从哪里来，怎么去
	e[nume][0] = to;
	e[nume][1] = head[from];
	head[from] = nume;
	e[nume][2] = w;									//先运算后++
	nume++;
}

void clear()											//初始化函数  
{
	for (int i = 0; i < 2000; i++)
		head[i] = -1;
	for (int i = 0; i < 500; i++)
		for (int j = 0; j < 300; j++)
			table_op[i][j] = table[i][j] = -10;
	nume = 0;
}
inline bool xmEqual(xiangmu a, xiangmu b)
{
	if (a.fst == b.fst&&a.id == b.id&&a.nump == b.nump)return 1;
	return 0;
}
bool isin(xiangmu a, vector<xiangmu> b)					//xm a is in xmji b  
{
	for (int i = 0; i < b.size(); i++)
	{
		if (xmEqual(a, b[i]))return 1;
	}
	return 0;
}
vector<xiangmu>  xmCombine(vector<xiangmu>a, vector<xiangmu>b)  //合并项目集 a,b 复给 a  
{
	for (int i = 0; i < b.size(); i++)
	{
		if (isin(b[i], a))continue;
		else
			a.push_back(b[i]);
	}
	return a;
}
bool xmjEqual(vector<xiangmu> a, vector<xiangmu> b)		//两个项目集是否相等  
{
	if (a.size() != b.size())return 0;
	for (int i = 0; i < a.size(); i++)
	{
		if (!isin(a[i], b))return 0;
	}
	return 1;
}
int xmjHasBeen(vector<xiangmu>a, vector<vector<xiangmu> >b)  //查找项目集，若有，则返回编号,一举俩得  
{
	for (int i = 0; i < b.size(); i++)
	{
		if (xmjEqual(a, b[i]))return i;
	}
	return -1;
}

void appendfst(string &str1, string str2) {
	for (int i = 0; i < str2.length(); i++) {
		if (str1.find(str2[i]) == string::npos)
			str1 += str2[i];
	}
}

//"GABC"
string getForward(string str, int point, string curStr) {
	string resultStr = "";
	if (point == str.length() - 1)
		return resultStr += curStr;
	resultStr += first[getnum[str[++point]]];
	while (first[getnum[str[point]]].find(char('0' + numvt)) != string::npos) {
		if (point == str.length() - 1) {
			appendfst(resultStr, curStr);
			break;
		}
		appendfst(resultStr, first[getnum[str[++point]]]);

	}
	return resultStr;
}

vector<xiangmu> getClosure(xiangmu t)					//对项目 T作闭包  
{
	vector<xiangmu> temp;
	temp.push_back(t);
	queue<xiangmu> q;									//bfs完成闭包  
	q.push(t);
	while (!q.empty())
	{
		xiangmu cur = q.front();
		q.pop();
		if (cur.id == proce[cur.nump].size())			//归约项舍去 则不进行操作  
			continue;
		int tt = getnum[proce[cur.nump][cur.id]];       //tt is thm num of '.'zhihoudefuhao  
		if (tt <= numvt)   continue;					//若是终结符，则不必找了  
														//接下来的字符是非终结符，则产生新的项目
		for (int i = 0; i < getp[tt].size(); i++)         //对应产生式的编号  getp是对应非终结符的产生式序号
		{
			xiangmu c;
			c.id = 1;									//  
			c.nump = getp[tt][i] - '0';					//  C的产生式序号

			if (proce[cur.nump].size() - cur.id == 1)   // the last : A->BC.D,a/b  
				c.fst += cur.fst;
			else										//not the last  ：A->B.CFb,a/b  
			{
				c.fst = getForward(proce[cur.nump], cur.id, cur.fst);
				//int tttnum = getnum[proce[cur.nump][cur.id + 1]];
				//c.fst += first[getnum[proce[cur.nump][cur.id + 1]]];
			}

			/*
			if (proce[cur.nump].size() - cur.id == 1)   // the last : A->BC.D,a/b
				c.fst += cur.fst;
			else										//not the last  ：A->B.CFb,a/b
			{
				int tttnum = getnum[proce[cur.nump][cur.id + 1]];
				c.fst += first[tttnum];
				int temId = cur.id + 1;
				while (first[tttnum].find(char('0'+numvt)) != string::npos) {
					if (proce[cur.nump].size() - temId == 1) {
						appendfst(c.fst, cur.fst);
						break;
					}
					tttnum = getnum[proce[cur.nump][++temId]];
					appendfst(c.fst, first[tttnum]);
				}
			}*/

			if (!isin(c, temp))							//排重，新的项目就加入。  
			{
				q.push(c);
				temp.push_back(c);
			}
		}
	}
	return temp;
}

void buildXMJF()									//获得项目集族  
{
	vector<xiangmu>temp;								//项目集
	xiangmu t;
	t.nump = 0; t.id = 1; t.fst += '0';					//初始的项目集：0  
	temp = getClosure(t);
	queue<vector<xiangmu> >q;							//bfs法获得  
	q.push(temp);
	v.push_back(temp);									//第一个入  
	while (!q.empty())
	{
		vector<xiangmu> cur = q.front();				//队列顶项目集
		q.pop();
		//对于每一个字符来讲，查找该项目集里可能下一个将会接收他的项目
		//项目集之间不会改变展望符
		for (int i = 1; i <= num; i++)					//num大小为所有的符号包括空字
		{
			if (i == numvt)continue;					//'#'  空字
			vector<xiangmu> temp;
			for (int j = 0; j < cur.size(); j++)			//该项目集中的所有项目  
			{
				if (cur[j].id == proce[cur[j].nump].size())continue;  //是规约项目，无法再读入了  
				int tt = getnum[proce[cur[j].nump][cur[j].id]];
				if (tt == i)                                          //can read in 符号i  
				{
					xiangmu tempt;
					tempt.fst = cur[j].fst;
					tempt.id = cur[j].id + 1;
					tempt.nump = cur[j].nump;
					temp = xmCombine(temp, getClosure(tempt));
				}
			}
			if (temp.size() == 0)continue;			    //该符号无法读入。  
			int numcur = xmjHasBeen(cur, v);		//当前节点标号  
			int tttnum = xmjHasBeen(temp, v);		//新目标标号  
			if (tttnum == -1)							//新的项目集  
			{
				v.push_back(temp);
				q.push(temp);
				addegde(numcur, v.size() - 1, i);		//添加边，权为读入的符号  
			}
			else										//老的项目集  
			{
				addegde(numcur, tttnum, i);
			}
		}
	}
}
void printXMJF()										//打印项目集族  
{
	for (int i = 0; i < v.size(); i++)
	{
		cout << "项目集" << i << ":" << endl;
		for (int j = 0; j < v[i].size(); j++)
		{
			cout << proce[v[i][j].nump] << " " << v[i][j].id << " " << v[i][j].fst << endl;
		}
		cout << endl;
	}
	for (int i = 0; i < v.size(); i++)
	{
		for (int j = head[i]; j != -1; j = e[j][1])
		{
			cout << "  " << Getchar[e[j][2]] << endl;
			cout << i << "--->" << e[j][0] << endl;
		}
	}
}

bool getTable()										//获得分析表table[i][j]=w:状态i-->j,读入符号W。  
{

	for (int i = 0; i < v.size(); i++)					//遍历图  
	{
		for (int j = head[i]; j != -1; j = e[j][1])		//等于-1则意味着没有连接
		{
			if (table[i][e[j][2]] != -1)return 0;		//多重入口，报错.  
			table[i][e[j][2]] = e[j][0];
			table_op[i][e[j][2]] = -1;					//移近项-1。  
		}
	}

	for (int i = 0; i < v.size(); i++)					//遍历所有项目  
	{
		for (int j = 0; j < v[i].size(); j++)
		{
			if (v[i][j].id == proce[v[i][j].nump].size() || proce[v[i][j].nump][1] == Getchar[numvt])  //归约项  
			{
				for (int k = 0; k < v[i][j].fst.size(); k++)						//对于每一个展望符
				{
					///////////////////////////////////////////////////////////////////////////////////////////////
					//if (table[i][(v[i][j].fst)[k] - '0'] != -1)return 0;		//多重入口，报错.  
					if ((v[i][j].fst)[k] == '0'&&v[i][j].nump == 0) {
						table[i][(v[i][j].fst)[k] - '0'] = -3;
						//table[i][(v[i][j].fst)[k] - '0'] = v[i][j].nump;
						table_op[i][(v[i][j].fst)[k] - '0'] = -2;						//接受态。

					}
					else
					{
						table[i][(v[i][j].fst)[k] - '0'] = v[i][j].nump;
						table_op[i][(v[i][j].fst)[k] - '0'] = -2;					//归约态  
					}
				}
			}
		}
	}

	return 1;
}
void printAnaTable()
{
	cout << "\t\t\t=============LR(1)分析表============\t\t\t\n" << endl;
	cout << "状态   " << "         actoin     " << endl;
	for (int j = 0; j <= num; j++)
	{
		if (j == numvt)continue;
		cout << "    " << Getchar[j];
	}
	cout << endl;
	for (int i = 0; i < v.size(); i++)
	{
		cout << i << "   ";
		for (int j = 0; j <= num; j++)
		{
			if (j == numvt)continue;
			if (table[i][j] == -3)  cout << "acc" << "  ";					  //接受  
			else if (table[i][j] == -1)cout << "     ";						  //空  
			else if (table_op[i][j] == -1)cout << "s" << table[i][j] << "   ";  //移近  
			else if (table_op[i][j] == -2)cout << "r" << table[i][j] << "   ";  //归约  
		}
		cout << endl;
	}
}

void  printStackTopState(int count, stack<int>state, stack<int>wd, int i)
{
	cout << count << '\t' << '\t';
	stack<int>temp;
	while (!state.empty())
	{
		temp.push(state.top());
		state.pop();
	}
	while (!temp.empty())
	{
		cout << temp.top();
		temp.pop();
	}
	cout << '\t' << '\t';
	while (!wd.empty())
	{
		temp.push(wd.top());
		wd.pop();
	}
	while (!temp.empty())
	{
		cout << Getchar[temp.top()];
		temp.pop();
	}
	cout << '\t' << '\t';
	for (int j = i; j < dealStack.size(); j++)
		cout << dealStack[j];
	cout << '\t' << '\t';
}

bool analyze()
{
	cout << "       " << word << "的分析过程：" << endl;
	cout << "步骤\t\t" << "状态栈\t\t" << "符号栈\t\t" << "输入串\t\t" << "动作说明" << endl;
	stack<int>state;		//俩个栈：状态栈和符号栈  
	stack<int>wd;
	int count = 0;
	state.push(0);			//初始化  
	wd.push(0);				//'#'  
	for (int i = 0;;)       //i，读入文本的长度 
	{
		int cur = state.top();
		cout << "当前栈顶： " << cur << "\n";
		if (table[cur][getnum[word[i]]] == -10)			// 空白，报错误  
			return 0;
		if (table[cur][getnum[word[i]]] == -3)			//接受态  
		{
			printStackTopState(count++, state, wd, i);
			cout << "      恭喜！acc!" << endl;
			return 1;
		}
		if (table_op[cur][getnum[word[i]]] == -1)			//移进项  
		{
			printStackTopState(count++, state, wd, i);
			int newstate = table[cur][getnum[word[i]]];
			cout << "action[" << cur << "," << getnum[word[i]] << "]=" << newstate;
			cout << "，状态" << newstate << "入栈" << endl;
			wd.push(getnum[word[i]]);
			state.push(newstate);
			i++;
		}
		else if (table_op[cur][getnum[word[i]]] == -2)         //归约  
		{
			printStackTopState(count++, state, wd, i);
			int numpro = table[cur][getnum[word[i]]];		//用该产生式归约  
			int len = proce[numpro].size() - 1;
			if (proce[numpro][1] == Getchar[numvt]) {

			}
			else {
				for (int ii = 0; ii < len; ii++)                 //弹栈  
				{
					wd.pop();
					state.pop();
				}
			}
			wd.push(getnum[proce[numpro][0]]);    //新入  
			int cur = state.top();
			cout << "用" << proce[numpro][0] << "->";
			for (int ii = 1; ii <= len; ii++)
				cout << proce[numpro][ii];
			cout << "进行归约," << "goto[" << cur << "," << getnum[word[i]] << "]=" << table[cur][getnum[proce[numpro][0]]];
			cout << "入栈" << endl;
			state.push(table[cur][getnum[proce[numpro][0]]]);
		}
	}
	return 1;
}

map<string, string> biaozhifu;		//记录标识符保存的序号
vector<Detail> detailVec;			//记录标识符的信息
void detailStr() {
	int num = 0;
	for (auto i = 0; i < WordStack.size(); i++) {
		if (isVaria(WordStack[i]) && WordStack[i] != "#") {
			dealStack.push_back("i");
		}
		else {
			if (WordStack[i] == "for") {
				dealStack.push_back("f");
				continue;
			}
			if (WordStack[i] == "++") {
				dealStack.push_back(":");
				continue;
			}
			if (WordStack[i] == "--") {
				dealStack.push_back("\"");
				continue;
			}
			dealStack.push_back(WordStack[i]);
		}
	}
}

void refreshWordStack() {
	vector<string> temRefresh;
	temRefresh.clear();
	bool temFag = true;
	for (auto i = 0; i < WordStack.size(); i++) {
		if (WordStack[i] == "{") {
			temFag = false;
			continue;
		}
		if (temFag)
			continue;
		if (WordStack[i] == "}")
			continue;
		temRefresh.push_back(WordStack[i]);
	}
	WordStack.clear();
	for (auto i = 0; i < temRefresh.size(); i++) {
		WordStack.push_back(temRefresh[i]);
	}
}

map<string, Detail> nameDict;

void makeDict() {
	for (auto i = 0; i < WordStack.size(); i++) {
		if (isVaria(WordStack[i]) && WordStack[i] != "#") {
			if (nameDict.count(WordStack[i]) == 0) {
				nameDict[WordStack[i]] = Detail{ WordStack[i], "0", "0",false };
			}
		}
	}
}

void appendDict(string name, string num) {
	if (nameDict.count(name) != 0) {
		cout << "使用了禁止使用的标志符号\n";
		exit(0);
	}
	nameDict[name] = Detail{ name,num,"0" ,true };
}

string judgeName1 = "";		//判断循环里面的名称；
string judgeName2 = "";		//右条件名称
bool judgeBool = false;		//for循环里面是++（true）还是--
bool judgeSym = false;		//判断条件是>(true)还是<(false)
bool enablePro = true;		//true为满足运行条件
int temCount = 0;

bool analyze1()
{
	bool a = false;
	detailStr();
	cout << endl;
	cout << "\n\n\t\t\t============源程序的分析过程=========\t\t\t\n" << endl;
	cout << "序号\t\t" << "状态栈\t\t" << "符号栈\t\t" << "输入串\t\t" << "动作说明" << endl;
	stack<int>state;		//俩个栈：状态栈和符号栈  
	stack<int>wd;
	int count = 0;
	state.push(0);			//初始化  
	wd.push(0);				//'#'  
	for (int i = 0;;)       //i，读入文本的长度 
	{
		int cur = state.top();
		cout << "当前栈顶： " << cur << "\n";
		if (table[cur][getnum[dealStack[i][0]]] == -10)			// 空白，报错误  
			return 0;
		if (table[cur][getnum[dealStack[i][0]]] == -3)			//接受态  
		{
			printStackTopState(count++, state, wd, i);
			cout << "      恭喜！acc!" << endl;
			return 1;
		}
		if (table_op[cur][getnum[dealStack[i][0]]] == -1)			//移进项  
		{
			printStackTopState(count++, state, wd, i);
			int newstate = table[cur][getnum[dealStack[i][0]]];
			cout << "action[" << cur << "," << getnum[dealStack[i][0]] << "]=" << newstate;
			cout << "，状态" << newstate << "入栈" << endl;
			wd.push(getnum[dealStack[i][0]]);
			state.push(newstate);
			proStack.push(WordStack[i]);
			i++;
		}
		else if (table_op[cur][getnum[dealStack[i][0]]] == -2)         //归约  
		{
			string temResu = "(\t";
			printStackTopState(count++, state, wd, i);
			int numpro = table[cur][getnum[dealStack[i][0]]];		//用该产生式归约  
			int len = proce[numpro].size() - 1;
			/*
			0		"APBQ",
			1		"B;PB",
			2		"B@",
			3		"Pi=E",
			4		"ETC",
			5		"C+TC",
			6		"C-TC",
			7		"C@",
			8		"TFG",
			9		"G*FG",
			10		"G/FG",
			11		"G@",
			12		"F(E)",
			13		"Fi",
			14		"Fj",
			15		"Q@",
			16		"Qf(W)H",
			17		"WP;J;K",
			18		"JiOi",
			29		"KiM",		//在23/24步直接规约并运算
			20		"O>",
			21		"O<",
			22		"M:",
			23		"M\"",
			24		"H{A}",*/
			string tem1 = "", tem2 = "", tem3 = "", tem4 = "";
			bool oppo1 = false;					//	true为加法，false为减法
			bool oppo2 = false;					//	true 为乘法，false为除法
			double leftNum = 0;
			double rightNum = 0;
			switch (numpro)
			{
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				tem3 = proStack.top();
				proStack.pop();
				if (isdigit(tem1[0])) {
					firstGet(nameDict[tem3], Detail{ "",tem1,"0",true });
				}
				else {
					firstGet(nameDict[tem3], nameDict[tem1]);
				}
				temResu += tem2 + "\t" + tem3 + "\t" + tem1 + "\t" + tem3 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 4:
				break;
			case 9:
				oppo2 = true;
				//break;
			case 10:
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				tem3 = proStack.top();
				proStack.pop();
				tem4 = "$" + to_string(temCount++);
				appendDict(tem4, "0");
				proStack.push(tem4);
				if (oppo2) {
					if (isdigit(tem1[0]) && !isdigit(tem3[0])) {
						thirdGet(nameDict[tem4], nameDict[tem3], Detail{ "",tem1,"0",true }, 1);
					}
					if (isdigit(tem3[0]) && !isdigit(tem1[0]))
						thirdGet(nameDict[tem4], Detail{ "",tem3,"0",true }, nameDict[tem1], 1);
					if (isdigit(tem3[0]) && isdigit(tem1[0]))
						thirdGet(nameDict[tem4], Detail{ "",tem3,"0",true }, Detail{ "",tem1,"0",true }, 1);
					if (!isdigit(tem1[0]) && !isdigit(tem3[0])) {
						thirdGet(nameDict[tem4], nameDict[tem3], nameDict[tem1], 1);
					}
				}
				else {
					if (isdigit(tem1[0]) && !isdigit(tem3[0])) {
						thirdGet(nameDict[tem4], nameDict[tem3], Detail{ "",tem1,"0",true }, 2);
					}
					if (isdigit(tem3[0]) && !isdigit(tem1[0]))
						thirdGet(nameDict[tem4], Detail{ "",tem3,"0",true }, nameDict[tem1], 2);
					if (isdigit(tem3[0]) && isdigit(tem1[0]))
						thirdGet(nameDict[tem4], Detail{ "",tem3,"0",true }, Detail{ "",tem1,"0",true }, 2);
					if (!isdigit(tem1[0]) && !isdigit(tem3[0])) {
						thirdGet(nameDict[tem4], nameDict[tem3], nameDict[tem1], 2);
					}
				}
				temResu += tem2 + "\t" + tem3 + "\t" + tem1 + "\t" + tem4 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 5:
				oppo1 = true;
				//break;
			case 6:
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				tem3 = proStack.top();
				proStack.pop();
				tem4 = "$" + to_string(temCount++);
				appendDict(tem4, "0");
				proStack.push(tem4);
				if (oppo1) {
					if (isdigit(tem1[0]) && !isdigit(tem3[0])) {
						secondGet(nameDict[tem4], nameDict[tem3], Detail{ "",tem1,"0",true }, 1);
					}
					if (isdigit(tem3[0]) && !isdigit(tem1[0]))
						secondGet(nameDict[tem4], Detail{ "",tem3,"0",true }, nameDict[tem1], 1);
					if (isdigit(tem3[0]) && isdigit(tem1[0]))
						secondGet(nameDict[tem4], Detail{ "",tem3,"0",true }, Detail{ "",tem1,"0",true }, 1);
					if (!isdigit(tem1[0]) && !isdigit(tem3[0])) {
						secondGet(nameDict[tem4], nameDict[tem3], nameDict[tem1], 1);
					}
				}
				else {
					if (isdigit(tem1[0]) && !isdigit(tem3[0])) {
						secondGet(nameDict[tem4], nameDict[tem3], Detail{ "",tem1,"0",true }, 2);
					}
					if (isdigit(tem3[0]) && !isdigit(tem1[0]))
						secondGet(nameDict[tem4], Detail{ "",tem3,"0",true }, nameDict[tem1], 2);
					if (isdigit(tem3[0]) && isdigit(tem1[0]))
						secondGet(nameDict[tem4], Detail{ "",tem3,"0",true }, Detail{ "",tem1,"0",true }, 2);
					if (!isdigit(tem1[0]) && !isdigit(tem3[0])) {
						secondGet(nameDict[tem4], nameDict[tem3], nameDict[tem1], 2);
					}
				}
				temResu += tem2 + "\t" + tem3 + "\t" + tem1 + "\t" + tem4 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 7:
				break;
			case 8:
				break;
			case 11:
				break;
			case 12:
				proStack.pop();
				tem1 = proStack.top();
				proStack.pop();
				proStack.pop();
				proStack.push(tem1);
				temResu += "()\t" + tem1 + "\t\t" + tem1 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 13:
				break;
			case 14:
				break;
			case 15:
				break;
			case 16:
				/*整个完整for循环规约*/
				tem1 = proStack.top();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.push(tem1);
				break;
			case 17:
				/*for语句循环条件规约*/
				tem1 = proStack.top();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.pop();
				proStack.push(tem1);
				break;
			case 18:
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				tem3 = proStack.top();
				proStack.pop();
				leftNum = stod(nameDict[tem3].first);
				if (isdigit(tem1[0]))
					rightNum = stod(tem1);
				else {
					rightNum = stod(nameDict[tem1].first);
				}
				judgeName1 = tem3;
				judgeName2 = to_string(rightNum);
				if (judgeBool) {
					enablePro = leftNum > rightNum ? true : false;
					temResu += ">\t" + to_string(leftNum) + "\t" + to_string(rightNum) + "\t";
					if (!enablePro)
						return false;
				}
				else {
					enablePro = leftNum < rightNum ? true : false;
					temResu += "<\t" + to_string(leftNum) + "\t" + to_string(rightNum) + "\t";
					if (!enablePro)
						return false;
				}
				if (enablePro) {
					proStack.push("true");
				}
				else {
					proStack.push("false");
				}
				temResu += proStack.top() + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 19:
				//tem1 = proStack.top();
				//proStack.pop();
				//tem2 = proStack.top();
				//proStack.pop();
				break;
			case 20:
				judgeSym = true;
				break;
			case 21:
				judgeSym = false;
				break;
			case 22:
				judgeBool = true;
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				secondGet(nameDict[tem2], nameDict[tem2], Detail{ tem2,"1","0",true }, 1);
				a = true;
				proStack.push(tem2);
				temResu += "++\t" + tem2 + "\t\t" + tem2 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 23:
				judgeBool = false;
				tem1 = proStack.top();
				proStack.pop();
				tem2 = proStack.top();
				proStack.pop();
				secondGet(nameDict[tem2], nameDict[tem2], Detail{ "","1","0",true }, 2);
				proStack.push(tem1);
				temResu += "--\t" + tem2 + "\t\t" + tem2 + "\t)";
				siyuanshi.push_back(temResu);
				break;
			case 24:
				break;
			default:
				break;
			}

			if (proce[numpro][1] == Getchar[numvt]) {

			}
			else {
				for (int ii = 0; ii < len; ii++)                 //弹栈  
				{
					wd.pop();
					state.pop();
				}
				//temResu += "(\t" + WordStack[i - 3] + "\t" + WordStack[i - 2] + "\t" + WordStack[i-1];

			}
			wd.push(getnum[proce[numpro][0]]);    //新入  
			int cur = state.top();
			cout << "用" << proce[numpro][0] << "->";
			for (int ii = 1; ii <= len; ii++)
				cout << proce[numpro][ii];
			cout << "进行归约," << "goto[" << cur << "," << getnum[dealStack[i][0]] << "]=" << table[cur][getnum[proce[numpro][0]]];
			cout << "入栈" << endl;
			state.push(table[cur][getnum[proce[numpro][0]]]);
			cout << i << endl;
			if (a)
				if (stod(nameDict["a"].first) == 0) {
					cout << i << endl;
					exit(0);
				}
		}
	}
	return 1;
}

int main()
{
	clear();
	readin();
	getSameLeft();
	getFirst();
	buildXMJF();
	// printXMJF(); 
	getTable();
	printAnaTable();

	printf("\n\n\t\t\t\t\t==============first集合=============\t\t\t\t\t\n");
	for (int i = 0; i < num; i++) {
		cout << Getchar[i] << "\t";
		for (int j = 0; j < first[i].length(); j++)
			cout << Getchar[first[i][j] - '0'] << "\t";
		cout << endl;
	}
	printf("\n\n\t\t\t\t\t==============项目集族=============\t\t\t\t\t\n");
	for (int i = 0; i < v.size(); i++) {
		cout << "==================" << i << "===================" << endl;
		for (int j = 0; j < v[i].size(); j++) {
			for (int k = 0; k < proce[v[i][j].nump].length(); k++) {
				if (k == v[i][j].id)
					cout << ".";
				cout << proce[v[i][j].nump][k];
			}
			if (v[i][j].id == proce[v[i][j].nump].length())
				cout << ".";
			cout << "\t";
			for (int m = 0; m < v[i][j].fst.length(); m++)
				cout << Getchar[v[i][j].fst[m] - '0'] << "\t";
			cout << endl;
		}
		cout << endl;
	}
	/*
		cout << "请输入字：" << endl;
		cin >> word;
		word += '#';
	*/
	/*		开始分析源程序		*/		
	wordAnaly();
	WordStack.push_back("#");
	makeDict();
	bool firstTime = true;
	bool forI = true;
	for (auto i : WordStack)
		if (i == "for")
			forI = false;
	
	while (true) {
		if (!analyze1())
			break;
		if (forI)
			break;
		dealStack.clear();
		if (firstTime) {
			refreshWordStack();
			firstTime = false;
			if (judgeBool) {
				secondGet(nameDict[judgeName1], nameDict[judgeName1], Detail{ "","1","0",true }, 2);
			}
			else {
				secondGet(nameDict[judgeName1], nameDict[judgeName1], Detail{ "","1","0",true }, 1);
			}
		}
		if (0 == WordStack.size())
			break;
		/*				进行+1/-1操作			*/			
		if (judgeBool) {
			secondGet(nameDict[judgeName1], nameDict[judgeName1], Detail{ "","1","0",true }, 1);
		}
		else {
			secondGet(nameDict[judgeName1], nameDict[judgeName1], Detail{ "","1","0",true }, 2);
		}
		/*				判断是否符合条件循环		*/
		if (judgeSym) {
			if(stod(nameDict[judgeName1].first) <= stod(judgeName2))
				break;
		}
		else {
			if (stod(nameDict[judgeName1].first) >= stod(judgeName2))
				break;
		}
	}

	/*
	wordAnaly();
	for (auto iter = WordStack.begin(); iter != WordStack.end(); iter++) {
		cout << *iter << "\t";
	}*/
	printf("\n\n\t\t\t\t\t==============四元式=============\t\t\t\t\t\n");
	for (auto i = 0; i < siyuanshi.size(); i++)
		cout << siyuanshi[i] << endl;
	printf("\n\n\t\t\t\t\t==============中间变量值=============\t\t\t\t\t\n");
	for (auto iter = nameDict.begin(); iter != nameDict.end(); iter++) {
		cout << iter->first << "\t\t" << iter->second.first << endl;
	}

	return 0;
}