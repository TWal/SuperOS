#include "../src/utility.h"
#include "../src/HDD/HardDrive.h"

void unittest(){
    const size_t emptyOff = 0x80120;

    HDD first(1,true);
    first.init();

    int data[1000];
    for(int i = 0 ; i < 1000 ; ++i){
        data[i] = i;
    }
    first.writeaddr(emptyOff,&data,4*1000);
    int res;
    first.readaddr(emptyOff + 4 *42,&res,4);
    printf("%d\n",res);
    first.readaddr(emptyOff + 4 *999,&res,4);
    printf("%d\n",res);
    res = 45;
    first.writeaddr(emptyOff + 4 *537,&res,4);
    first.readaddr(emptyOff + 4 *753,&res,4);
    printf("%d\n",res);
    first.readaddr(emptyOff + 4 *537,&res,4);
    printf("%d\n",res);
}
