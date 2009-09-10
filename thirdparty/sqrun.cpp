// This is emulator of SUBLEQ language. Run with no arguments to see usage.
// Oleg Mazonka, 10 Nov 2006, 19 Jan 2009, 22 Sept 2009

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <string>

using namespace std;

struct Mem
{
	vector<int> v;
	int & operator[](int i);
	int size(){ return v.size(); }

	void dump();
};

void Mem::dump()
{
	for( int i=0; i<v.size(); i++ )
	cout<<i<<":"<<v[i]<<' ';
}
                    
int & Mem::operator[](int i)
{
	if( i<0 ) throw "Access violation: ";
	if( i>=v.size() ) v.resize(i+1,0);
	return v[i];
}

Mem mem;
string sout="OUT", sin="IN";
int iout=-1, iin=-1;
enum { IOCHAR, IOINT } io_type = IOCHAR;

int ip=0;

bool loadfrom(istream &in)
{
	while(1)
	{
		string s;
		in>>s;
		if( !in ) break;
		if( s.size() < 1 ) break;
		if( s[0]=='#' )
		{
		  if( s.size()==sout.size()+1 && s.substr(1)==sout ) mem[ip++] = iout;
		  else
		  if( s.size()==sin.size()+1 && s.substr(1)==sin ) mem[ip++] = iin;
		  else
		  {
			cerr<<"Unresolved register "<<s<<'\n';
			return false;
		  }
		}
		else mem[ip++] = atoi(s.c_str());
	}

	return true;
}

bool load(const char *f)
{
	ifstream in(f);
	if( !in )
	{
		cout<<"cannot open "<<f<<"\n";
		return false;
	}

	return loadfrom(in);
}

void usage()
{
	cout<<"Usage: sqrun [-stdin] [-trace] [-steps] [-outname=OUT] [-outaddr=-1] "
	"[-inname=IN] [-inaddr=-1] [-iotype=c|i] file1 file2 file3 ...\n";
}

int main(int ac, char *av[])
{
	bool trace = false, steps=false;
	bool bcin = false;

	if( ac < 2 )
	{
		usage();
		return 0;
	}

	for( int i=1; i<ac; i++ )
	{
		if( av[i][0] != '-' ) continue;

		string s = av[i];
		if( s.size()>9 && s.substr(1,8)=="outname=" ) sout = s.substr(9);
		else
		if( s.size()>9 && s.substr(1,8)=="outaddr=" ) iout = atoi(s.substr(9).c_str());
		else
		if( s.size()>8 && s.substr(1,7)=="inname=" ) sin = s.substr(8);
		else
		if( s.size()>8 && s.substr(1,7)=="inaddr=" ) iin = atoi(s.substr(8).c_str());
		else
		if( s.size()==6 && s.substr(1,5)=="trace" ) trace=true;
		else
		if( s.size()==6 && s.substr(1,5)=="steps" ) steps=true;
		else
		if( s.size()==6 && s.substr(1,5)=="stdin" ) bcin=true;
		else
		if( s.size()==9 && s.substr(1,6)=="iotype" )
		{
			if( s[8] == 'c' ) io_type = IOCHAR;
			if( s[8] == 'i' ) io_type = IOINT;
		}
		else
		{
			cerr<<"Unknown option ["<<s<<"]\n";
			usage();
			return 2;
		}
	}

	if( bcin ){ if( !loadfrom(cin) ) return 1; }
	else
	for( int i=1; i<ac; i++ ) if( av[i][0] != '-' ) if( !load(av[i]) ) return 1;

	if( trace ) cout<<"using outname="<<sout<<" outaddr="<<iout<<" inname="<<sin<<" inaddr="<<iin<<'\n';

	ip=0;
	int iter=0;
	int iter2=0;
	try{
	while(++iter)
	{
		if( iter==1000000 ) { iter2++; iter=0; }
		if( ip>=mem.size() ) break;

		int a = mem[ip++];
		int b = mem[ip++];
		if( ip>=mem.size() ){ cerr<<"Incomplete instruction\n"; break; }
		int c = mem[ip++];

		if( trace ) cout<<"exec "<<ip-3<<":\t"<<a<<' '<<b<<' '<<c<<' ';

		if( a == iin && b==iout )
		{
			int x=0;

			if( io_type == IOCHAR )
			{
				char c;
				cin.get(c);
				x = (unsigned char)c;
				if(cin) cout<<c<<flush;
			}
			else if( io_type == IOINT )
			{
				cin>>x;
				if(cin)cout<<x<<' '<<flush;
			}

			if( trace ) cout<<" \tinout "<<x<<' ';
		}
		else if( a == iin )
		{
			int x=0;
			if( io_type == IOCHAR )
			{
				char c;
				cin.get(c);
				x = (unsigned char)c;
			}
			else if( io_type == IOINT ) cin>>x;

			if(cin)
				mem[b] += x;
			else
				mem[b] -= 1;

			if( mem[b]<=0 ) ip=c;
			if( trace ) cout<<" \tinput [b]="<<mem[b]<<" ip="<<ip<<' ';

		}else if( b==iout )
		{
			if( io_type == IOCHAR )
				cout<<(unsigned char)(mem[a])<<flush;
			else if( io_type == IOINT )
				cout<<mem[a]<<' '<<flush;
			if( trace ) cout<<" \toutput "<<mem[a]<<' ';
		}
		else
		{
			int ma = mem[a];
			// need ma because [] may resize hence invalidate mem[b]
			mem[b]-=ma;
			if( mem[b]<=0 ) ip=c;
			if( trace ) cout<<" \t[a]="<<mem[a]<<" [b]="<<mem[b];
		}

		if( trace ) cout<<"\n";
		if( ip<0 ) break;

	}
	}catch(const char *s)
	{
		cout<<s<<" ip="<<ip<<'\n';
		if( ip>=0 ) cout<<"mem[ip]="<<mem[ip]<<'\n';
		mem.dump(); cout<<'\n';
	}

	if(steps) 
	cout<<"\nsteps="<<iter2<<std::setfill('0')<<std::setw(6)<<iter<<'\n';
}
