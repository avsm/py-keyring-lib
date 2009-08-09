#include <Security/Security.h>

#include "Python.h"
#include "keyring_util.h"

/*
 * Comments from Subversion's macos_keychain.c
 * XXX (2005-12-07): If no GUI is available (e.g. over a SSH session),
 * you won't be prompted for credentials with which to unlock your
 * keychain.  Apple recognizes lack of TTY prompting as a known
 * problem.
 *
 *
 * XXX (2005-12-07): SecKeychainSetUserInteractionAllowed(FALSE) does
 * not appear to actually prevent all user interaction.  Specifically,
 * if the executable changes (for example, if it is rebuilt), the
 * system prompts the user to okay the use of the new executable.
 *
 * Worse than that, the interactivity setting is global per app (not
 * process/thread), meaning that there is a race condition in the
 * implementation below between calls to
 * SecKeychainSetUserInteractionAllowed() when multiple instances of
 * the same Subversion auth provider-based app run concurrently.
 */


static PyObject *
keychain_password_set(PyObject *self, PyObject *args)
{
    const char *realmstring;
    const char *username;
    const char *password;
    OSStatus status;
    SecKeychainRef keychain;
    SecKeychainItemRef item;

    if (!PyArg_ParseTuple(args, "sss", &realmstring, &username, &password)){
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError,
            "password_set() must be called as (servicename,username,password)");                                               
        return NULL;                                                        
    }
    
    if (SecKeychainOpen("login.keychain",&keychain) != 0 ){
        PyErr_Clear();
        PyErr_SetString(PyExc_OSError,
                    "can't access the login.keychain, Authorization failed");                                             
        return NULL;                                                        
    }
    
    status = SecKeychainFindGenericPassword(keychain, strlen(realmstring),
                                            realmstring, username == NULL
                                              ? 0
                                              : strlen(username),
                                            username, 0, NULL, &item);

    if (status){
        if (status == errSecItemNotFound)
          status = SecKeychainAddGenericPassword(keychain, strlen(realmstring),
                                                 realmstring, username == NULL
                                                   ? 0
                                                   : strlen(username),
                                                 username, strlen(password),
                                                 password, NULL);
    }
    else{
        status = SecKeychainItemModifyAttributesAndData(item, NULL,
                                                        strlen(password),
                                                        password);
        CFRelease(item);
    }
    
    return Py_BuildValue("i",(status==0));
}


static PyObject *
keychain_password_get(PyObject *self, PyObject *args)
{
    const char *realmstring;
    const char *username;
    char *password;
    OSStatus status;
    UInt32 length;
    SecKeychainRef keychain;
    void *data;
    
    if (!PyArg_ParseTuple(args, "ss", &realmstring, &username)){
        PyErr_Clear();
        PyErr_SetString(PyExc_TypeError,
            "password_get() must be called as (servicename,username)");                                                
        return NULL;                                                        
    }
    
    if (SecKeychainOpen("login.keychain", &keychain) != 0 ){
        PyErr_Clear();
        PyErr_SetString(PyExc_OSError,
                "can't access the login.keychain, Authorization failed");                                             
        return NULL;                                                        
    }
    
    status = SecKeychainFindGenericPassword(keychain, strlen(realmstring),
                                            realmstring, username == NULL
                                              ? 0
                                              : strlen(username),
                                            username, &length, &data, NULL);
    
    if (status != 0)
      return Py_BuildValue("s", "");
    
    password = string_dump(data, length);
    SecKeychainItemFreeContent(NULL, data);
    return Py_BuildValue("s",password);
}

static struct PyMethodDef keychain_methods[] ={
    {"password_set", keychain_password_set, METH_VARARGS},
    {"password_get", keychain_password_get, METH_VARARGS},
    {NULL,NULL} /* Sentinel */
};

PyMODINIT_FUNC
initosx_keychain(void)
{
    Py_InitModule("osx_keychain", keychain_methods);
}

