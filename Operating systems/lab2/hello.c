
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

int main(int argc, char * argv[]){
    struct passwd *pw = getpwuid(getuid());
    printf("Hello %s\n", pw->pw_name);
}
