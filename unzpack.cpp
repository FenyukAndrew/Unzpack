//g++ Unpack.cpp -lboost_system -lboost_filesystem && ./a.out N

#include <fstream>
#include <iostream>

#include <limits>

#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

#include <vector>

class my_item
{
public:
	my_item(const string& name) : name(name) {}
	virtual ~my_item() {};

	string name;
};

class my_folder : public my_item
{
public:
	my_folder(const string& name) : my_item(name) {}
	void add(my_item* item)
	{
		items.push_back(item);
	}
	
	//override
	~my_folder()
	{
		for(auto item : items)
			delete item;
	}
	
	vector<my_item*> items;
};

class my_file : public my_item
{
public:
	my_file(const string& name,int size) : my_item(name), size(size) {}
	
	int size;
};


void recursive_dir1(const path& dir_path,my_folder&,const int tabs);
void export_dirs(my_folder& folder1,const int tabs);
void export_data(my_folder& folder1,const string& path1);
void copy_file(string name_file);
void import_dirs(my_folder& folder1,int tabs);
void import_data(my_folder& folder1,const string& path1);
void create_file(string name_file,int size);

//Есть версия - recursive_dir

ofstream out_file;
ifstream un_file;

int main(int argv,char* arvgs[])
{
	if (argv<3)
	{
		cerr << "Упаковка множества маленьких файлов в один" << endl;
		cerr << "Не хватает аргументов" << endl;
		cerr << "Использование:" << endl;
		cerr << "Распаковка:unzpack U файл_архива директория_для_распаковки " << endl;
		cerr << "Упаковка:unzpack Z файл_архива директория_для_упаковки " << endl;
		return 0;
	}
	
	if (string(arvgs[1])=="U")
	{
		un_file.open(argvs[2],ios::in | ios::binary);
		my_folder folder1("");
		import_dirs(folder1,0);
		import_data(folder1,arvgs[3]);
	}
	else
	{
		my_folder folder1("");
		path path1(arvgs[3]);
		recursive_dir1(path1,folder1,0);
		out_file.open(argvs[2],ios::out | ios::binary);
		export_dirs(folder1,0);
		out_file << "FINISH:" << endl;
		export_data(folder1,arvgs[3]);
	}

	return 0;
}

void import_dirs(my_folder& folder1,int tabs)
{
	char s1[1024];
	while(!un_file.eof())
	{
		un_file.getline(s1,1024,'\n');
		string s(s1);
		int q=s.find(':');
		string str_type=s.substr(0,q);
		if (str_type=="FINISH") break;
		int q2=s.find(':',q+1);
		string str_name=s.substr(q+1,q2-q-1);
		for(int i=0;i<tabs;++i) cout << "\t";
		//cout << s << "=";
		if (str_type=="D") 
		{
			my_folder* folder2=new my_folder(str_name);
			folder1.add(folder2);
			cout << "D:" << str_name << endl;
			import_dirs(*folder2,tabs+1);
		}
		else
		if (str_type=="F")
		{
			int size=atoi(s.substr(q2+1).c_str());
			folder1.add(new my_file(str_name,size));
			cout << "F:" << str_name << " " << size << endl;
		}
		else
		if (str_type=="END") 
		{
			cout << "<-" << endl;
			return;
		}
	}
}

void import_data(my_folder& folder1,const string& path1)
{
	for(auto item : folder1.items)
	{
		my_folder* folder2=dynamic_cast<my_folder*>(item);
		if(folder2)
		{
			string s=path1+"/"+folder2->name;
			path path_folder(s);
			create_directory(path_folder);
			import_data(*folder2,s);
		}
		else
		{
			my_file* file1=dynamic_cast<my_file*>(item);
			string s=path1+"/"+file1->name;
			create_file(s,file1->size);
		}
	}
}

void create_file(string name_file,int size)
{
	ofstream create_file(name_file,ios::out | ios::binary);

	constexpr int SIZE=1024;
	char t[SIZE];
	
	cout << "Create file:" << name_file << " " << size << endl;

	while (!un_file.eof() && size)
	{
		int chunk=(size<SIZE) ? size : SIZE;
		int size_get=un_file.readsome(t,chunk);//Может считать меньше байт чем указано и соответственно есть в файле
		size-=size_get;
		
		create_file.write(t,size_get);
	}
}

void recursive_dir1(const path& dir_path,my_folder& folder1,const int tabs)
{
//for(auto& entry : boost::make_iterator_range(directory_iterator(p), {})) std::cout << entry << "\n";
	
directory_iterator it_dir(dir_path);
directory_iterator end;
for(;it_dir!=end;it_dir++)
{
	for(int i=0;i<tabs;++i) cout << "\t";
	string filename=it_dir->path().filename().native();
	if(is_directory(it_dir->path()))
	{
		cout << "D:" << filename << endl;
		
		my_folder* folder2=new my_folder(filename);
		folder1.add(folder2);
		cout << "->";
		recursive_dir1(it_dir->path(),*folder2,tabs+1);
	}
	else
	{
		cout << "F:" << filename << " " << file_size(it_dir->path()) << endl;

		my_file* file=new my_file(filename,file_size(it_dir->path()));
		folder1.add(file);
	}
}

return;
}

void export_dirs(my_folder& folder1,const int tabs)
{
	for(auto item : folder1.items)
	{
		//for(int i=0;i<tabs;++i) cout << "\t";
		my_folder* folder2=dynamic_cast<my_folder*>(item);
		if(folder2)
		{
			out_file << "D:" << folder2->name << endl;
			export_dirs(*folder2,tabs+1);
			out_file << "END:" << folder2->name << endl;
		}
		else
		{
			my_file* file1=dynamic_cast<my_file*>(item);
			out_file << "F:" << file1->name << ":" << file1->size << endl;
		}
		
	}
}

void export_data(my_folder& folder1,const string& path1)
{
	for(auto item : folder1.items)
	{
		my_folder* folder2=dynamic_cast<my_folder*>(item);
		if(folder2)
		{
			string s=path1+"/"+folder2->name;
			export_data(*folder2,s);
		}
		else
		{
			my_file* file1=dynamic_cast<my_file*>(item);
			string s=path1+"/"+file1->name;
			copy_file(s);
		}
		
	}
}

void copy_file(string name_file)
{

	ifstream in_file(name_file,ios::in | ios::binary);

	constexpr int SIZE=1024;
	char t[SIZE];

	while (!in_file.eof())//НЕ РАБОТАЕТ eof
	{
		int size=in_file.readsome(t,SIZE);//Может считать меньше байт чем указано и соответственно есть в файле
		if(!size) break;
		
		out_file.write(t,size);
	}
}

//НЕИСПОЛЬЗУЕТСЯ
void copy_file1(string name_file,string name_out)
{

	ifstream in_file(name_file,ios::in | ios::binary);
	ofstream out_file(name_out,ios::out | ios::binary);

	constexpr int SIZE=1024;
	char t[SIZE];

//int size=numeric_limits<int>::max();
/*while (!in_file.eof())
{
	int size=in_file.readsome(t,SIZE);
	if(!size) break;
	
	out_file.write(t,size);

	cout << t << endl;
}*/
	out_file << in_file.rdbuf();
}