#include <iostream>
#include <stdio.h>
#include <map>
using namespace std;

map <void* , long> Objects;

int main(int argc, char* argv[])
{
    int counter = 0;
	if (argc != 2 )
	{
		cout << "Dumpster - counts instants of pobject classes";
		cout << "usage: dumpster  dump_file \n";
		return 0;
	}
    FILE * file = fopen(argv[1],"rb");
	if (file == NULL)
	{
		cout << "can't open dump file\n";
		return 1;
	}
	char temp[4097];
	int read;
	unsigned long last[4] = {0,0,0,0};
    while ((read = fread(temp,1,4096,file)))
	{
		for (int i=0; i< read - 3; i++)
		{
			unsigned long l = *((unsigned long *)(temp + i));
			if (l == 0x5A5AF1F1) // pobject validity flag
			{
                counter++;
                ++Objects[(void*)last[0]];
			}
			last[0] = last[1];
			last[1] = last[2];
			last[2] = last[3];
			last[3] = l;
		}
	}
    //cout << counter << endl;
    
    fclose(file);

    map <void* , long> ::iterator i = Objects.begin();
    while (i != Objects.end())
    {
        cout << dec << i->second << " vtbl: " << hex << i->first << endl;
        ++i;
    }
    
    
	return 0;
}
