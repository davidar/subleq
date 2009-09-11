// Usage: sqasm < file.in > file.out
// Oleg Mazonka: 10 Nov 2006; 22 Jan 2009; 11 Sept 2009

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctype.h>

using namespace std;

int error_status = 0;

int str2int(const string &s)
{
	int ret=0;
	sscanf(s.c_str(),"%d",&ret);
	return ret;
}

string int2str(int x, int pr=0)
{
	char buf[40];
	sprintf(buf,"%d",x);
	if( !pr ) return buf;
	
	string s = buf;
	while( s.size() < (unsigned)pr ) s = string("0")+s;
	return s;
}

/* grammar
program	:= list of intructions
intruction := [.] list of items ( ';' | '\n' )
item 	:= [label:]expression
label	:= id
expression := ( term | term+expression | term-expression )
term 	:= ( -term | (expression) | const | id )
const	:= ( number | 'letter' | ? )
*/

struct item
{
	int addr;
	string s;
	int i;
	enum { EMPTY, STR, RES } state;
	item() : state(EMPTY) {}
	string dump(bool=false);
};

string item::dump(bool extra)
{
	string r;
	if( extra ) r = int2str(addr)+":";
	if( state==RES ) return r+int2str(i);
	return r+ string("#")+s;
}

struct instruction
{
	vector<item> items;
	string dump(bool=false);
};

string instruction::dump(bool extra)
{
	string r;
	for( size_t i=0; i<items.size(); i++ ) r += items[i].dump(extra) + ' ';
	return r;
}

map<string,int> lab2adr;
string prog;
typedef string::size_type sint;
sint pip = 0;
int line=1;
int addr = 0;
map<string,int> unres;

void eat()
{ 
	while( pip<prog.size() && (isspace(prog[pip]) &&  prog[pip]!='\n') ) 
		pip++; 

	if( prog[pip] == '#' )
		while( pip<prog.size() && prog[pip]!='\n' )
			pip++;
}
void eatn(){ eat(); while(prog[pip] == '\n' ) { pip++; line++; eat(); } }


void getid(string &s)
{
	eat();
	if( isalpha(prog[pip]) || prog[pip]=='_' )
	{
	  s+=prog[pip];
	  while( pip<prog.size() && 
		( isalnum(prog[++pip]) || prog[pip]=='_' ) ) s+=prog[pip];
	}
}

void getid(item& i)
{
	getid(i.s);

	i.state = item::STR;
	if( lab2adr.find(i.s) != lab2adr.end() )
	{
		i.i = lab2adr[i.s];
		i.state = item::RES;
	}
}

char getChr(int chr=false)
{
	  if( prog[++pip]!='\\' )
	  {
		return prog[pip];
	  }else 		// escape symbol
	  {
		switch(prog[++pip])
		{
		  case 'n': return '\n';
		  case '\\': return '\\';
		  case '"': return '"';
		  case '\'': return '\'';
		  case '0': return '\0';
		}
	  }

    cerr<<"Warning "<<line<<" unknown escape char '"<<prog[pip]<<"'\n";
    if(!chr) pip--;
    return prog[pip];
}

bool getconst(item& i)
{
	eat();
	if( prog[pip]=='\'' )
	{
	  i.i = (unsigned char)getChr(true);
	  if( prog[++pip] != '\'' )
	  {
		cerr<<"Error "<<line<<" closing ' expected was '"<<prog[pip]<<"'\n";
		error_status = __LINE__;
		return false;
	  }
	  i.state = item::RES;
	  pip++;
	  return true;
	}

	if( prog[pip]=='?' )
	{
		i.i = addr;
		i.state = item::RES;
		pip++;
		return true;
	}

	if( !isdigit(prog[pip]) ) return false;
	while( pip<prog.size() && isdigit(prog[pip]) ) i.s+=prog[pip++];
	i.i = str2int(i.s);
	i.state = item::RES;
	return true;
}

void getexpr(item& i);

void getterm(item& i)
{
	eat();

	if( prog[pip] == '-' )
	{
		pip++;
		getterm(i);
		i.i = -i.i;
		return;
	}

	if( prog[pip] == '(' )
	{
		pip++;
		getexpr(i);
		if( prog[pip] != ')' )
		{
			cerr<<"Error "<<line<<" closing ) expected\n";
			error_status = __LINE__;
		}
		else pip++;
		return;
	}

	if( getconst(i) ) return;
	getid(i);
}

void getexpr(item& i)
{
	eat();

	getterm(i);

tryterm:
	eat();
	if( prog[pip] == '-' )
	{
		pip++;
		item j;
		getterm(j);
		if( j.state==item::RES && i.state==item::RES ) i.i -= j.i;
		else
		{
			if( i.state==item::RES ) i.s = int2str(i.i);
			if( j.state==item::RES ) j.s = int2str(j.i);
			i.s = i.s +"-" +j.s;
			i.state=item::STR;
		}
		goto tryterm;
	}

	if( prog[pip] == '+' )
	{
		pip++;
		item j;
		getterm(j);
		if( j.state==item::RES && i.state==item::RES ) i.i += j.i;
		else
		{
			if( i.state==item::RES ) i.s = int2str(i.i);
			if( j.state==item::RES ) j.s = int2str(j.i);
			i.s = i.s +"+" +j.s;
			i.state=item::STR;
		}
		goto tryterm;
	}
}

bool getlabel(string &s)
{
 sint mypip = pip;
 getid(s);
 if( prog[pip] == ':' )
 {
	pip++; 
	eatn();
	return true; 
 }
 pip = mypip;
 return false;
}

bool getStr(item &i)
{
  static int j=0;

  if( j==0 ) j++; // start

  if( pip+j >=prog.size() )
  {
	cerr<<"Error "<<line<<" string not closed\n";
	error_status = __LINE__;
	pip += j;
	return false;
  }

  if( prog[pip+j] == '"' ) // end
  {
	pip+=j+1;
	j=0;
	return false;
  }

  {
	int p0 = pip;
	pip += j-1;
	i.i = (unsigned char)getChr();
	j+=pip-j+1-p0;
	pip=p0;
  }

  i.state = item::RES;

  return true;

}

bool getitem(item &i)
{
begin:
	eat();
	if( prog[pip] == '\n' ) { pip++; line++; return false; }
	if( prog[pip] == ';' ) { pip++; return false; }

	if( pip>=prog.size() ) return false;
	
	string lab;
	while( getlabel(lab) )
	{
		if( lab2adr.find(lab) == lab2adr.end() )
			lab2adr[lab] = addr;
		else
		{
			cerr<<"Error "<<line<<": label "<<lab<<" was defined\n";
			error_status = __LINE__;
		}
		lab="";
	}

	eat();
	i.addr = addr++;

	if( prog[pip] == '"' )
	{
		if( getStr(i) ) return true;
		addr--; // finished with string - try again
		goto begin;
	}

	getexpr(i);

	return true;
}

bool getinstr(instruction &i)
{
	eatn();
	bool data = false;
	if( prog[pip] == '.' ) { data = true; pip++; }
	while( pip<prog.size() )
	{
		item t;
		if( getitem(t) ) i.items.push_back(t);
		else if( i.items.size() == 0 ) continue;
		else break;
	}

	if( i.items.size() == 0 ) return false;

	if( i.items.size() == 1 )
	{
		item &k = i.items.front();
		if( k.state==item::STR && k.s=="" ) return false;
	}

	if( !data && i.items.size() == 1 )
	{
		item k = i.items.front();
		k.addr = addr++;
		i.items.push_back(k);
	}
	
	if( !data && i.items.size() == 2 )
	{
		item k;
		k.addr = addr++;
		k.i = addr;
		k.state = item::RES;
		i.items.push_back(k);
	}

	return true;
}

vector<instruction> program()
{
	vector<instruction> r;
	while( pip<prog.size() )
	{
		instruction i;
		if ( getinstr(i) ) r.push_back(i);
		else break;
	}
	return r;
}

void resolve(item &i)
{
	if( i.state==item::EMPTY )
	{
		cerr<<"Internal Error: empty item\n";
		error_status = __LINE__;
		return;
	}

	if( i.state==item::RES ) return;

	prog = i.s;
	pip=0;
	item k;
	getitem(k);
	if( k.state==item::RES )
	{
		i.state = item::RES;
		i.i = k.i;
		return;
	}

	i.s = k.s;
	unres[i.s] = i.addr;
}

void resolve(instruction &n)
{
  for( size_t i=0; i<n.items.size(); i++ )
  resolve(n.items[i]);
}

void resolve(vector<instruction> &pr)
{
  for( size_t i=0; i<pr.size(); i++ )
  {
	resolve(pr[i]);
  }
}

int main()
{
	while(1)
	{
		string s;
		getline(cin,s);
		prog += s+'\n';
		if( !cin ) break;
	}

	vector<instruction> pr = program();

	if( error_status ) return error_status;
	
	for( size_t i=0; i<pr.size(); i++ )
	{
		resolve(pr[i]);
		cout<<pr[i].dump()<<"\n";
	}

	if( unres.size() )
	{
	   cerr<<"Warning: unresolved symbols: ";
	   for( map<string,int>::iterator i=unres.begin(); i!=unres.end(); i++ )
	   cerr<<" ["<<i->first<<":"<<i->second<<"]";
	   cerr<<'\n';
	}
}
