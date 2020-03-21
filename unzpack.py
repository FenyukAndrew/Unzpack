import sys
import os
#Упаковка/распаковка большого количества маленьких файлов в 1 файл

class DIR:
	def __init__(self,_name):
		self.name=_name
		self.file_dirs=[]
	
	def append(self,element):
		self.file_dirs.append(element)

	def	show(self,tabs):
		for i in self.file_dirs:
			for z in range(0,tabs):
				print("--->",end="")
			if type(i)==DIR:
				print(i.name)
			i.show(tabs+1)

	def	export(self,export_file):
		for i in self.file_dirs:
			if type(i)==DIR:
				print("D:",i.name,file=export_file)
				i.export(export_file)
				print("END:",i.name,file=export_file)
			else:
				i.export(export_file)

	def	export_data(self,export_file,current_path):
		for i in self.file_dirs:
			if type(i)==DIR:
				new_path=os.path.join(current_path,i.name)
				i.export_data(export_file,new_path)
			else:
				i.export_data(export_file,current_path)
				
	def create(self,current_path,import_file):
		for i in self.file_dirs:
			if type(i)==DIR:
				new_path=os.path.join(current_path,i.name)
				print("Create dir:",new_path)
				os.mkdir(new_path)
				i.create(new_path,import_file)
			else:
				i.create(current_path,import_file)

class FILE:
	def __init__(self,_name,_size):
		self.name=_name
		self.size=_size

	def	show(self,tabs):
		print(self.name,self.size)	

	def	export(self,export_file):
		print("F:",self.name,self.size,file=export_file)

	def	export_data(self,export_file,current_path):
		chunksize=65536
		use_path=os.path.join(current_path,self.name)
		with open(use_path,"rb+") as input_file:
			while True:
				chunk = input_file.read(chunksize)
				if chunk:
					export_file.write(chunk)
				else:
					break;
					
	def create(self,current_path,import_file):
		new_file_name=os.path.join(current_path,self.name)
		size=int(self.size)
		print(new_file_name,size)
		chunksize=65536
		with open(new_file_name,"wb") as out_file:
			while size>0:
				if size<chunksize:
					chunksize=size
				chunk=import_file.read(chunksize)
				out_file.write(chunk)
				size-=chunksize
		

def recursiveDir(path,use_dir):
	listOfFiles = os.listdir(path)
	for l in listOfFiles:
		n_path=os.path.join(path,l)
		if os.path.isdir(n_path):
			new_dir=DIR(l)
			use_dir.append(new_dir)
			#print("DIR:",n_path)
			recursiveDir(n_path,new_dir)
		else:
			_size=os.path.getsize(n_path)
			new_file=FILE(l,_size)
			use_dir.append(new_file)
			#print("FILE:",n_path)


def Pack(name_file_pack,path):
	root_dir=DIR("")
	recursiveDir(path,root_dir)

	tabs=0
	root_dir.show(tabs)
	with open(name_file_pack,"w+") as export_file:
		root_dir.export(export_file)
		print("FINISH:",file=export_file)
	
	with open(name_file_pack,"ab+") as export_file:
		root_dir.export_data(export_file,path)

#main
if len(sys.argv)<3:
	print("Usage:")
	print("pack: zpack.py A dir_to_pack name_of_pack")
	print("unpack: zpack.py U name_of_pack dir_to_unpack")
	exit()

if sys.argv[1]=="A":
	path=sys.argv[2]
	name_file_pack=sys.argv[3]
	Pack(name_file_pack,path)
	exit()
#else:
name_file_pack=sys.argv[2]
path_to_unpack=sys.argv[3]

#Unpack

def import_dirs(import_file,current_DIR):
	line=import_file.readline()
	while line:
		line=line.replace("\n","")
		if line.startswith("FINISH:"):
			break		
		lines=line.split(" ")
		name_dir_file=lines[1]
		if line.startswith("D:"):
			new_DIR=DIR(name_dir_file)
			current_DIR.append(new_DIR)
			import_dirs(import_file,new_DIR)
		if line.startswith("F:"):
			new_file=FILE(name_dir_file,lines[2])
			current_DIR.append(new_file)
		if line.startswith("END:"):
			return
		line=import_file.readline()

root_dir=DIR("")
last_postion=0
with open(name_file_pack,"r+") as import_file:
	import_dirs(import_file,root_dir)
	last_position=import_file.tell()
			
tabs=0
root_dir.show(tabs)

os.mkdir(path_to_unpack)
with open(name_file_pack,"rb+") as import_file:
	import_file.seek(last_position)#Переоткрыть файл в бинарном режиме и установить позицию после списка файлов
	root_dir.create(path_to_unpack,import_file)
	




#import sys

#walk_dir=""
#for root, subdirs, files in os.walk(walk_dir):
#    print(root,files)
#    for file in files:
#	    print(file)
		
#    for subdir in subdirs:
#	    print(subdir)
		
#glob.glob() - Allows you to use shell-style wildcards

#import glob
#dirList = glob.glob("")
#for d in dirList:
#    print(d)