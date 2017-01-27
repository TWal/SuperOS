

void kmain(){
    unsigned char * fb = (unsigned char*) 0x000B8000;
    char* s = "Hello World!";
    for(int i = 0 ; i < 80 * 25 ; ++i){
        fb[2*i] = 0;
        fb[2*i+1] = 0;
    }
    for(int i = 0; i < 12; ++i) {
        fb[2*i] = s[i];
        fb[2*i+1] = 15;
    }
}
