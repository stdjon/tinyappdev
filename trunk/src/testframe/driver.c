#include "testframe/test.h" // For TFR_*
#include <stdlib.h> // For EXIT_*


/** Driver for the test framework. Can be optionally compiled into the lib.
 */
int main(int argc, char **argv)
{
    int result = EXIT_FAILURE;

    if(TFR_args(&argc, &argv))
    {
        TFR_main(argc, argv); /* user-supplied function */

        if(TFR_run())
        {
            result = EXIT_SUCCESS;
        }
    }

    return result;
}
