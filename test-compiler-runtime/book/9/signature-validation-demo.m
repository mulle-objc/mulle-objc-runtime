// Test for signature validation
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    const char *signature = "@@:@";
    const char *expected = "@@:@";
    
    if (_mulle_objc_methodsignature_compare((char *)signature, (char *)expected) == 0)
    {
        mulle_printf("Signature '%s' matches expected format\n", signature);
    }
    else
    {
        mulle_printf("Signature '%s' does not match expected format\n", signature);
    }
    
    return 0;
}