#include <stdio.h>
#include <dlfcn.h>
#include "./include/list_interface.h"

int main(int argc, char *argv[]){
    void * libm_handle = NULL;
    struct link_list  (*cosf_method)(struct link_list *, const size_t , const size_t);
    char *errorInfo;
    struct link_list re = {0};
    struct link_list *agrc1;
    //re = (struct link_list*)malloc(sizeof(struct link_list));
    libm_handle = dlopen("list/lib/list.so", RTLD_LAZY );
    if (!libm_handle){
        printf("Open Error:%s.\n",dlerror());
        return 0;
    }

    cosf_method = dlsym(libm_handle,"create_list");
    errorInfo = dlerror();
    if (errorInfo != NULL){
        printf("Dlsym Error:%s.\n",errorInfo);
        return 0;
    }

    re = (*cosf_method)(agrc1,10,10);
    printf("result = %s\n",re);
    
    dlclose(libm_handle);

    return 0;
}
