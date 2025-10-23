// Test for signature validation
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    const char *signature = "@@:@";
    const char *expected = "@@:@";
    
    if (_mulle_objc_methodsignature_compare((char *)signature, (char *)expected) == 0)
    {
        printf("Signature '%s' matches expected format\n", signature);
    }
    else
    {
        printf("Signature '%s' does not match expected format\n", signature);
    }
    
    return 0;
}