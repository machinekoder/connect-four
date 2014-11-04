#include <dlfcn.h>
#include <stdlib.h> 
#include "interface_dl.h"
#include "debug.h"

void interface_openLibrary(char *name)
{
    const char *error;
    interfaceLib = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
    
    if (interfaceLib == NULL)
    {
        ERROR(dlerror());
        exit(EXIT_FAILURE);
    }
    
    *(void **)(&interface_init) = dlsym(interfaceLib, "interface_init");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_cleanup) = dlsym(interfaceLib, "interface_cleanup");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_openClient) = dlsym(interfaceLib, "interface_openClient");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_closeClient) = dlsym(interfaceLib, "interface_closeClient");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_readClient) = dlsym(interfaceLib, "interface_readClient");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_writeClient) = dlsym(interfaceLib, "interface_writeClient");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_openServer) = dlsym(interfaceLib, "interface_openServer");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_closeServer) = dlsym(interfaceLib, "interface_closeServer");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_readServer) = dlsym(interfaceLib, "interface_readServer");
    if ((error = dlerror())) {
        ERROR(error);
    }
    *(void **)(&interface_writeServer) = dlsym(interfaceLib, "interface_writeServer");
    if ((error = dlerror())) {
        ERROR(error);
    }
}

void interface_closeLibrary()
{
    dlclose(interfaceLib);
}
